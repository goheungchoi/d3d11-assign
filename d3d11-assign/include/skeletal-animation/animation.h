#pragma once

#include "common.h"

#include "d3d_utility.h"

#include "model.h"

struct AnimationNode {
	XMMATRIX transform;
	std::string name;
	int numChildren;
	std::vector<AnimationNode> children;
};

/**
 * @brief Reads the animation file and 
 */
class Animation {
	float _duration;	// Animation duration
	int _ticksPerSecond;	// Animation speed

	// Array of bones. Each bone contains animation key frame data of itself
	std::vector<Bone> _bones;
	AnimationNode _rootNode;
	std::unordered_map<std::string, BoneInfo> _boneInfoMap;

public:
	XMMATRIX _globalInverseTransform;

	Animation() = delete;
	Animation(const std::string& animationPath, Model* model) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			animationPath, 
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded
		);
		if (!scene || !scene->mRootNode) {
			char buf[256];
			sprintf(buf, "ERROR::ASSIMP::%s\n", importer.GetErrorString());
			throw std::exception(buf);
		}

		if (scene->mNumAnimations == 0) {
			throw std::exception("Animation:: No animation found!");
		}

		auto animation = scene->mAnimations[0];
		_duration = animation->mDuration;
		_ticksPerSecond = animation->mTicksPerSecond;

		_globalInverseTransform = XMMatrixTranspose(XMMATRIX(&scene->mRootNode->mTransformation.a1));
		_globalInverseTransform = XMMatrixInverse(nullptr, _globalInverseTransform);
		ReadHierarchyData(_rootNode, scene->mRootNode);
		ReadMissingBones(animation, model);
	}
	~Animation() {}

	Bone* FindBone(const std::string& name) {
		auto it = std::find_if(
			_bones.begin(), _bones.end(),
			[&](const Bone& bone) {
				return bone.GetBoneName() == name;
			}
		);
		if (it == _bones.end()) 
			return nullptr;
		
		return &(*it);
	}

	/**
	 * @brief Get the animation speed
	 * @return ticks per second
	 */
	float GetTicksPerSecond() { return _ticksPerSecond; }
	float GetDuration() { return _duration; }
	const AnimationNode& GetRootNode() { return _rootNode; }
	const std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() {
		return _boneInfoMap;
	}


private:
	/**
	 * @brief Read bone hierarchy data and retrieve bone information.
	 * @param dest AnimationNode to be filled
	 * @param src Assimp node data
	 */
	void ReadHierarchyData(AnimationNode& dest, const aiNode* src) {
		if (!src) {
			throw std::exception("Animation:: src is nullptr!");
		}

		dest.name = src->mName.data;
		dest.transform = XMMatrixTranspose(XMMATRIX(&src->mTransformation.a1));
		dest.numChildren = src->mNumChildren;

		for (int i = 0; i < src->mNumChildren; ++i) {
			AnimationNode newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	void ReadMissingBones(const aiAnimation* animation, Model* model) {
		// Get the number of bone animation channels
		// Each channel affects a single node.
		int numChannels = animation->mNumChannels;

		auto& modelBoneInfoMap = model->GetBoneInfoMap();
		int& modelNumBones = model->GetNumBones();

		// Retrieve bone info and find bones that are not extracted
		// when loading a model
		for (int i = 0; i < numChannels; ++i) {
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			// If this bone is not found in the model
			if (modelBoneInfoMap.find(boneName) == modelBoneInfoMap.end()) {
				// Add this bone to the model
				modelBoneInfoMap[boneName].id = modelNumBones;
				modelNumBones++;
			}

			// Create a bone and store the information
			_bones.push_back(
				Bone(
					boneName,
					modelBoneInfoMap[boneName].id,
					channel
				)
			);
		}

		// Copy the bone info map of the model
		_boneInfoMap = modelBoneInfoMap;
	}

};
