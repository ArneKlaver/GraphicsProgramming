#pragma once
#include "Scenegraph/GameScene.h"

class ParticleEmitterComponent;
class ModelComponent;

class ParticleTestScene : public GameScene
{
public:
	ParticleTestScene(void);
	virtual ~ParticleTestScene(void);

protected:

	void Initialize(const GameContext& gameContext) override;
	void Update(const GameContext& gameContext) override;
	void Draw(const GameContext& gameContext) override;

private:

	ParticleEmitterComponent* m_pParticleEmitter = nullptr;
	ModelComponent* m_pModel = nullptr;
	GameObject* m_pTeapot = nullptr;

private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ParticleTestScene(const ParticleTestScene &obj);
	ParticleTestScene& operator=(const ParticleTestScene& obj);
};

