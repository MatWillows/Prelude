#ifndef SPELL_FUNCS_H
#define SPELL_FUNCS_H

class Object;

typedef enum
{
	SPELL_CALL_CAST,
	SPELL_CALL_END,
	SPELL_CALL_ADVANCE
} SPELL_CALL_TYPES;

//flame spells
int FlameFinger(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int FireBallSpell(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int FlamingHands(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Berserk(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Haste(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Flash(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Immolation(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Spear(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Sentinel(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int ImmolationActivate(Object *pBase, int Level, int Damage);

//river spells
int MothersHeal(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Slow(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Shield(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Growth(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Visage(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Weaken(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Strength(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Root(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Cycle(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);

//Thaumaturgy
int LightningStone(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int KnockoutDust(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int SharpWeapon(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int FastWeapon(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int Golem(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int HardenArmour(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
int FixEnchantment(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);








#endif