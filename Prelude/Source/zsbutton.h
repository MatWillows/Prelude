#ifndef ZSBUTTON_H
#define ZSBUTTON_H

#include "zswindow.h"

typedef enum
{
	BUTTON_NONE,
	BUTTON_NORMAL,
	BUTTON_PLUS,
	BUTTON_MINUS,
	BUTTON_NORTH,
	BUTTON_EAST,
	BUTTON_SOUTH,
	BUTTON_WEST,
	BUTTON_THUMB,
	BUTTON_BULLET,
	BUTTON_CHECK,
	BUTTON_CAMERA_ROTATE_RIGHT,
	BUTTON_CAMERA_ROTATE_LEFT,
	BUTTON_CAMERA_ROTATE_OVER,
	BUTTON_CAMERA_ROTATE_UNDER,
	BUTTON_PIC,
	
} BUTTON_T;



//WINDOW_BUTTON,
class ZSButton : public ZSWindow
{
private:
	RECT Normal;
	RECT MouseOver;
	RECT Pressed;
	int NumFrames;
	int CurFrame;
	int SubFrame;
	int BaseHeight;
	DWORD NextUpdate;
	static RECT rButtonNormal[BUTTON_PIC];
	static RECT rButtonMouseOver[BUTTON_PIC];
	static RECT rButtonPressed[BUTTON_PIC];
	static LPDIRECTDRAWSURFACE7	 ButtonSurface;

public:

	static void Init();
	static void ShutDown();
	int Draw();
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);

	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);

	int MoveMouse(long *x, long *y, long *z);
	ZSButton(BUTTON_T BType, int NewID, int x, int y, int width, int height);
	ZSButton(char *picname, int NewID, int x, int y, int width, int height, int basewidth, int baseheight, int NumFrames);

};

#endif