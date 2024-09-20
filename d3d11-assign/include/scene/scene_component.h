#pragma once

#include "common.h"
#include "d3d_utility.h"

class Mesh {
	Mesh* _parent;
	std::list<Mesh*> _children;

public:

	std::vector<Vertex> vertices;
	std::vector<Color> colors;
	std::vector<Index> indices;

	XMVECTOR _translation;
	XMVECTOR _scale;
	XMVECTOR _rotation;	// quaternion.

	XMMATRIX _localTransform;

	Mesh() :
		_parent{ nullptr },
		_translation{ XMVectorZero() },
		_scale{ XMVectorZero() },
		_rotation{ XMVectorZero() },
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

	void SetTranslation(XMVECTOR translation) {
		_translation = translation;
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

	void Update(float dt) {
		_localTransform = 
	}
};