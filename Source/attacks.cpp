#include "attacks.h"
#include "world.h"
#include "combatmanager.h"
#include "gameitem.h"
#include "party.h"
#include "mainwindow.h"
#include "modifiers.h"
#include "zsparticle.h"
#include "zsWeapontrace.h"
#include "missile.h"
#include "items.h"


//calculate base chance of succes with an attack, before any modifiers based on the type of attack
int GetBaseToHit(Thing *Attacker, Thing *Defender)
{
	int BaseToHit;

	switch(PreludeWorld->GetDifficulty())
	{
	default:
	case 1:
		BaseToHit = 80;
		break;
	case 0: //easy
		if(PreludeParty.IsMember((Creature *)Attacker))
		{
			BaseToHit = 90;
		}
		else
		{
			BaseToHit = 70;
		}
		break;
	case 2: //hard
		if(PreludeParty.IsMember((Creature *)Attacker))
		{
			BaseToHit = 60;
		}
		else
		{
			BaseToHit = 85;
		}
		break;
	}
	//all things equal there is a 80% chance to hit
	//get the type of the attacker's weapon

	//modify base to hit by speed	
	int AttackSpeed;
	int DefendSpeed;

	DefendSpeed = Defender->GetData(INDEX_SPEED).Value;
	AttackSpeed = Attacker->GetData(INDEX_SPEED).Value;

	BaseToHit += (AttackSpeed - DefendSpeed) * SPEED_MULTIPLIER;

	int AttackSkill = 0;
	int DefendSkill = 0;
	int AttackDex = 0;
	int DefendDex = 0;
	
	AttackSkill = ((Creature *)Attacker)->GetWeaponSkill();
	AttackDex = Attacker->GetData(INDEX_DEXTERITY).Value;
	
	DefendSkill = ((Creature *)Defender)->GetWeaponSkill();
	DefendDex = Defender->GetData(INDEX_DEXTERITY).Value;

	BaseToHit += (AttackSkill - DefendSkill);

	float DexOffset = 1.0f + ((float)(AttackDex - 13) / 20.0f);

	BaseToHit = (int)(((float)BaseToHit * DexOffset) + 0.5f);

	if(Defender->GetData(INDEX_FEINT).Value)
	{
		if(Defender->GetData(INDEX_FEINT).Value > 0)
		{
			Defender->SetData(INDEX_FEINT,0);
		}
		BaseToHit += FEINT_BONUS;
	}

#ifndef NDEBUG

	char Blarg[256];

	sprintf(Blarg,"%s attacking %s base to hit of %i\n", Attacker->GetData(INDEX_NAME).String, Defender->GetData(INDEX_NAME).String, BaseToHit);
	DEBUG_INFO(Blarg);

#endif

	int WillPowerModifier;

	//check for additional characters
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		WillPowerModifier = 10 + (13 - Defender->GetData(INDEX_WILLPOWER).Value);
		if(WillPowerModifier > 0)
		{
			WillPowerModifier *= 2;
			float fDistance;
			int DefenderSide;
			DefenderSide = Defender->GetData(INDEX_BATTLESIDE).Value;
			Creature *pCombatant;
			pCombatant = (Creature *)PreludeWorld->GetCombat()->GetCombatants();
			while(pCombatant)
			{
				if(pCombatant != Attacker && pCombatant != Defender)
				{
					fDistance = GetDistance(pCombatant, (Creature *)Defender);
					if(fDistance <= 1.5f)
					{
						if(pCombatant->GetData(INDEX_BATTLESIDE).Value != DefenderSide)
						{
							BaseToHit += WillPowerModifier;
						}
						else
						{
							BaseToHit -= WillPowerModifier / 2;
						}
					}
				}
			
				pCombatant = (Creature *)pCombatant->GetNextUpdate();
			}
			
		}
	}

	return BaseToHit;
}

