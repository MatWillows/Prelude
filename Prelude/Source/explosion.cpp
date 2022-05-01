#include "explosion.h"
#include "world.h"
#include "zsfire.h"

#define INIT_INNER	20.0f
#define INIT_MID		18.0f
#define INIT_OUTER	16.0f
#define NUM_FACTOR	10
#define FADE_FACTOR	0.85f



Explosion::Explosion(D3DVECTOR Location, float Size, int MinDam, int MaxDam, Thing *pThingSource)
{
	pSource = pThingSource;
	MinDamage = MinDam;
	MaxDamage = MaxDam;
	vCenter = Location;
	Radius = Size;


	Red = *Engine->Graphics()->GetMaterial(COLOR_RED);
	Yellow = *Engine->Graphics()->GetMaterial(COLOR_YELLOW);
	Orange = *Engine->Graphics()->GetMaterial(COLOR_ORANGE);

	Red.diffuse.a = 1.0f;
	Yellow.diffuse.a = 1.0f;
	Orange.diffuse.a = 1.0f;


	A.SetOrigin(Location);
	A.SetInitialVector(D3DVECTOR(Size/INIT_INNER,Size/INIT_INNER,Size/INIT_INNER));
	A.SetInitialVariance(D3DVECTOR(Size/(INIT_INNER / 2.0f),Size/(INIT_INNER / 2.0f),Size/(INIT_INNER / 2.0f)));
	A.SetOp(D3DTOP_ADD);
	A.SetMaterial(&Yellow);
	A.SetLife(600);
	A.SetLifeVariance(0);
	A.SetParticleType(PARTICLE_SPHERE);
	A.SetRadius(Size/2.0f);
	A.SetEmissionQuantity(1);
	A.SetSourceBlend(D3DBLEND_SRCALPHA);
//	A.SetDestBlend(D3DBLEND_ONE);
	A.AdjustCamera();	
	A.AdvanceFrame();
	
	B.SetOrigin(Location);
	B.SetInitialVector(D3DVECTOR(Size/INIT_MID,Size/INIT_MID,Size/INIT_MID));
	B.SetInitialVariance(D3DVECTOR(Size/(INIT_MID / 2.0f),Size/(INIT_MID / 2.0f),Size/(INIT_MID / 2.0f)));
	B.SetOp(D3DTOP_ADD);
	B.SetMaterial(&Orange);
	B.SetLife(600);
	B.SetLifeVariance(0);
	B.SetParticleType(PARTICLE_SPHERE);
	B.SetRadius(Size/2.0f);
	B.SetEmissionQuantity(0);
	B.SetSourceBlend(D3DBLEND_SRCALPHA);
//	B.SetDestBlend(D3DBLEND_ONE);
	B.AdjustCamera();	
	B.AdvanceFrame();
	
	C.SetOrigin(Location);
	C.SetInitialVector(D3DVECTOR(Size/INIT_OUTER,Size/INIT_OUTER,Size/INIT_OUTER));
	C.SetInitialVariance(D3DVECTOR(Size/(INIT_OUTER / 2.0f),Size/(INIT_OUTER / 2.0f),Size/(INIT_OUTER / 2.0f)));
	C.SetOp(D3DTOP_ADD);
	C.SetMaterial(&Red);
	C.SetLife(600);
	C.SetLifeVariance(0);
	C.SetParticleType(PARTICLE_SPHERE);
	C.SetRadius(Size/2.0f);
	C.SetEmissionQuantity(0);
	C.SetEnd(12);
	C.SetSourceBlend(D3DBLEND_SRCALPHA);
//	C.SetDestBlend(D3DBLEND_ONE);
	C.AdjustCamera();	
	C.AdvanceFrame();

	int NumPart = (int)(Size*10.0f) * NUM_FACTOR;

	for(int n = 0; n < NumPart; n++)
	{
		A.AddParticle();
		B.AddParticle();
		C.AddParticle();
	}

	
	pTexture = Engine->GetTexture("particle");

//	Engine->Sound()->PlayEffect("explosion");

	Duration = 12;
	Frame = 0;
	
	
	ExplodeLight.dltType = D3DLIGHT_POINT;
	ExplodeLight.dcvDiffuse.r = 1.5f;
	ExplodeLight.dcvDiffuse.g = 1.5f;
	ExplodeLight.dcvDiffuse.b = 1.1f;
	ExplodeLight.dcvDiffuse.a = 1.0f;

	ExplodeLight.dcvSpecular.r =  1.2f;
	ExplodeLight.dcvSpecular.g =  1.2f;
	ExplodeLight.dcvSpecular.b =  1.0f;	
	ExplodeLight.dcvSpecular.a =  1.0f;

	ExplodeLight.dcvAmbient.r =  0.6f;
	ExplodeLight.dcvAmbient.g =  0.6f;
	ExplodeLight.dcvAmbient.b =  0.3f;
	ExplodeLight.dcvAmbient.a =  1.0f;

	ExplodeLight.dvPosition = Location;

	ExplodeLight.dvRange = 2.0f;

	ExplodeLight.dvFalloff = 1.0f;

	ExplodeLight.dvAttenuation0 = 1.0f;
	ExplodeLight.dvAttenuation1 = 0.0f;
	ExplodeLight.dvAttenuation2 = 0.0f;

	Engine->Graphics()->GetD3D()->SetLight(1, &ExplodeLight);

	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->LightEnable(1, TRUE);

	if(hr != D3D_OK)
	{
		Engine->ReportError("Could Not Enable Explosion Lighting");
	}


}

