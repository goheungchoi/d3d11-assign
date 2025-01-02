#pragma once

#include "D3DEngine/GameEngine/GameEngine.h"
#include "Common.h"

class DemoApp : public GameEngine {
  using Super = GameEngine;

#ifndef NDEBUG
  float frameTime{0.f};
  std::size_t fps{0};
  std::size_t count{0};
#endif

  class IRenderer* _renderer;

 public:
  bool isInitialized{false};

  void Initialize();
  void Execute();
  void Shutdown();

 private:
  void FixedUpdate(float) override;
  void Update(float) override;
  void Render() override;

 private:

 private:
  void InitImgui();
};
