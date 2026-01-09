#pragma once

#include "Core.hpp"
#include "MaterialSystem/Material.hpp"

namespace BeEngine {
/**
 * @brief Manages material assets with caching
 */
class BE_API MaterialLibrary {

public:
  /**
   * @brief Add a material to the library
   */
  void Add(const std::string &name, const Ref<Material> &material);

  /**
   * @brief Create and add a material
   */
  Ref<Material> Create(const std::string &name, const Ref<Shader> &shader);

  /**
   * @brief Get a material by name
   */
  NODISCARD Ref<Material> Get(const std::string &name) const;

  /**
   * @brief Check if material exists
   */
  NODISCARD bool Exists(const std::string &name) const;

  /**
   * @brief Remove a material
   */
  void Remove(const std::string &name);

  /**
   * @brief Clear all materials
   */
  void Clear();

  /**
   * @brief Get all material names
   */
  NODISCARD std::vector<std::string> GetAllNames() const;

  /**
   * @brief Get material count
   */
  NODISCARD size_t Size() const { return m_Materials.size(); }

private:
  std::unordered_map<std::string, Ref<Material>> m_Materials;
};

} // namespace BeEngine
