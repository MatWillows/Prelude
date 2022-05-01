#ifndef SKILLWIN_H
#define SKILLWIN_H

#include "zswindow.h"

class Creature;

class SkillWin : public ZSWindow
{
private:
	Creature *pTarget;
	int StartXP;
	int StartValues[24];

public:
	Creature *GetTarget() { return pTarget; }

	void SetTarget(Creature *NewTarget) { pTarget = NewTarget; }

	int Command(int IDFrom, int Command, int Param);

	SkillWin(int NewID, int x, int y, int width, int height, Creature *pNewTarget);

	void Reset();
};

#endif