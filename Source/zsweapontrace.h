#ifndef WEAPONTRACE_H
#define WEAPONTRACE_H

#include "zsparticle.h"

class EquipObject;

class WeaponTracer : public Object
{
public:

	D3DVERTEX Verts[64];
	int DrawList[64];
	int NumVerts;
	D3DMATERIAL7 Material;

	EquipObject *pLink; 
	int LastFrame;
	float LastAngle;
	D3DVECTOR LastPosition;

	void Draw();

	BOOL AdvanceFrame();

	void AddLine();

	WeaponTracer() 
	{ 
		LastFrame = -1;
		pNext = NULL;
		Data = NULL;
		Angle = 0.0f;
		TextureNum = 0;
		pTexture = NULL;
		Frame = 0;
		Position = D3DVECTOR(0.0f,0.0f,0.0f);
		Scale = 1.0f;
		MeshNum = 0;
		pMesh = NULL;
		pNextUpdate = NULL;
		pPrevUpdate = NULL;
		pContents = NULL;
		NumVerts = 0;
	}
	
};

#endif
