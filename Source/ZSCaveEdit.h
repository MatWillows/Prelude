#ifndef ZSCAVEEDIT_H
#define ZSCAVEEDIT_H

#include "zswindow.h"

class ZSCaveEdit : public ZSWindow
{
private:
	int x1;
	int y1;
	int x2;
	int y2;

public:

	int Draw();
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	//int RightButtonDown(int x, int y);
	//int RightButtonUp(int x, int y);
	//int MoveMouse(long *x, long *y, long *z);
	//int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSCaveEdit(int NewID);


};



#endif