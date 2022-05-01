#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include "zsparticle.h"
#include "objects.h"

class Fountain : public Object
{
private:

	float Vertical;
	float Horizontal;
	float DropletGravity;
	float Variance;
	ParticleSystem *Droplets;
	int NumStreams;

public:

	void SetVertical(float NewVert) { Vertical = NewVert; SetNumStreams(GetNumStreams()); };
	void SetHorizontal(float NewHorizontal) { Horizontal = NewHorizontal; SetNumStreams(GetNumStreams());}
	void SetDropletGravity(float NewGravity) { DropletGravity = NewGravity; SetNumStreams(GetNumStreams());};
	void SetVariance(float NewVariance) { Variance = NewVariance; SetNumStreams(GetNumStreams());};

	float GetVertical() { return Vertical; };
	float GetHorizontal() { return Horizontal; };
	float GetDropletGravity() { return DropletGravity; };
	float GetVariance() { return Variance; };

	int GetNumStreams() { return NumStreams; };
	
	void SetNumStreams(int NewNum);

	OBJECT_T GetObjectType() { return OBJECT_FOUNTAIN; }

	BOOL AdvanceFrame();

   int GetDefaultAction(Object *pactor);

	void AdjustCamera();

	void Load(FILE *fp);
	void Save(FILE *fp);
	
	int HighLight(COLOR_T Color);
	
	void Draw();

	~Fountain();
	Fountain();

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);


};

#endif
