#include "castaura.h"

#define AURA_RADIUS	0.6f
#define BASE_VELOCITY 0.02f
#define ORBIT_VELOCITY 0.04f

BOOL CastAura::AdvanceFrame()
{
	Frame ++;
	D3DVECTOR vAdd;
	D3DVECTOR Velocity(0.0f, 0.0f, ORBIT_VELOCITY);

	float x;
	float y;
	
	if(Frame < AuraDuration)
	{
		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x,y,0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraOne.AddParticle(&vAdd,&Velocity);


		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x,y,0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraTwo.AddParticle(&vAdd,&Velocity);

		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x,y,0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraThree.AddParticle(&vAdd,&Velocity);
	}

	AuraOne.AdvanceFrame();
	AuraTwo.AdvanceFrame();	
	return AuraThree.AdvanceFrame();
}

void CastAura::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	AuraOne.Draw();
	AuraTwo.Draw();
	AuraThree.Draw();


}

void CastAura::AdjustCamera()
{
	AuraOne.AdjustCamera();
	AuraTwo.AdjustCamera();
	AuraThree.AdjustCamera();
}

CastAura::CastAura(Thing *pTarget, int Duration, D3DMATERIAL7 *Color1, D3DMATERIAL7 *Color2, D3DMATERIAL7 *Color3, char *ParticleName)
{
	AuraOne.SetLife(Duration/2);
	AuraOne.SetLifeVariance(Duration/5);
	AuraOne.SetInitialVector(D3DVECTOR(0.0f,0.0f, ORBIT_VELOCITY));
	AuraOne.SetRotation(D3DVECTOR(0.0f,0.0f, ORBIT_VELOCITY));
	AuraOne.SetMaterial(Color1);
	AuraOne.SetRadius(0.3f);
	AuraOne.SetLink(pTarget);
	AuraOne.AdjustCamera();
	AuraOne.SetEmissionQuantity(0);
	
	AuraTwo.SetLife(Duration/2);
	AuraTwo.SetLifeVariance(Duration/5);
	AuraTwo.SetInitialVector(D3DVECTOR(0.0f,0.0f,ORBIT_VELOCITY));
	AuraTwo.SetRotation(D3DVECTOR(0.0f,0.0f,ORBIT_VELOCITY));
	AuraTwo.SetMaterial(Color2);
	AuraTwo.SetRadius(0.3f);
	AuraTwo.SetLink(pTarget);
	AuraTwo.AdjustCamera();
	AuraTwo.SetEmissionQuantity(0);
	
	AuraThree.SetLife(Duration/2);
	AuraThree.SetLifeVariance(Duration/5);
	AuraThree.SetInitialVector(D3DVECTOR(0.0f,0.0f, ORBIT_VELOCITY));
	AuraThree.SetRotation(D3DVECTOR(0.0f,0.0f, ORBIT_VELOCITY));
	AuraThree.SetMaterial(Color3);
	AuraThree.SetRadius(0.3f);
	AuraThree.AdjustCamera();
	AuraThree.SetLink(pTarget);
	AuraThree.SetEnd(Duration);
	AuraThree.SetEmissionQuantity(0);
	
	D3DVECTOR vAdd;
	D3DVECTOR Velocity(0.0f,0.0f,ORBIT_VELOCITY);

	float x;
	float y;
	int n;

	for(n = 0; n < 8; n++)
	{
		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x, y, 0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraOne.AddParticle(&vAdd,&Velocity);
	}


	for(n = 0; n < 8; n++)
	{
		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x, y, 0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraTwo.AddParticle(&vAdd,&Velocity);
	}

	for(n = 0; n < 8; n++)
	{
		x = (float)rand() / (float)RAND_MAX - 0.5f;
		y = (float)rand() / (float)RAND_MAX - 0.5f;

		vAdd = Normalize(D3DVECTOR(x, y, 0.0f));

		vAdd.x *= AURA_RADIUS;
		vAdd.y *= AURA_RADIUS;
		AuraThree.AddParticle(&vAdd, &Velocity);
	}

	pTexture = Engine->GetTexture(ParticleName);
	Frame = 0;
	AuraDuration = Duration;

}
