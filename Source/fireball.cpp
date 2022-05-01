#include "fireball.h"
#include "explosion.h"
#include "world.h"
#include "zsutilities.h"

#define INIT_INNER	50.0f
#define INIT_MID	45.0f
#define INIT_OUTER	40.0f
#define NUM_FACTOR	10
#define FADE_FACTOR	0.85f



FireBall::FireBall(D3DVECTOR Start, D3DVECTOR End, float Speed, float BallSize, float ExplodeSize, int MinDam, int MaxDam, Thing *pDamSource)
{
	MinDamage = MinDam;
	MaxDamage = MaxDam;
	pSource = pDamSource;

	vEnd = End;
	ExplosionSize = ExplodeSize;

	Inner.SetOrigin(Start);
	Inner.SetInitialVector(D3DVECTOR(BallSize/INIT_INNER,BallSize/INIT_INNER,BallSize/INIT_INNER));
	Inner.SetInitialVariance(D3DVECTOR(BallSize/(INIT_INNER / 2.0f),BallSize/(INIT_INNER / 2.0f),BallSize/(INIT_INNER / 2.0f)));
	Inner.SetOp(D3DTOP_ADD);
	Inner.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
	Inner.SetLife(7);
	Inner.SetLifeVariance(0);
	Inner.SetParticleType(PARTICLE_SPHERE);
	Inner.SetRadius(BallSize/2.0f);
	Inner.SetEmissionQuantity(1);
	
	Inner.AdjustCamera();	
	Inner.AdvanceFrame();
	
	Outer.SetOrigin(Start);
	Outer.SetInitialVector(D3DVECTOR(BallSize/INIT_MID,BallSize/INIT_MID,BallSize/INIT_MID));
	Outer.SetInitialVariance(D3DVECTOR(BallSize/(INIT_MID / 2.0f),BallSize/(INIT_MID / 2.0f),BallSize/(INIT_MID / 2.0f)));
	Outer.SetOp(D3DTOP_ADD);
	Outer.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_ORANGE));
	Outer.SetLife(7);
	Outer.SetLifeVariance(0);
	Outer.SetParticleType(PARTICLE_SPHERE);
	Outer.SetRadius(BallSize/2.0f);
	Outer.SetEmissionQuantity(5);
	
	Outer.AdjustCamera();	
	Outer.AdvanceFrame();
	
	Trail.SetOrigin(Start);
	Trail.SetInitialVector(D3DVECTOR(BallSize/INIT_OUTER,BallSize/INIT_OUTER,BallSize/INIT_OUTER));
	Trail.SetInitialVariance(D3DVECTOR(BallSize/(INIT_OUTER / 2.0f),BallSize/(INIT_OUTER / 2.0f),BallSize/(INIT_OUTER / 2.0f)));
	Trail.SetGravity(D3DVECTOR(0.0f,0.0f,-0.001f));
	Trail.SetOp(D3DTOP_ADD);
	Trail.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
	Trail.SetLife(7);
	Trail.SetLifeVariance(0);
	Trail.SetParticleType(PARTICLE_SPHERE);
	Trail.SetRadius(BallSize);
	Trail.SetEmissionQuantity(1);

	Trail.AdjustCamera();	
	Trail.AdvanceFrame();

	Inner.AddParticle();
	Outer.AddParticle();
	Trail.AddParticle();
	
	pTexture = Engine->GetTexture("particle");

	MoveFrame = 0;
	
	float Length = GetDistance(&Start,&End);

	NumMoveFrames = (int)((30.0f * Length) / Speed);

	xoff = (End.x - Start.x) / (float)NumMoveFrames;
	yoff = (End.y - Start.y) / (float)NumMoveFrames;
	zoff = (End.z - Start.z) / (float)NumMoveFrames;

	Inner.SetMotion(D3DVECTOR(xoff,yoff,zoff));
	Outer.SetMotion(D3DVECTOR(xoff,yoff,zoff));
	Trail.SetMotion(D3DVECTOR(xoff,yoff,zoff));

	BallLight.dltType = D3DLIGHT_POINT;
	BallLight.dcvDiffuse.r = 1.5f;
	BallLight.dcvDiffuse.g = 1.5f;
	BallLight.dcvDiffuse.b = 1.1f;
	BallLight.dcvDiffuse.a = 1.0f;

	BallLight.dcvSpecular.r =  1.2f;
	BallLight.dcvSpecular.g =  1.2f;
	BallLight.dcvSpecular.b =  1.0f;	
	BallLight.dcvSpecular.a =  1.0f;

	BallLight.dcvAmbient.r =  0.6f;
	BallLight.dcvAmbient.g =  0.6f;
	BallLight.dcvAmbient.b =  0.3f;
	BallLight.dcvAmbient.a =  1.0f;

	BallLight.dvPosition = Start;

	BallLight.dvRange = BallSize * 3.0f;

	BallLight.dvFalloff = 1.0f;

	BallLight.dvAttenuation0 = 1.0f;
	BallLight.dvAttenuation1 = 0.0f;
	BallLight.dvAttenuation2 = 0.0f;

	Engine->Graphics()->GetD3D()->SetLight(1, &BallLight);

	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->LightEnable(1, TRUE);

	if(hr != D3D_OK)
	{
		Engine->ReportError("Could Not Enable Explosion Lighting");
	}


}

BOOL FireBall::AdvanceFrame()
{
	Inner.AdvanceFrame();
	Outer.AdvanceFrame();
	Trail.AdvanceFrame();

	BallLight.dvPosition.x += xoff;
	BallLight.dvPosition.y += yoff;
	BallLight.dvPosition.z += zoff;

	Engine->Graphics()->GetD3D()->SetLight(1,&BallLight);


	MoveFrame++;
	if(MoveFrame > NumMoveFrames)
	{
		BOOL IsEnabled;
		Engine->Graphics()->GetD3D()->GetLightEnable(1,&IsEnabled);

		if(IsEnabled)
			Engine->Graphics()->GetD3D()->LightEnable(1, FALSE);
		Explosion *pExplosion;
		pExplosion = new Explosion(vEnd, ExplosionSize, MinDamage, MaxDamage, pSource);

		PreludeWorld->AddMainObject((Object *)pExplosion);
	
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}


void FireBall::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	
	Trail.Draw();
	Outer.Draw();
	Inner.Draw();

}


void FireBall::AdjustCamera()
{
	Inner.AdjustCamera();
	Outer.AdjustCamera();
	Trail.AdjustCamera();
}
