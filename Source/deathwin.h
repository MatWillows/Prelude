#ifndef DEATHWIN_H
#define DEATHWIN_H

#include "ZSWindow.h"

#define IDC_DEATH_CLOSE	1
#define IDC_DEATH_TEXT	2


class DeathWin : public ZSWindow
{
private:

public:

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);

	DeathWin();
};

#endif