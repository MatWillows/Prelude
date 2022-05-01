#ifndef TRANSWINDOW_H
#define TRANSWINDOW_H

#include "ZSwindow.h"
#include "objects.h"


class TransWin : public Object
{

private:
	ZSWindow *pWinLink;
	D3DCOLOR	Color;
	int Border;

public:
	
	void Draw();
	int AdvanceFrame();

	TransWin(ZSWindow *pLink, D3DCOLOR TransColor, int BorderWidth);
	
};

#endif