#ifndef CAVEWALL_H
#define CAVEWALL_H

#include "Objects.h"

typedef struct
{
	DIRECTION_T FromAngle;
	DIRECTION_T ThisAngle;
	D3DVECTOR vStart;
	D3DVECTOR vEnd;
	DIRECTION_T ToAngle;
} WALL_SEGMENT_T;

class	WallSegmentList
{
public:
	static WallSegmentList *pStart;
	WALL_SEGMENT_T Seg;
	WallSegmentList *pNext;

	WallSegmentList();
	~WallSegmentList();
};

class CaveWall : public Object
{
private:
	int Type;
	int NumVerts;
	D3DVERTEX *pVertsTop;
	D3DVERTEX *pVertsMiddle;
	D3DVERTEX *pVertsBottom;
		
	int NumSegments;
	WALL_SEGMENT_T *Segments;
	
	float Height;

public:
	int GetNumSegments() { return NumSegments; }

	void AddSegment(int x1, int y1, int x2, int y2);

	void FlipSegment(int SegNum);

	OBJECT_T GetObjectType() { return OBJECT_BARRIER; }

	void SetType(int NewType) { Type = NewType; }
	int GetType() { return Type; }

	void CreateSegments(int x1, int y1, int x2, int y2);
	
	void Build(WallSegmentList *pSegList);
	void BuildFlat(WallSegmentList *pSegList);
	void BuildCurved(WallSegmentList *pSegList);

	void BuildVerts();

	void DrawTemp();

	void Draw();


	void FlipSides();

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

	BOOL TileIntersect(int TileX, int TileY);
	BOOL TileBlock(int TileX, int TileY);

	void Link(CaveWall *pLinkTo);

	void Load(FILE *fp);
	void Save(FILE *fp);

	void SetHeight(float newHeight) { Height = newHeight; };

	float GetHeight() { return Height; }
		
	void Block();

	CaveWall();

	~CaveWall();

	void Flatten();
	void Curve();

};

#endif