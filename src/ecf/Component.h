#pragma once
#include "Common.h"

namespace Vu {

    struct GameObject;


    struct Component
    {
        GameObject* gameObject = nullptr;
        bool        enabled    = true;

        virtual ~Component() = default;

        virtual void Start()
        {
        }

        virtual void Update()
        {
        }
    };


    struct Transform : Component
    {
        float3 position;

        void Start() override
        {
            std::cout << "Transform Start()\n";
        }

        void Update() override
        {
        }
    };

} // Vu

