#ifndef WATER_H
#define WATER_H

#include "objects.h"
#include "world.h"
#include "zstexture.h"

class Water : public Object
{
private:
	static ZSTexture *WaterTexture[9];
	DWORD DrawTime;
	static D3DMATERIAL7 matWater;
	float Verts[3 * 5];
	float UV[2 * 5];
	D3DDRAWPRIMITIVESTRIDEDDATA StridedInfo;

public:

	void Draw();
	int HighLight(COLOR_T Color);

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	OBJECT_T GetObjectType() { return OBJECT_WATER; }
   int GetDefaultAction(Object *pactor) { return 0; };
	int LookAt(Object *looker) { return 0; };
	
	BOOL AdvanceFrame();

	void Save(FILE *fp);
	void Load(FILE *fp);

	Water();

	void Expand(float fN, float fS, float fE, float fW);
	void RaiseEdge(float fN, float fS, float fE, float fW);


	void MakeVerts(int ChunkX, int ChunkY, float zNW, float zNE, float zSW, float zSE);
	void MakeVerts(float Left, float Right, float Top, float Bottom, float Height);

	void MoveVerts(D3DVECTOR *vNW, D3DVECTOR *vNE, D3DVECTOR *vSW, D3DVECTOR *vSE);
	
	void AdjustWater(Chunk *pChunk);

	void Reset();

	friend class Area;

};


#endif
