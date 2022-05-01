#ifndef SPELLBOOK_H
#define SPELLBOOK_H

#include "things.h"

class Spellbook : public Thing
{
private:
	static char SpellbookFieldNames[96*32];
	static Spellbook *pFirst;
	static int NumSpellbooks;

public:

	friend void LoadSpellbooks(FILE *fp);
	friend void SaveSpellbooks(FILE *fp);

	static Thing *GetFirst() { return pFirst; }
	void SetFirst(Thing *NewFirst) { pFirst = (Spellbook *)NewFirst; }

	Spellbook();
	~Spellbook();

};

void LoadSpellbooks(FILE *fp);
void SaveSpellbooks(FILE *fp);

#endif