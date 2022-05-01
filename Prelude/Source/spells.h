#ifndef SPELLS_H
#define SPELLS_H


#include "objects.h"

typedef enum
{
	SPELL_FLAME,
	SPELL_RIVER,
	SPELL_THAUMATURGY,
	SPELL_DEATH,
	SPELL_OTHER,
} SPELL_T;

typedef enum
{
	SPELL_TARGET_NONE = -1,
	SPELL_TARGET_ANYWHERE,
	SPELL_TARGET_FRIEND,
	SPELL_TARGET_ENEMY,
	SPELL_TARGET_ITEM,
	SPELL_TARGET_SELF,
	SPELL_TARGET_CREATURE,
	SPELL_TARGET_EMPTY,
} SPELL_TARGET_T;

class SpellLevel
{
public:
	char Name[32];
	int RestCost;
	int HitCost;
	int WillCost;
	int SpeedCost;
	float MinRange;
	float MaxRange;
	int SkillReq;
	int Duration;
	int Combat;
	int TargetType;
	char *Description;
	int NeedBlood;
	int NeedGoddess;
	int ConfirmationEvent;

	SpellLevel()
	{
		Name[0] = '\0';
		RestCost = 0;
		HitCost = 0;
		WillCost = 0;
		SpeedCost = 0;
		MinRange = 0.0f;
		MaxRange = 0.0f;
		SkillReq = 0;
		Duration = 0;
		Combat = 0;
		TargetType = 0;
		Description = NULL;
		NeedBlood = 0;
		NeedGoddess = 0;
		ConfirmationEvent = 0;
	}
	
	BOOL Load(FILE *fp);
	BOOL LoadText(FILE *fp);
	BOOL Save(FILE *fp);

	~SpellLevel()
	{
		if(Description)
			delete[] Description;
	}

};

class Spell
{
private:
	int ID;
	int Type;
	int NumLevels;
	SpellLevel *Levels;
	char *Description;

public:
	BOOL Load(FILE *fp);
	BOOL LoadText(FILE *fp);
	BOOL Save(FILE *fp);

	int GetID() { return ID; }
	int GetType() { return Type; }
	int GetNumLevels() { return NumLevels; }
	SpellLevel *GetLevel(int Num) { return &Levels[Num]; }
	char *GetDescription() { return Description; }

	friend class SpellList;

	Spell()
	{ 
		ID = -1;
		Type = -1;
		NumLevels = 0;
		Levels = NULL;
		Description = NULL;
	}
	
	~Spell()
	{
		if(Levels)
			delete[] Levels;
		if(Description)
			delete[] Description;
	}

};

class Flag;

class SpellList 
{
private:
	int NumSpells;
	Spell *Spells;
	Flag *GoddessAngered;
	Flag *BloodKnown;
	
public:
	int GetNumSpells() { return NumSpells; }

	BOOL Load(const char *filename);
	BOOL Save(const char *filename);
	
	//cast a spell, pretty much just call the cast script routine
	BOOL Cast(int Num, int Level, Object *pCaster, Object *pTarget);
	
	//test a spell to see if it is castable on a given target
	BOOL Test(int Num, int Level, Object *pCaster, Object *pTarget);

	//for spells which must continually go off
	int AdvanceSpell(int Num, int Level, int Tick, Object *pCaster, Object *pTarget);

	Spell *GetSpell(int Num)
	{
		return &Spells[Num]; 
	}

	SpellList();

	void Init(); 

	~SpellList()
	{
		if(Spells)
		{
			delete[] Spells;
		}
	}

};

//spelltracker is used to track spells which have a duration

class SpellTracker : public Object
{
public:
	Object *pCaster;
	Object *pTarget;
	int CurTick;
	ULONG LastCall;
	int Duration;
	int SpellNum;
	int SpellLevel;

	SpellTracker(Object *pCast, Object *pTarg, int SNum, int LNum);

	BOOL AdvanceFrame();

	void Draw() { return; }

};

extern int (*SpellFunctions[MAX_SPELLS])(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
extern SpellList PreludeSpells;

#endif