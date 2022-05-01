#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ZSwindow.h"
#include "path.h"

class EEWin;
class WorldEditWin;
class RegionEditWin;
class ZSText;
class ZSScriptWin;
class Object;
class PeopleEditWin;
class ZSDescribe;
class ZSMenuBar;

//Main window
class ZSMainWindow : public ZSWindow
{
private:
	float MouseX;
	float MouseY;
	float MouseZ;
	int LastX;
	int LastY;


	EEWin *pEEWin;
	WorldEditWin *pWorldEdit;
	RegionEditWin *pRegionEdit;
	ZSScriptWin *pScriptWin;
	ZSText *pFPS;
	ZSText *pDrawTime;
	ZSText *pTargetString;
	PeopleEditWin* pPeopleEdit;
	ZSDescribe *pDescribe;
	ZSMenuBar *pMenuBar;

	char TargetString[128];

	ZSTexture *pGoddessTextures[4];

	COLOR_T HighLightColor;
	BOOL ShowRange;
	BOOL HighlightNonStatic;

	BOOL ShowInv;

	int SkipFrames;
	long MouseWheel;

	Path MainPath;

	Object *pTarget;
	Object *pSpeaker;

	D3DCOLORVALUE Blue;
	D3DCOLORVALUE Red;

	BOOL DrawMouseSquare;

	D3DVECTOR vRayStart;
	D3DVECTOR vRayEnd;

	BOOL DrawWorld;
	BOOL Pathing;
	BOOL ShowFrames;
	int FrameRate;
	float ScrollFactor;
	BOOL Zooming;
	float ZoomFactor;
	BOOL Rotating;
	float RotationFactor;
	BOOL Arcing;
	float Arcfactor;
	BOOL Scrolling;
	D3DVECTOR vOffset;
	D3DVECTOR ScrollVector;
	DWORD LastMoved;
	BOOL GotTarget;
	int Frame;
	BOOL LBDown;
	int xLeft;
	int yLeft;

	int iMouseX;
	int iMouseY;

	DWORD NextFrame;
	int FrameNum;
	DWORD FrameLength;
	DWORD LastFrame;


public:
	void GetTarget();

	int GetFrame() { return Frame; }
	
	int Draw();
	int DrawAndUpdate();
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonUp(int x, int y);
	int MoveMouse(long *x, long *y, long *z);
	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);
	int GoModal();

	int GetFrameRate() { return FrameRate; }
	int SetFrameRate(int NewRate) { FrameRate = NewRate; }
	
	void SetDrawWorld(BOOL NewState) { DrawWorld = NewState; }
	void SetSpeaker(Object *pNewSpeaker) { pSpeaker = pNewSpeaker; }
	Object *GetSpeaker() { return pSpeaker; }

	void SetScrolling(BOOL NewScroll) { Scrolling = NewScroll; }
	void SetZoom(BOOL NewZoom) { Zooming = NewZoom; }
	void SetRotate(BOOL NewRotate) { Rotating = NewRotate; }
	void SetArcing(BOOL NewArc) { Arcing = NewArc; }

	void ShowInventory() { ShowInv = TRUE; }
	void HideInventory() { ShowInv = FALSE; }

	BOOL InventoryShown() { return ShowInv; }

	float GetScrollFactor() { return ScrollFactor; }
	void SetScrollFactor(float NewFactor) { ScrollFactor = NewFactor; }

	Object *Target() { return pTarget; }
	void SetTarget(Object *NewTarget) { pTarget = NewTarget; }
	ZSMainWindow();
	void Describe(char *DText);
	void ClearDescribe();

	BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);

	void SetHighLightNonStatic(BOOL NewVal) { HighlightNonStatic = NewVal; };

	void SetTargetString();

	void TabToNextTarget();
	//~ZSMainWindow();
};

#endif