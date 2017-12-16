#include "stdafx.h"

#include "ModelAnimator.h"
#include "../Diagnostics/Logger.h"


ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
m_pMeshFilter(pMeshFilter),
m_Transforms(vector<XMFLOAT4X4>()),
m_IsPlaying(false), 
m_Reversed(false),
m_ClipSet(false),
m_TickCount(0),
m_AnimationSpeed(1.0f)
{
	SetAnimation(0);
}


ModelAnimator::~ModelAnimator()
{
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (clipNumber < GetClipCount())
	{
		SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber]);		
	}
	else
	{
		//If not,
		//	Call Reset
		//	return
		Reset();
		Logger::LogWarning(L"ModelAnimator::SetAnimation(UINT clipNumber) > \'clipNumber\' not smaller then clipCount!");
		return;
	}
}

void ModelAnimator::SetAnimation(wstring clipName)
{
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	for (auto tempClip :m_pMeshFilter->m_AnimationClips)
	{
		if (tempClip.Name == clipName)
		{
			SetAnimation(tempClip);
			return;
		}
	}
	//	Call Reset
	Reset();
	Logger::LogWarning(L"ModelAnimator::SetAnimation(wstring clipName) > No clip fount with clipName");	
}

void ModelAnimator::SetAnimation(AnimationClip clip)
{
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	//Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//If pause is true, set m_IsPlaying to false
	if (pause) {m_IsPlaying = false; }
	//Set m_TickCount to zero
	m_TickCount = 0;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.0f;
	//If m_ClipSet is true
	if (m_ClipSet)
	{	
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(m_CurrentClip.Keys[0].BoneTransforms.begin() , m_CurrentClip.Keys[0].BoneTransforms.end());
	}
	else
	{
		//	Create an IdentityMatrix 
		//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		XMMATRIX identity = XMMatrixIdentity();
		XMFLOAT4X4 identityMatrix;
		XMStoreFloat4x4(&identityMatrix, identity);
		m_Transforms.assign(m_CurrentClip.Keys.size(), identityMatrix);
	}
}

void ModelAnimator::Update(const GameContext& gameContext)
{
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//Calculate the passedTicks
		auto passedTicks = gameContext.pGameTime->GetElapsed() * m_CurrentClip.TicksPerSecond * m_AnimationSpeed;
		passedTicks = fmod(passedTicks, m_CurrentClip.Duration);
		//IF m_Reversed is true
		//	Subtract passedTicks from m_TickCount
		//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
		if (m_Reversed)
		{
			m_TickCount -= passedTicks;
			if (m_TickCount < 0)
			{
				m_TickCount += m_CurrentClip.Duration;
			}
		}
		else
		{
			//	Add passedTicks to m_TickCount
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			m_TickCount += passedTicks;
			if (m_TickCount > m_CurrentClip.Duration)
			{
				m_TickCount -= m_CurrentClip.Duration;
			}
		}
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for (unsigned int i = 0 ; i < m_CurrentClip.Keys.size(); i++)
		{
			if (m_CurrentClip.Keys[i].Tick > m_TickCount)
			{
				keyA = m_CurrentClip.Keys[i-1];
				if (!(m_CurrentClip.Keys[i-1].Tick < m_TickCount))
				{
					Logger::LogWarning(L"ModelAnimator::Update(const GameContext& gameContext) > fout keyB");
				}
				keyB = m_CurrentClip.Keys[i];
				break;
			}
		}
		//Interpolate between keys
		//Figure out the BlendFactor

		auto blendFactor = (m_TickCount - keyA.Tick) / (keyB.Tick - keyA.Tick);

		//Clear the m_Transforms vector
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (size_t i = 0; i < keyA.BoneTransforms.size(); i++)
		{
			XMFLOAT4X4 transformA = keyA.BoneTransforms[i];
			XMFLOAT4X4 transformB = keyB.BoneTransforms[i];

			//	Retrieve the transform from keyA (transformA)
			//	Decompose both transforms
			auto matrixA = FXMMATRIX(XMLoadFloat4x4(&transformA));
			auto matrixB = FXMMATRIX(XMLoadFloat4x4(&transformB));
			XMVECTOR  scaleA;
			XMVECTOR  rotA;
			XMVECTOR  transA;
			XMMatrixDecompose(&scaleA, &rotA, &transA, matrixA);
			// 	Retrieve the transform from keyB (transformB)
			XMVECTOR  scaleB ;
			XMVECTOR  rotB;
			XMVECTOR  transB ;
			XMMatrixDecompose(&scaleB,&rotB, &transB, matrixB);

			//	Lerp between all the transformations (Position, Scale, Rotation)
			XMVECTOR scale = XMVectorLerp(scaleA, scaleB, blendFactor);
			XMVECTOR trans = XMVectorLerp(transA, transB, blendFactor);
			XMVECTOR rot = XMQuaternionSlerp(rotA, rotB, blendFactor);

			//	Compose a transformation matrix with the lerp-results

			XMMATRIX matScale = XMMatrixScalingFromVector(scale);
			XMMATRIX matTrans = XMMatrixTranslationFromVector(trans);
			XMMATRIX matRot = XMMatrixRotationQuaternion(rot);
			XMMATRIX test =  matScale * matRot * matTrans;

			XMFLOAT4X4 result ;
			XMStoreFloat4x4(&result, test);

			//	Add the resulting matrix to the m_Transforms vector
			m_Transforms.push_back(result);
		}
	}
}
