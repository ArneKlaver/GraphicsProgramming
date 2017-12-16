//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

#include "ParticleTestScene.h"
#include "Scenegraph\GameObject.h"
#include "Components\TransformComponent.h"
#include "Components\ParticleEmitterComponent.h"
#include "Components\ModelComponent.h"
#include <Materials/ColorMaterial.h>
#include <Diagnostics/DebugRenderer.h>

ParticleTestScene::ParticleTestScene(void) :
	GameScene(L"ParticleTestScene")
{
}


ParticleTestScene::~ParticleTestScene(void)
{
}

void ParticleTestScene::Initialize(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	DebugRenderer::ToggleDebugRenderer();

	//PARTICLE-EMITTER
	//****************

	auto pObj = new GameObject();

	m_pParticleEmitter = new ParticleEmitterComponent(L"./Resources/Textures/Smoke.png", 60);
	m_pParticleEmitter->SetRandomVelocity(XMFLOAT3(-5, -5, -5), XMFLOAT3(5, 5, 5));
	m_pParticleEmitter->SetMinSize(1.0f);
	m_pParticleEmitter->SetMaxSize(3.0f);
	m_pParticleEmitter->SetMinEnergy(1.0f);
	m_pParticleEmitter->SetMaxEnergy(2.0f);
	m_pParticleEmitter->SetMinSizeGrow(3.5f);
	m_pParticleEmitter->SetMaxSizeGrow(6.5f);
	m_pParticleEmitter->SetMinEmitterRange(3.0f);
	m_pParticleEmitter->SetMaxEmitterRange(5.0f);
	m_pParticleEmitter->SetColor(XMFLOAT4(1.f, 1.f, 1.f, 0.6f));
	pObj->AddComponent(m_pParticleEmitter);
	AddChild(pObj);
	pObj->GetTransform()->Translate(10, 10, 0);
	auto pMat = new ColorMaterial();
	gameContext.pMaterialManager->AddMaterial(pMat, 0);

	auto pMat2 = new ColorMaterial();
	gameContext.pMaterialManager->AddMaterial(pMat2, 1);
	//pMat2->SetColor(XMFLOAT4(Colors::Red));
	
	//Teapot
	//******
	m_pTeapot = new GameObject();
	m_pModel = new ModelComponent(L"./Resources/Meshes/Teapot.ovm");
	m_pModel->SetMaterial(1);
	m_pTeapot->AddComponent(m_pModel);
	AddChild(m_pTeapot);

	m_pTeapot->GetTransform()->Scale(.3f, .3f, .3f);


	//Ground Mesh
	//***********
	auto pGroundObj = new GameObject();
	auto pGroundModel = new ModelComponent(L"./Resources/Meshes/UnitPlane.ovm");
	pGroundModel->SetMaterial(0);

	pGroundObj->AddComponent(pGroundModel);
	pGroundObj->GetTransform()->Scale(10.0f, 10.0f, 10.0f);

	AddChild(pGroundObj);

	gameContext.pInput->AddInputAction(InputAction(0, Down, VK_RIGHT));
	gameContext.pInput->AddInputAction(InputAction(1, Down, VK_LEFT));
	gameContext.pInput->AddInputAction(InputAction(2, Down, VK_UP));
	gameContext.pInput->AddInputAction(InputAction(3, Down, VK_DOWN));
}

void ParticleTestScene::Update(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);

	auto initPos = m_pParticleEmitter->GetTransform()->GetPosition();
	auto moveSpeed = 10.0f * gameContext.pGameTime->GetElapsed();
	initPos.x += moveSpeed * sin(gameContext.pGameTime->GetTotal());

	if (gameContext.pInput->IsActionTriggered(0))
	{
		initPos.x += moveSpeed;
	}
	if (gameContext.pInput->IsActionTriggered(1))
	{
		initPos.x -= moveSpeed;
	}
	if (gameContext.pInput->IsActionTriggered(2))
	{
		initPos.z += moveSpeed;
	}
	if (gameContext.pInput->IsActionTriggered(3))
	{
		initPos.z -= moveSpeed;
	}

	m_pParticleEmitter->GetTransform()->Translate(initPos);
}

void ParticleTestScene::Draw(const GameContext& gameContext)
{
	UNREFERENCED_PARAMETER(gameContext);
}
