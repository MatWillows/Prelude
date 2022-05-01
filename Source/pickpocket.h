#ifndef PICKPOCKET_H
#define PICKPOCKET_H

#include "zswindow.h"


class Creature;

class PickPocketWin : public ZSWindow
{
	private:
		Creature *pTarget;
		Creature *pPlayer;

	public:
		Creature *GetPlayer() { return pPlayer; }
		Creature *GetTarget() { return pTarget; }

		void SetPlayer(Creature *pCreature) { pPlayer = pCreature; }
		void SetTarget(Creature *pCreature) { pTarget = pCreature; }

		PickPocketWin(int NewID, int x, int y, int width, int height, Creature *pNewPlayer, Creature *pNewTarget);

		int Command(int IDFrom, int Command, int Param);

};

#endif