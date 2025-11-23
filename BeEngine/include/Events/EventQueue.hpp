// EventQueue.hpp
#pragma once

#include "Core.hpp"
#include "Events/Event.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace BeEngine {

// EVENT LISTENER
using EventListener = std::function<bool(Event &)>;

struct EventListenerHandle {
  uint64_t id;
  EventType type;
  EventListener callback;
  int priority;

  bool operator<(const EventListenerHandle &other) const {
    return priority < other.priority;
  }
};

// EVENT STATISTICS
struct EventStats {
  size_t totalEventsQueued{};
  size_t totalEventsProcessed{};
  size_t totalEventsDropped{};
  size_t currentQueueSize{};
  size_t maxQueueSize{};
  double averageProcessTimeMs{};

  std::unordered_map<EventType, size_t> eventTypeCount;
};

// Configuration
struct Config {
  size_t maxQueue{1000};
  bool dropOnOverflow{true};
  bool enableProfiling{false};
  bool enableLogging{false};
};

class BE_API EventQueue {
public:
  EventQueue(const Config &config = Config());
  ~EventQueue();

  // EVENT QUEUEING

  // Queue Event (Takes Ownersship via unique pointer)
  template <typename T, typename... Args> void QueueEvent(Args &&...args) {
    auto event = std::make_unique<T>(std::forward<Args>(args)...);
    QueueEventInternal(std::move(event));
  }

  // Queue existing event
  void QueueEvent(std::unique_ptr<Event> event);

  // Queue Immediate Event (Bypass Queue, Process Now)
  void QueueImmediate(std::unique_ptr<Event> event);

  // EVENT PROCESSING

  // Process all queued events
  void ProcessEvents();

  // Process upto N Events
  void ProcessEvents(size_t maxEvents);

  // Process Events With Time Budget (ms)
  void ProcessEventsWithBudget(double maxTimeMs);

  // Clear All Pending Events
  void clear();

  // EVENT LISTENERS

  // Subscribe to specific event type
  uint64_t Subscribe(EventType type, EventListener listener, int priority = 0);

  // Subscribe to all events
  uint64_t Subscribe(EventListener listener, int priority = 0);

  // Unsubscribe
  void Unsubscribe(uint64_t listenerID);

  // Unsubscribe All
  void UnsubscribeAll(EventType type);

  // EVENT FILTERING

  // Block Specific Events
  void BlockEventType(EventType type);
  void UnBlockEventType(EventType type);
  NODISCARD bool IsEventTypeBlocked(EventType type) const;

  // Block Specific Category
  void BlockCategory(EventCategory category);
  void UnBlockCategory(EventCategory category);

  // STATISTICS & DEBUGGING

  NODISCARD EventStats GetStats() const;
  void ResetStats();

  size_t GetQueueSize();
  NODISCARD bool isEmpty() const;

  // Enable/Disable Event Logging
  void SetLogging(bool enabled) { m_Config.enableLogging = enabled; }

  // Enable/Disable Profiling
  void SetProfiling(bool enabled) { m_Config.enableProfiling = enabled; }

private:
  void QueueEventInternal(std::unique_ptr<Event> event);
  void ProcessEvent(Event &event);
  void NotifyListener(Event &event);

  // Event Queue with Priority
  struct QueuedEvent {
    std::unique_ptr<Event> event;
    EventPriority priority;

    bool operator<(const QueuedEvent &other) const {
      return priority < other.priority;
    }
  };

  std::priority_queue<QueuedEvent> m_EventQueue;
  mutable std::mutex m_QueueMutex;

  // Event Listener
  std::unordered_map<EventType, std::vector<EventListenerHandle>> m_Listeners;
  std::vector<EventListenerHandle> m_GlobalListeners;
  mutable std::mutex m_ListenersMutex;

  uint64_t m_NextListenerID{1};

  // Blocked Events
  std::unordered_set<EventType> m_BlockedEvents;
  uint32_t m_BlockedCategories{};
  mutable std::mutex m_BlockedMutex;

  // Configuration
  Config m_Config;

  // Statistics
  mutable EventStats m_Stats;
  mutable std::mutex m_StatsMutex;
};

// SCOPED EVENT LISTENER
class BE_API ScopedEventListener {
public:
  ScopedEventListener(EventQueue &queue, EventType type, EventListener listener)
      : m_Queue(queue),
        m_ListenerID(queue.Subscribe(type, std::move(listener))) {}

  ~ScopedEventListener() { m_Queue.Unsubscribe(m_ListenerID); }

  // Non-copyable, movable
  ScopedEventListener(const ScopedEventListener &) = delete;
  ScopedEventListener &operator=(const ScopedEventListener &) = delete;

  ScopedEventListener(ScopedEventListener &&other) noexcept
      : m_Queue(other.m_Queue), m_ListenerID(other.m_ListenerID) {
    other.m_ListenerID = 0;
  }

private:
  EventQueue &m_Queue;
  uint64_t m_ListenerID;
};

} // namespace BeEngine
