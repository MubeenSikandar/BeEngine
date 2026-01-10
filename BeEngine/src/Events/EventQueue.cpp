#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

// ============================================================================
// Constructor & Destructor
// ============================================================================

EventQueue::EventQueue(const Config &config) : m_Config(config) {

  if (m_Config.enableLogging) {
    BE_CORE_INFO("EventQueue initialized (max queue: {})", m_Config.maxQueue);
  }
}

EventQueue::~EventQueue() {
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  if (m_Config.enableLogging && !m_EventQueue.empty()) {
    BE_CORE_WARN("EventQueue destroyed with {} pending events",
                 m_EventQueue.size());
  }

  while (!m_EventQueue.empty()) {
    m_EventQueue.pop();
  }
}

// ============================================================================
// Event Queueing
// ============================================================================

void EventQueue::QueueEvent(Scope<Event> event) {
  if (!event) {
    BE_CORE_ERROR("Attempt to queue null event");
    return;
  }

  QueueEventInternal(std::move(event));
}

void EventQueue::QueueEventInternal(Scope<Event> event) {
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  // Check if queue is full
  if (m_EventQueue.size() >= m_Config.maxQueue) {
    if (m_Config.dropOnOverflow) {
      // Update drop stats
      {
        std::lock_guard<std::mutex> statsLock(m_StatsMutex);
        m_Stats.totalEventsDropped++;
      }

      if (m_Config.enableLogging) {
        BE_CORE_WARN("Event queue full, dropping event: {}", event->ToString());
      }
      return;
    } else {
      // Could implement blocking here in the future
      BE_CORE_ERROR("Event queue full and blocking not implemented");
      return;
    }
  }

  // Check if event type is blocked
  {
    std::lock_guard<std::mutex> blockedLock(m_BlockedMutex);

    if (m_BlockedEvents.find(event->GetEventType()) != m_BlockedEvents.end()) {
      if (m_Config.enableLogging) {
        BE_CORE_TRACE("Event type blocked: {}", event->GetName());
      }
      return;
    }

    // Check if any category is blocked (bitwise operation)
    if ((m_BlockedCategories & event->GetCategoryFlags()) != 0) {
      if (m_Config.enableLogging) {
        BE_CORE_TRACE("Event category blocked: {}", event->GetName());
      }
      return;
    }
  }

  // Queue the event with its priority
  EventPriority priority = event->GetPriority();
  m_EventQueue.push({std::move(event), priority});

  // Cache size before releasing queue mutex
  size_t newQueueSize = m_EventQueue.size();

  // Update statistics (after queue operations are done)
  {
    std::lock_guard<std::mutex> statsLock(m_StatsMutex);
    m_Stats.totalEventsQueued++;
    m_Stats.currentQueueSize = newQueueSize; // Use cached value
    m_Stats.maxQueueSize =
        std::max(m_Stats.maxQueueSize, m_Stats.currentQueueSize);
  }
}

void EventQueue::QueueImmediate(Scope<Event> event) {
  if (!event) {
    BE_CORE_ERROR("Attempted to process null event immediately");
    return;
  }

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Processing immediate event: {}", event->ToString());
  }
  ProcessEvent(*event);
}

// ============================================================================
// Event Processing
// ============================================================================

void EventQueue::ProcessEvents() {
  while (!isEmpty()) {
    Scope<Event> event;

    {
      std::lock_guard<std::mutex> lock(m_QueueMutex);
      if (m_EventQueue.empty()) {
        break;
      }

      auto &top = const_cast<QueuedEvent &>(m_EventQueue.top());
      event = std::move(top.event);
      m_EventQueue.pop();

      std::lock_guard<std::mutex> statsLock(m_StatsMutex);
      m_Stats.currentQueueSize = m_EventQueue.size();
    }
    ProcessEvent(*event);
  }
}

void EventQueue::ProcessEvents(size_t maxEvents) {
  size_t processed{};
  while (processed < maxEvents && !isEmpty()) {
    Scope<Event> event;

    {
      std::lock_guard<std::mutex> lock(m_QueueMutex);
      if (m_EventQueue.empty()) {
        break;
      }

      auto &top = const_cast<QueuedEvent &>(m_EventQueue.top());
      event = std::move(top.event);
      m_EventQueue.pop();

      std::lock_guard<std::mutex> statsLock(m_StatsMutex);
      m_Stats.currentQueueSize = m_EventQueue.size();
    }

    ProcessEvent(*event);
    processed++;
  }
}