BOOL Parried(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI = NULL;
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	GameItem *pGIShield = NULL;
	int FieldIndex;
	
	//check for a shield block
	//does the defender have a shield?
	pGIShield = pDefender->GetEquipment("LEFTHAND");
	if(pGIShield && pDefender->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS)
	{
		if(pGIShield->GetData("ITEMTYPE").Value == ITEM_TYPE_ARMOR)
		{
			//yup, they do
			//can they block the hit?
			//chance % is (defender's armor skill - (attacker's skill - defender's armor skill))/2
			int BlockPercent  = (pDefender->GetData(INDEX_ARMOR).Value - (pAttacker->GetWeaponSkill() - pDefender->GetData(INDEX_ARMOR).Value))/2;
			BlockPercent += pGIShield->GetData("MAXDAMAGE").Value;
			int BlockRoll = rand() % 100;
			if(BlockRoll < BlockPercent)
			{
				//do shield blocked result
				Describe("Deflected.");
				Engine->Sound()->PlayEffect(12);
				if(PreludeParty.IsMember(pDefender))
					pDefender->ImproveSkill(INDEX_ARMOR);
				return TRUE;
			}
		}
		else
		if(pGIShield->GetData("ITEMTYPE").Value == ITEM_TYPE_WEAPON)
		{
			if(!pGI || !(pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE))
			{
			//off hand weapon used for parrying
				int BlockPercent = ((pDefender->GetData(INDEX_ARMOR).Value - (pAttacker->GetWeaponSkill() - pDefender->GetData(INDEX_ARMOR).Value))/2) / 3;
				int BlockRoll = rand() % 100;
				if(BlockRoll < BlockPercent)
				{
					//do shield blocked result
					Describe("Parried by off hand.");
					Engine->Sound()->PlayEffect(11);
					return TRUE;
				}
			}
		}
	}

	//can not parry missile weapons
	if(pGI && pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
	{
		return FALSE;
	}

	//check for a parry
	//chance to parry is difference of defender's and attacker's skill
	int DefenderSkill;
	DefenderSkill = pDefender->GetWeaponSkill();
	int AttackSkill;
	AttackSkill = pAttacker->GetWeaponSkill();
	int ParryPercent;
	int ParryRoll;
	int DefenderWeaponType;
	GameItem *pGIDefenderWeapon;
	pGIDefenderWeapon = pDefender->GetEquipment("RIGHTHAND");
	if(pGIDefenderWeapon && pGIDefenderWeapon->GetData(INDEX_TYPE).Value == ITEM_TYPE_WEAPON)
	{
		DefenderWeaponType = pGIDefenderWeapon->GetData("SUBTYPE").Value;	

		if(DefenderWeaponType != WEAPON_TYPE_MISSILE &&
			DefenderWeaponType != WEAPON_TYPE_THROWN)
		{
			ParryPercent = DefenderSkill - AttackSkill;

			if((DefenderWeaponType == WEAPON_TYPE_SWORD ||
				DefenderWeaponType == WEAPON_TYPE_POLEARM) &&
				DefenderSkill >= 20)
			{
				ParryPercent += 10;
			}

			ParryRoll = rand() % 100;
/*
#ifndef NDEBUG
	sprintf(Blarg,"Parry change: %i  and parry roll: %i\n",ParryPercent, ParryRoll);
	DEBUG_INFO(Blarg);
#endif
*/		
			if(ParryRoll < ParryPercent && pDefender->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS)
			{
				//do parry result
				Describe("Parried");
				Engine->Sound()->PlayEffect(11);
				if(PreludeParty.IsMember(pDefender))
				{
					switch(pGIDefenderWeapon->GetData("SUBTYPE").Value)
					{
					case WEAPON_TYPE_SWORD:
						FieldIndex = INDEX_SWORD;
						break;
					case WEAPON_TYPE_DAGGER:
						FieldIndex = INDEX_DAGGER;
						break;
					case WEAPON_TYPE_AXE:
						FieldIndex = INDEX_AXE;
						break;
					case WEAPON_TYPE_BLUNT:
						FieldIndex = INDEX_BLUNT;
						break;
					case WEAPON_TYPE_POLEARM:
						FieldIndex = INDEX_POLEARM;
						break;
					case WEAPON_TYPE_MISSILE:
						FieldIndex = INDEX_MISSILE;
						break;
					default:
					case WEAPON_TYPE_UNARMED:
						FieldIndex = INDEX_UNARMED;
						break;
					case WEAPON_TYPE_THROWN:
						FieldIndex = INDEX_THROWING;
						break;
					}
					pDefender->ImproveSkill(FieldIndex);
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}


void RapidAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	float CurAngle;
	float DestAngle;
	CurAngle = pAttacker->GetMyAngle();
	DIRECTION_T facing;
	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
	{
		pAttacker->InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
		
		do
		{
			((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
			pCurAction = pAttacker->GetAction();
		}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);
	}		

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");

	if(PreludeParty.IsMember(pAttacker))
		pAttacker->ImproveSkill(INDEX_DAGGER);

	int AdditionalChance = 200;
	int AdditionalRoll = 0;
	int NumAttacks = 0;
	int NumHits = 0;

do
{
	if(NumAttacks) AdditionalChance = 50;
	NumAttacks ++;
	if(pGI)
	{
		int AnimRange;
		AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
		if(AnimRange)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}
	
	WeaponTracer *pTrace;
	pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_YELLOW);
	PreludeWorld->AddMainObject(pTrace);
	pTrace->AdvanceFrame();

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pTrace);

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(pAttacker, pDefender);

	ToHit -= NumAttacks * 5;

	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

	if(AttackRoll < ToHit)
	{
		NumHits++;
		if(Parried(pAttacker, pDefender))
		{
			return;
		}
		
		int BaseDamage;
		
		BaseDamage = pAttacker->GetDamage(FALSE);

		//modify by strength
		float FinDamage;

		float DamagePercent;
		DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

		FinDamage = DamagePercent * (float)BaseDamage;
		FinDamage = FinDamage - (FinDamage * (0.06f * (float)NumAttacks));

		BaseDamage = (int)(FinDamage + 0.5f); 

		BaseDamage = AbsorbBlow(pAttacker, pDefender, BaseDamage);

		if(BaseDamage <= 0)
		{
			//do miss result
		}
		else
		{
			//hit!
			if(pGI)
			{
				if(pGI->GetData("FX1").Value)
				{
					Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
				}
				else	
				{
					Engine->Sound()->PlayEffect(6);
				}
			}
			else
			{
				if(pAttacker->GetData(INDEX_TYPE).Value)
				{
					Engine->Sound()->PlayEffect(8);
				}
				else
				{
					Engine->Sound()->PlayEffect(6);
				}
			}

			pDefender->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

			if(PreludeParty.IsMember(pDefender))
				pDefender->ImproveSkill(INDEX_ARMOR);
		}
	}
	else
	{
		Engine->Sound()->PlayEffect(10);
	}
	AdditionalRoll = rand() % 100;
}while(AdditionalRoll < AdditionalChance && pDefender->GetData(INDEX_HITPOINTS).Value > 0 && PreludeWorld->InCombat());

	if(NumHits <= 0)
	{
		//do miss result
		Describe("Failed.");
	}

	return;
}

void SpinAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	
	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int Anim = PUNCH_ATTACK;
	if(pGI)
	{
		Anim = pGI->GetData("ANIMATIONRANGE").Value;
		if(Anim)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)Anim, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}

	WeaponTracer *pTrace;
	pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_WHITE);
	PreludeWorld->AddMainObject(pTrace);
	pTrace->AdvanceFrame();

	int NumFrames;
	NumFrames = pAttacker->GetAnimRange(pAttacker->GetData(INDEX_TYPE).Value % 10)->GetAnim((ANIMATION_RANGE_T)Anim)->EndFrame -
		pAttacker->GetAnimRange(pAttacker->GetData(INDEX_TYPE).Value % 10)->GetAnim((ANIMATION_RANGE_T)Anim)->StartFrame;

	float AngleOff;
	AngleOff = PI_MUL_2 / (float)NumFrames;
	
	pAttacker->SetAngleOff(AngleOff);

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pTrace);
	delete pTrace;

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	Creature *pCreature;
	Object *pOb;
	pOb = PreludeWorld->GetCombat()->GetCombatants();
	int NumAttacks = 100;
	int NumHits = 0;

	if(PreludeParty.IsMember(pAttacker))
		pAttacker->ImproveSkill(INDEX_SWORD);

