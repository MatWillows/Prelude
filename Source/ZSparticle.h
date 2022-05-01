#ifndef MYPARTICLE_H
#define MYPARTICLE_H

#include "ZSEngine.h"
#include "things.h"
#include "objects.h"

#define MAX_PARTICLES	128

typedef enum
{
	PARTICLE_SPHERE,
	PARTICLE_BUBBLE,
	PARTICLE_STAR,
	PARTICLE_FLAME,
	PARTICLE_VLINE,
	PARTICLE_HLINE,
	PARTICLE_CIRCLE,
} PARTICLE_T;

class ParticleSystem : public Object
{
private:

	Object *pLink;
	int LinkPoint;

	unsigned short ParticleDrawList[MAX_PARTICLES * 3];
	int DrawLength;

	D3DVERTEX Particles[MAX_PARTICLES * 3];

	D3DVECTOR Velocity[MAX_PARTICLES];
	
	D3DVECTOR Position[MAX_PARTICLES];

	D3DVECTOR ParticleOrigin[MAX_PARTICLES];

	D3DBLEND SourceBlend;
	D3DBLEND DestBlend;
		
	int LifeFrame[MAX_PARTICLES];

	int LifeLength[MAX_PARTICLES];

	int NumParticles;
	int DrawParticles;
	float Expansion;

	int Life;
	int LifeVariance;

	PARTICLE_T	ParticleType;
	 
	float PRadius;

	D3DVECTOR vGravity;
	D3DVECTOR vRotation;
	D3DVECTOR vWind;
	D3DVECTOR vOrigin;

	D3DVECTOR vDestination;
	D3DVECTOR vMotion;

	D3DVECTOR vOrbit;

	float Pull;

	D3DTEXTUREOP AlphaOp;
	D3DMATERIAL7 Material;
	D3DVECTOR vAdd;
	D3DVECTOR vInitial;
	D3DVECTOR vInitialVariance;

	D3DVECTOR CameraFactorA;
	D3DVECTOR CameraFactorB;

	int FrameRate;
	int SubFrame;
	int Start;
	int End;

	int EmissionRate;
	int EmissionFrame;
	int EmissionQuantity;

	BOOL Orbitting;
	BOOL Pulling;

	void FillVertices();

public:

//accessors
	Object *GetLink() { return pLink; };
	int GetNumParticles() { return NumParticles; }
	D3DVECTOR GetGravity() { return vGravity; }
	D3DVECTOR GetRotation() { return vRotation; }
	float GetExpansion() { return Expansion; }
	D3DVECTOR GetWind() { return vWind; }
	D3DVECTOR GetOrigin() { return vOrigin; }
	D3DTEXTUREOP GetAlphaOp() { return AlphaOp; }
	D3DMATERIAL7 GetMaterial() { return Material; }
	D3DVECTOR GetAddVector() { return vAdd; }
	D3DVECTOR GetInitialVector() { return vInitial; }
	int GetFrameRate() { return FrameRate; }
	int GetSubFrame() { return SubFrame; }
	int GetStart() { return Start; }
	int GetEnd() { return End; }
	D3DVECTOR GetVariance() { return vInitialVariance; }
	float GetRadius() { return PRadius; }
	PARTICLE_T	GetParticleType() { return ParticleType; }
	int GetEmissionRate() { return EmissionRate; }
	int GetEmissionQuantity() { return EmissionQuantity; }
	D3DBLEND GetSourceBlend() { return SourceBlend; }
	D3DBLEND GetDestBlend()	{ return DestBlend; }
	int GetLinkPoint() { return LinkPoint; }
	D3DVECTOR GetDestination() { return vDestination;}
	D3DVECTOR GetMotion() { return vMotion; }
	D3DVECTOR GetOrbit() { return vOrbit; }
	float GetPull() { return Pull; }
	BOOL IsOrbitting() { return Orbitting; }
	BOOL IsPulling() { return Pulling; }

	OBJECT_T GetObjectType() { return OBJECT_PARTICLESYSTEM; }

//mutators
	void SetLink(Object *NewLink) { pLink = NewLink; }
	void SetGravity(D3DVECTOR NewGrav) { vGravity = NewGrav; }
	void SetRotation(D3DVECTOR NewRotation) { vRotation = NewRotation; }
	void SetExpansion(float NewExpansion) { Expansion = NewExpansion; }
	void SetWind(D3DVECTOR NewWind) { vWind = NewWind; }
	void SetOrigin(D3DVECTOR NewOrigin) { vOrigin = NewOrigin; }
	void SetOp(D3DTEXTUREOP NewOp) { AlphaOp = NewOp; }
	void SetMaterial(D3DMATERIAL7 *NewMaterial) { Material = *NewMaterial; }
	void SetFrameRate(int NewRate) { FrameRate = NewRate; }
	void SetStart(int NewStart) { Start = NewStart; }
	void SetEnd(int NewEnd) { End = NewEnd; }
	void SetAddVector(D3DVECTOR NewAdd) { vAdd = NewAdd; }
	void SetInitialVector(D3DVECTOR NewInitial) { vInitial = NewInitial; }
	void SetInitialVariance(D3DVECTOR NewInitial) { vInitialVariance = NewInitial; }
	void SetLife(int NewLife) { Life = NewLife; }
	void SetLifeVariance(int NewVar) { LifeVariance = NewVar; }
	void SetRadius(float NewRad) { PRadius = NewRad; }
	void SetParticleType(PARTICLE_T NewType);
	void SetEmissionRate(int NewEm) { EmissionRate = NewEm; }
	void SetEmissionQuantity(int NewQuan) { EmissionQuantity = NewQuan; }
	void SetSourceBlend(D3DBLEND NewBlend) { SourceBlend = NewBlend; }
	void SetDestBlend(D3DBLEND NewBlend) { DestBlend = NewBlend; }
	void SetLinkPoint(int NewPoint) { LinkPoint = NewPoint; }
	void SetDestination(D3DVECTOR vNew) { vDestination = vNew; }
	void SetMotion(D3DVECTOR vNew) { vMotion = vNew; }
	void SetOrbit(D3DVECTOR vNew) { vOrbit = vNew; }
	void SetPull(float NewPull) { Pull = NewPull; }
	void SetOrbitting(BOOL NewOrbit) { Orbitting = NewOrbit; }
	void SetPulling(BOOL NewPulling) { Pulling = NewPulling;  }


	BOOL AdvanceFrame();
	
	void Draw();

	void AdjustCamera();

	int AddParticle(D3DVECTOR *pvPosition, D3DVECTOR *pvVelocity);
	int AddParticle(D3DVECTOR *pvVelocity);
	int AddParticle();

	void Move(D3DVECTOR *vDirection);

//constructor
	ParticleSystem();
	ParticleSystem(ParticleSystem *pFrom);

//destructor
	~ParticleSystem();

//debuggin
	BOOL TestDraw();

//i/o
	void Load(FILE *fp);
	void Save(FILE *fp);
	
//intersections
	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

};















#endif

