#ifndef CREATEPARTYWIN_H
#define CREATEPARTYWIN_H

#include "zswindow.h"
#include "script.h"
#include "creatures.h"

#define CREATE_PARTY_ID			20202
#define CREATE_CHARACTER_ID	30303
#define NUM_CREATE_MEMBERS		3

class CharacterBackground
{
public:
	char *Name;
	char *Description;
	ScriptBlock Requirements;
	ScriptBlock SelectResult;
	ScriptBlock Finished;

	CharacterBackground() { Name = NULL; Description = NULL; }
	~CharacterBackground() { if(Name) delete Name; if(Description) delete Description; }

};



class CreatePartyWin	: public ZSWindow
{
private:
	Creature *pMembers[NUM_CREATE_MEMBERS];
		
public:

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
			
	CreatePartyWin();

};


class CreateCharacterWin : public ZSWindow
{
private:

	CharacterBackground *Backgrounds;
	int NumBackgrounds;
	int EndSkill;
	int NumCombat;
	Creature *pFlameBase;
	Creature *pRiverBase;
	Creature *pMixedBase;
	Creature *pTarget;
	int CreationPoints;
	int PortraitNum;
	int StartSkill;
	int NumSkills;
	int StoredStats[10];
	int MaxFree;
	int MaxCombat;
	int MaxNon;
	int CurFree;
	int CurCombat;
	int CurNon;
	int NumNonMagic;
	int StoredSkills[32];

	void RaiseStat(int Num);
	void LowerStat(int Num);
	void ChangeRace();
	void GetValidProfessions();
	void SetPortrait();
	float HeightVariance;
	void HideSkills();
	void ShowSkills();
	void StoreTarget();
	void ResetTarget();
	void AddSkill(int Num);
	void RemoveSkill(int Num);

	ScriptBlock GetSkillValueBlock;


public:

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);
	void SetTarget(Creature *NewTarget);
	void Show();
	
	CreateCharacterWin(Creature *pToCreate);

};

#endif