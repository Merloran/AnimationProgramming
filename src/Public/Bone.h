#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <assimp/scene.h>
#include <list>
#include "BoneInfo.h"

struct KeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat orientation;
	float timeStamp;
};

class Bone
{
public:
	Bone(const std::string& name, int32_t ID, const aiNodeAnim* channel);

	void Update(float animationTime);

	glm::mat4 GetLocalTransform();
	std::string GetBoneName() const;
	int32_t GetBoneID();

	int32_t GetPositionIndex(float animationTime);
	int32_t GetRotationIndex(float animationTime);
	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
private:

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

	glm::mat4 InterpolatePosition(float animationTime);
	glm::mat4 InterpolateRotation(float animationTime);

	int32_t m_NumPositions;
	int32_t m_NumRotations;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int32_t m_ID;
};

