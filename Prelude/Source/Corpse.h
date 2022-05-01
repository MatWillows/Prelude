#ifndef CORPSE_H
#define CORPSE_H

#include "objects.h"

class Creature;

class Corpse : public Object
{
private:


public:

	void Draw();
		
	Corpse(Creature *pFrom);

	~Corpse();
};


#endif
