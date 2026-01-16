// ScriptRegistry.cpp
#include <PCH/BeEnginePCH.hpp>
#include <set>

namespace BeEngine {

void ScriptRegistry::RegisterInternal(const std::string &name,
                                      ScriptInfo &&info) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  if (registry.find(name) != registry.end()) {
    BE_CORE_WARN("[ScriptRegistry] Overwriting existing script: {}", name);
  }

  registry[name] = std::move(info);
  BE_CORE_TRACE("[ScriptRegistry] Registered script: {} ({})", name,
                ScriptLanguageToString(registry[name].language));
}

void ScriptRegistry::RegisterFromEngine(const std::string &name,
                                        ScriptLanguage language,
                                        ScriptInstanceFactory factory,
                                        const ScriptFieldMap &defaultFields) {
  ScriptInfo info;
  info.name = name;
  info.displayName = name;
  info.language = language;
  info.factory = std::move(factory);
  info.defaultFields = defaultFields;

  RegisterInternal(name, std::move(info));
}

void ScriptRegistry::Unregister(const std::string &name) {
  std::lock_guard<std::mutex> lock(GetMutex());
  GetRegistry().erase(name);
  BE_CORE_TRACE("[ScriptRegistry] Unregistered script: {}", name);
}

void ScriptRegistry::UnregisterLanguage(ScriptLanguage language) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  for (auto it = registry.begin(); it != registry.end();) {
    if (it->second.language == language) {
      BE_CORE_TRACE("[ScriptRegistry] Unregistered script: {}", it->first);
      it = registry.erase(it);
    } else {
      ++it;
    }
  }
}

void ScriptRegistry::Clear() {
  std::lock_guard<std::mutex> lock(GetMutex());
  GetRegistry().clear();
  BE_CORE_INFO("[ScriptRegistry] Cleared all registrations");
}

bool ScriptRegistry::IsRegistered(const std::string &name) {
  std::lock_guard<std::mutex> lock(GetMutex());
  return GetRegistry().contains(name);
}

const ScriptRegistry::ScriptInfo *
ScriptRegistry::GetScriptInfo(const std::string &name) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();
  auto it = registry.find(name);
  return it != registry.end() ? &it->second : nullptr;
}

std::vector<std::string> ScriptRegistry::GetRegisteredScripts() {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  std::vector<std::string> names;
  names.reserve(registry.size());

  for (const auto &[name, info] : registry) {
    names.push_back(name);
  }

  std::ranges::sort(names);
  return names;
}

std::vector<std::string>
ScriptRegistry::GetScriptsByCategory(const std::string &category) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  std::vector<std::string> names;
  for (const auto &[name, info] : registry) {
    if (info.category == category) {
      names.push_back(name);
    }
  }

  std::ranges::sort(names);
  return names;
}

std::vector<std::string>
ScriptRegistry::GetScriptsByLanguage(ScriptLanguage language) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  std::vector<std::string> names;
  for (const auto &[name, info] : registry) {
    if (info.language == language) {
      names.push_back(name);
    }
  }

  std::ranges::sort(names);
  return names;
}

std::vector<std::string> ScriptRegistry::GetCategories() {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  std::set<std::string> categories;
  for (const auto &[name, info] : registry) {
    if (!info.category.empty()) {
      categories.insert(info.category);
    }
  }

  return {categories.begin(), categories.end()};
}

Ref<ScriptInstance> ScriptRegistry::CreateInstance(const std::string &name,
                                                   Entity entity) {
  std::lock_guard<std::mutex> lock(GetMutex());
  auto &registry = GetRegistry();

  auto it = registry.find(name);
  if (it == registry.end()) {
    BE_CORE_ERROR("[ScriptRegistry] Script not found: {}", name);
    return nullptr;
  }

  if (!it->second.factory) {
    BE_CORE_ERROR("[ScriptRegistry] Script has no factory: {}", name);
    return nullptr;
  }

  auto instance = it->second.factory(entity);
  if (instance) {
    instance->SetScriptName(name);
    // Copy default fields
    for (const auto &[fieldName, field] : it->second.defaultFields) {
      instance->GetFields().Add(field);
    }
  }

  return instance;
}

Ref<ScriptInstance>
ScriptRegistry::CreateInstance(const std::string &name, Entity entity,
                               const ScriptFieldMap &initialFields) {
  auto instance = CreateInstance(name, entity);
  if (instance) {
    instance->GetFields().CopyValuesFrom(initialFields);
  }
  return instance;
}

} // namespace BeEngine
