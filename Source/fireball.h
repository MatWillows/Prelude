#ifndef FIREBALL_H
#define FIREBALL_H

#include "zsparticle.h"

class FireBall : public Object
{
private:
	D3DVECTOR vEnd;
	float ExplosionSize;
	D3DLIGHT7 BallLight;
	
	ParticleSystem Inner;
	ParticleSystem Outer;
	ParticleSystem Trail;
	int MoveFrame;
	int NumMoveFrames;

	Thing *pSource;
	int MinDamage;
	int MaxDamage;
	float xoff;
	float yoff;
	float zoff;

public:
	
	int GetNumMoveFrames() { return NumMoveFrames; }

	FireBall(D3DVECTOR Start, D3DVECTOR End, float Speed, float BallSize, float ExplodeSize, int MinDam, int MaxDam, Thing *pDamSource);
	BOOL AdvanceFrame();
	void Draw();
	void AdjustCamera();
};


#endif