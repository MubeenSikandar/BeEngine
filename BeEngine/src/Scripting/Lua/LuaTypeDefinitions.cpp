// LuaTypeDefinitions.cpp

#include <PCH/BeEnginePCH.hpp>
#include <sstream>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace BeEngine {
// ============================================================================
// LuaRef Implementation
// ============================================================================

void LuaRef::Release() {
  if (L != nullptr && ref != LUA_NOREF) {
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    ref = LUA_NOREF;
  }
  L = nullptr;
}

void LuaRef::Push() const {
  if (L != nullptr && ref != LUA_NOREF) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  } else if (L != nullptr) {
    lua_pushnil(L);
  }
}

// ============================================================================
// LuaRefGuard Implementation
// ============================================================================

LuaRefGuard::~LuaRefGuard() { m_Ref.Release(); }

LuaRefGuard::LuaRefGuard(LuaRefGuard &&other) noexcept : m_Ref(other.m_Ref) {
  other.m_Ref = LuaRef{};
}

LuaRefGuard &LuaRefGuard::operator=(LuaRefGuard &&other) noexcept {
  if (this != &other) {
    m_Ref.Release();
    m_Ref = other.m_Ref;
    other.m_Ref = LuaRef{};
  }
  return *this;
}

LuaRef LuaRefGuard::Release() {
  LuaRef ref = m_Ref;
  m_Ref = LuaRef{};
  return ref;
}

// ============================================================================
// LuaValue Utility Functions
// ============================================================================

std::string LuaValueToString(const LuaValue &value) {
  return std::visit(
      [](auto &&arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          return "nil";
        } else if constexpr (std::is_same_v<T, bool>) {
          return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, double>) {
          // Remove trailing zeros for cleaner output
          std::string str = std::to_string(arg);
          size_t dotPos = str.find('.');
          if (dotPos != std::string::npos) {
            size_t lastNonZero = str.find_last_not_of('0');
            if (lastNonZero != std::string::npos && lastNonZero > dotPos) {
              str.erase(lastNonZero + 1);
            }
            // Remove trailing dot if no decimals
            if (str.back() == '.') {
              str.pop_back();
            }
          }
          return str;
        } else if constexpr (std::is_same_v<T, std::string>) {
          return "\"" + arg + "\"";
        } else if constexpr (std::is_same_v<T, LuaRef>) {
          if (!arg.IsValid()) {
            return "<invalid ref>";
          }
          if (arg.IsNil()) {
            return "nil";
          }
          return "<ref:" + std::to_string(arg.ref) + ">";
        } else if constexpr (std::is_same_v<T, void *>) {
          if (arg == nullptr) {
            return "<null userdata>";
          }
          // Format pointer address
          std::ostringstream oss;
          oss << "<userdata: " << arg << ">";
          return oss.str();
        } else {
          return "<unknown>";
        }
      },
      value);
}

LuaType GetLuaValueType(const LuaValue &value) {
  return std::visit(
      [](auto &&arg) -> LuaType {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          return LuaType::Nil;
        } else if constexpr (std::is_same_v<T, bool>) {
          return LuaType::Boolean;
        } else if constexpr (std::is_same_v<T, double>) {
          return LuaType::Number;
        } else if constexpr (std::is_same_v<T, std::string>) {
          return LuaType::String;
        } else if constexpr (std::is_same_v<T, LuaRef>) {
          // LuaRef could be table, function, or userdata
          // We can't know without pushing and checking
          // Default to Table as most common case
          return LuaType::Table;
        } else if constexpr (std::is_same_v<T, void *>) {
          return LuaType::LightUserdata;
        } else {
          return LuaType::None;
        }
      },
      value);
}

} // namespace BeEngine
