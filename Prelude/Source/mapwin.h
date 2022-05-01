#ifndef MAPWIN_H
#define MAPWIN_H

#include "zswindow.h"

#define MAP_WIN_ID	50505
#define MAX_LOCATORS 256

class MapLocator;

class MapWin : public ZSWindow
{
private:
	RECT rArea;
	RECT rDrawFrom;
	RECT rMapArea;
	RECT rParty;
	BOOL DrawParty;

	int ZoomLevel;
	int NumLocators;
	MapLocator *pLocs[MAX_LOCATORS];
	int RandomEvent;

	MapLocator *pCurLoc;
	int TagX;
	int TagY;
	int CurTown;
	int NumTowns;
	char TownNames[16][32];
	char TownBitmaps[16][32];


	void SwitchTowns(int NewTown);

public:
	static LPDIRECTDRAWSURFACE7 MapSurface;
	static LPDIRECTDRAWSURFACE7 MapBackground;

	int Draw();

	void Scroll(int XOffSet, int YOffset);

	void ZoomIn();
	void ZoomOut();

	int Command(int IDFrom, int Command, int Param);

	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);

	int MoveMouse(long *x, long *y, long *z);

	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	int GoModal();

	void Show();

	MapWin();
	~MapWin();

};

#endif
