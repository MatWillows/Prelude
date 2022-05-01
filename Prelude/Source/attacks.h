#ifndef ATTACKS_H
#define ATTACKS_H
#include "creatures.h"
#include "items.h"

#define SPEED_MULTIPLIER	2
#define FEINT_BONUS			50

#define MELEE_STRONG_TOHIT_MODIFIER		(-10)
#define MELEE_RAPID_TOHIT_MODIFIER		(-10)
#define MELEE_AIMED_TOHIT_MODIFIER		(10)
#define MELEE_FEINT_TOHIT_MODIFIER		(0)

#define MISSILE_RAPID_TOHIT_MODIFIER	(-25)
#define MISSILE_AIMED_TOHIT_MODIFIER	(10)

#define MELEE_STRONG_DAMAGE_MULTIPLIER		(1.5)
#define MELEE_RAPID_DAMAGE_MULTIPLIER		(0.8)
#define MELEE_AIMED_DAMAGE_MULTIPLIER		(1.2)
#define MELEE_FEINT_DAMAGE_MULTIPLIER		(0)

#define MISSILE_RAPID_DAMAGE_MULTIPLIER	(0.75)
#define MISSILE_AIMED_DAMAGE_MULTIPLIER	(1.25)

#define CRITICAL_AIMED_MULTIPLIER		(0.25)
#define CRITICAL_NORMAL_MULTIPLIER		(0.10)

#define CRITICAL_DAMAGE_MULTIPLIER		(2)

#define BLOW_LANDED_HANDS_CHANCE		4
#define BLOW_LANDED_FEET_CHANCE			8
#define BLOW_LANDED_HEAD_CHANCE			20
#define BLOW_LANDED_BODY_CHANCE			70
#define BLOW_LANDED_LEGS_CHANCE			100

#define STRONG_AP_MULTIPLIER	1.25f		
#define AIMED_AP_MULTIPLIER	1.333f
#define QUICK_AP_MULTIPLIER	0.75f
#define FEINT_AP_MULTIPLIER	1.333f	
#define SPECIAL_AP_MULTIPLIER 1.25f

typedef enum
{
	BLOW_LANDED_HEAD,
	BLOW_LANDED_BODY,
	BLOW_LANDED_LEGS,
	BLOW_LANDED_FEET,
	BLOW_LANDED_HANDS,
} BLOW_LANDED_T;

int GetBaseToHit(Thing *Attacker, Thing *Defender);
BOOL Parried(Creature *pAttacker, Creature *pDefender);
void RapidAttack(Creature *pAttacker, Creature *pDefender);
void SpinAttack(Creature *pAttacker, Creature *pDefender);
void RendAttack(Creature *pAttacker, Creature *pDefender);
void BrainAttack(Creature *pAttacker, Creature *pDefender);
void SweepAttack(Creature *pAttacker, Creature *pDefender);
void TargettedAttack(Creature *pAttacker, Creature *pDefender);
void DisarmAttack(Creature *pAttacker, Creature *pDefender);
int AbsorbBlow(Creature *pAttacker, Creature *pDefender, int Damage, int AbsorbModifier = 100);








#endif