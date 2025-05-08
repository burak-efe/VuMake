#pragma once
#include <iostream>

#include "10_Core/Common.h"
#include "10_Core/math/VuFloat3.h"

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
        vec3 position;

        void Start() override
        {
            std::cout << "Transform Start()\n";
        }

        void Update() override
        {
        }
    };

} // Vu

