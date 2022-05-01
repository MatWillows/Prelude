#ifndef CASTAURA_H
#define CASTAURA_H

#include "zsparticle.h"

class CastAura : public Object
{
private:

	ParticleSystem AuraOne;
	ParticleSystem AuraTwo;
	ParticleSystem AuraThree;

	int AuraDuration;
	
public:

	BOOL AdvanceFrame();
	void Draw();
	void AdjustCamera();

	CastAura(Thing *pTarget, int Duration, D3DMATERIAL7 *Color1, D3DMATERIAL7 *Color2, D3DMATERIAL7 *Color3, char *ParticleName);

};

#endif