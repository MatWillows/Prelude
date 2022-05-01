#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "zsparticle.h"
#include "objects.h"

class Explosion : public Object
{
	private:
		ParticleSystem A;
		ParticleSystem B;
		ParticleSystem C;

		D3DMATERIAL7	Red;
		D3DMATERIAL7	Yellow;
		D3DMATERIAL7   Orange;
		D3DLIGHT7		ExplodeLight;

		int MinDamage;
		int MaxDamage;
		float Radius;
		D3DVECTOR vCenter;
		int Duration;
		Thing *pSource;

	public:

		Explosion(D3DVECTOR Location, float Size, int MinDam, int MaxDam, Thing * pThingSource);

		BOOL AdvanceFrame();

		void Draw();

		void AdjustCamera();

};


#endif
