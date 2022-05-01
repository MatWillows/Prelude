#ifndef BLOOD_H
#define BLOOD_H

#include "zsparticle.h"

class Blood : public Object
{
private:
	static BOOL BloodEnabled;

	ParticleSystem BloodSys;

public:

	static void EnableBlood() { BloodEnabled = TRUE; }
	static void DisableBlood() { BloodEnabled = FALSE; }
	
	static BOOL IsEnabled() { return BloodEnabled; }


	void Draw();
	BOOL AdvanceFrame();
	Blood(float size, Thing *pLink);
	void AdjustCamera();

	ParticleSystem *GetSys() { return &BloodSys; }


};

#endif