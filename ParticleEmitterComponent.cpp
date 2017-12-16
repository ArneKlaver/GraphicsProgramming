//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleEmitterComponent.h"
#include "../Helpers/EffectHelper.h"
#include "../Content/ContentManager.h"
#include "../Content/TextureDataLoader.h"
#include "../Graphics/Particle.h"
#include "../Components/TransformComponent.h"
#include "../Diagnostics/Logger.h"


ParticleEmitterComponent::ParticleEmitterComponent(const wstring& assetFile, int particleCount) :
	m_ParticleCount(particleCount),
	m_AssetFile(assetFile)
{
	for (size_t i = 0; i < particleCount; i++)
	{
		if (m_UseRandomRange)
		{
			XMFLOAT3 randomVec = XMFLOAT3(randF(m_VelocityRangeMin.x, m_VelocityRangeMax.x), randF(m_VelocityRangeMin.y, m_VelocityRangeMax.y), randF(m_VelocityRangeMin.z, m_VelocityRangeMax.z));
			m_Settings.Velocity = randomVec;
		}

		m_Particles.push_back(new Particle(m_Settings));
	}
}


ParticleEmitterComponent::~ParticleEmitterComponent(void)
{
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		SafeDelete(m_Particles[i]);
	}
	m_Particles.clear();
	SafeRelease(m_pInputLayout);
	SafeRelease(m_pVertexBuffer);
}

void ParticleEmitterComponent::Initialize(const GameContext& gameContext)
{
	LoadEffect(gameContext);
	CreateVertexBuffer(gameContext);
	m_pParticleTexture = ContentManager::Load<TextureData>(m_AssetFile);
}

void ParticleEmitterComponent::LoadEffect(const GameContext& gameContext)
{
	m_pEffect = ContentManager::Load<ID3DX11Effect>(L"./Resources/Effects/ParticleRenderer.fx");
	m_pDefaultTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pWvpVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pWvpVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect > \'m_pWvpVariable\' variable not found!");
		m_pWvpVariable = nullptr;
	}

	m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pWvpVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect > \'m_pViewInverseVariable\' variable not found!");
		m_pViewInverseVariable = nullptr;
	}

	m_pTextureVariable = m_pEffect->GetVariableByName("gParticleTexture")->AsShaderResource();
	if (!m_pWvpVariable->IsValid())
	{
		Logger::LogWarning(L"ParticleEmitterComponent::LoadEffect > \'m_pTextureVariable\' variable not found!");
		m_pTextureVariable = nullptr;
	}

	EffectHelper::BuildInputLayout(gameContext.pDevice, m_pDefaultTechnique, &m_pInputLayout , m_pInputLayoutSize);
}

void ParticleEmitterComponent::CreateVertexBuffer(const GameContext& gameContext)
{
	if (m_pVertexBuffer) { SafeRelease(m_pVertexBuffer); }

	//Vertexbuffer
	D3D11_BUFFER_DESC bd = {};
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ParticleVertex) * m_ParticleCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	//initData.pSysMem = m_Particles.data(); //TODO: might be wrong
	HRESULT hr = gameContext.pDevice->CreateBuffer(&bd, nullptr, &m_pVertexBuffer);
	Logger::LogHResult(hr, L"Failed to Create Vertexbuffer");
}

void ParticleEmitterComponent::Update(const GameContext& gameContext)
{
	m_AliveTime += gameContext.pGameTime->GetElapsed();
	if (m_AliveTime > m_LifeTime)
	{
		m_Destroy = true;
	}
	//
	float particleInterval = ((m_Settings.MaxEnergy + m_Settings.MinEnergy) / 2) / m_ParticleCount ;
	m_LastParticleInit += gameContext.pGameTime->GetElapsed();
	m_ActiveParticles = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// map the vertexBuffer
	gameContext.pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParticleVertex* pBuffer = (ParticleVertex*)mappedResource.pData;
	int index = 0;
	for (Particle* tempParticle : m_Particles)
	{
		tempParticle->Update(gameContext);
		if (tempParticle->IsActive())
		{
			pBuffer[m_ActiveParticles] = tempParticle->GetVertexInfo();
			++m_ActiveParticles;
		}
		else if(m_Burst && !m_BurstHasBeenActivated)
		{
			m_BurstHasBeenActivated = true;
			if ( index + m_BurstAmount > m_Particles.size())
			{
				Logger::LogWarning(L"use less burst or more max particles");
				m_BurstAmount = m_Particles.size() -1 - index;
			}
			for (int i = 0; i < 10; i++)
			{
				m_Particles[index + i]->Init(GetTransform()->GetWorldPosition());
				pBuffer[m_ActiveParticles] = m_Particles[index + i]->GetVertexInfo();
				++m_ActiveParticles;
				m_LastParticleInit = 0.0f;
			}			
		}
		else if (!m_Burst && m_LastParticleInit >= particleInterval)
		{
			tempParticle->Init(GetTransform()->GetWorldPosition());
			pBuffer[m_ActiveParticles] = tempParticle->GetVertexInfo();
			++m_ActiveParticles;
			m_LastParticleInit = 0.0f;
		}
		index++;
	}
	gameContext.pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void ParticleEmitterComponent::Draw(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
}

void ParticleEmitterComponent::PostDraw(const GameContext& gameContext)
{
	XMFLOAT4X4 viewProjection = gameContext.pCamera->GetViewProjection();
	m_pWvpVariable->SetMatrix(reinterpret_cast<float*>(&viewProjection));
	XMFLOAT4X4 viewInverse = gameContext.pCamera->GetViewInverse();
	m_pViewInverseVariable->SetMatrix(reinterpret_cast<float*>(&viewInverse));
	m_pTextureVariable->SetResource(m_pParticleTexture->GetShaderResourceView());

	gameContext.pDeviceContext->IASetInputLayout(m_pInputLayout);

	gameContext.pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	//set vertex buffer
	UINT offset = 0;
	UINT stride = sizeof(ParticleVertex);
	gameContext.pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	m_pDefaultTechnique->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		m_pDefaultTechnique->GetPassByIndex(i)->Apply(0, gameContext.pDeviceContext);

		gameContext.pDeviceContext->Draw(m_ActiveParticles, 0);
	}
}
