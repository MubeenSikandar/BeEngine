// HotReloadManager.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

HotReloadManager::HotReloadManager() {
  // Default extensions
  m_WatchedExtensions = {".lua", ".py", ".cs", ".rs"};
}

HotReloadManager::~HotReloadManager() { Stop(); }

void HotReloadManager::AddWatchDirectory(const std::filesystem::path &directory,
                                         bool recursive) {
  if (!std::filesystem::exists(directory)) {
    BE_CORE_WARN("[HotReload] Directory does not exist: {}",
                 directory.string());
    return;
  }

  WatchedDirectory wd;
  wd.path = std::filesystem::canonical(directory);
  wd.recursive = recursive;
  m_WatchedDirectories.push_back(wd);

  // Initial scan
  ScanDirectory(wd.path, wd.recursive);

  BE_CORE_INFO("[HotReload] Watching directory: {} (recursive: {})",
               directory.string(), recursive);
}

void HotReloadManager::RemoveWatchDirectory(
    const std::filesystem::path &directory) {
  auto canonicalPath = std::filesystem::canonical(directory);
  m_WatchedDirectories.erase(std::remove_if(m_WatchedDirectories.begin(),
                                            m_WatchedDirectories.end(),
                                            [&](const WatchedDirectory &wd) {
                                              return wd.path == canonicalPath;
                                            }),
                             m_WatchedDirectories.end());
}

void HotReloadManager::SetWatchedExtensions(
    const std::vector<std::string> &extensions) {
  m_WatchedExtensions = extensions;
}

void HotReloadManager::Start() {
  if (m_Running) {
    return;
  }

  m_StopRequested = false;
  m_Running = true;

  m_WatchThread = std::thread(&HotReloadManager::WatchThread, this);

  BE_CORE_INFO("[HotReload] Started file watcher");
}

void HotReloadManager::Stop() {
  if (!m_Running) {
    return;
  }

  m_StopRequested = true;

  if (m_WatchThread.joinable()) {
    m_WatchThread.join();
  }

  m_Running = false;

  BE_CORE_INFO("[HotReload] Stopped file watcher");
}

void HotReloadManager::PollChanges() {
  for (const auto &wd : m_WatchedDirectories) {
    ScanDirectory(wd.path, wd.recursive);
  }
}

std::vector<HotReloadManager::FileChange>
HotReloadManager::GetPendingChanges() {
  std::lock_guard<std::mutex> lock(m_ChangesMutex);
  auto changes = std::move(m_PendingChanges);
  m_PendingChanges.clear();
  return changes;
}

uint64_t HotReloadManager::GetLastModificationTime(
    const std::filesystem::path &path) const {
  std::lock_guard<std::mutex> lock(m_TimestampMutex);
  auto it = m_FileTimestamps.find(path.string());
  return it != m_FileTimestamps.end() ? it->second : 0;
}

bool HotReloadManager::HasFileChanged(const std::filesystem::path &path) const {
  if (!std::filesystem::exists(path)) {
    return false;
  }

  auto currentTime =
      std::filesystem::last_write_time(path).time_since_epoch().count();
  auto storedTime = GetLastModificationTime(path);

  return currentTime != storedTime;
}

void HotReloadManager::WatchThread() {
  while (!m_StopRequested) {
    PollChanges();

    // Process pending changes through callback
    {
      std::lock_guard<std::mutex> lock(m_ChangesMutex);
      if (m_Callback && !m_PendingChanges.empty()) {
        for (const auto &change : m_PendingChanges) {
          m_Callback(change);
        }
        m_PendingChanges.clear();
      }
    }

    // Sleep for poll interval
    std::this_thread::sleep_for(
        std::chrono::milliseconds(static_cast<int>(m_PollInterval * 1000)));
  }
}

void HotReloadManager::ScanDirectory(const std::filesystem::path &directory,
                                     bool recursive) {
  try {
    std::vector<std::pair<std::filesystem::path, FileChange::Type>> changes;

    auto scanFile = [this, &changes](const std::filesystem::path &path) {
      if (!ShouldWatch(path)) {
        return;
      }

      auto pathStr = path.string();
      auto currentTime =
          std::filesystem::last_write_time(path).time_since_epoch().count();

      std::lock_guard<std::mutex> lock(m_TimestampMutex);

      auto it = m_FileTimestamps.find(pathStr);
      if (it == m_FileTimestamps.end()) {
        m_FileTimestamps[pathStr] = currentTime;
        changes.emplace_back(path, FileChange::Type::Created);
      } else if (it->second != currentTime) {
        it->second = currentTime;
        changes.emplace_back(path, FileChange::Type::Modified);
      }
    };

    if (recursive) {
      for (const auto &entry :
           std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
          scanFile(entry.path());
        }
      }
    } else {
      for (const auto &entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
          scanFile(entry.path());
        }
      }
    }

    // Check for deleted files
    {
      std::lock_guard<std::mutex> lock(m_TimestampMutex);
      for (auto it = m_FileTimestamps.begin(); it != m_FileTimestamps.end();) {
        if (!std::filesystem::exists(it->first)) {
          changes.emplace_back(it->first, FileChange::Type::Deleted);
          it = m_FileTimestamps.erase(it);
        } else {
          ++it;
        }
      }
    }

    // Process changes outside of locks
    for (const auto &[path, type] : changes) {
      ProcessChange(path, type);
    }

  } catch (const std::exception &e) {
    BE_CORE_ERROR("[HotReload] Error scanning directory: {}", e.what());
  }
}

bool HotReloadManager::ShouldWatch(const std::filesystem::path &path) const {
  std::string ext = path.extension().string();

  for (const auto &watchedExt : m_WatchedExtensions) {
    if (ext == watchedExt) {
      return true;
    }
  }

  return false;
}

void HotReloadManager::ProcessChange(const std::filesystem::path &path,
                                     FileChange::Type type) {
  auto now = std::chrono::steady_clock::now();
  auto pathStr = path.string();

  auto it = m_LastChangeTime.find(pathStr);
  if (it != m_LastChangeTime.end()) {
    auto elapsed = std::chrono::duration<float>(now - it->second).count();
    if (elapsed < m_DebounceTime) {
      return; // Skip, within debounce window
    }
  }
  m_LastChangeTime[pathStr] = now;

  // Queue change
  FileChange change;
  change.path = path;
  change.type = type;
  change.timestamp = static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch())
          .count());

  {
    std::lock_guard<std::mutex> lock(m_ChangesMutex);
    m_PendingChanges.push_back(change);
  }

  const char *typeStr = "Unknown";
  switch (type) {
  case FileChange::Type::Created:
    typeStr = "Created";
    break;
  case FileChange::Type::Modified:
    typeStr = "Modified";
    break;
  case FileChange::Type::Deleted:
    typeStr = "Deleted";
    break;
  case FileChange::Type::Unknown:
    typeStr = "Unknown";
    break;
  }

  BE_CORE_TRACE("[HotReload] File {}: {}", typeStr, path.string());
}

std::vector<std::filesystem::path>
HotReloadManager::GetWatchedDirectories() const {
  std::lock_guard<std::mutex> lock(m_TimestampMutex);
  std::vector<std::filesystem::path> result;
  result.reserve(m_WatchedDirectories.size());
  for (const auto &wd : m_WatchedDirectories) {
    result.push_back(wd.path);
  }
  return result;
}

} // namespace BeEngine
