#pragma once

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

inline XMMATRIX CreateReversedZBufferPerspectiveMatrix(float fov, float aspect,
                                                       float nearPlane,
                                                       float farPlane) {
  float tan_half_fov = tanf(fov / 2.f);

  float _11 = 1.f / (aspect * tan_half_fov);
  float _22 = 1.f / tan_half_fov;
  float _33 = nearPlane / (nearPlane - farPlane);
  float _43 = -(farPlane * nearPlane) / (nearPlane - farPlane);

  return XMMATRIX{_11, 0.f, 0.f, 0.f, 
									0.f, _22, 0.f, 0.f,
                  0.f, 0.f, _33, 1.f, 
									0.f, 0.f, _43, 0.f};
}
