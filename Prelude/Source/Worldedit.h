#ifndef WORLDEDIT_H
#define WORLDEDIT_H

#include "ZSwindow.h"

class Object;

typedef enum
{
	EDIT_MODE_NONE,
	EDIT_MODE_TILE,
	EDIT_MODE_PLACE_OBJECT,
	EDIT_MODE_SELECT_OBJECT,
	EDIT_MODE_EDIT_OBJECT,
	EDIT_MODE_PLACE_EVENTS,
	EDIT_MODE_SELECT_EVENTS,
	EDIT_MODE_EDIT_EVENTS,
	EDIT_MODE_PLACE_ITEMS,
	EDIT_MODE_SELECT_ITEMS,
	EDIT_MODE_EDIT_ITEMS,
	EDIT_MODE_PLACE_CREATURES,
	EDIT_MODE_SELECT_CREATURES,
	EDIT_MODE_EDIT_CREATURE,
	EDIT_MODE_VERTEX,
	EDIT_MODE_TRIANGLE,
	EDIT_MODE_HEIGHT,
	EDIT_MODE_SMOOTH,
	EDIT_MODE_FOREST,
	EDIT_BRUSH_LARGER,
	EDIT_BRUSH_SMALLER,
	EDIT_BRUSH_SPRAY,
	EDIT_BRUSH_RANDOM,
	EDIT_BRUSH_NORMAL,
	EDIT_MODE_WATER,
	EDIT_MODE_FOUNTAIN,
	EDIT_MODE_BLOCKING,
	EDIT_MODE_TILE_HEIGHTS,
	EDIT_MODE_SELECT_AREA,
	EDIT_MODE_WATER_FILL,
	EDIT_MODE_WATER_FILL_EAST,
	EDIT_MODE_WATER_FILL_SOUTH,
} EDIT_MODE_T;

typedef enum
{
	EDIT_FILL_NORMAL,
	EDIT_FILL_RANDOM,
	EDIT_FILL_SPRAY
} EDIT_FILL_T;

class Thing;
class Event;

class WorldEditWin : public ZSWindow
{
private:
	EDIT_MODE_T	EditMode;
	EDIT_FILL_T FillType;
	int FillRate;
	float BrushWidth;
	float YMark;
	float XMark;
	float TileX;
	float TileY;
	BOOL ShowTiles;
	BOOL ShowBlocking;
	int CurMesh;
	float MouseX;
	float MouseY;
	float MouseZ;
	D3DVECTOR vObjectMove;
	float ObjectRotateAngle;
	float ObjectScale;
	BOOL MovingObject;
	BOOL RotatingObject;
	BOOL ScaleObject;
	Object *pCurObject;
	Thing *pCurThing;
	Event *pCurEvent;
	BOOL ShownEventControls;
	BOOL ShownObjectControls;
	BOOL ShownPersonControls;
	BOOL ShownItemControls;
	D3DVECTOR WaterStart;
	D3DVECTOR WaterEnd;
	BOOL WireFrame;
	BOOL LBDown;
	RECT rSelectBox;
	BOOL Selected;
	D3DVECTOR WaterNW;
	D3DVECTOR WaterNE;
	D3DVECTOR WaterSW;
	D3DVECTOR WaterSE;

	void DrawFire();
	void DrawFountain();
	void DrawWaterFill();
	
	void ShowEventControls();
	void HideEventControls();

	void ShowObjectControls();
	void HideObjectControls();

	void ShowPersonControls();
	void HidePersonControls();

	void ShowItemControls();
	void HideItemControls();

	void HandleSpecialCommand(int SCom);
	void SelectHeightChange(float Amount);
	void SelectTileHeightChange(float Amount);


public:

	float GetBrushWidth() { return BrushWidth; }
	void SetBrushWidth(float NewWidth) { BrushWidth = NewWidth; }

	int Draw();
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);
	int MoveMouse(long *x, long *y, long *z);
	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);
	void Show();

	void DrawWater();

	WorldEditWin();

	EDIT_MODE_T GetEditMode() { return EditMode; }
	void SetEditMode(EDIT_MODE_T NewMode); 

	int GetCurMesh() { return CurMesh; }
	void SetCurMesh(int NewMesh) { CurMesh = NewMesh; }
	
	EDIT_FILL_T GetFillType() { return FillType; }
	void SetFillType(EDIT_FILL_T NewType) { FillType = NewType; }

	int GetBrush();
	int GetFillRate() { return FillRate; }
	void SetFillRate(int NewRate) { FillRate = NewRate; }

	void GetObject();
	void GetEvent();
	void GetItem();
	void GetPerson();

	void DrawBox();

	int GoModal();




};

#endif
