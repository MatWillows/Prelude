#ifndef BARTER_H
#define BARTER_H

#include "zswindow.h"


class Creature;
class Flag;
class GameItem;


class BarterWin : public ZSWindow
{
	private:
		Creature *pMerchant;
		Creature *pBargainer;
		Creature *pTarget;
		Flag *DrachFlag;
		int DrachPlayer;
		int DrachMerchant;

	public:
		Creature *GetMerchant() { return pMerchant; }
		Creature *GetBargainer() { return pBargainer; }
		Creature *GetTarget() { return pTarget; }

		void SetMerchant(Creature *pCreature) { pMerchant = pCreature; }
		void SetBargainer(Creature *pCreature) { pBargainer = pCreature; }
		void SetTarget(Creature *pCreature) { pTarget = pCreature; }

		int AddMerchantItem(GameItem *ToAdd);
		int AddPlayerItem(GameItem *ToAdd);

		void Reset();
		void Transact();
		void CalculateValues();

		BarterWin(int NewID, int x, int y, int width, int height, Creature *pNewMerchant, Creature *pNewTarget);

		BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);

		int Command(int IDFrom, int Command, int Param);

		int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

};

#endif