while(pOb)
{
	if(pOb->GetObjectType() == OBJECT_CREATURE)
	{
		pCreature = (Creature *)pOb;
	if(pCreature->GetData(INDEX_BATTLESIDE).Value != pAttacker->GetData(INDEX_BATTLESIDE).Value)
	{
		float fDistance;
		fDistance = GetDistance(pAttacker, pCreature);
	

	if(fDistance <= pAttacker->GetRange())
	{
		//calculate base to hit
		ToHit = GetBaseToHit(pAttacker, pCreature);
	
		ToHit = ToHit * 100 / NumAttacks;
		
		//penalty per opponent or so 
		NumAttacks += 40;
		
		AttackRoll = rand() % 100;

		if(pCreature->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
		{
			AttackRoll = 0;
		}

	
		if(AttackRoll < ToHit)
		{
			NumHits++;
			if(Parried(pAttacker, pCreature))
			{
				return;
			}
			
			int BaseDamage;
			
			BaseDamage = pAttacker->GetDamage(FALSE);

			//modify by strength

			float DamagePercent;
			DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

			float FinDamage;

			FinDamage = DamagePercent * (float)BaseDamage;
		
			BaseDamage = (int)(FinDamage + 0.5f); 

			BaseDamage = AbsorbBlow(pAttacker, pCreature, BaseDamage);

			if(BaseDamage <= 0)
			{
				return;
			}

			//hit!
			if(pGI)
			{
				if(pGI->GetData("FX1").Value)
				{
					Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
				}
				else	
				{
					Engine->Sound()->PlayEffect(6);
				}
			}
			else
			{
				if(pAttacker->GetData(INDEX_TYPE).Value)
				{
					Engine->Sound()->PlayEffect(8);
				}
				else
				{
					Engine->Sound()->PlayEffect(6);
				}
			}
			
			((Creature *)pCreature)->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

			pCreature->ImproveSkill(INDEX_ARMOR);

			pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);
		}
	}
	}
	}
	pOb = pOb->GetNextUpdate();
}
	if(NumHits)
	{
		
	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	return;
}

void RendAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	float CurAngle;
	float DestAngle;
	CurAngle = pAttacker->GetMyAngle();
	DIRECTION_T facing;
	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
	{
		pAttacker->InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
		
		do
		{
			((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
			pCurAction = pAttacker->GetAction();
		}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);
	}		

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		int AnimRange;
		AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
		if(AnimRange)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}
	
	WeaponTracer *pTrace;
	pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_RED);
	PreludeWorld->AddMainObject(pTrace);
	pTrace->AdvanceFrame();

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pTrace);

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(pAttacker, pDefender);


	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

	if(PreludeParty.IsMember(pAttacker))
		pAttacker->ImproveSkill(INDEX_AXE);

	if(AttackRoll < ToHit)
	{
		if(Parried(pAttacker, pDefender))
		{
			return;
		}
		
		int BaseDamage;
	
		BaseDamage = pAttacker->GetDamage(FALSE);

		//modify by strength

		float DamagePercent;
		DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

		float FinDamage;

		FinDamage = DamagePercent * (float)BaseDamage;
		FinDamage *= 1.25;

		BaseDamage = (int)(FinDamage + 0.5f); 

		BaseDamage = AbsorbBlow(pAttacker, pDefender, BaseDamage);

		if(BaseDamage <= 0)
		{
			return;
		}

		//hit!
		if(pGI)
		{
			if(pGI->GetData("FX1").Value)
			{
				Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
			}
			else	
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		else
		{
			if(pAttacker->GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}

		ToHit /= 2;

		if(AttackRoll < ToHit)
		{
			//do miss result
			Describe("Blood begins to gush from the wounds!");
			//add the mod
			Modifier *pMod;
			pMod = new Modifier;
			PreludeWorld->GetCombat()->AddMod(pMod);
			pMod->SetAura(NULL);
			pMod->SetTarget(pDefender);
			pMod->SetDuration(BaseDamage / 4);
			pMod->SetStat(INDEX_HITPOINTS);
			pMod->SetAmount((BaseDamage / 2) + 1);
			pMod->SetProgression(-2);
		
			pMod->Apply();
		}
		else
		{

		}		
		((Creature *)pDefender)->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

		pDefender->ImproveSkill(INDEX_ARMOR);

		pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);

		
	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	return;
}

void BrainAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	float CurAngle;
	float DestAngle;
	CurAngle = pAttacker->GetMyAngle();
	DIRECTION_T facing;
	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
	{
		pAttacker->InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
		
		do
		{
			((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
			pCurAction = pAttacker->GetAction();
		}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);
	}		

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		int AnimRange;
		AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
		if(AnimRange)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}
	
	WeaponTracer *pTrace;
	pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_GREEN);
	PreludeWorld->AddMainObject(pTrace);
	pTrace->AdvanceFrame();

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pTrace);

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(pAttacker, pDefender);

	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

	if(PreludeParty.IsMember(pAttacker))
		pAttacker->ImproveSkill(INDEX_BLUNT);

	if(AttackRoll < ToHit)
	{
		if(Parried(pAttacker, pDefender))
		{
			return;
		}
		
		int BaseDamage;
	
		BaseDamage = pAttacker->GetDamage(FALSE);

		//modify by strength

		float DamagePercent;
		DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

		float FinDamage;

		FinDamage = DamagePercent * (float)BaseDamage;
		FinDamage *= 1.5;

		BaseDamage = (int)(FinDamage + 0.5f); 

		BaseDamage = AbsorbBlow(pAttacker, pDefender, BaseDamage);

		if(BaseDamage <= 0)
		{
			//do miss result
			Describe("Failed.");
			Engine->Sound()->PlayEffect(10);
			return;
		}

		//hit!
		if(pGI)
		{
			if(pGI->GetData("FX1").Value)
			{
				Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
			}
			else	
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		else
		{
			if(pAttacker->GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}

		ToHit /= 2;

		if(AttackRoll < ToHit)
		{
			int APLost;
			int HalfBase;
			HalfBase = (BaseDamage + 1) / 2;
			APLost = (rand() % HalfBase) + HalfBase;

			pDefender->SetData(INDEX_ACTIONPOINTS, pDefender->GetData(INDEX_ACTIONPOINTS).Value - APLost);
			char blarg[64];
			sprintf(blarg,"%s looks stunned.",pDefender->GetData(INDEX_NAME).String);
			Describe(blarg);
		}
		
		pDefender->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

		if(PreludeParty.IsMember(pDefender))
			pDefender->ImproveSkill(INDEX_ARMOR);

		pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);

	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	return;
}

void SweepAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;

	//otherwise
	float CurAngle;
	float DestAngle;
	DIRECTION_T facing;
	int xn;
	int yn;
	
	CurAngle = pAttacker->GetMyAngle();

	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	D3DVECTOR *pPosition;
	pPosition = pAttacker->GetPosition();

	int x = (int)pPosition->x;
	int y = (int)pPosition->y;

	//move in the correct direction
	switch(facing)
	{
		case NORTH:
			xn = x;
			yn = y - 1;
			break;
		case SOUTH:
			xn = x;
			yn = y + 1;
			break;
		case EAST:
			xn = x+1;
			yn = y;
			break;
		case WEST:
			xn = x-1;
			yn = y;
			break;
		case NORTHEAST:
			xn = x+1;
			yn = y - 1;
			break;
		case SOUTHEAST:
			xn = x+1;
			yn = y + 1;
			break;
		case NORTHWEST:
			xn = x-1;
			yn = y - 1;
			break;
		case SOUTHWEST:
			xn = x-1;
			yn = y+1;
			break;
		default:
			break;
	}//switch

	RECT rSweepArea;
	rSweepArea.left = xn - 1;	
	rSweepArea.right = xn + 1;
	rSweepArea.top = yn - 1;
	rSweepArea.bottom = yn + 1;


	DestAngle -= PI_DIV_2;

	pAttacker->SetAngle(DestAngle);
	pAttacker->SetData(INDEX_ROTATION, DestAngle);

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int Anim = PUNCH_ATTACK;
	if(pGI)
	{
		Anim = pGI->GetData("ANIMATIONRANGE").Value;
		if(Anim)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)Anim, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}

	WeaponTracer *pTrace;
	pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_PURPLE);
	PreludeWorld->AddMainObject(pTrace);
	pTrace->AdvanceFrame();

	int NumFrames;
	NumFrames = pAttacker->GetAnimRange(pAttacker->GetData(INDEX_TYPE).Value % 10)->GetAnim((ANIMATION_RANGE_T)Anim)->EndFrame -
		pAttacker->GetAnimRange(pAttacker->GetData(INDEX_TYPE).Value % 10)->GetAnim((ANIMATION_RANGE_T)Anim)->StartFrame;

	float AngleOff;
	AngleOff = PI / (float)NumFrames;
	
	pAttacker->SetAngleOff(AngleOff);

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pTrace);
	delete pTrace;

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	Creature *pCreature;
	Object *pOb;
	pOb = PreludeWorld->GetCombat()->GetCombatants();
	int NumAttacks = 100;
	int NumHits = 0;

	
