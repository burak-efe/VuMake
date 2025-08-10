#pragma once
#include <iostream>

#include "02_OuterCore/Common.h"
#include "02_OuterCore/math/VuFloat3.h"

namespace Vu {

struct GameObject;

struct Component {
  GameObject* m_gameObject = nullptr;
  bool        m_enabled    = true;

  virtual ~Component() = default;

  virtual void Start() {}

  virtual void Update() {}
};

struct Transform : Component {
  float3 m_position;

  void Start() override { std::cout << "Transform Start()\n"; }

  void Update() override {}
};

struct MeshDrawer : Component {

  void Start() override { std::cout << "MeshDrawer()\n"; }
  void Update() override {}
};

} // namespace Vu
