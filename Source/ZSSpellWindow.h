#ifndef SPELL_WINDOW_H
#define SPELL_WINDOW_H

#include "zswindow.h"

class Creature;

class SpellWin : public ZSWindow
{
private:
	Creature *pCaster;
	int SpellMap[MAX_SPELLS * 4];

public:
	
	Creature *GetCaster() { return pCaster; }
	
	void SetCaster(Creature *pNewCaster);

	int Command(int IDFrom, int Command, int Param);

	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	SpellWin(int NewID, int x, int y, int width, int height, Creature *pNewCaster);

};



#endif