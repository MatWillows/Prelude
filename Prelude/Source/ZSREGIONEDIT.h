#ifndef ZSREGIONEDIT_H
#define ZSREGIONEDIT_H

#include "ZSwindow.h"
#include "regions.h"
#include "editregion.h"
#include "zsshrinkwindow.h"

typedef enum
{
	TOOL_NONE,
	TOOL_ADD_WALL,
	TOOL_DIVIDE_WALL,
	TOOL_MOVE_POINT,
	TOOL_PAINT_WALL,
	TOOL_PAINT_FLOOR,
	TOOL_PAINT_ROOF,
	TOOL_ADD_DOOR,
	TOOL_ADD_WINDOW,
	TOOL_SELECT_WALL,
	TOOL_SELECT_FLOOR,
	TOOL_LINK_DOOR,
} TOOL_T;


class FloorToolWin : public ZSShrinkWindow
{
private:

public:

	int Command(int IDFrom, int Command, int Param);

	FloorToolWin(int NewID, int x, int y, int width, int height);


};


class WallToolWin : public ZSShrinkWindow
{
private:

public:

	int Command(int IDFrom, int Command, int Param);

	WallToolWin(int NewID, int x, int y, int width, int height);
	
};



class RegionToolWin : public ZSShrinkWindow
{
public:

	int Command(int IDFrom, int Command, int Param);

	RegionToolWin(int NewID, int x, int y, int width, int height);


};

class RoofToolWin : public ZSShrinkWindow
{
public:

	int Command(int IDFrom, int Command, int Param);

	RoofToolWin(int NewID, int x, int y, int width, int height);


};



class RegionEditWin : public ZSWindow
{
private:
	WallSide *pWallSide;
	Region *pRegion;
	Region *pFloorRegion;
	EditRegion ERegion;
	D3DVECTOR VertList[16];
	int NumBounds;

	TOOL_T Tool;

	BOOL Recording;
	
	float YMark;
	float XMark;
	float TileX;
	float TileY;

	float MouseX;
	float MouseY;
	float MouseZ;

public:

	void PaintWall();
	void GetWall();
	void DivideWall();
	void WallTextureChange(float uOff, float vOff);

	void ScaleFloor(float Amount);
	void GetFloor();
	void PaintFloor();
	void RotateFloor(float Angle);

	void PaintRoof();
	void ScaleRoof(float Amount);


	int Draw();
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);
	int MoveMouse(long *x, long *y, long *z);
	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	void LinkDoor(int x, int y);
	
	void Show();

	int GoModal();

	RegionEditWin();
	~RegionEditWin();
};

#endif
