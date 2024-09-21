#include "scene/cube.h"

static SimpleVertex cubeVertices[8] = {
	SimpleVertex{{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
	SimpleVertex{{-1.0f, +1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
	SimpleVertex{{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
	SimpleVertex{{+1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
	SimpleVertex{{-1.0f, -1.0f, +1.0f}, {0.0f, 1.0f, 1.0f}},
	SimpleVertex{{-1.0f, +1.0f, +1.0f}, {1.0f, 1.0f, 1.0f}},
	SimpleVertex{{+1.0f, +1.0f, +1.0f}, {1.0f, 0.0f, 1.0f}},
	SimpleVertex{{+1.0f, -1.0f, +1.0f}, {1.0f, 0.0f, 0.0f}},
};

static Index cubeIndices[36] = {
	// front face
	0, 1, 2,
	0, 2, 3,

	// back face
	4, 6, 5,
	4, 7, 6,

	// left face
	4, 5, 1,
	4, 1, 0,

	// right face
	3, 2, 6,
	3, 6, 7,

	// top face
	1, 5, 6,
	1, 6, 2,

	// bottom face
	4, 0, 3,
	4, 3, 7
};

Cube::Cube(ID3D11Device* device, ID3D11DeviceContext* context)
	: Mesh(device, context) {

	const int numVertices = std::size(cubeVertices);
	const int numIndices = std::size(cubeIndices);

	vertices.resize(numVertices);
	indices.resize(numIndices);

	memcpy(vertices.data(), cubeVertices, sizeof(SimpleVertex) * numVertices);
	memcpy(indices.data(), cubeIndices, sizeof(Index) * numIndices);

	InitPipeline();
	InitBuffers();
}
