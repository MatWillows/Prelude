#ifndef SAYCHAR_H
#define SAYCHAR_H

#include "zswindow.h"
#include "things.h"

class ZSSayChar : public ZSWindow
{
private:
	DWORD StartTime;
	DWORD EndTime;
	BOOL RButtonDown;
	BOOL LButtonDown;

public:

	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);

	int LeftButtonUp(int x, int y);
	int LeftButtonDown(int x, int y);
	
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);
	int GoModal();

	ZSSayChar(int NewID, int x, int y, int Width, int Height, char *Text, Thing *Sayer);

};

#endif