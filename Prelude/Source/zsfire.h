#ifndef MYFIRE_H
#define MYFIRE_H

#include "zsparticle.h"

class Fire : public Object
{

protected:

	ParticleSystem	*Red;
	ParticleSystem	*Yellow;
	float Size;
	int Duration;
	D3DVECTOR vAdd;

public:

	OBJECT_T GetObjectType() { return OBJECT_FIRE; }

	Fire(D3DVECTOR Location, float newsize, Thing *pLink, int newDuration);
	
	Fire();


	BOOL AdvanceFrame();
	void Draw();
	void AdjustCamera();
	void SetLinkPoint(int NewPoint);
	void SetLink(Object *NewLink);
	int SetScale(float NewScale);
	
	~Fire();

	void Move(D3DVECTOR *vDirection);

	int HighLight(COLOR_T Color);

	void Setup();


//i/o
	void Load(FILE *fp);
	void Save(FILE *fp);
	
//intersections
	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

	D3DVECTOR GetAdd() { return vAdd; }
	void SetAdd(D3DVECTOR vNewAdd);


};




#endif
