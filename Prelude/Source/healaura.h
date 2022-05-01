#ifndef HEALAURA_H
#define HEALAURA_H

#include "zsparticle.h"
#include "things.h"

class HealAura : public Object
{
private:
	int HitHeal;
	int RestHeal;

	ParticleSystem Stars;
	ParticleSystem Bubbles;

public:
	
	BOOL AdvanceFrame();
	void Draw();
	void AdjustCamera();

	HealAura(Thing *pTarget, int HitFactor, int RestFactor);

};

#endif