void EventQueue::ProcessEventsWithBudget(double maxTimeMs) {
  auto startTime = std::chrono::high_resolution_clock::now();

  while (!isEmpty()) {
    // Check time budget
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration<double, std::milli>(currentTime - startTime);

    if (elapsed.count() >= maxTimeMs) {
      if (m_Config.enableLogging) {
        BE_CORE_TRACE("Event processing time budget exhausted ({:.2f}ms)",
                      elapsed.count());
      }
      break;
    }

    Scope<Event> event;
    {
      std::lock_guard<std::mutex> lock(m_QueueMutex);
      if (m_EventQueue.empty()) {
        break;
      }

      auto &top = const_cast<QueuedEvent &>(m_EventQueue.top());
      event = std::move(top.event);
      m_EventQueue.pop();

      std::lock_guard<std::mutex> statsLock(m_StatsMutex);
      m_Stats.currentQueueSize = m_EventQueue.size();
    }
    ProcessEvent(*event);
  }
}

void EventQueue::ProcessEvent(Event &event) {
  auto startTime = std::chrono::high_resolution_clock::now();

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Process event: {}", event.ToString());
  }

  // Notify All Listeners
  NotifyListener(event);

  // Update Statistics
  {
    std::lock_guard<std::mutex> lock(m_StatsMutex);
    m_Stats.totalEventsProcessed++;
    m_Stats.eventTypeCount[event.GetEventType()]++;

    if (m_Config.enableProfiling) {
      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration<double, std::milli>(endTime - startTime);

      // Update Running Average
      double totalTime =
          m_Stats.averageProcessTimeMs *
          static_cast<double>((m_Stats.totalEventsProcessed - 1));
      m_Stats.averageProcessTimeMs =
          (totalTime + duration.count()) /
          static_cast<double>(m_Stats.totalEventsProcessed);
    }
  }
}

void EventQueue::NotifyListener(Event &event) {
  std::lock_guard<std::mutex> lock(m_ListenersMutex);

  // Call global listeners first
  for (auto &handle : m_GlobalListeners) {
    if (event.IsHandled) {
      break;
    }
    try {
      if (handle.callback(event)) {
        event.IsHandled = true;
      }
    } catch (const std::exception &e) {
      BE_CORE_ERROR("Exception in global event listener: {}", e.what());
    }
  }

  // Call type-specific listeners
  auto it = m_Listeners.find(event.GetEventType());
  if (it != m_Listeners.end()) {
    // Sort by Priority
    auto &listeners = it->second;
    std::ranges::sort(
        listeners.begin(), listeners.end(),
        [](const EventListenerHandle &a, const EventListenerHandle &b) {
          return a.priority > b.priority;
        });

    for (auto &handle : listeners) {
      if (event.IsHandled) {
        break;
      }

      try {
        if (handle.callback(event)) {
          break;
        }
      } catch (const std::exception &e) {
        BE_CORE_ERROR("Exception in event listener: {}", e.what());
      }
    }
  }
}

void EventQueue::clear() {
  std::lock_guard<std::mutex> lock(m_QueueMutex);

  size_t cleared = m_EventQueue.size();

  while (!m_EventQueue.empty()) {
    m_EventQueue.pop();
  }

  {
    std::lock_guard<std::mutex> lock(m_StatsMutex);
    m_Stats.currentQueueSize = 0;
  }

  if (m_Config.enableLogging && cleared > 0) {
    BE_CORE_INFO("Cleared {} pending events from queue", cleared);
  }
}

// ============================================================================
// Listener Management
// ============================================================================

uint64_t EventQueue::Subscribe(EventType type, EventListener listener,
                               int priority) {
  std::lock_guard<std::mutex> lock(m_ListenersMutex);

  uint64_t id = m_NextListenerID++;

  EventListenerHandle handle;
  handle.id = id;
  handle.type = type;
  handle.callback = std::move(listener);
  handle.priority = priority;

  m_Listeners[type].push_back(std::move(handle));

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Subscribed listener {} to event type {}", id,
                  static_cast<int>(type));
  }

  return id;
}

