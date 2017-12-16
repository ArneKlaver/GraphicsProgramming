#pragma once
#include "BaseComponent.h"


struct ParticleEmitterSettings
{
public:

	ParticleEmitterSettings() :
		MinSize(0.1f),
		MaxSize(2.0f),
		MinEnergy(1.0f),
		MaxEnergy(2.0f),
		Velocity(XMFLOAT3(0, 0, 0)),
		MaxEmitterRange(10.0f),
		MinEmitterRange(9.0f),
		MinSizeGrow(1.0f),
		MaxSizeGrow(1.0f),
		Color((XMFLOAT4)Colors::White) {}

	//The minimum size each particle can be at the time when it is spawned
	float MinSize;
	//The maximum size each particle can be at the time when it is spawned
	float MaxSize;
	//The minimum lifetime of each particle, measured in seconds
	float MinEnergy;
	//The maximum lifetime of each particle, measured in seconds
	float MaxEnergy;
	//The starting speed of particles along X, Y and Z, measured in the object's orientation
	XMFLOAT3 Velocity;
	//The maximum radius that the particles are spawned in
	float MaxEmitterRange;
	//The minimum radius that the particles are spawned in
	float MinEmitterRange;
	//The percentual maximum change in size during the particle's lifetime
	float MinSizeGrow;
	//The percentual minimum change in size during the particle's lifetime
	float MaxSizeGrow;
	//The color of a particle
	XMFLOAT4 Color;

};

struct ParticleVertex
{
public:

	ParticleVertex(XMFLOAT3 pos = XMFLOAT3(0, 0, 0), XMFLOAT4 col = (XMFLOAT4)Colors::White, float size = 5.0f, float rotation = 0) :
		Position(pos),
		Color(col),
		Size(size),
		Rotation(rotation) {}

	XMFLOAT3 Position;
	XMFLOAT4 Color;
	float Size;
	float Rotation;
};

class Particle;
class TextureData;

class ParticleEmitterComponent : public BaseComponent
{
public:
	//Constructor, particle texture path and max particles
	ParticleEmitterComponent(const wstring& assetFile, int particleCount = 50);
	~ParticleEmitterComponent(void);

	//EmitterSettings SETTERS
	void SetMinSize(float minSize) { m_Settings.MinSize = minSize; }
	void SetMaxSize(float maxSize) { m_Settings.MaxSize = maxSize; }
	void SetMinEnergy(float minEnergy) { m_Settings.MinEnergy = minEnergy; }
	void SetMaxEnergy(float maxEnergy) { m_Settings.MaxEnergy = maxEnergy; }
	void SetVelocity(XMFLOAT3 velocity) { m_Settings.Velocity = velocity; }
	void SetMinSizeGrow(float minSizeGrow) { m_Settings.MinSizeGrow = minSizeGrow; }
	void SetMaxSizeGrow(float maxSizeGrow) { m_Settings.MaxSizeGrow = maxSizeGrow; }
	void SetMinEmitterRange(float minEmitterRange) { m_Settings.MinEmitterRange = minEmitterRange; }
	void SetMaxEmitterRange(float maxEmitterRange) { m_Settings.MaxEmitterRange = maxEmitterRange; }
	void SetColor(XMFLOAT4 color) { m_Settings.Color = color; }

	//custom 
	void SetRandomVelocity(XMFLOAT3 minVel, XMFLOAT3 maxVel) { m_VelocityRangeMin = minVel; m_VelocityRangeMax = maxVel; m_UseRandomRange = true; }
	bool GetDestroy() { return m_Destroy; }
protected:
	virtual void Initialize(const GameContext& gameContext);
	virtual void Update(const GameContext& gameContext);
	virtual void Draw(const GameContext& gameContext);
	virtual void PostDraw(const GameContext& gameContext);

private:

	//Method to load effect-related stuff
	void LoadEffect(const GameContext& gameContext);
	//Method to create the vertex buffer
	void CreateVertexBuffer(const GameContext& gameContext);

	//The actual vertex buffer, containing ParticleVertex information for each Particle
	ID3D11Buffer* m_pVertexBuffer = nullptr;

	//The effect we'll use to render the particles
	ID3DX11Effect *m_pEffect = nullptr;
	//The default technique
	ID3DX11EffectTechnique *m_pDefaultTechnique = nullptr;
	//Shader matrix variables
	ID3DX11EffectMatrixVariable *m_pWvpVariable, *m_pViewInverseVariable = nullptr;
	//Shader texture variable
	ID3DX11EffectShaderResourceVariable *m_pTextureVariable = nullptr;
	//ShaderResourceView, containing the particle texture
	TextureData *m_pParticleTexture = nullptr;

	//Vertex input layout variables
	ID3D11InputLayout *m_pInputLayout = nullptr;
	UINT m_pInputLayoutSize = 0;

	//Vector of particle pointers
	vector<Particle*> m_Particles;
	//The settings for this particle system
	ParticleEmitterSettings m_Settings = ParticleEmitterSettings();
	//The total amount of particles (m_Particles.size() == m_ParticleCount)
	int m_ParticleCount = 0;
	//The active particles for a certain frame
	int m_ActiveParticles = 0;
	//Total seconds since the last particle initialisation
	float m_LastParticleInit = 0.0f;
	//string containing the path to the particle texture
	wstring m_AssetFile;

	//******
	//emiter settings
	//******
	XMFLOAT3 m_VelocityRangeMin, m_VelocityRangeMax;
	bool m_UseRandomRange = false;
		//burst
		bool m_Burst = true;
		int m_BurstAmount = 50;
		bool m_BurstHasBeenActivated = false;
	//dead
		float m_LifeTime = 1;
		float m_AliveTime = 0;
		bool m_Destroy = false;
private:
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	ParticleEmitterComponent(const ParticleEmitterComponent& yRef);
	ParticleEmitterComponent& operator=(const ParticleEmitterComponent& yRef);
};