while(pOb)
{
	if(pOb->GetObjectType() == OBJECT_CREATURE)
	{
		pCreature = (Creature *)pOb;
	if(pCreature->GetData(INDEX_BATTLESIDE).Value != pAttacker->GetData(INDEX_BATTLESIDE).Value)
	{
		pPosition = pCreature->GetPosition();
		x = (int)pPosition->x;
		y = (int)pPosition->y;
			
	if((pCreature->IsLarge() &&  x >= rSweepArea.left - 1 && x <= rSweepArea.right && y >= rSweepArea.top - 1 && y <= rSweepArea.left) || (!pCreature->IsLarge() && x >= rSweepArea.left && x <= rSweepArea.right && y >= rSweepArea.top && y <= rSweepArea.left))
	{
		//calculate base to hit
		ToHit = GetBaseToHit(pAttacker, pCreature);
	
		if(pCreature != pDefender)
			ToHit = ToHit * 80 / 100;
		
		//penalty per opponent or so 
		NumAttacks += 40;
		
		AttackRoll = rand() % 100;

		if(pCreature->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
		{
			AttackRoll = 0;
		}

		if(AttackRoll < ToHit)
		{
			NumHits++;
			if(Parried(pAttacker, pCreature))
			{
				return;
			}
			
			int BaseDamage;
			
			BaseDamage = pAttacker->GetDamage(FALSE);

			//modify by strength

			float DamagePercent;
			DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

			float FinDamage;

			FinDamage = DamagePercent * (float)BaseDamage;
		
			if(pCreature != pDefender)
			{
				FinDamage *= 0.8f;
			}

			BaseDamage = (int)(FinDamage + 0.5f); 

			BaseDamage = AbsorbBlow(pAttacker, pCreature, BaseDamage);

			if(BaseDamage <= 0)
			{
				return;
			}

			//hit!
			if(pGI)
			{
				if(pGI->GetData("FX1").Value)
				{
					Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
				}
				else	
				{
					Engine->Sound()->PlayEffect(6);
				}
			}
			else
			{
				if(pAttacker->GetData(INDEX_TYPE).Value)
				{
					Engine->Sound()->PlayEffect(8);
				}
				else
				{
					Engine->Sound()->PlayEffect(6);
				}
			}
			
			((Creature *)pCreature)->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

			if(PreludeParty.IsMember(pCreature))
				pCreature->ImproveSkill(INDEX_ARMOR);

			pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);
		}
	}
	}
	}
	pOb = pOb->GetNextUpdate();
}
	if(NumHits)
	{

	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	return;
}

void TargettedAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	float CurAngle;
	float DestAngle;
	CurAngle = pAttacker->GetMyAngle();
	DIRECTION_T facing;
	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
	{
		pAttacker->InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
		
		do
		{
			((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
			pCurAction = pAttacker->GetAction();
		}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);
	}		

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		int AnimRange;
		AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
		if(AnimRange)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}
	
	WeaponTracer *pTrace;
	if(pGI->GetItem()->GetData("SUBTYPE").Value != WEAPON_TYPE_THROWN)
	{
		pTrace = pAttacker->CreateWeaponParticleSystem(pAttacker, COLOR_WHITE);
		PreludeWorld->AddMainObject(pTrace);
		pTrace->AdvanceFrame();
	}

	int NumWait;
	Missile *pMissile;
	ZSModelEx *pMissileMesh = NULL;
	ParticleSystem *pSys1;
	BOOL OutOfThrown = FALSE;		
	
	if(pGI)
	{
		if(pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
		{
			//check for ammo
			int AmmoType;
			AmmoType = pGI->GetItem()->GetData("AMMOTYPE").Value;
			GameItem *pAmmo;
			
			pAmmo = pAttacker->GetEquipment("AMMO");
			if(AmmoType)
			{
				if(!pAmmo || pAmmo->GetItem()->GetData("AMMOTYPE").Value != AmmoType)
				{
					pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value + FinalApCost);
					Describe("Not enough Ammo.");
					return;
				}

				pMissileMesh = pAmmo->GetMesh();
				pAttacker->SetAmmoItemNumber(pAmmo->GetData(INDEX_ID).Value);
				if(pAmmo->GetQuantity() <= 1)
				{
					char dstring[64];
					sprintf(dstring,"%s is out of ammo.",pAttacker->GetData(INDEX_NAME).String);
					Describe(dstring);
					pAttacker->Unequip(pAmmo);
					delete pAmmo;
				}
				else
				{
					pAmmo->SetQuantity(pAmmo->GetQuantity() - 1);
				}
			}

			if(!pMissileMesh)
			{
				DEBUG_INFO("Missile Ammo has no mesh\n");
				pMissileMesh = Engine->GetMesh("arrow");
			}
		}
		else
		if(pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_THROWN)
		{
			pMissileMesh = pGI->GetMesh();
			
		
			if(pGI->GetQuantity() - 1 <= 0)
			{
				OutOfThrown = TRUE;
				char dstring[64];
				sprintf(dstring,"%s is out of %s.",pAttacker->GetData(INDEX_NAME).String,pGI->GetItem()->GetData(INDEX_NAME).String);
				Describe(dstring);
				pAttacker->Unequip(pGI);
			}

			pGI->SetQuantity(pGI->GetQuantity() - 1);


			if(!pMissileMesh)
			{
				DEBUG_INFO("thrown object does not have mesh\n");
				pMissileMesh = Engine->GetMesh("stonebullet");
			}
		}


		D3DVECTOR vStart;

		vStart = *pAttacker->GetData(INDEX_POSITION).pVector;

		vStart.z += (pAttacker->GetMesh()->GetHeight() * 0.75f);

		D3DVECTOR vEnd;

		vEnd = *pDefender->GetData(INDEX_POSITION).pVector;

		vEnd.z += (pDefender->GetMesh()->GetHeight() * 0.75f);

		pMissile = new Missile(pMissileMesh,Engine->GetTexture("items"),&vStart,&vEnd, 15.0f);

		pSys1 = new ParticleSystem();
		pSys1->SetLink(pMissile);
		pSys1->SetLinkPoint(0);
		pSys1->SetLife(2);
		pSys1->SetLifeVariance(0);
		pSys1->SetInitialVector(D3DVECTOR(0.0f,0.0f, 0.0f));
		pSys1->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
		pSys1->SetRadius(0.35f);
		pSys1->SetEmissionQuantity(1);
		//pSys->SetGravity(AuraGravity);
		
		
		//InsertAction(ACTION_MISSILE,(DWORD)pMissile,0);
		NumWait = pMissile->GetNumMoveFrames();

		pSys1->SetEnd(NumWait - 1);
		if(NumWait > 0)
		{
		//	Describe ("adding wait");
		//	InsertAction(ACTION_WAITUNTIL,(void *)NumWait,0, TRUE);
		}
		
	}


	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->AddMainObject((Object *)pMissile);
	PreludeWorld->AddMainObject(pSys1);
	pSys1->AdjustCamera();
	pSys1->AdvanceFrame();

	pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)NumWait,0, TRUE);

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);


	PreludeWorld->RemoveMainObject(pSys1);
	PreludeWorld->RemoveMainObject(pTrace);

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(pAttacker, pDefender);

	ToHit /= 2;

	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

	if(PreludeParty.IsMember(pAttacker))
	{
		if(pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
			pAttacker->ImproveSkill(INDEX_MISSILE);
		else
			pAttacker->ImproveSkill(INDEX_THROWING);
	}
	if(AttackRoll < ToHit)
	{
		if(Parried(pAttacker, pDefender))
		{
			if(OutOfThrown)
			{
				delete pGI;
			}
			return;
		}
		
		int BaseDamage;
		int MinDamage;
		int AmmoDamage = 0;
		int AmmoMin = 0;
		int AmmoRange = 1;
		MinDamage = pAttacker->GetData(INDEX_MINDAMAGE).Value;
		if(pAttacker->GetAmmoItemNumber())
		{
			Item *pItem;
			pItem = (Item *)Thing::Find((Thing *)Item::GetFirst(),pAttacker->GetAmmoItemNumber());
			if(pItem)
			{
				AmmoRange += (pItem->GetData("MAXDAMAGE").Value - pItem->GetData("MINDAMAGE").Value);
				AmmoMin = pItem->GetData("MINDAMAGE").Value;
				AmmoDamage = rand() % AmmoRange + AmmoMin;
			}

			pAttacker->SetAmmoItemNumber(0);
		}

		BaseDamage = pAttacker->GetDamage(FALSE) + AmmoDamage;

		if(rand() % 100 < (pAttacker->GetWeaponSkill() / 3))
		{
			if(BaseDamage < 5) 
				BaseDamage = 3 + (rand() % 3) + 1;
			BaseDamage *= 1.5;
			
			char blargsuccess[64];
			sprintf(blargsuccess,"%s is crippled!",pDefender->GetData(INDEX_NAME).String);
			Describe(blargsuccess);
		}
		else
		{
			BaseDamage = AbsorbBlow(pAttacker, pDefender, BaseDamage);

			if(BaseDamage <= 0)
			{
				//do miss result
				Describe("Failed.");
				Engine->Sound()->PlayEffect(10);
				if(OutOfThrown)
				{
					delete pGI;
				}
				return;
			}
		}

		//hit!
		if(pGI)
		{
			if(pGI->GetData("FX1").Value)
			{
				Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
			}
			else	
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		else
		{
			if(pAttacker->GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}

		pDefender->SetData(INDEX_MOVEPOINTS,pDefender->GetData(INDEX_MOVEPOINTS).Value + 1 + rand() % 3);
	
		pDefender->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

		if(PreludeParty.IsMember(pDefender))
			pDefender->ImproveSkill(INDEX_ARMOR);

		pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);
	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	if(OutOfThrown)
	{
		delete pGI;
	}
	return;
}

void DisarmAttack(Creature *pAttacker, Creature *pDefender)
{
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	Action *pCurAction;
	//otherwise
	float CurAngle;
	float DestAngle;
	CurAngle = pAttacker->GetMyAngle();
	DIRECTION_T facing;
	facing = FindFacing(pAttacker->GetData(INDEX_POSITION).pVector, pDefender->GetData(INDEX_POSITION).pVector);
	DestAngle = DIRECTIONANGLES[facing];

	if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
	{
		pAttacker->InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
		
		do
		{
			((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
			pCurAction = pAttacker->GetAction();
		}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);
	}		

	//Describe("Calculating cost");
	float APCost;
	APCost = (float)pAttacker->GetData(INDEX_ATTACKPOINTS).Value;

	APCost *= SPECIAL_AP_MULTIPLIER;
	
	int FinalApCost = (int)(APCost + 0.5f);

	pAttacker->SetData(INDEX_ACTIONPOINTS,pAttacker->GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
	
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		int AnimRange;
		AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
		if(AnimRange)
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
		}
		else
		{
			pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
		}
	}
	else
	{
		pAttacker->InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
	}
	
	ParticleSystem *pSys1, *pSys2;
	ZSModelEx *pMesh;
	
	pMesh = pAttacker->GetMesh();
	
	pSys1 = new ParticleSystem();
	pSys1->SetLink(pAttacker);
	pSys1->SetLinkPoint(pMesh->equipmentlist[EQUIP_POS_RHAND].RayFromIndex);
	pSys1->SetLife(5);
	pSys1->SetLifeVariance(0);
	pSys1->SetInitialVector(D3DVECTOR(0.0f,0.0f, 0.0f));
	pSys1->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
	pSys1->SetRadius(0.5f);
	pSys1->SetEmissionQuantity(1);
	//pSys->SetGravity(AuraGravity);
	pSys1->AdjustCamera();
	pSys1->AdvanceFrame();
	
	PreludeWorld->AddMainObject(pSys1);


	pSys2 = new ParticleSystem();
	pSys2->SetLink(pAttacker);
	pSys2->SetLinkPoint(pMesh->equipmentlist[EQUIP_POS_LHAND].RayFromIndex);
	pSys2->SetLife(10);
	pSys2->SetLifeVariance(0);
	pSys2->SetInitialVector(D3DVECTOR(0.0f,0.0f, 0.0f));
	pSys2->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
	pSys2->SetRadius(0.5f);
	pSys2->SetEmissionQuantity(1);
	//pSys->SetGravity(AuraGravity);
	pSys2->AdjustCamera();
	pSys2->AdvanceFrame();

	PreludeWorld->AddMainObject(pSys2);

	do
	{
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
		pCurAction = pAttacker->GetAction();
	}while(pCurAction->GetType() != ACTION_SPECIAL_ATTACK);

	PreludeWorld->RemoveMainObject(pSys1);
	PreludeWorld->RemoveMainObject(pSys2);

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(pAttacker, pDefender);

	ToHit /= 2;

	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

	if(PreludeParty.IsMember(pAttacker))
		pAttacker->ImproveSkill(INDEX_UNARMED);

	if(AttackRoll < ToHit)
	{
		if(Parried(pAttacker, pDefender))
		{
			return;
		}
		
		int BaseDamage;
	
		BaseDamage = pAttacker->GetDamage(FALSE);

		//modify by strength

		float DamagePercent;
		DamagePercent = ((float)pAttacker->GetData(INDEX_STRENGTH).Value) / 13.0f;

		float FinDamage;

		FinDamage = DamagePercent * (float)BaseDamage;
		FinDamage *= 0.5;

		BaseDamage = (int)(FinDamage + 0.5f); 

		BaseDamage = AbsorbBlow(pAttacker, pDefender, BaseDamage);

		if(BaseDamage <= 0)
		{
			//do miss result
			Describe("Failed.");
			Engine->Sound()->PlayEffect(10);
			return;
		}

		//hit!
		if(pGI)
		{
			if(pGI->GetData("FX1").Value)
			{
				Engine->Sound()->PlayEffect(pGI->GetData("FX1").Value);
			}
			else	
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		else
		{
			if(pAttacker->GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}

		GameItem *pToDrop = NULL;

		pToDrop = pDefender->GetEquipment("RIGHTHAND");
		if(pToDrop)
		{
			pDefender->Unequip(pToDrop);
			pDefender->AddItem(pToDrop);
			pDefender->InsertAction(ACTION_DROP,pToDrop,0,0);
			pDefender->Update();
			char blarg[64];
			sprintf(blarg,"%s drops their weapon.",pDefender->GetData(INDEX_NAME).String);
			Describe(blarg);
		}
		pDefender->TakeDamage(pAttacker,BaseDamage,(DAMAGE_T)pAttacker->GetData(INDEX_DAMAGETYPE).Value);

		if(PreludeParty.IsMember(pDefender))
			pDefender->ImproveSkill(INDEX_ARMOR);

		pAttacker->InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);

	}
	else
	{
		//do miss result
		Describe("Failed.");
		Engine->Sound()->PlayEffect(10);
	}

	return;



	return;
}

int AbsorbBlow(Creature *pAttacker, Creature *pDefender, int Damage, int AbsorbModifier)
{
	GameItem *pGI, *pGIArmor;
	pGI = pAttacker->GetEquipment("RIGHTHAND");
	int FieldIndex;
	int BaseDamage;
	int IsMelee = FALSE;
	BaseDamage = Damage;
	if(!pGI)
	{
		FieldIndex = INDEX_UNARMED;
		IsMelee = TRUE;
	}
	else
	{
		switch(pGI->GetData("SUBTYPE").Value)
		{
		case WEAPON_TYPE_SWORD:
			FieldIndex = INDEX_SWORD;
			break;
		case WEAPON_TYPE_DAGGER:
			break;
		case WEAPON_TYPE_AXE:
			break;
		case WEAPON_TYPE_BLUNT:
			break;
		case WEAPON_TYPE_POLEARM:
			break;
		case WEAPON_TYPE_MISSILE:
			break;
		default:
		case WEAPON_TYPE_UNARMED:
			break;
		case WEAPON_TYPE_THROWN:
			break;
		}
	}

	//check for armor absorbtion
	//calculate where blow landed
	int BlowLandedRoll = rand() % 100;
	int BlowLanded = BLOW_LANDED_HEAD;
	int ArmorNumber = 0;

	if(BlowLandedRoll < BLOW_LANDED_HANDS_CHANCE)
	{
		pGIArmor = pDefender->GetEquipment("GLOVES");
		BlowLanded = BLOW_LANDED_HANDS;
		DEBUG_INFO("Blow Landed on the Hands\n");
		if(PreludeWorld->VerboseCombat())
		{
			Describe("Hit on the hands!");
		}
	}
	else
	if(BlowLandedRoll < BLOW_LANDED_FEET_CHANCE)
	{
		pGIArmor = pDefender->GetEquipment("FEET");
		BlowLanded = BLOW_LANDED_FEET;
		DEBUG_INFO("Blow Landed on the Feet\n");
		if(PreludeWorld->VerboseCombat())
		{
			Describe("Hit on the feet!");
		}
	}
	else
	if(BlowLandedRoll < BLOW_LANDED_HEAD_CHANCE)
	{
		pGIArmor = pDefender->GetEquipment("HEAD");
		BlowLanded = BLOW_LANDED_HEAD;
		DEBUG_INFO("Blow Landed on the Head\n");
		if(PreludeWorld->VerboseCombat())
		{
			Describe("Hit on the head!");
		}
	}
	else
	if(BlowLandedRoll < BLOW_LANDED_BODY_CHANCE)
	{
		pGIArmor = pDefender->GetEquipment("CHEST");
		BlowLanded = BLOW_LANDED_BODY;
		DEBUG_INFO("Blow Landed on the Body\n");
		if(PreludeWorld->VerboseCombat())
		{
			Describe("Hit in the torso!");
		}
	}
	else
	{
		pGIArmor = pDefender->GetEquipment("LEGS");
		BlowLanded = BLOW_LANDED_LEGS;
		DEBUG_INFO("Blow Landed on the Legs\n");
		if(PreludeWorld->VerboseCombat())
		{
			Describe("Hit in the legs!");
		}
	}

	if(pGIArmor)
	{
/*
#ifndef NDEBUG
	sprintf(Blarg,"Armor # = %i\n",ArmorNumber);
	DEBUG_INFO(Blarg);
#endif
*/
		int ArmorRoll = 0;
		
		int AbsorbAmount = 0;
		int AbsorbRange = 0;
		AbsorbRange = pGIArmor->GetData("ARMORMAX").Value - 
						pGIArmor->GetData("ARMORMIN").Value;
		if(AbsorbRange <= 0)
		{
			AbsorbRange = 1;
		}
/*
#ifndef NDEBUG
	sprintf(Blarg,"Absorb Range = %i\n",AbsorbRange);
	DEBUG_INFO(Blarg);
#endif
*/				
		if(AbsorbRange)
		{
			AbsorbAmount = rand() % AbsorbRange + pGIArmor->GetData("ARMORMIN").Value;
		}

		AbsorbAmount = (AbsorbAmount * AbsorbModifier) / 100;
	
		if(AbsorbAmount > 0 && PreludeParty.IsMember(pDefender))
			pDefender->ImproveSkill(INDEX_ARMOR);
	

/*
#ifndef NDEBUG
	sprintf(Blarg,"Absorb amount = %i\n",AbsorbAmount);
	DEBUG_INFO(Blarg);
#endif
*/	
		BaseDamage = BaseDamage - AbsorbAmount;

		if(BaseDamage <= 0)
		{ 
			//armor absorbed the blow
			Describe("Absorbed");
			DEBUG_INFO("Absorbed");
			//pGI = GetEquipment("RIGHTHAND");
			if(pGI)
			{
				if(pGI->GetData("FX2").Value)
				{
					Engine->Sound()->PlayEffect(pGI->GetData("FX2").Value);
				}
				else
				{
					Engine->Sound()->PlayEffect(7);
				}
			}
			else
			{
				if(pAttacker->GetData(INDEX_TYPE).Value)
				{
					Engine->Sound()->PlayEffect(9);
				}
				else
				{
					Engine->Sound()->PlayEffect(7);
				}
			}
		
		}
	}

	return BaseDamage;
}


