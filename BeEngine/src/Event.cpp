#include "Events/Event.hpp"
#include <atomic>

namespace BeEngine {
std::atomic<uint64_t> Event::s_NextEventID{1};
} // namespace BeEngine
