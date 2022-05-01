#ifndef ZSASKWIN_H
#define ZSASKWIN_H

#include "zswindow.h"
#include "script.h"

class ZSAskWin : public ZSWindow
{
private:

public:
	static LPDIRECTDRAWSURFACE7 AskSurface;

	int LeftButtonDown(int x, int y);

	int Command(int IDFrom, int Command, int Param);

	ZSAskWin(int NewID, int x, int y, int Width, int Height, ScriptArg *pOptions);
};



#endif