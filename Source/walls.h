#ifndef WALLS_H
#define WALLS_H

#include "zsmodelex.h"
#include "zsutilities.h"

typedef enum
{
	WALL_INTERIOR,
	WALL_EXTERIOR,
	WALL_CAVERN,
	WALL_TUNNEL,
	WALL_DOOR,
	WALL_WINDOW,
	WALL_OPEN,
} WALL_T;

typedef enum
{
	WALL_STATE_NONE,
	WALL_STATE_FRONT,
	WALL_STATE_BACK,
	WALL_STATE_OPAQUE,
} WALL_STATE_T;

class Region;

class WallSide
{
public:
	WallSide *pNext;
//	D3DDRAWPRIMITIVESTRIDEDDATA StridedInfo;
	D3DVERTEX *Verts;
//	float *VertexCoordinates;
//	float *UV;
//	float *Normals;
	unsigned short *DrawList;
	int NumTriangles;
	int NumVerts;
	int RegionNum;
	Region *RegionFacing;
	ZSTexture *pTexture;
	BOOL DrawMe;

	void Draw();
	void Save(FILE *fp);
	void Load(FILE *fp);

	void AdjustUV(float uOff, float vOff);
	void Tile(float Amount);
	void FixUV();


	WallSide();
	~WallSide();
};

class Wall
{
private:
	static unsigned short DrawList[12];
	Wall *pNext;

	D3DLVERTEX TBVerts[8];
	
	WallSide *Front;
	WallSide *Back;

	WALL_T Type;
		
	BOOL Transparent;

public:


	ZSTexture *GetTextureFront() { return Front->pTexture; }
	ZSTexture *GetTextureBack() { return Back->pTexture; }
	
	Region *GetRegionFront() { return Front->RegionFacing; }
	Region *GetRegionBack() { return Back->RegionFacing; }

	void SetNext(Wall *NewNext) { pNext = NewNext; }
	Wall *GetNext() { return pNext; }

	WallSide *GetFront() { return Front; }
	WallSide *GetBack() { return Back; }

	void SetFront(WallSide *NewFront) { Front = NewFront; }
	void SetBack(WallSide *NewBack) { Back = NewBack; }
		
	WALL_T GetType() { return Type; }
	void SetType(WALL_T NewType) { Type = NewType; }

	void SetTransparent(BOOL nt) { Transparent = nt; }

	void Move(D3DVECTOR *vMoveRay);

	void Draw();

	Wall();
	~Wall();

	WallSide *RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

	
	void GetStart(D3DVECTOR *pvDest);
	void GetEnd(D3DVECTOR *pvDest);
	void GetCenter(D3DVECTOR *pvDest);

	void OutPutDebugInfo(FILE *fp);

	void Save(FILE *fp);
	void Load(FILE *fp);

	BOOL LineIntersect(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);

	friend class EditRegion;
	friend class EditWall;
};

#endif