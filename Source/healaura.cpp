#include "healaura.h"

#define HEAL_GRAVITY		-0.003f
#define HEAL_ROTATION	-0.05f
#define HEAL_DURATION		60

BOOL HealAura::AdvanceFrame()
{
	Stars.AdvanceFrame();

	return Bubbles.AdvanceFrame();
}

void HealAura::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	Stars.Draw();

	Engine->Graphics()->SetTexture(Engine->GetTexture("bubble"));
	Bubbles.Draw();

}

void HealAura::AdjustCamera()
{
	Stars.AdjustCamera();
	Bubbles.AdjustCamera();
}

HealAura::HealAura(Thing *pTarget, int HitFactor, int RestFactor)
{
	Stars.SetOrigin(D3DVECTOR(0.0f,0.0f,pTarget->GetMesh()->GetHeight()/2.0f));
	//Stars.SetGravity(D3DVECTOR(0.0f,0.0f, HEAL_GRAVITY));
	Stars.SetLife(20);
	Stars.SetLifeVariance(5);
	Stars.SetRotation(D3DVECTOR(0.0f,0.0f,HEAL_ROTATION));
	Stars.SetParticleType(PARTICLE_STAR);
	Stars.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
	Stars.SetAddVector(D3DVECTOR(0.5f,0.5f,pTarget->GetMesh()->GetHeight()/2.0f));
	Stars.SetRadius(0.25f);
	Stars.SetEmissionQuantity(3);
	Stars.SetLink(pTarget);
	Stars.AddParticle();
	Stars.AddParticle();
	Stars.AddParticle();
	Stars.AdjustCamera();

	Bubbles.SetOrigin(D3DVECTOR(0.0f,0.0f,pTarget->GetMesh()->GetHeight()));
	Bubbles.SetGravity(D3DVECTOR(0.0f,0.0f, HEAL_GRAVITY));
	Bubbles.SetLife(30);
	Bubbles.SetLifeVariance(5);
	Bubbles.SetParticleType(PARTICLE_BUBBLE);
	Bubbles.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLUE));
	Bubbles.SetAddVector(D3DVECTOR(0.6f,0.6f,0.0f));
	Bubbles.SetRadius(0.5f);
	Bubbles.SetEmissionQuantity(2);
	Bubbles.SetLink(pTarget);
	Bubbles.AddParticle();
	Bubbles.AddParticle();
	Bubbles.AddParticle();
	Bubbles.SetEnd(HEAL_DURATION);
	Bubbles.AdjustCamera();

	pTexture = Engine->GetTexture("star");

}