uint64_t EventQueue::Subscribe(EventListener listener, int priority) {
  std::lock_guard<std::mutex> lock(m_ListenersMutex);

  uint64_t id = m_NextListenerID++;

  EventListenerHandle handle;
  handle.id = id;
  handle.type = EventType::None; // Global Listener
  handle.callback = std::move(listener);
  handle.priority = priority;

  m_GlobalListeners.push_back(std::move(handle));

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Subscribed global listener {}", id);
  }

  return id;
}

void EventQueue::Unsubscribe(uint64_t listenerID) {
  std::lock_guard<std::mutex> lock(m_ListenersMutex);

  // Check Global Listeners
  auto globalIt =
      std::ranges::find_if(m_GlobalListeners.begin(), m_GlobalListeners.end(),
                           [listenerID](const EventListenerHandle &handle) {
                             return handle.id == listenerID;
                           });
  if (globalIt != m_GlobalListeners.end()) {
    m_GlobalListeners.erase(globalIt);
    if (m_Config.enableLogging) {
      BE_CORE_TRACE("Unsubscribed global listener {}", listenerID);
    }
    return;
  }

  // Check type-specific listeners
  for (auto &[type, listeners] : m_Listeners) {
    auto it =
        std::ranges::find_if(listeners.begin(), listeners.end(),
                             [listenerID](const EventListenerHandle &handle) {
                               return handle.id == listenerID;
                             });

    if (it != listeners.end()) {
      listeners.erase(it);
      if (m_Config.enableLogging) {
        BE_CORE_TRACE("Unsubscribed listener {}", listenerID);
      }
      return;
    }
  }
  BE_CORE_WARN("Attempted to unsubscribe non-existent listener {}", listenerID);
}

void EventQueue::UnsubscribeAll(EventType type) {
  std::lock_guard<std::mutex> lock(m_ListenersMutex);

  auto it = m_Listeners.find(type);
  if (it != m_Listeners.end()) {
    size_t count = it->second.size();
    m_Listeners.erase(it);

    if (m_Config.enableLogging) {
      BE_CORE_INFO("Unsubscribed {} listeners from event type {}", count,
                   static_cast<int>(type));
    }
  }
}

// ============================================================================
// Event Filtering
// ============================================================================

void EventQueue::BlockEventType(EventType type) {
  std::lock_guard<std::mutex> lock(m_BlockedMutex);
  m_BlockedEvents.insert(type);

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Blocked event type {}", static_cast<int>(type));
  }
}

void EventQueue::UnBlockEventType(EventType type) {
  std::lock_guard<std::mutex> lock(m_BlockedMutex);
  m_BlockedEvents.erase(type);

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Unblocked event type {}", static_cast<int>(type));
  }
}

bool EventQueue::IsEventTypeBlocked(EventType type) const {
  std::lock_guard<std::mutex> lock(m_BlockedMutex);
  return m_BlockedEvents.find(type) != m_BlockedEvents.end();
}

void EventQueue::BlockCategory(EventCategory category) {
  std::lock_guard<std::mutex> lock(m_BlockedMutex);
  m_BlockedCategories |= static_cast<uint32_t>(category);

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Blocked event category {}", static_cast<uint32_t>(category));
  }
}

void EventQueue::UnBlockCategory(EventCategory category) {
  std::lock_guard<std::mutex> lock(m_BlockedMutex);
  m_BlockedCategories &= ~static_cast<uint32_t>(category);

  if (m_Config.enableLogging) {
    BE_CORE_TRACE("Unblocked event category {}",
                  static_cast<uint32_t>(category));
  }
}

// ============================================================================
// Statistics & Debugging
// ============================================================================

EventStats EventQueue::GetStats() const {
  std::lock_guard<std::mutex> lock(m_StatsMutex);
  return m_Stats;
}

void EventQueue::ResetStats() {
  std::lock_guard<std::mutex> lock(m_StatsMutex);
  m_Stats = EventStats{};
  m_Stats.currentQueueSize = m_EventQueue.size();

  if (m_Config.enableLogging) {
    BE_CORE_INFO("Event statistics reset");
  }
}

size_t EventQueue::GetQueueSize() {
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  return m_EventQueue.size();
}

bool EventQueue::isEmpty() const {
  std::lock_guard<std::mutex> lock(m_QueueMutex);
  return m_EventQueue.empty();
}

} // namespace BeEngine
