#pragma once

#include "common.h"
#include "d3d_utility.h"

class aiNodeAnim;

struct BoneInfo {
	// The id of the bone
	int id;
	// Convert the positions and orientations of the bone's local space
	// to the model space.
	XMMATRIX offset;
};

struct KeyPosition {
	XMVECTOR position;
	float timeStamp;
};

struct KeyRotation {
	/**
	 * @brief Orientation angles represented by quaternion.
	 * RotationAngle is in radians
	 * x = RotationAxis.x * sin(RotationAngle / 2)
	 * y = RotationAxis.y * sin(RotationAngle / 2)
	 * z = RotationAxis.z * sin(RotationAngle / 2)
	 * w = cos(RotationAngle / 2)
	 * 
	 * To get the rotation angle, 
	 * std::acos(w) * 2.
	 */
	XMVECTOR orientation;
	float timeStamp;
};

struct KeyScale {
	XMVECTOR scale;
	float timeStamp;
};

class Bone {
	std::vector<KeyPosition> _positions;	// Animation keyframe positions
	std::vector<KeyRotation> _rotations;	// Animation keyframe rotations
	std::vector<KeyScale> _scales;	// Animation keyframe scales
	int _numPositions;	// The number of position transform keyframes
	int _numRotations;	// The number of rotation transform keyframes
	int _numScalings;		// The number of scaling transform keyframes

	XMMATRIX _localTransform;
	std::string _name;
	int _ID;

public:
	Bone(const std::string& name, int ID, const aiNodeAnim* channel);

	/**
	 * @brief Interpolates between positions, rotations, and scaling keys
	 * based on the current time of the animation and prepares the local
	 * transformation matrix by combining all keys transformations.
	 * @param animationTime The current animation time
	 */
	void Update(float animationTime) {
		XMMATRIX translate = InterpolatePosition(animationTime);
		XMMATRIX rotation = InterpolateRotation(animationTime);
		XMMATRIX scaling = InterpolateScaling(animationTime);
		_localTransform = scaling * rotation * translate;
	}

	XMMATRIX GetLocalTransform() const { return _localTransform; }
	std::string GetBoneName() const { return _name; }
	int GetBoneID() const { _ID; }

	/**
	 * @brief Find the current index of the keyframe position to interpolate.
	 * based on the current animation time.
	 * @param animationTime The current animation time
	 * @return The index of the keyframe position.
	 */
	int GetPositionIndex(float animationTime) const {
		for (int index = 0; index < _numPositions - 1; ++index) {
			if (animationTime < _positions[index + 1].timeStamp)
				return index;
		}
		return -1;
	}

	/**
	 * @brief Find the current index of the keyframe rotation to interpolate.
	 * @param animationTime The current animation time
	 * @return The index of the keyframe rotation.
	 */
	int GetRotationIndex(float animationTime) const {
		for (int index = 0; index < _numRotations - 1; ++index) {
			if (animationTime < _rotations[index + 1].timeStamp)
				return index;
		}
		return -1;
	}

	/**
	 * @brief Find the current index of the keyframe scale to interpolate.
	 * @param animationTime The current animation time
	 * @return The index of the keyframe scale.
	 */
	int GetScaleIndex(float animationTime) const {
		for (int index = 0; index < _numScalings - 1; ++index) {
			if (animationTime < _scales[index + 1].timeStamp)
				return index;
		}
		return -1;
	}

private:
	/**
	 * @brief Get the normalized scale factor for lerp and slerp.
	 * @param prevKeyframeTimeStamp The time stamp of the previous keyframe.
	 * @param nextKeyframeTimeStamp The time stamp of the next keyframe.
	 * @param animationTime The current animation time.
	 * @return The scale factor in [0, 1].
	 */
	float GetScaleFactor(float prevKeyframeTimeStamp, float nextKeyframeTimeStamp, float animationTime) {
		float scaleFactor{ 0.f };
		float midwayLength = animationTime - prevKeyframeTimeStamp;
		float keyframeLength = nextKeyframeTimeStamp - prevKeyframeTimeStamp;
		scaleFactor = midwayLength / keyframeLength;
		return scaleFactor;
	}

	XMMATRIX InterpolatePosition(float animationTime) {
		if (_numPositions == 1)
			return XMMatrixTranslationFromVector(_positions[0].position);

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;

		float scaleFactor = GetScaleFactor(
			_positions[p0Index].timeStamp,
			_positions[p1Index].timeStamp,
			animationTime
		);

		XMVECTOR interpolatedPosition = XMVectorLerp(
			_positions[p0Index].position,
			_positions[p1Index].position,
			scaleFactor
		);

		return XMMatrixTranslationFromVector(interpolatedPosition);
		//return XMMatrixIdentity();
	}

	XMMATRIX InterpolateRotation(float animationTime) {
		if (_numRotations == 1) {
			// https://users.ncsa.illinois.edu/kindr/emtc/quaternions/
			// For any unit quaternion, q, we have q^(-1) = q^H,
			// where q^H = [w, -v], meaning that the inverse is the conjugate.
			// Rotation of a 3D vector, v, by a unit quaternion, q, is defined as
			// R_q(v) = q[0, v]q^(-1) where [0, v] is a pure quaternion
			// built from v by adding a zero real part.
			XMVECTOR quatRotation = XMQuaternionNormalize(_rotations[0].orientation);
			return XMMatrixRotationQuaternion(quatRotation);
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		
		float scaleFactor = GetScaleFactor(
			_rotations[p0Index].timeStamp,
			_rotations[p1Index].timeStamp,
			animationTime
		);

		XMVECTOR interpolatedQuatRotation = XMQuaternionSlerp(
			XMQuaternionNormalize(_rotations[p0Index].orientation),
			XMQuaternionNormalize(_rotations[p1Index].orientation),
			scaleFactor
		);
		return XMMatrixRotationQuaternion(interpolatedQuatRotation);
		//return XMMatrixRotationQuaternion(XMQuaternionIdentity());
	}

	XMMATRIX InterpolateScaling(float animationTime) {
		if (_numScalings == 1) {
			return XMMatrixScalingFromVector(_scales[0].scale);
		}

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;

		float scaleFactor = GetScaleFactor(
			_scales[p0Index].timeStamp,
			_scales[p1Index].timeStamp,
			animationTime
		);

		XMVECTOR interpolatedScale = XMVectorLerp(
			_scales[p0Index].scale,
			_scales[p1Index].scale,
			scaleFactor
		);
		return XMMatrixScalingFromVector(interpolatedScale);
	}

};
