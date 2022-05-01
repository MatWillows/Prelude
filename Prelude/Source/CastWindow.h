#ifndef CASTWINDOW_H
#define CASTWINDOW_H

#include "ZSwindow.h"
#include "spells.h"
#include "spellbook.h"

#define CAST_MENU_ID	666

class CastWindow : public ZSWindow
{
private:
	Thing *pTarget;
	Thing *pCaster;

public:
	int Command(int IDFrom, int Command, int Param);

	int RightButtonUp(int x, int y);

	CastWindow(int x, int y, Object *NewCaster, Object *NewTarget);
	~CastWindow();

};


#endif