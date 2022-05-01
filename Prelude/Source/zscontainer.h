#ifndef ZSCONTAINER_H
#define ZSCONTAINER_H

#include "zswindow.h"

class Object;

class ZSContainerWin : public ZSWindow
{
private:

public:

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSContainerWin(int NewID, int x, int y, int width, int height, Object *pFrom);

};

#endif