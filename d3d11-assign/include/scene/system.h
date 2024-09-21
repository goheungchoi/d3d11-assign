#pragma once

#include "scene/mesh.h"

class System : public Mesh {

public:
	System(ID3D11Device* device,
		ID3D11DeviceContext* context);
};
