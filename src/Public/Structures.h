//#pragma once
//#include <cstdint>
//#include <string>
//#include <vector>
//#include "glm/glm.hpp"
//#include "glm/gtx/compatibility.hpp"
//#include "glm/gtx/rotate_vector.hpp"
//
//static const uint32_t InitialFNV = 2166136261U;
//static const uint32_t FNVMultiple = 16777619U;
//
//struct Rig
//{
//	std::vector<HashedName> BoneNameHashes;
//	std::vector<int16_t> Parents;
//	std::vector<Xform> RefPoses;
//	uint32_t NumBones;
//};
//
//struct HashedName
//{
//	uint32_t Value;
//
//	HashedName(const char* name)
//	{
//		Value = InitialFNV;
//		const uint64_t len = strlen(name);
//
//		for (uint64_t i = 0; i < len; ++i)
//		{
//			Value = (Value ^ name[i]) * FNVMultiple;
//		}
//	}
//};
//
//struct Xform
//{
//	glm::vec4 Position;
//	glm::quat Rotation;
//};
//
//struct Pose
//{
//	Xform* Xforms;
//	uint32_t NumXforms;
//	float* Tracks;
//	uint32_t NumTracks;
//};
//
//struct AnimCurveKey
//{
//	uint16_t KeyTime;
//	uint16_t KeyData[3];
//};
//
//struct AnimCurve
//{
//	uint16_t Index;
//	uint16_t NumKeys;
//	uint16_t NumPosKeys;
//	AnimCurveKey* Keys;
//	AnimCurveKey* PosKeys;
//};
//
//struct AnimClip
//{
//	HashedName Name;
//	float Duration;
//	uint32_t NumCurves;
//	uint32_t NumDataKeys;
//	AnimCurve* Curves;
//	AnimCurveKey* DataKeys;
//};
//
//// 4(sqrt(2)-1)
//const float Km = 4.0f * 0.4142135679721832275390625f;
//
//// sqrt(2)+1 = 1/(sqrt(2)-1)
//const float Khf = 2.414213657379150390625f;
//
//// 3-2sqrt(2)
//const float Khi = 0.17157287895679473876953125f;
//
//// compression
//glm::vec3 QuatFhm(const glm::quat& q)
//{
//	glm::vec3 v;
//
//	float s = Khf / (1.0f + q.w + sqrt(2.0f + 2.0f * q.w));
//
//	v.x = q.x * s;
//	v.y = q.y * s;
//	v.z = q.z * s;
//
//	return v;
//}
//
//// decompression
//glm::quat QuatIhm(const glm::vec3& v)
//{
//	float d = Khi * glm::dot(v, v);
//	float a = (1.0f + d);
//	float b = (1.0f - d) * Km;
//	float c = 1.0f / (a * a);
//
//	float bc = b * c;
//
//	glm::quat q;
//	q.x = v.x * bc;
//	q.y = v.y * bc;
//	q.z = v.z * bc;
//	q.w = (1.0f + d * (d - 6.0f)) * c;
//
//	return q;
//}
//
//float DecompressFloatMinusOnePlusOne(uint16_t Value)
//{
//	return (float(Value) / 65535.0f) * 2.0f - 1.0f;
//}
//
//uint16_t CompressFloatMinusOnePlusOne(float Value)
//{
//	return uint16_t(((Value + 1.0f) / 2.0f) * 65535.0f);
//}
//
//uint16_t* Vec3To16bit(const glm::vec3& v)
//{
//	uint16_t result[3];
//
//	result[0] = CompressFloatMinusOnePlusOne(v.x);
//	result[1] = CompressFloatMinusOnePlusOne(v.y);
//	result[2] = CompressFloatMinusOnePlusOne(v.z);
//
//	return result;
//}
//
//glm::vec3 I6bitToVec3(const uint16_t* b)
//{
//	glm::vec3 result;
//
//	result.x = CompressFloatMinusOnePlusOne(b[0]);
//	result.y = CompressFloatMinusOnePlusOne(b[1]);
//	result.z = CompressFloatMinusOnePlusOne(b[2]);
//
//	return result;
//}
//
//const float PosRange = 20.0f;
//
//uint16_t* Vec4Com16bit(const glm::vec4& v)
//{
//	glm::vec3 vec = { v.x / PosRange, v.y / PosRange, v.z / PosRange };
//
//	return Vec3To16bit(vec);
//}
//
//glm::vec4 Vec4Decom16bit(const uint16_t* b)
//{
//	glm::vec3 vec = I6bitToVec3(b);
//
//	glm::vec4 result;
//	result.x = vec.x * PosRange;
//	result.y = vec.y * PosRange;
//	result.z = vec.z * PosRange;
//	result.w = 0.0f;
//
//	return result;
//}
//
//uint16_t* QuatFhm16bit(const glm::quat& q)
//{
//	glm::vec3 vec = QuatFhm(q);
//
//	return Vec3To16bit(vec);
//}
//
//glm::quat QuatIhm16bit(const uint16_t* b)
//{
//	glm::vec3 vec = I6bitToVec3(b);
//
//	return QuatIhm(vec);
//}
//
//glm::quat DecompressRotationKey(const AnimCurveKey& key)
//{
//	const uint16_t* keyData = key.KeyData;
//
//	return QuatIhm16bit(keyData);
//}
//
//glm::vec4 DecompressPositionKey(const AnimCurveKey& key)
//{
//	const uint16_t* keyData = key.KeyData;
//
//	return Vec4Decom16bit(keyData);
//}
//
//float DecompressKeyTime(const uint16_t time)
//{
//	return float(time) / 24.0f;
//}
//
//glm::vec4 fm_quat_rotate(const glm::quat& q, const glm::vec4& v)
//{
//	glm::vec3 uv, uuv;
//	glm::vec3 qvec(q.x, q.y, q.z);
//	uv = glm::cross(qvec, glm::vec3(v));
//	uuv = glm::cross(qvec, uv);
//	uv *= (2.0f * q.w);
//	uuv *= 2.0f;
//	return glm::vec4(glm::vec3(v) + uv + uuv, v.w);
//}
//
//Xform fm_xform_mul(const Xform& a, const Xform& b)
//{
//	Xform result;
//	// Oblicz pozycj� wynikow�
//	result.Position = a.Position + fm_quat_rotate(b.Rotation, a.Position);
//	
//	// Oblicz obr�t wynikowy
//	result.Rotation = b.Rotation * a.Rotation;
//	return result;
//}
//
//
//void PoseLocalToModel(Pose* modelPose, const Pose* localPose, const int16_t* parentIndices)
//{
//	const Xform* localXforms = localPose->Xforms;
//	Xform* modelXforms = modelPose->Xforms;
//
//	uint32_t numBones = std::min(modelPose->NumXforms, localPose->NumXforms);
//
//	for (uint16_t i = 0; i < numBones; ++i)
//	{
//		const int16_t idxParent = parentIndices[i];
//		if (idxParent >= 0)
//		{
//			// model-parent * local = model
//			modelXforms[i] = fm_xform_mul(modelXforms[idxParent], localXforms[i]);
//		}
//		else
//		{
//			modelXforms[i] = localXforms[i];
//		}
//	}
//
//	uint32_t numTracks = std::min(modelPose->NumTracks, localPose->NumTracks);
//	if (numTracks > 0U)
//	{
//		memcpy(modelPose->Tracks, localPose->Tracks, sizeof(float) * numTracks);
//	}
//}
//
//void AnimClipSample(const AnimClip* clip, float time, Pose* pose)
//{
//    // iterate each curve (bone) 
//    const uint32_t numCurves = clip->NumCurves;
//
//    for (uint32_t i = 0; i < numCurves; ++i)
//    {
//        const AnimCurve* curve = &clip->Curves[i];
//
//        const uint16_t idxXform = curve->Index;
//
//        // rotation
//        {
//            const uint16_t numKeys = curve->NumKeys;
//            uint16_t idx = 0;
//
//            // find upper index by time (this could be a binary search)
//            while (idx < (numKeys - 1) && DecompressKeyTime(curve->Keys[idx].KeyTime) < time)
//            {
//                ++idx;
//            }
//
//            const uint16_t upperIdx = idx;
//            const uint16_t lowerIdx = idx == 0 ? idx : idx - 1;
//
//            glm::quat rot;
//
//            if (lowerIdx == upperIdx)  // constant key 
//            {
//				rot = DecompressRotationKey(curve->Keys[idx]);
//            }
//            else  // at least two keys - interpolate 
//            {
//                glm::quat rot1 = DecompressRotationKey(curve->Keys[lowerIdx]);
//
//				glm::quat rot2 = DecompressRotationKey(curve->Keys[upperIdx]);
//
//                const float time1 = DecompressKeyTime(curve->Keys[lowerIdx].KeyTime);
//                const float time2 = DecompressKeyTime(curve->Keys[upperIdx].KeyTime);
//
//                float alpha = (time - time1) / (time2 - time1);
//				rot = glm::lerp(rot1, rot2, alpha);
//
//                // because we do LERP on quaternion (faster on CPU), we have to normalise it 
//				rot = glm::normalize(rot);
//            }
//
//            pose->Xforms[idxXform].Rotation = rot;
//        }
//
//        // position
//        {
//            const uint16_t numKeys = curve->NumPosKeys;
//            uint16_t idx = 0;
//
//            // find upper index by time (this could be a binary search)
//            while (idx < (numKeys - 1) && DecompressKeyTime(curve->PosKeys[idx].KeyTime) < time)
//            {
//                ++idx;
//            }
//
//            const uint16_t upperIdx = idx;
//            const uint16_t lowerIdx = idx == 0 ? idx : idx - 1;
//
//            glm::vec4 pos;
//
//            if (lowerIdx == upperIdx)  // constant key 
//            {
//				pos = DecompressPositionKey(curve->PosKeys[idx]);
//            }
//            else  // at least two keys - interpolate 
//            {
//                glm::vec4 pos1 = DecompressPositionKey(curve->PosKeys[lowerIdx]);
//
//                glm::vec4 pos2 = DecompressPositionKey(curve->PosKeys[upperIdx]);
//
//                const float time1 = DecompressKeyTime(curve->PosKeys[lowerIdx].KeyTime);
//                const float time2 = DecompressKeyTime(curve->PosKeys[upperIdx].KeyTime);
//
//                float alpha = (time - time1) / (time2 - time1);
//				pos = glm::lerp(pos2, pos1, alpha);
//				
//            }
//
//            pose->Xforms[idxXform].Position = pos;
//        }
//    }
//}