#ifndef REGIONS_H
#define REGIONS_H

#include "zstexture.h"
#include "walls.h"
#include "objects.h"

typedef enum
{
	REGION_INTERIOR,	//Interior regions have floors and interior walls
	REGION_EXTERIOR	//exterior regions have roofs and exterio walls
} REGION_T;

typedef enum
{
	REGION_UNSEEN = 0,
	REGION_SEEN,
	REGION_OCCUPIED
} REGION_VIEW_T;

typedef enum
{
	ROOF_NONE,
	ROOF_FLAT,
	ROOF_POINT,
	ROOF_GABLE,
	ROOF_SLOPE,
	ROOF_DOME,
	FLOOR_NONE = 0,
	FLOOR_FLAT,
} FLOORROOF_T;

class EditRegion;

class TextureList
{
public:
	ZSTexture *pTexture;
	WallSide *pWall;
	TextureList *pNext;

	TextureList(ZSTexture *pTex)
	{
		pTexture = pTex;
		pWall = NULL;
		pNext = NULL;
	}
};

class Region
{
private:

	BOOL Drawn;
	BOOL LOSChecked;

	int ID;

	REGION_VIEW_T Occupied;

	REGION_T Type;

	TextureList *TextureWalls;

	int NumSubRegions;
	Region *SubRegions;
	Region *Parent;

	D3DVERTEX *Verts;
	int NumVerts;
	unsigned short *DrawList;
	int NumTriangles;
	int NumBorderTriangles;

	ZSTexture *pFRTexture;		//floor or roof texture
	ZSTexture *pBorderTexture; //border for roof

	ZSTexture *pWallTexture;  //Texture of walls

	FLOORROOF_T FRType;

	Region *pNext;
	
	RECT Bounds;

	float Height;
	float FRHeight;

	D3DVECTOR vCenter;

	Wall *Walls;		 						

	Object *pObjectList;
	Object *pObjectListEnd;
	int NumObjects;

public:

	void SetCheckedLOS() { LOSChecked = TRUE; }
	void ClearCheckedLOS() { LOSChecked = FALSE; }

	BOOL CheckedLOS() { return LOSChecked; }

	void Load(FILE *fp);
	void Save(FILE *fp);

	int GetNumVerts() { return NumVerts; }
	void SetNumVerts(int NewVerts) { NumVerts = NewVerts; }
	
	int GetNumTriangles() { return NumTriangles; }
	void SetNumTriangles(int NewNum) { NumTriangles = NewNum; }

	void ScaleFloor(float Amount);
	void RotateFloor(float Angle);

	void ScaleRoof(float Amount);

	Region *GetSubRegion(float x, float y);
	Region *GetSubRegion() { return SubRegions; }
	Region *GetParent() { return Parent; }
	Region *GetNext() { return pNext; }

	Wall *GetNearestWall(float x, float y);
	D3DVECTOR GetNearestPoint(float x, float y);

	void CreateFRMesh();
	
	ZSTexture *GetFRTexture() { return pFRTexture; };
	void SetFRTexture(ZSTexture *pNT, ZSTexture *pBT = NULL);

	ZSTexture *GetWallTexture() { return pWallTexture; };
	void SetWallTexture(ZSTexture *pNT) { pWallTexture = pNT; };

	void RotateFRTexture(float Angle);

	void GetBounds(RECT *rTo) { *rTo = Bounds; }
	void SetBounds(RECT *rFrom) { Bounds = *rFrom; }
	void MakeBounds();

	BOOL IsIn(D3DVECTOR *vCheck);

	void SetWallHeights(float NewHeight);
	
	Region();
	Region(REGION_T NewType, D3DVECTOR *Bounds, int NumBounds);
	
	void CreateWalls(D3DVECTOR *Bounds, int NumBounds);

	~Region();
	
	int Draw();
	void UnDraw();

	void DrawObjects();

	void AddWall(Wall *ToAdd);

	void AddWallSide(WallSide *ToAdd);

	void OutputDebugInfo(FILE *fp);

	void FindCenter(D3DVECTOR *vCenter);

	void Highlight();

	D3DVECTOR GetCenter() { return vCenter; }

	REGION_T GetType() { return Type; }
	void SetType(REGION_T NewType) { Type = NewType; }

	REGION_VIEW_T IsOccupied() { return Occupied; }
	void SetOccupancy(REGION_VIEW_T NewOc);
	void SortView();
	void UnOccupy();

	void CreateDrawList();

	void SetCenter(D3DVECTOR NewCenter) { vCenter = NewCenter; }

	int AddObject(Object *pAddObject);
	void RemoveObject(Object *pToRemove);
	void ClearTile(int x, int y);

	int GetID() { return ID; }
	void SetID(int NewID) { ID = NewID; }

	void AssignWalls();
	Region *GetSubRegion(int ID);

	Region *PointIntersect(D3DVECTOR *vAt);

	Object *GetObjects() { return pObjectList; }
	Object *GetLastObject() { return pObjectListEnd; }

	Wall *GetWall(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	WallSide *GetWallSide(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	Region *GetRoom(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	Region *GetFloor(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	Region *GetRoof(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	
	void Move(D3DVECTOR *vMoveRay);

	void ReBlock();
	void RePortal();

	void PaintInterior(ZSTexture *pText, float TileAmount = 1.0f);
	void PaintExterior(ZSTexture *pText, float TileAmount = 1.0f);
	void PaintFloors(ZSTexture *pText);
	
	void Devolve(EditRegion *pER);
	void SortWalls();
	void GatherObjects();
	void UnGatherObjects();
	
	friend class EditRegion;
	friend class EditRoom;

	BOOL LineIntersect(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);
};




#endif