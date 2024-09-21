#pragma once

#include "common.h"
#include "d3d_utility.h"

class Mesh {
	ID3D11Device* const _device;
	ID3D11DeviceContext* const _context;

	Mesh* _parent;
	std::list<Mesh*> _children;

public:

	std::vector<SimpleVertex> vertices;
	std::vector<Index> indices;
	std::vector<Texture> textures;

	XMVECTOR _translation;
	XMVECTOR _scale;
	XMVECTOR _rotation;	// quaternion.

	XMMATRIX _localTransform;

	bool _bShouldUpdateModelTransform{ true };
	XMMATRIX _modelTransform{ XMMatrixIdentity() };

	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* context) :
		_device{ device },
		_context{ context },
		_parent{ nullptr },
		_translation{ XMVectorZero() },
		_scale{ 1.f, 1.f, 1.f, 1.f },
		_rotation{ XMQuaternionIdentity()},
		_localTransform{ XMMatrixIdentity() } {}

	void AddChildSceneComponent(Mesh* child) {
		child->_parent = this;
		_children.push_back(child);
	}

	void RemoveChildSceneComponent(Mesh* child) {
		child->_parent = nullptr;
		_children.remove(child);
	}

	void SetTranslation(XMVECTOR translation) {
		_translation = translation;
	}

	void SetScale(XMVECTOR scale) {
		_scale = scale;
	}

	void SetRotation(XMVECTOR rotation) {
		_rotation = rotation;
	}

	void Translate(XMVECTOR translate) {
		_translation += translate;
	}

	void Scale(XMVECTOR scale) {
		_scale += scale;
	}

	/**
	 * @brief Rotate the scene object
	 * @param axis The normalized axis vector
	 * @param degree The degree to rotate around the axis
	 */
	void RotateAxis(XMVECTOR axis, float degree) {
		float rad = XMConvertToRadians(degree);

		XMVECTOR rotationQuat = XMQuaternionRotationNormal(axis, rad);

		// Multiply the current rotation quaternion by the new rotation quaternion
		_rotation = XMQuaternionMultiply(_rotation, rotationQuat);
		_rotation = XMQuaternionNormalize(_rotation);
	}

	void RotateX(float degree) {
		RotateAxis({ 1.f, 0.f, 0.f, 0.f }, degree);
	}

	void RotateY(float degree) {
		RotateAxis({ 0.f, 1.f, 0.f, 0.f }, degree);
	}

	void RotateZ(float degree) {
		RotateAxis({ 0.f, 0.f, 1.f, 0.f }, degree);
	}

	void UpdateLocalTransform() {
		XMMATRIX scaling = XMMatrixScalingFromVector(_scale);
		XMMATRIX rotation = XMMatrixRotationQuaternion(_rotation);
		XMMATRIX translation = XMMatrixTranslationFromVector(_translation);
		_localTransform = scaling * rotation * translation;

		_bShouldUpdateModelTransform = true;
	}

	XMMATRIX GetModelTransform() {
		if (_bShouldUpdateModelTransform) {
			if (_parent) {
				XMMATRIX _parentModel = _parent->GetModelTransform();
				_modelTransform = _localTransform * _parentModel;
			}
			else {
				_modelTransform = _localTransform;
			}
			_bShouldUpdateModelTransform = false;
		}

		return _modelTransform;
	}

	void Draw();

protected:

	ID3D11InputLayout* _inputLayout{ nullptr };
	ID3D11VertexShader* _vs{ nullptr };
	ID3D11PixelShader* _ps{ nullptr };

	bool InitPipeline();

protected:

	ID3D11Buffer* _vbo{ nullptr };
	UINT _vbStride{ 0U };
	UINT _vbOffset{ 0U };
	UINT _vertexCount{ 0U };

	ID3D11Buffer* _ibo{ nullptr };
	UINT _ibStride{ 0U };
	UINT _ibOffset{ 0U };
	UINT _indexCount{ 0U };

	ID3D11Buffer* _cboPerFrame{ nullptr };
	cbPerFrame _cbPerFrame{};
	ID3D11Buffer* _cboPerObject{ nullptr };
	cbPerObject _cbPerObject{};

	bool InitBuffers();
};