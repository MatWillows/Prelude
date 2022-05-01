#ifndef ZSMENUBAR_H
#define ZSMENUBAR_H

#include "zswindow.h"
#include "zsportrait.h"
#include "script.h"

typedef enum
{
	IDC_INTERFACEBAR_JOURNAL = 100,
	IDC_INTERFACEBAR_MAP,
	IDC_INTERFACEBAR_OPTIONS,
	IDC_INTERFACEBAR_REST,
	IDC_INTERFACEBAR_HELP,
	IDC_INTERFACEBAR_GATHER,
	IDC_INTERFACEBAR_ZOOMIN,
	IDC_INTERFACEBAR_ZOOMOUT,
	IDC_INTERFACEBAR_PAN_LEFT,
	IDC_INTERFACEBAR_PAN_DOWN,
	IDC_INTERFACEBAR_PAN_UP,
	IDC_INTERFACEBAR_PAN_RIGHT,
	IDC_INTERFACEBAR_ROTATE_LEFT,
	IDC_INTERFACEBAR_ROTATE_RIGHT,
	IDC_INTERFACEBAR_ROTATE_OVER,
	IDC_INTERFACEBAR_ROTATE_UNDER,

} INTERFACEBAR_CONTROLS;

class ZSMenuBar : public ZSWindow
{
private:
	LPDIRECTDRAWSURFACE7 ClockFace;
	RECT rClockTo;
	ZSPortrait *pPortrait[6];
	Flag *pDrachFlag;
	ZSWindow *pDrachWin;
	ZSWindow *pHourWin;
	int LastDrachs;
	int LastHour;

public:

	ZSPortrait *GetPortrait(int Num) { return pPortrait[Num]; }

	void SetPortraits();
	
	int Draw();

	int Command(int IDFrom, int Command, int Param);

	ZSMenuBar(int NewID, int x, int y, int width, int height);

	~ZSMenuBar();
};

#endif
