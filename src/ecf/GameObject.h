#pragma once

#include <typeindex>

#include "Common.h"
#include "Component.h"

namespace Vu
{


    struct GameObject
    {
        string name;
        bool   active = true;

    private:
        std::vector<std::unique_ptr<Component>>         components;
        std::unordered_map<std::type_index, Component*> componentMap;

    public:
        // Add a component of type T
        template <typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must be a Component");

            auto comp        = std::make_unique<T>(std::forward<Args>(args)...);
            comp->gameObject = this;

            T* ptr = comp.get();
            components.emplace_back(std::move(comp));
            componentMap[std::type_index(typeid(T))] = ptr;

            ptr->Start(); // Call Start() immediately after adding
            return ptr;
        }

        // Get component of type T
        template <typename T>
        T* GetComponent()
        {
            auto it = componentMap.find(std::type_index(typeid(T)));
            if (it != componentMap.end())
                return static_cast<T*>(it->second);
            return nullptr;
        }

        // Check for component
        template <typename T>
        bool HasComponent()
        {
            return GetComponent<T>() != nullptr;
        }

        // Update all components
        void Update()
        {
            if (!active) return;
            for (auto& comp : components)
            {
                if (comp->enabled)
                {
                    comp->Update();
                }
            }
        }
    };
}
