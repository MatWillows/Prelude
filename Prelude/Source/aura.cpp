#include "aura.h"
#include "zsengine.h"
#include "spellfuncs.h"
#include "world.h"
#include "combatmanager.h"

Object *CreateAura(Object *pTarget, COLOR_T AuraColor, float AuraGravity, float AuraSpeed)
{
	Aura *pAura;
	ParticleSystem *pSys;

	pAura = new Aura;

	pAura->SetSpellNum(0);
	pAura->SetLastRoundActive(0);
	pAura->SetEffectLevel(0);
	pAura->SetDamageLevel(0);
	
	pSys = pAura->GetSys();

	pSys->SetLife(15);
	pSys->SetLifeVariance(5);
	pSys->SetInitialVector(D3DVECTOR(0.0f,0.0f, AuraGravity));
	pSys->SetRotation(D3DVECTOR(0.0f,0.0f, AuraSpeed));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(AuraColor));
	pSys->SetRadius(0.06f);
	pSys->SetEmissionQuantity(4);
	//pSys->SetGravity(AuraGravity);
	pSys->SetLink(pTarget);
	pSys->AdjustCamera();
	pSys->AdvanceFrame();
	pSys->SetParticleType(PARTICLE_SPHERE);

	pAura->SetTexture(Engine->GetTexture("particle"));

	return pAura;

}

BOOL Aura::AdvanceFrame()
{
	 ZSModelEx *pMesh;
	 pMesh = ParSys.GetLink()->GetMesh();
	
	 int NewLink;
	 NewLink = rand() % pMesh->numvertex;
	 ParSys.SetLinkPoint(NewLink);
	 ParSys.AdvanceFrame();

	if(SpellNum)
	{
		int CurRound;
		CurRound = PreludeWorld->GetCombat()->GetRound();
		if(CurRound > LastRoundActive)
		{
			LastRoundActive = CurRound;
			ImmolationActivate(ParSys.GetLink(),EffectLevel, DamageLevel);
		}
	}

	 return TRUE;
}

void Aura::AdjustCamera()
{
	ParSys.AdjustCamera();
	return;
}


void Aura::Draw()
{
	
	ParSys.Draw();
}
