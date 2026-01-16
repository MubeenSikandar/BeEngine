// HotReloadManager.hpp
#pragma once

#include <Core.hpp>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <thread>

namespace BeEngine {

struct WatchedDirectory {
  std::filesystem::path path;
  bool recursive{true};
};

/**
 * @brief Watches script files for changes and triggers hot reload
 */
class BE_API HotReloadManager {
public:
  struct FileChange {
    std::filesystem::path path;
    enum class Type : uint8_t {
      Unknown = 0,
      Created,
      Modified,
      Deleted
    } type{Type::Unknown};
    uint64_t timestamp;
  };

  using FileChangeCallback = std::function<void(const FileChange &)>;

  HotReloadManager();
  ~HotReloadManager();

  // Non-copyable
  HotReloadManager(const HotReloadManager &) = delete;
  HotReloadManager &operator=(const HotReloadManager &) = delete;

  HotReloadManager(HotReloadManager &&) = delete;
  HotReloadManager &operator=(HotReloadManager &&) = delete;

  // ===== Configuration =====

  /// Add a directory to watch
  void AddWatchDirectory(const std::filesystem::path &directory,
                         bool recursive = true);

  NODISCARD std::vector<std::filesystem::path> GetWatchedDirectories() const;

  /// Remove a watched directory
  void RemoveWatchDirectory(const std::filesystem::path &directory);

  /// Set file extensions to watch (e.g., {".lua", ".py", ".cs"})
  void SetWatchedExtensions(const std::vector<std::string> &extensions);

  /// Set minimum time between reloads (debounce)
  void SetDebounceTime(float seconds) { m_DebounceTime = seconds; }

  void ClearWatchDirectories() {
    std::lock_guard<std::mutex> lock(m_TimestampMutex);
    m_WatchedDirectories.clear();
    m_FileTimestamps.clear();
  }

  NODISCARD std::vector<WatchedDirectory> GetWatchedDirectoriesInfo() const {
    std::lock_guard<std::mutex> lock(m_TimestampMutex);
    return m_WatchedDirectories;
  }

  // ===== Lifecycle =====

  /// Start watching for file changes
  void Start();

  /// Stop watching
  void Stop();

  /// Check if watching is active
  NODISCARD bool IsRunning() const { return m_Running; }

  // ===== Manual Polling =====

  /// Check for changes (call this each frame if not using background thread)
  void PollChanges();

  void SetPollInterval(float seconds) { m_PollInterval = seconds; }
  NODISCARD float GetPollInterval() const { return m_PollInterval; }

  /// Get pending changes and clear the queue
  NODISCARD std::vector<FileChange> GetPendingChanges();

  // ===== Callbacks =====

  void SetFileChangeCallback(FileChangeCallback callback) {
    m_Callback = std::move(callback);
  }

  NODISCARD float GetDebounceTime() const { return m_DebounceTime; }

  // ===== State Query =====

  /// Get last modification time for a file
  NODISCARD uint64_t
  GetLastModificationTime(const std::filesystem::path &path) const;

  /// Check if a file has been modified since last check
  NODISCARD bool HasFileChanged(const std::filesystem::path &path) const;

private:
  void WatchThread();
  void ScanDirectory(const std::filesystem::path &directory, bool recursive);
  NODISCARD bool ShouldWatch(const std::filesystem::path &path) const;
  void ProcessChange(const std::filesystem::path &path, FileChange::Type type);

  std::vector<WatchedDirectory> m_WatchedDirectories;
  std::vector<std::string> m_WatchedExtensions;

  // File timestamps
  std::unordered_map<std::string, uint64_t> m_FileTimestamps;
  mutable std::mutex m_TimestampMutex;

  // Pending changes
  std::vector<FileChange> m_PendingChanges;
  std::mutex m_ChangesMutex;

  // Threading
  std::thread m_WatchThread;
  std::atomic<bool> m_Running{false};
  std::atomic<bool> m_StopRequested{false};

  // Configuration
  float m_DebounceTime{0.5F}; // Seconds
  float m_PollInterval{0.1F}; // Seconds

  // Callback
  FileChangeCallback m_Callback;

  // Debounce tracking
  std::unordered_map<std::string, std::chrono::steady_clock::time_point>
      m_LastChangeTime;
};
} // namespace BeEngine
