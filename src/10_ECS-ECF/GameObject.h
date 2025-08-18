#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "02_OuterCore/Common.h"
#include "Component.h"

namespace Vu {

struct GameObject {
  std::string m_name {};
  bool        m_active {true};

private:
  std::unordered_map<std::type_index, std::shared_ptr<Component>> m_componentMap {};

public:
  // Add a component of type T
  template <typename T, typename... Args>
  T*
  AddComponent(Args&&... args) {
    static_assert(std::is_base_of_v<Component, T>, "T must be a Component");

    auto comp        = std::make_shared<T>(std::forward<Args>(args)...);
    comp->gameObject = this;

    T* ptr = comp.get();

    m_componentMap[std::type_index(typeid(T))] = ptr;

    ptr->Start(); // Call Start() immediately after adding
    return ptr;
  }

  // Get component of type T
  template <typename T>
  T*
  GetComponent() {
    auto it = m_componentMap.find(std::type_index(typeid(T)));
    if (it != m_componentMap.end()) return static_cast<T*>(it->second);
    return nullptr;
  }

  // Check for component
  template <typename T>
  bool
  HasComponent() {
    return GetComponent<T>() != nullptr;
  }

  template <typename T>
  void
  RemoveComponent() {
    auto it = m_componentMap.find(std::type_index(typeid(T)));
    if (it != m_componentMap.end()) { m_componentMap.erase(it); }
  }

  // Update all components
  void
  Update() {
    if (!m_active) return;
    for (auto& [typeID, comp] : m_componentMap) {
      if (comp->m_enabled) { comp->Update(); }
    }
  }
};

} // namespace Vu