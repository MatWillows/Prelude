#ifndef PATTERN_H
#define PATTERN_H

#include "objects.h"
#include "zsparticle.h"


#define MAX_PATTERN_NODES	32

class Pattern : public Object
{
private:
	D3DVECTOR vNodes[MAX_PATTERN_NODES];
	int NumNodes;
	ParticleSystem ParSys;

public:
	int GetNumNodes()
	{
		return NumNodes;
	}
	void AddNode(D3DVECTOR vToAdd);
	
	D3DVECTOR *GetNode(int Num) { return &vNodes[Num]; }

	ParticleSystem *GetSys() { return &ParSys; }

	Pattern();
	
	BOOL AdvanceFrame();
	void Draw();
	void AdjustCamera();
	
	void Move(D3DVECTOR *vDirection);

//intersections
	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

};

#define MAX_PATTERN_NODES	32

class Bolt : public Object
{
private:
	D3DVERTEX vNodes[MAX_PATTERN_NODES];
	D3DVECTOR vBaseNodes[MAX_PATTERN_NODES];
	int NumNodes;
	COLOR_T Color;
	float Variation;
	int Duration;

public:

	int GetNumNodes()
	{
		return NumNodes;
	}

	float GetVariation() {return Variation; }
	void SetVariation(float fNewVar) { Variation = fNewVar; }

	COLOR_T GetColor() { return Color; }
	
	void SetColor(COLOR_T NewColor) { Color = NewColor; }

	D3DVERTEX *GetNode(int Num) { return &vNodes[Num]; }
	D3DVECTOR *GetBaseNode(int Num) { return &vBaseNodes[Num]; }

	void AddNode(D3DVECTOR vToAdd);

	void Create(D3DVECTOR *vStart, D3DVECTOR *vEnd, COLOR_T cColor, float fVariation);

	void SetDuration(int NewDur) { Duration = NewDur; }
	int GetDuration() { return Duration; }

	Bolt();
	
	BOOL AdvanceFrame();
	void Draw();
};



#endif
