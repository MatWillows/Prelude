#ifndef MODIFIERS_H
#define MODIFIERS_H

//modifiers
// used for spells or spell-like effects that have a duration
// modifier is always a fixed amount, but it may be calculated off a percentage base
// if the base changes however, the modifier does not

#include "objects.h"

typedef enum
{
	MOD_NONE,
	MOD_STAT,
	MOD_AURA,
	MOD_ITEM,
	MOD_SPELL,
} MOD_T;

class Modifier : public Object
{
private:

	//type of modifier
	MOD_T ModType;  //type, 99% spell
	BOOL Combat;  //combat or not
	unsigned long Start; //start round if combat, start minute if not
	unsigned long Duration; //duration in rounds of combat or in minutes
	int Progression; //progressive change to stat on a per round or per half hour basis
	int Stat; //the stat being modified
	int Amount;	//the amount of the modifier
	Object *pAura; //magical effect displayed, usually on the target
	Object *pSource; //source of the modifier
	Object *pTarget; //creature being affected
	int SpellNum; //spell that activated it.

public:
//accesssors
	MOD_T GetModType() { return ModType; }
	BOOL IsCombat() { return Combat; }
	unsigned long GetStart() { return Start; }
	unsigned long GetDuration() { return Duration; }
	int GetProgression() { return Progression; }
	int GetAmount() { return Amount; }
	Object *GetSource() { return pSource; }
	Object *GetTarget() { return pTarget; }
	Object *pGetAura() { return pAura; }
	int GetStat() { return Stat; }
	OBJECT_T GetObjectType() { return OBJECT_MODIFIER; }

//mutators
	void SetModType(MOD_T NewType) { ModType = NewType; }
	void SetCombat(BOOL NewCombat) { Combat = NewCombat; }
	void SetStart(unsigned long NewStart) { Start = NewStart; }
	void SetDuration(unsigned long NewDuration) { Duration = NewDuration; }
	void SetProgression(int NewProgression) { Progression = NewProgression; }
	void SetAmount(int NewAmount) { Amount = NewAmount; }
	void SetSource(Object *pNewSource) { pSource = pNewSource; }
	void SetTarget(Object *pNewTarget) { pTarget = pNewTarget; }
	void SetAura(Object *pNewAura) { pAura = pNewAura; }
	void SetStat(int NewStat)  { Stat = NewStat; }

	void SetUp(MOD_T mtype, unsigned long lStart, unsigned long lDuration, int prog, int modamount, Object *pSource, Object *pTarget, int Stat);

	Modifier();

	BOOL AdvanceFrame();

	void Remove();
	void Apply();

//IO
	void Save(FILE *fp);
	void Load(FILE *fp);

};

#endif