#ifndef PARTY_H
#define PARTY_H

#define MAX_PARTY_MEMBERS		6
#define MAX_NUM_KEYS			64
#define BEST_WEAPON_SKILL		9990
#define BEST_NONWEAPON_SKILL	9991
#define BEST_MAGIC_SKILL		9992
#define BEST_DAMAGE				9993

#include "things.h"
#include "creatures.h"
#include "journal.h"


class MapLocator;

class MovePointer;

class Region;

class Party
{
private:

	MovePointer *MoveMarks[MAX_PARTY_MEMBERS];
		
	int RestActions[MAX_PARTY_MEMBERS];
	
	Creature *pMembers[MAX_PARTY_MEMBERS];
	int NumMembers;

	Creature *pLeader;

	int Keys[MAX_NUM_KEYS];

	Region *pRegions[MAX_PARTY_MEMBERS];

	//journal
	Journal PartyJournal;

	BOOL RegionChange;
	BOOL AllInside;

	MapLocator *pLocations;

public:

	int GetRestAction(int Num) { return RestActions[Num]; };
	void SetRestAction(int Num, int NewAction) { RestActions[Num] = NewAction; }

	Creature *GetLeader() { return pLeader; }
	
	Creature *GetMember(int Num) { return pMembers[Num]; }

	int GetNumMembers() { return NumMembers; }

	BOOL IsMember(Creature *ToTest);
	BOOL IsLeader(Creature *ToText);
	int GetMemberNum(Creature *ToGet);

	BOOL AddMember(Creature *ToAdd);

	BOOL RemoveMember(Creature *ToRemove);

	BOOL SetLeader(Creature *pNewLeader);

	void LoadDefaultParty(const char *Name);
		
	Creature *GetBest(int StatNum);
	int GetAverage(int StatNum);
	Creature *GetWorst(int StatNum);

	BOOL Give(Thing *pToGive, int Quantity = 1);
	BOOL Has(Thing *pToCheck);
	BOOL Take(Thing *pToTake, int Quantity = 1);

	void SetMember(int Num, Creature *pNew) { pMembers[Num] = pNew; }

	Party();

	Party(FILE *fp);
	void Save(FILE *fp);
	void Load(FILE *fp);

	BOOL HasKey(int Num);
	void AddKey(int Num);
	void TakeKey(int Num);

	Journal *GetJournal() { return &PartyJournal; }
		
	void MoveParty(int xTo, int yTo); 

	void Teleport(int xTo, int yTo);

	void Occupy();
	void UnOccupy();

	void ChangedPosition() { RegionChange = TRUE; }
	BOOL HasChangedPosition() { return RegionChange; }

	BOOL Inside() { return AllInside; }

	BOOL AddLocation(char *Tag, int x, int y, BOOL Tele = FALSE, int MapNum = 0, int MapX = 0, int MapY = 0);
	void RemoveLocation(char *Tag);
	void RemoveLocation(int x, int y);
	void ClearLocations();
	void ClearJournal();

	MapLocator *GetLocations() { return pLocations; };
	
	void RemoveMarker(MovePointer *pRemove);

};

extern Party PreludeParty;


#endif