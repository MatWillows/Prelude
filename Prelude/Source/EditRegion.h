#ifndef EDITREGION_H
#define EDITREGION_H

#include "regions.h"

#include "defs.h"

typedef enum
{
	EWALL_NONE,
	EWALL_NORMAL,
	EWALL_WINDOW,
	EWALL_DOOR,
} EDIT_WALL_T;

class EditPoint
{
public:
	BOOL Touched;
	int ID;
	D3DVECTOR Location;

	EditPoint *pNext;

	void Draw();

	EditPoint();
	EditPoint(D3DVECTOR *pVector);
};

class EditRoom;

class EditWall
{
public:
	int ID;
	int NumRefs;
	BOOL Exterior;

	EditPoint *Start;
	EditPoint *End;
	float Height;
	EDIT_WALL_T Type;

	EditWall *pNext;

	EditRoom *pRoomRight;
	EditRoom *pRoomLeft;

	Wall *Here;
	
	void Draw();

	EditRoom *GetRight(EditPoint *pFrom);
	EditRoom *GetLeft(EditPoint *pFrom);

	void SetLeft(EditPoint *pFrom, EditRoom *pRoom);
	void SetRight(EditPoint *pFrom, EditRoom *pRoom);

	EditPoint *GetEnd(EditPoint *pFrom);

	void CreateMesh();

	EditWall();
	EditWall(EditPoint *vStart, EditPoint *vEnd, float StartHeight = 4.0f);

	float GetWallAngle();

	DIRECTION_T GetOrientation();

	BOOL CanDoor();

};

class EditRoom
{
public:
	int ID;
	EditWall *Walls[32];
	int NumWalls;
	float AngleTotal;

	EditRoom *pNext;

	Region *Here;
	
	void Draw();

	void SortWalls();

	BOOL TestPoint(D3DVECTOR *vToTest);

	BOOL HasWall(EditWall *ToTest);
	void AddWall(EditWall *ToAdd);
	void OutputDebugInfo(FILE *fp);
	float SumAngles();

	void CreateFloor();
	void CreateRoof(FLOORROOF_T Type, float RoofHeight, float OverHang);

	BOOL Validate();

	D3DVECTOR FindCenter();

	EditRoom();
	~EditRoom();
};


class EditRegion
{
private:
	RECT Bounds;
	int NextID;
	//the list of points
	EditPoint *Points;
	int NumPoints;

	//list of all walls
	EditWall *Walls;
	int NumWalls;
	
	EditRoom *Exterior;

	EditRoom *Interior;
	int NumRooms;

	float WallHeight;
	float	RoofHeight;
	float OverHang;

	FLOORROOF_T RoofType;
	float RoofScale;
	

public:
	BOOL CheckWallRelation(EditWall *A, EditWall *B);

	void AddWall(EditWall *ToAdd);
	
	void AddPoint(EditPoint *ToAdd);

	void AddRoom(EditRoom *ToAdd);

	void DefineWall(EditPoint *A, EditPoint *B);

	void DrawPoints();

	void DrawInterior();

	void DrawWalls();

	void Draw();
	
	EditPoint *GetNearestPoint(D3DVECTOR *vCloseTo);

	EditWall *GetNearestWall(D3DVECTOR *vCloseTo);

	EditRoom *GetNearestRoom(D3DVECTOR *vCloseTo);

	void RemovePoint(EditPoint *ToRemove);

	void RemoveWall(EditWall *ToRemove);

	void RemoveRoom(EditRoom *ToRemove);

	EditWall *FindWall(EditPoint *, EditPoint *);

	FLOORROOF_T GetRoofType() { return RoofType; }
	void SetRoofType(FLOORROOF_T NewType) { RoofType = NewType; }

	void CreateRooms();

	int PointsAround(EditPoint *pPoint, EditPoint *List[16]);

	EditPoint *GetCW(EditPoint *pPoint, EditPoint *pFrom);
	EditPoint *GetCCW(EditPoint *pPoint, EditPoint *pFrom);

	Region *CreateGameRegion(ZSTexture *,ZSTexture *,ZSTexture *,ZSTexture *);

	EditRegion();

	~EditRegion();

	void CreateInterior();

	EditPoint *FindPoint(D3DVECTOR *vToFind);

	void OutputDebugInfo(char *FileName);

	void ClearAll();

	void ClearRooms();

	float GetOverHang() { return OverHang; }
	void SetOverHang(float NewOverHang) { OverHang = NewOverHang; }

	float GetRoofHeight() { return RoofHeight; }
	void SetRoofHeight(float NewRoofHeight) { RoofHeight = NewRoofHeight; }

	float GetWallHeight() { return WallHeight; }
	void SetWallHeight(float NewWallHeight);

	void DivideWall(EditWall *ToDivide, D3DVECTOR *vAt);

	void DoorWall(EditWall *ToDoor, D3DVECTOR *vAt);

	void GetBounds();
};


void DrawDoor(D3DVECTOR *vA, EditWall *vOn);





#endif