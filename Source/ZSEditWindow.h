#ifndef ZSEDITWINDOW_H
#define ZSEDITWINDOW_H

#include "zswindow.h"

class ZSEdit : public ZSWindow
{
private:
	static char ClipBoard[512];
	int CharOffset;
	BOOL DrawCursor;
	int CursorFrame;

	void AddChar(int ASCIICode);
	void BackSpace();
	void Delete();
	
public:
	
	int GetOffset() { return CharOffset; }
	void SetOffset(int n) { CharOffset = n; }

	int Draw();
	int MoveMouse(long *x, long *y, long *z);
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
	void GainFocus();
	void LoseFocus();

	ZSEdit(WINDOW_T NewType, int NewID, int x, int y, int width, int height);

};

#endif