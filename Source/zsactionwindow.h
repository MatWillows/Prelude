#ifndef ZSACTIONWINDOW_H
#define ZSACTIONWINDOW_H

#include "ZSwindow.h"
#include "actionmenuclass.h"

#define ACTION_MENU_ID	666

class Creature;

class ZSActionWin : public ZSWindow
{
private:
	Creature *pActor;
	BOOL Casting;
	ActionMenu *pMenu;
	void CastMenu(Object *pCaster, Object *pTarget);

public:
	int Command(int IDFrom, int Command, int Param);

	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);

	int MoveMouse(long *x, long *y, long *z);

	ZSActionWin(ActionMenu *NewMenu,int x, int y, ZSWindow *NewParent);
	~ZSActionWin();

	Creature *GetActor();
	void SetActor(Creature *NewCreature);

};




#endif