BOOL Explosion::AdvanceFrame()
{
	Red.diffuse.r *= FADE_FACTOR;
	Yellow.diffuse.r *= FADE_FACTOR;
	Orange.diffuse.r *= FADE_FACTOR;
	
	Red.diffuse.g *= FADE_FACTOR;
	Yellow.diffuse.g *= FADE_FACTOR;
	Orange.diffuse.g *= FADE_FACTOR;
	
	Red.diffuse.b *= FADE_FACTOR;
	Yellow.diffuse.b *= FADE_FACTOR;
	Orange.diffuse.b *= FADE_FACTOR;

	Red.ambient.r *= FADE_FACTOR;
	Yellow.ambient.r *= FADE_FACTOR;
	Orange.ambient.r *= FADE_FACTOR;
	
	Red.ambient.g *= FADE_FACTOR;
	Yellow.ambient.g *= FADE_FACTOR;
	Orange.ambient.g *= FADE_FACTOR;
	
	Red.ambient.b *= FADE_FACTOR;
	Yellow.ambient.b *= FADE_FACTOR;
	Orange.ambient.b *= FADE_FACTOR;

	A.SetMaterial(&Yellow);
	B.SetMaterial(&Orange);
	C.SetMaterial(&Red);

	BOOL Continue;
	A.AdvanceFrame();
	B.AdvanceFrame();
	Continue = C.AdvanceFrame();

	ExplodeLight.dvRange *= (1.0f + (Radius/12.0f));

	Engine->Graphics()->GetD3D()->SetLight(1,&ExplodeLight);

	Frame++;
	if(Frame == (Duration / 2))
	{
		if((MaxDamage - MinDamage) >= 0)
			Valley->Damage(Radius, vCenter, MinDamage, MaxDamage, DAMAGE_FIRE, pSource);

		float RandX;
		float RandY;
		int Duration;
		float FireSize;

		D3DVECTOR FireVector;
		Fire *pFire;

		for(int n = 0; n < (int)Radius * 5; n++)
		{
			RandX = ((float)rand() / (float)RAND_MAX) * Radius * 2.0f;
			RandY = ((float)rand() / (float)RAND_MAX) * Radius * 2.0f;

			Duration = 50 + rand() % 50;
			FireSize = ((float)rand() / (float)RAND_MAX) * 0.1;

			FireVector.x = vCenter.x - Radius + RandX;
			FireVector.y = vCenter.y - Radius + RandY;
			FireVector.z = Valley->GetZ(FireVector.x, FireVector.y);

			pFire = new Fire(FireVector,FireSize, NULL, Duration);
			PreludeWorld->AddMainObject((Object *)pFire);
		}


	}	

	if(!Continue)
	{
		BOOL IsEnabled;
		Engine->Graphics()->GetD3D()->GetLightEnable(1,&IsEnabled);

		if(IsEnabled)
			Engine->Graphics()->GetD3D()->LightEnable(1,FALSE);
		
		return FALSE;
	}
	else
	{ 
		return TRUE;
	}


}

void Explosion::Draw()
{

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	C.Draw();
	B.Draw();
	A.Draw();


}

void Explosion::AdjustCamera()
{
	A.AdjustCamera();
	B.AdjustCamera();
	C.AdjustCamera();
}
