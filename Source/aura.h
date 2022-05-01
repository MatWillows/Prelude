#ifndef AURA_H
#define AURA_H

#include "zsparticle.h"
#include "objects.h"

class Aura : public Object
{
private:
	ParticleSystem ParSys;
	int SpellNum;
	int EffectLevel;
	int DamageLevel;
	int LastRoundActive;

public:
	ParticleSystem *GetSys() { return &ParSys; }

	BOOL AdvanceFrame();
	void AdjustCamera();
	void Draw();
	int GetSpellNum() { return SpellNum; }
	void SetSpellNum(int Num) { SpellNum = Num; }

	int GetEffectLevel() { return EffectLevel; }
	void SetEffectLevel(int Num) { EffectLevel = Num; }

	int GetLastRoundActive() { return LastRoundActive; }
	void SetLastRoundActive(int NewRound) { LastRoundActive = NewRound; }

	int GetDamageLevel() { return DamageLevel; }
	void SetDamageLevel(int NewDam) { DamageLevel = NewDam; }
};

Object *CreateAura(Object *pTarget, COLOR_T AuraColor, float AuraGravity, float AuraSpeed);

#endif