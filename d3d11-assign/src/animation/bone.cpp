#include "bone.h"

#include <assimp/scene.h>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : _name(std::move(const_cast<std::string&>(name))),
      _ID(ID),
      _localTransform(XMMatrixIdentity()) {
  // Read positions
  _numPositions = channel->mNumPositionKeys;
  _positions.resize(_numPositions);
  for (int positionIndex = 0; positionIndex < _numPositions; ++positionIndex) {
    aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
    float timeStamp = channel->mPositionKeys[positionIndex].mTime;

    KeyPosition data;
    data.position = XMVECTOR{aiPosition.x, aiPosition.y, aiPosition.z, 1.f};
    data.timeStamp = timeStamp;
    _positions[positionIndex] = data;
  }

  // Read rotations
  _numRotations = channel->mNumRotationKeys;
  _rotations.resize(_numRotations);
  for (int rotationIndex = 0; rotationIndex < _numRotations; ++rotationIndex) {
    aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
    float timeStamp = channel->mRotationKeys[rotationIndex].mTime;

    KeyRotation data;
    data.orientation = XMVECTOR{aiOrientation.x, aiOrientation.y,
                                aiOrientation.z, aiOrientation.w};
    data.orientation = XMQuaternionNormalize(data.orientation);
    data.timeStamp = timeStamp;
    _rotations[rotationIndex] = data;
  }

  // Read scales
  _numScalings = channel->mNumScalingKeys;
  _scales.resize(_numScalings);
  for (int scaleIndex = 0; scaleIndex < _numScalings; ++scaleIndex) {
    aiVector3D aiScale = channel->mScalingKeys[scaleIndex].mValue;
    float timeStamp = channel->mScalingKeys[scaleIndex].mTime;

    KeyScale data;
    data.scale = XMVECTOR{aiScale.x, aiScale.y, aiScale.z, 0.f};
    data.timeStamp = timeStamp;
    _scales[scaleIndex] = data;
  }
}
