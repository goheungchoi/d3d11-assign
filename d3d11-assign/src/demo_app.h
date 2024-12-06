#pragma once

#include "common.h"

#include "D3DEngine/GameEngine/GameEngine.h"

#include "types.h"

class DemoApp : public GameEngine
{
	using Super = GameEngine;

#ifndef NDEBUG 
	float frameTime{ 0.f };
	std::size_t fps{ 0 };
	std::size_t count{ 0 };
#endif

	class D3D11Renderer* _renderer;
	class Camera* _camera;
  
	cbTransformConstants _transformConstants;
	cbShadingConstants _shadingConstants;

 public:

	bool isInitialized{ false };

	void Initialize();
	void Execute();
	void Shutdown();

private:
	void FixedUpdate(float) override;
	void Update(float) override;
	void Render() override;

private:
	float rotation{75};
	bool _useIBL{false};

	XMMATRIX _proj;
	XMMATRIX _view;

  ComPtr<ID3D11RasterizerState> _defaultRasterizerState;
  ComPtr<ID3D11DepthStencilState> _defaultDepthStencilState;
  ComPtr<ID3D11DepthStencilState> _skyboxDepthStencilState;

  ComPtr<ID3D11SamplerState> _defaultSampler;
  ComPtr<ID3D11SamplerState> _computeSampler;
  ComPtr<ID3D11SamplerState> _spBRDF_Sampler;

  ComPtr<ID3D11Buffer> _cboTransform;
  ComPtr<ID3D11Buffer> _cboShading;

  ShaderProgram _pbrProgram;
  ShaderProgram _skyboxProgram;
  ShaderProgram _tonemapProgram;

  MeshBuffer _pbrModel;
  MeshBuffer _skybox;

  Texture _albedoTexture;
  Texture _normalTexture;
  Texture _metalnessTexture;
  Texture _roughnessTexture;

  Texture _envTexture;
  Texture _irmapTexture;
  Texture _spBRDF_LUT;

	void InitTransformMatrices();
	void InitCamera();

	void InitBuffers();
	void InitShaders();
	void InitTextures();
  void InitSamplers();

	void InitSpecularBRDF_LUT();

	void InitIBL();

	void InitMeshes();
	void InitLights();

private:

	void InitImgui();

};
