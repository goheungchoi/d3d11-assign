#include "scene/cube.h"

static Vector3 cubeVertices[8] = {


};

static Vector3 vertexColors[8] = {


};

static Index cubeIndices[12] = {

};

Cube::Cube(ID3D11Device* device, ID3D11DeviceContext* context)
	: Mesh(device, context) {

	vertices.resize(8);
	indices.resize(12);

	for (int i = 0; i < 8; ++i) {
		vertices[i].position = cubeVertices[i];
		vertices[i].color = vertexColors[i];
	}

	for (int i = 0; i < 12; ++i) {
		indices[i] = cubeIndices[i];
	}

	bool InitPipeline();
	bool InitBuffers();
}
