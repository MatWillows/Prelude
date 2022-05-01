#include "spellfuncs.h"
#include "objects.h"
#include "world.h"
#include "creatures.h"
#include "items.h"
#include "explosion.h"
#include "zsfire.h"
#include "party.h"
#include "healaura.h"
#include "pattern.h"
#include "events.h"
#include "script.h"
#include "scriptfuncs.h"
#include "zsmessage.h"
#include "gameitem.h"
#include "fireball.h"
#include "modifiers.h"
#include "Meshfx.h"
#include "combatmanager.h"
#include "aura.h"
#include "mainwindow.h"

//TBD:  add check for half-bloods
//TBD:  add check for knowledge of blood magic
//TBD:  blarg

int GetFlameSkill(Object *pCaster)
{
	Creature *pcCaster;
	pcCaster = (Creature *)pCaster;
	int Skill;
	Skill = pcCaster->GetData(INDEX_POWER_OF_FLAME).Value;

	if(pcCaster->GetData(INDEX_FLAMEBLOOD).Value != 8)
	{
		return Skill / 2;
	}

	else return Skill;
}

int GetRiverSkill(Object *pCaster)
{
	Creature *pcCaster;
	pcCaster = (Creature *)pCaster;
	int Skill;
	Skill = pcCaster->GetData(INDEX_GIFTS_OF_GODDESS).Value;

	if(pcCaster->GetData(INDEX_RIVERBLOOD).Value != 8)
	{
		return Skill / 2;
	}
	else return Skill;
}

//nevermodified, can just get directly
int GetThaumaturgySkill(Object *pCaster)
{
	Creature *pcCaster;
	pcCaster = (Creature *)pCaster;
	int Skill;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;
	return Skill;
}


int FlameFinger(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{	
	int MinDamage;
	int MaxDamage;
	Creature *pcCaster;
	D3DVECTOR vFrom;
	D3DVECTOR vTo;

	pcCaster = (Creature *)pCaster;
	vFrom = *pCaster->GetPosition();
	vTo = *pTarget->GetPosition();
	vFrom.z += 1.5f;
	vTo.z += 1.5f;

	int Skill;
	Skill = GetFlameSkill(pCaster);
	
	MinDamage = 4;
	MaxDamage = 9;
	switch(Level)
	{
	default:
	case 0:
		MaxDamage += Skill / 5;
		break;
	case 1:
		MaxDamage += Skill / 9; 
		break;

	case 2:
		MaxDamage += Skill / 4;
		break;
	case 3:
		MaxDamage += Skill / 3;
		break;
	}

	//now we have to trace the line of the "burst"
	D3DVECTOR vRay, vCur, vLast;
	float CurLength = 1.0f;
	float MaxLength;
	vRay = vTo - vFrom;
	
	MaxLength = Magnitude(vRay);

	vRay = Normalize(vRay);

	vLast = vFrom;

	Object *pOb;
	int Damage;
	Creature *pCreature;
	Explosion *pExplosion;

	//create a couple of particle systems to show the "flame effect"
	
	int iLength;
	iLength = (int)(MaxLength + 0.5f);
	
	Pattern *pPattern;
	pPattern = new Pattern();
	pPattern->AddNode(vFrom);
	for(int n = 0; n <= MaxLength + 1; n++)
	{
		D3DVECTOR vNode;
		vNode = vFrom + vRay * (1.0 * (float)n);
		pPattern->AddNode(vNode);
	}
	pPattern->GetSys()->SetEnd(35);
	pPattern->AdjustCamera();
	PreludeWorld->AddMainObject(pPattern);

	if(Level != 1)
	{
		pPattern = new Pattern();
		pPattern->AddNode(vFrom);
		for(int n = 1; n <= MaxLength + 1; n++)
		{
			D3DVECTOR vNode;
			vNode = vFrom + vRay * (1.0 * (float)n);
			pPattern->AddNode(vNode);
		}
		pPattern->GetSys()->SetEnd(35);
		pPattern->AdjustCamera();
		PreludeWorld->AddMainObject(pPattern);
	}

	if(Level == 4)
	{
		pPattern = new Pattern();
		pPattern->AddNode(vFrom);
		for(int n = 1; n <= MaxLength + 1; n++)
		{
			D3DVECTOR vNode;
			vNode = vFrom + vRay * (1.0 * (float)n);
			pPattern->AddNode(vNode);
		}
		pPattern->GetSys()->SetEnd(35);
		pPattern->AdjustCamera();
		PreludeWorld->AddMainObject(pPattern);

	}

	
	
/*

	ParticleSystem *pSys;
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.1f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.2f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(55);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
	
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.1f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_ORANGE));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.2f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(55);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
	
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.1f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.2f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(55);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
*/
	

	
	while(CurLength < (MaxLength + 0.2f))
	{
		vCur = vFrom + (vRay * CurLength);

		if((int)vCur.x != (int)vLast.x || (int)vCur.y != (int)vLast.y)
		{
			vLast = vCur;
			pOb = Valley->FindObject(vLast.x, vLast.y, OBJECT_CREATURE);
			if(pOb)
			{	
				Damage = MinDamage + (rand() % (MaxDamage - MinDamage));
				if(Level != 1 || !PreludeParty.IsMember((Creature *)pOb))
				{
					pCreature = (Creature *)pOb;
					pCreature->TakeDamage((Thing *)pCaster,Damage, DAMAGE_FIRE);
					vCur = *pCreature->GetPosition();
					vCur.z += 1.5f;
					pExplosion = new Explosion(vCur, 0.5f, 0, 0, (Thing *)pcCaster);
					PreludeWorld->AddMainObject(pExplosion);
				}
			}
		}
		CurLength += 0.1f;
	}
	return TRUE;
}


int FireBallSpell(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//add missile which will handle explosion
	//basic
	//farther, but small and weaker.
	//closer and more damaging
	//greatest range, size, and damage.
	FireBall *pFireBall;
	Creature *pcCaster;

	pcCaster = (Creature *)pCaster;

	int Skill;
	Skill = GetFlameSkill(pCaster);

	D3DVECTOR vStart =  *pCaster->GetPosition();
	D3DVECTOR vEnd = *pTarget->GetPosition();
	
	vStart.z += 1.0f;
	vEnd.z += 1.0f;

	int MinDamage;
	int MaxDamage;
	int HPDown = 0;
	int RPDown = 0;
	float Radius;

	MaxDamage = 7;
	switch(Level)
	{
	default:
	case 0:
		MinDamage = 3;
		MaxDamage += Skill / 5;
		Radius = 1.5f;
		break;
	case 1:
		MinDamage = 2;
		MaxDamage += Skill / 8; 
		Radius = 0.5f;
		break;
	case 2:
		MinDamage = 4;
		MaxDamage += Skill / 4;
		Radius = 2.5f;
		break;
	case 3:
		MinDamage = 5;
		MaxDamage += Skill / 3;
		Radius = 2.5f;
		break;
	}
		
	pFireBall = new FireBall(vStart,
									 vEnd,
									 8.0f, 
									 0.9f, 
									 Radius,
									 MinDamage,
									 MaxDamage,
									 (Thing *)pCaster);	
	
	PreludeWorld->AddMainObject((Object *)pFireBall);
	
	int NumWait;
	
	NumWait = pFireBall->GetNumMoveFrames();
	
	if(NumWait > 0)
	{
		((Creature *)pCaster)->InsertAction(ACTION_WAITUNTIL, (void *)NumWait, (void *)0);
	}
 
	return TRUE;
}	
	
int FlamingHands(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//add aura
	//add flames to each hand and link to character
	ZSModelEx *pMesh;
	Modifier *pMod;
	Fire *pFire;

		int Skill;
	Skill = GetFlameSkill(pCaster);
	
	int MinDamage = 2;
	int MaxDamage = 5;
	int Duration = 1;	

	switch(Level)
	{
	default:
	case 0:
		MaxDamage += Skill / 5;
		Duration += Skill / 7;
		break;
	case 1:
		MaxDamage += Skill / 9; 
		Duration += Skill / 5;
		break;

	case 2:
		MinDamage = 3;
		MaxDamage += Skill / 4;
		Duration += Skill / 9;
		break;
	case 3:
		MinDamage = 4;
		MaxDamage += Skill / 3;
		Duration += Skill / 5;
		break;
	}

	pMesh = pTarget->GetMesh();
	pFire = new Fire(D3DVECTOR(0.0f,0.0f,0.0f),0.10f,(Thing *)pTarget,0);
	pFire->SetLinkPoint(pMesh->equipmentlist[EQUIP_POS_LHAND].RayFromIndex);
	pFire->SetLink(pTarget);
	PreludeWorld->AddMainObject(pFire);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura((Object *)pFire);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise minimum damage
	pMod->SetStat(((Creature *)pTarget)->GetIndex("MINDAMAGE"));
	pMod->SetAmount(MinDamage);
	pMod->Apply();

	pFire = new Fire(D3DVECTOR(0.0f,0.0f,0.0f),0.10f,(Thing *)pTarget,0);
	pFire->SetLinkPoint(pMesh->equipmentlist[EQUIP_POS_RHAND].RayFromIndex);
	pFire->SetLink(pTarget);
	PreludeWorld->AddMainObject(pFire);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura((Object *)pFire);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(((Creature *)pTarget)->GetIndex("MAXDAMAGE"));
	pMod->SetAmount(MaxDamage);
	pMod->Apply();

	return TRUE;
}
	
int Berserk(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//remove control of player for duration
	//add aura to target
	//add a modifier which increases the target's speed.
	//duration is based on power level
	//amount of increas is based on caster's skill
	//add aura
	//add flames to each hand and link to character
	Modifier *pMod;

	int Skill;
	Skill = GetFlameSkill(pCaster);
	
	int SpeedMod = 1;
	int StrengthMod = 1;
	int DexMod = 1;
	int DamageMod = 300;
	int MaxDamageMod = 1;
	int Duration = Skill / 8;	
	if(Duration < 5) 
		Duration = 5;

	switch(Level)
	{
	default:
		//normal
	case 0:
		SpeedMod += Skill / 20;
		DexMod += Skill / 20;
		StrengthMod += Skill / 7;
		MaxDamageMod += Skill / 5;
		DamageMod = 75;
		break;
		//longer, but weaker
	case 1:
		SpeedMod += Skill / 15;
		DexMod += Skill / 20;
		StrengthMod += Skill / 4;
		MaxDamageMod += Skill / 3;
		DamageMod = 150;
		break;
		//shorter but stronger
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_RED, 0.003f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	//raise minimum damage
	pMod->SetStat(INDEX_SPEED);
	pMod->SetAmount(SpeedMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_ORANGE, 0.003f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_MAXACTIONPOINTS);
	pMod->SetAmount(SpeedMod);
	pMod->Apply();

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_STRENGTH);
	pMod->SetAmount(StrengthMod);
	pMod->Apply();

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(((Creature *)pTarget)->GetIndex("RESISTNORMAL"));
	pMod->SetAmount(DamageMod);
	pMod->Apply();

	//clear armor skill
	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_ARMOR);
	pMod->SetAmount(((Creature *)pTarget)->GetData(INDEX_ARMOR).Value);
	pMod->Apply();

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_FEINT);
	pMod->SetAmount(-5);
	pMod->Apply();

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_MAXDAMAGE);
	pMod->SetAmount(MaxDamageMod);
	pMod->Apply();
/*
	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_WEAPONSKILL);
	pMod->SetAmount(-30);
	pMod->Apply();
*/
	return TRUE;
}

int Haste(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//add a modifier which increases the target's speed.
	//duration is based on power level
	//amount of increas is based on caster's skill
	//add aura
	//add flames to each hand and link to character
	Modifier *pMod;

	int Skill;
	Skill = GetFlameSkill(pCaster);
	
	int SpeedMod = 1;
	int Duration = 1;	

	switch(Level)
	{
	default:
		//normal
	case 0:
		SpeedMod += Skill / 10;
		Duration += Skill / 7;
		break;
		//longer, but weaker
	case 1:
		SpeedMod += Skill / 13; 
		Duration += Skill / 5;
		break;
		//shorter but stronger
	case 2:
		SpeedMod += Skill / 8;
		Duration += Skill / 10;
		break;
		//longer and stronger
	case 3:
		SpeedMod += Skill / 8;
		Duration += Skill / 7;
		break;
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_ORANGE, 0.001f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	
	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise minimum damage
	pMod->SetStat(((Creature *)pTarget)->GetIndex("SPEED"));
	pMod->SetAmount(SpeedMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_YELLOW, 0.001f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(((Creature *)pTarget)->GetIndex("MAXACTIONPOINTS"));
	pMod->SetAmount(SpeedMod);
	pMod->Apply();
	
	return TRUE;
}

int Flash(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//flash simply removes action points from anyone who has not already used them all up.
	//stun/blind everyone around the caster (but not the caster)
	//stun/blind only enemies around the caster for a shorter amount of time
	//stun/blind everyone around the caster for a longer amount of time
	//stun/blind everyone in a greater area around the caster for a longer amount of time
	
	ZSMeshFX *pFX;
	Creature *pcCaster;

	pcCaster = (Creature *)pCaster;

	int Skill;
	Skill = GetFlameSkill(pCaster);

	D3DVECTOR vStart =  *pCaster->GetPosition();
	
	vStart.z += 1.0f;
	
	float Radius;

	int StunLength;
	
	switch(Level)
	{
	default:
	case 0:
		StunLength = Skill / 2 + 5;
		Radius = 2.5f;
		Radius += (float)Skill / 5.0f;
		break;
	case 1:
		StunLength = Skill / 3 + 5;
		Radius = 2.5f;
		Radius += (float)Skill / 5.0f;
		break;
	case 2: 
		StunLength = (Skill / 3) * 2 + 5;
		Radius = 2.5f;
		Radius += (float)Skill / 8.0f;
		break;
	case 3:
		StunLength = Skill;
		Radius = 2.5f;
		Radius += (float)Skill / 4.0f;
		break;
	}

	int NumFrames = 5;

	pFX = new ZSMeshFX();
	pFX->SetMesh(Engine->GetMesh("sphere"));
	pFX->SetAlpha(.2f);
	pFX->SetAlphaRate(1.0f / 5.f);
	pFX->SetBlue(0.0f);
	pFX->SetBlueRate(0.2f);
	pFX->SetEnd(5);
	pFX->SetScale(0.1f);
	pFX->SetExpansionRate(Radius / 5.0f);
	pFX->SetPosition(&vStart);
	pFX->SetLighting(TRUE);

	
	PreludeWorld->AddMainObject((Object *)pFX);

	//check all creatures around the caster;
	Object *pOb;
	pOb = PreludeWorld->GetCombat()->GetCombatants();

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			if(pOb != pCaster && GetDistance((Creature *)pOb,(Creature *)pCaster) < Radius)
			{
				if(Level == 2 && PreludeParty.IsMember((Creature *)pOb))
				{
				
				}
				else
				{
					((Creature *)pOb)->SetData(INDEX_ACTIONPOINTS, ((Creature *)pOb)->GetData(INDEX_ACTIONPOINTS).Value - StunLength);
					((Creature *)pOb)->InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)GET_HIT, TRUE);
				}
			}
		}
		pOb = pOb->GetNextUpdate();
	}
	
	int NumWait;
	
	NumWait = 8;
	
	if(NumWait > 0)
	{
		((Creature *)pCaster)->InsertAction(ACTION_WAITUNTIL, (void *)NumWait, (void *)0);
	}
 
	return TRUE;
}

int Immolation(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//duration is dependant on skill, but will not last beyond combat
	//every round damage anything in tiles surrounding the caster.
	//do not damage friends.
	//deal greater damage
	//deal greater damage in a two tile radius
	int Damage;
	Creature *pcCaster;
	D3DVECTOR vFrom;
	D3DVECTOR vTo;
	D3DVECTOR vMid;

	pcCaster = (Creature *)pCaster;
	
	int Skill;
	Skill = GetFlameSkill(pCaster);

	int Duration;
	switch(Level)
	{
	default:
	case 0:
		Damage = Skill / 4 + 2;
		Duration = Skill / 6 + 1;
		break;
	case 1:
		Damage = Skill / 5 + 3;
		Duration = Skill / 6 + 1;
		break;
	case 2:
		Damage = Skill / 3;
		Duration = Skill / 6 + 1;
		break;
	case 4:
		Damage = Skill / 3;
		Duration = Skill / 6 + 1;
		break;
	}

	Modifier *pMod;
	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_ORANGE, 0.002f, 0.0f);
	PreludeWorld->AddMainObject(pAura);
	((Aura *)pAura)->SetDamageLevel(Damage);
	((Aura *)pAura)->SetSpellNum(6);
	((Aura *)pAura)->SetEffectLevel(Level);
	((Aura *)pAura)->SetLastRoundActive(PreludeWorld->GetCombat()->GetRound());

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_MAXACTIONPOINTS);
	pMod->SetAmount(0);
	pMod->Apply();
	
	pAura = CreateAura(pTarget, COLOR_YELLOW, 0.002f, 0.0f);
	PreludeWorld->AddMainObject(pAura);
	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_MAXACTIONPOINTS);
	pMod->SetAmount(0);
	pMod->Apply();
	
	pAura = CreateAura(pTarget, COLOR_RED, 0.002f, 0.0f);
	PreludeWorld->AddMainObject(pAura);
	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(INDEX_MAXACTIONPOINTS);
	pMod->SetAmount(0);
	pMod->Apply();
	
	if(Level >= 2)
	{
		pAura = CreateAura(pTarget, COLOR_ORANGE, 0.002f, 0.0f);
		PreludeWorld->AddMainObject(pAura);
		pMod = new Modifier;
		PreludeWorld->GetCombat()->AddMod(pMod);
		pMod->SetTarget(pTarget);
		pMod->SetDuration(Duration);
		//raise maximum
		pMod->SetStat(INDEX_MAXACTIONPOINTS);
		pMod->SetAmount(0);
		pMod->Apply();
		pMod->SetAura(pAura);
	}

	if(Level == 3)
	{
		pAura = CreateAura(pTarget, COLOR_YELLOW, 0.002f, 0.0f);
		PreludeWorld->AddMainObject(pAura);
		pMod = new Modifier;
		PreludeWorld->GetCombat()->AddMod(pMod);
		pMod->SetTarget(pTarget);
		pMod->SetDuration(Duration);
		//raise maximum
		pMod->SetStat(INDEX_MAXACTIONPOINTS);
		pMod->SetAmount(0);
		pMod->Apply();
		pMod->SetAura(pAura);
	}

	return TRUE;
}

int Spear(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	int MinDamage;
	int MaxDamage;
	Creature *pcCaster;
	D3DVECTOR vFrom;
	D3DVECTOR vTo;
	D3DVECTOR vMid;

	pcCaster = (Creature *)pCaster;
	vFrom = *pCaster->GetPosition();
	vTo = *pTarget->GetPosition();
	vFrom.z += 1.5f;
	vTo.z += 1.5f;

	int Skill;
	Skill = GetFlameSkill(pCaster);
	
	MinDamage = 4;
	MaxDamage = 10;
	switch(Level)
	{
	default:
	case 0:
		MinDamage += Skill / 6;
		MaxDamage += Skill / 3;
		break;
	case 1:
		MinDamage += Skill / 4;
		MaxDamage += Skill / 2; 
		break;
	}

	int Damage;
	Damage = (rand() % (MaxDamage - MinDamage)) + MinDamage;

	//now we have to trace the line of the "burst"
	D3DVECTOR vRay, vCur, vLast;
	float CurLength = 1.0f;
	float MaxLength;
	
	vMid = (vFrom + vTo) * 0.5f;
	
	vMid.z += 6.0f;

	vFrom = vMid;

	vRay = vTo - vFrom;

	MaxLength = Magnitude(vRay);

	vRay = Normalize(vRay);

	vLast = vFrom;

	Creature *pCreature;
	Explosion *pExplosion;

	//create a couple of particle systems to show the "flame effect"
	
	int iLength;
	iLength = (int)(MaxLength + 0.5f);
	
	int NumFrames;
	NumFrames = (int)((MaxLength + 0.5f) / 0.2f);
	
	ParticleSystem *pSys;
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.2f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.3f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(NumFrames);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
	
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.2f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_ORANGE));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.3f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(NumFrames);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
	
	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.5f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.2f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.3f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(NumFrames);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);

	pSys = new ParticleSystem;
	pSys->SetOrigin(vFrom + vRay * 0.6f);
	//pSys->SetInitialVector(vRay * 0.1f);
	pSys->SetMotion(vRay * 0.2f);
	pSys->SetInitialVariance(_D3DVECTOR(0.01f,0.01f,0.01f));
	pSys->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
	pSys->SetParticleType(PARTICLE_SPHERE);
	pSys->SetRadius(0.3f);
	pSys->SetLife(30);
	pSys->SetLifeVariance(10);
	pSys->SetGravity(_D3DVECTOR(0.0f,0.0f,0.002f));
	pSys->SetEnd(NumFrames);
	pSys->AdjustCamera();
	PreludeWorld->AddMainObject(pSys);
	
	if(NumFrames > 0)
	{
		((Creature *)pCaster)->InsertAction(ACTION_WAITUNTIL, (void *)NumFrames, (void *)0);
	}

	ZSMainWindow *pMain;
	pMain = (ZSMainWindow *)ZSWindow::GetMain();

	for(int fn = 0; fn < NumFrames; fn++)
	{
		pMain->DrawAndUpdate();
	}

	pCreature = (Creature *)pTarget;

	pCreature->TakeDamage((Thing *)pCaster, Damage, DAMAGE_FIRE);
	
	pExplosion = new Explosion(vTo, 0.5f, 0, 0, (Thing *)pcCaster);
	PreludeWorld->AddMainObject(pExplosion);




	return TRUE;
}

int Sentinel(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//create sentinel
	int MinDamage;
	int MaxDamage;
	Creature *pcCaster;
	D3DVECTOR vFrom;
	D3DVECTOR vTo;
	D3DVECTOR vMid;

	pcCaster = (Creature *)pCaster;

	int Skill;
	Skill = GetFlameSkill(pCaster);
	
	MinDamage = 1;
	MaxDamage = 5;
	float fRange;
	fRange = 2.5f;

	switch(Level)
	{
	default:
	case 0:
		MinDamage += Skill / 8;
		MaxDamage += Skill / 5;
		break;
	case 1:
		MinDamage += Skill / 6;
		MaxDamage += Skill / 4; 
		fRange = 3.5f;
		break;
	}

	Push((Creature *)pCaster);
	Push((int)pTarget->GetPosition()->y);
	Push((int)pTarget->GetPosition()->x);

	PreludeEvents.RunEvent(376);

	ScriptArg *pSA;
	pSA = Pop();
	Creature *pSentinel;

	pSentinel = pSA->GetCreature();
	delete pSA;

	pSentinel->SetData(INDEX_MINDAMAGE,MinDamage);
	pSentinel->SetData(INDEX_MAXDAMAGE,MaxDamage);

	pSentinel->SetData(INDEX_RANGE,fRange);

	pSentinel->SetData(INDEX_MOVEPOINTS, 99);

	pSentinel->SetData(INDEX_AICODE, 1);

	pSentinel->SetData(INDEX_DAMAGETYPE, DAMAGE_FIRE);

	pSentinel->SetData(INDEX_WEAPONSKILL, 80);
	pSentinel->SetLastPlacedTime(-1);

	return TRUE;
}


int MothersHeal(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	int HitHeal;
	int Skill;
	Skill = GetRiverSkill(pCaster);
		
	switch(Level)
	{
	default:
	case 1:
		HitHeal = 7 + Skill / 3;
		break;
	case 2:
		HitHeal = 4 + Skill / 4;
		break;
	}

	HealAura *pHealAura;

	pHealAura = new HealAura((Creature *)pTarget, HitHeal, HitHeal);

	int OldHP = ((Creature *)pTarget)->GetData(INDEX_HITPOINTS).Value;
	int CurHP;
	CurHP = OldHP + HitHeal;
	
	if(CurHP <= ((Creature *)pTarget)->GetData(INDEX_MAXHITPOINTS).Value)
	{
		((Creature *)pTarget)->SetData(INDEX_HITPOINTS,CurHP);
	}
	else
	{
		((Creature *)pTarget)->SetData(INDEX_HITPOINTS,((Creature *)pTarget)->GetData(INDEX_MAXHITPOINTS).Value);
	}

	CurHP = ((Creature *)pTarget)->GetData(INDEX_HITPOINTS).Value - OldHP;

	char HealText[64];

	sprintf(HealText,"%s is healed %i points.",((Creature *)pTarget)->GetData(INDEX_NAME).String, CurHP);
	
	Describe(HealText);

	PreludeWorld->AddMainObject((Object *)pHealAura);
	
	return TRUE;
}

//river spells
int Slow(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//lower the targets speed (and actionpoints) appropriate for several rounds	
	//duration is based on power level
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int SpeedMod = 1;
	int Duration = 1;	

	SpeedMod += Skill / 8;
	Duration += Skill / 7;

	SpeedMod *= -1;

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_BLUE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise minimum damage
	pMod->SetStat(((Creature *)pTarget)->GetIndex("SPEED"));
	pMod->SetAmount(SpeedMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_WHITE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	//raise maximum
	pMod->SetStat(((Creature *)pTarget)->GetIndex("MAXACTIONPOINTS"));
	pMod->SetAmount(SpeedMod);
	pMod->Apply();
	
	return TRUE;
}


int Shield(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//reduce damage on target for several rounds
	//add modifier to physical damage.
	//reduction and duration dependant on skill
	//self is better than other
	//duration is based on power level
	
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int ShieldMod = 5;
	int Duration = 1;	

	if(!Level)
	{
		ShieldMod += Skill / 2;
		Duration += Skill / 10;
	}
	else
	{
		ShieldMod += Skill / 4;
		Duration += Skill / 10;
	}

	ShieldMod *= -1;

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_BLUE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("RESISTNORMAL"));
	pMod->SetAmount(ShieldMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_BLUE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();
	
	return TRUE;
}


int Growth(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//increase target's size by some amount
	//gradually raise strength of target
	//self is better than other
	//duration is based on power level
	
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	int Duration = 1;

	if(!Level)
	{
		Duration += Skill / 4;
	}
	else
	{
		Duration += Skill / 8;
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_GREEN, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("STRENGTH"));
	pMod->SetAmount(1);
	pMod->SetProgression(1);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_GREEN, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();
	
	return TRUE;
}


int Visage(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//all friend characters get a boost to hit (dex)
	//all opposing characters get a penalty to hit (dex)
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);

	int Duration;
	Duration = (Skill / 10) + 1;
	int DexMod;
	DexMod = (Skill / 15) + 1;

	Object *pOb;
	pOb = PreludeWorld->GetCombat()->GetCombatants();

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			if(PreludeParty.IsMember((Creature *)pOb))
			{
				pMod = new Modifier;
				PreludeWorld->GetCombat()->AddMod(pMod);
				pMod->SetTarget(pTarget);
				pMod->SetDuration(Duration);
				
				pMod->SetStat(((Creature *)pTarget)->GetIndex("DEXTERITY"));
				pMod->SetAmount(DexMod);
				pMod->Apply();
			}
			else
			if(((Creature *)pOb)->GetData(INDEX_BATTLESIDE).Value)
			{
				pMod = new Modifier;
				PreludeWorld->GetCombat()->AddMod(pMod);
				pMod->SetTarget(pTarget);
				pMod->SetDuration(Duration);
				
				pMod->SetStat(((Creature *)pTarget)->GetIndex("DEXTERITY"));
				pMod->SetAmount(-DexMod);
				pMod->Apply();
			}
		}
		pOb = pOb->GetNextUpdate();
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_BLUE, 0.05f, 0.01f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pCaster);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_WHITE, 0.05f, 0.01f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetTarget(pCaster);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();
		
	return TRUE;
}

int Weaken(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//lower base damage of target (strength)
	//increase target damage taken
	//touch is more effective then breath
	//duration is based on power level
	
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int StrengthMod = 0;
	int Duration = 1;	
	int ResistMod = -1;

	if(!Level)
	{
		StrengthMod += Skill / 7;
		Duration += Skill / 8;
		ResistMod  *= Skill / 3;
	}
	else
	{
		StrengthMod += Skill / 10;
		Duration += Skill / 10;
		ResistMod *= Skill / 4;
	}
	StrengthMod *= -1;
	
	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_GREEN, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("RESISTNORMAL"));
	pMod->SetAmount(ResistMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_YELLOW, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("STRENGTH"));
	pMod->SetAmount(StrengthMod);
	pMod->Apply();

	return TRUE;

}

int Strength(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//raise strength of target
	//self is better than other
	//duration is based on power level
	
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int StrengthMod = 0;
	int Duration = 1;	

	if(!Level)
	{
		StrengthMod += Skill / 7;
		Duration += Skill / 8;
	}
	else
	{
		StrengthMod += Skill / 10;
		Duration += Skill / 10;
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_BLUE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("STRENGTH"));
	pMod->SetAmount(StrengthMod);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_WHITE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();
	
	return TRUE;
}

int Root(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//vastly inhibits targets ability to move
	//self is better than other
	//duration is based on power level
	

	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int Duration = 1;	

	if(!Level)
	{
		Duration += Skill / 8;
	}
	else
	{
		Duration += Skill / 11;
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_GREEN, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("MOVEPOINTS"));
	pMod->SetAmount(10);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_WHITE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(0);
	pMod->SetAmount(0);
	pMod->Apply();
	
	return TRUE;
}

int Cycle(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//boost stength, speed, and hitpoints
	//lower each every round.
	//do not kill
	//raise strength of target
	//self is better than other
	//duration is based on power level
	
	Modifier *pMod;

	int Skill;
	Skill = GetRiverSkill(pCaster);
	
	int StrengthMod = 0;
	int Duration = 1;	

	if(!Level)
	{
		StrengthMod += Skill / 7;
		Duration += Skill / 8;
	}
	else
	{
		StrengthMod += Skill / 10;
		Duration += Skill / 10;
	}

	Object *pAura;
	pAura = CreateAura(pTarget, COLOR_BLUE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("STRENGTH"));
	pMod->SetAmount(StrengthMod);
	pMod->SetProgression(-2);
	pMod->Apply();

	pAura = CreateAura(pTarget, COLOR_WHITE, 0.0f, 0.0f);
	PreludeWorld->AddMainObject(pAura);

	pMod = new Modifier;
	PreludeWorld->GetCombat()->AddMod(pMod);
	pMod->SetAura(pAura);
	pMod->SetTarget(pTarget);
	pMod->SetDuration(Duration);
	
	pMod->SetStat(((Creature *)pTarget)->GetIndex("SPEED"));
	pMod->SetAmount(StrengthMod);
	pMod->SetProgression(-2);
	pMod->Apply();

	return TRUE;
}


//Thaumaturgy
int LightningStone(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//confirm target is a stone
	Item *pItem;
	GameItem *pGI;
	Creature *pcCaster;
	int Skill;

	pcCaster = (Creature *)pCaster;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;

	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();
	if(pGI->GetData(INDEX_TYPE).Value != ITEM_TYPE_WEAPON
		|| pGI->GetData("SUBTYPE").Value != WEAPON_TYPE_THROWN)
	{
		Message("This spell only works on throwing weapons","ok");
		return FALSE;
	}

	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(394);


	int WepMod;
	int WepStat;
	int WepDuration;
	WepMod =  61; //lightning event
	WepStat = pItem->GetIndex("DAMAGEOVERRIDE");
	
	WepDuration = Skill * 30;

	pGI->SetModAmount(WepMod);
	pGI->SetModEnd(PreludeWorld->GetTotalTime() + WepDuration);
	pGI->SetModStat(WepStat);

	char blarg[128];
	sprintf(blarg,"The %s has been charged.",pGI->GetData(INDEX_NAME).String);
	Describe(blarg);

	return TRUE;
}

int KnockoutDust(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//convert ingredients to 
	//confirm target is sand
	Item *pItem;
	GameItem *pGI;
	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();
	if(strcmp(pItem->GetData(INDEX_NAME).String,"Sand"))
	{
		Message("This spell only works on sand","ok");
		return FALSE;
	}

	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(395);


	int Quantity;
	Quantity = pGI->GetQuantity();

	pCaster->RemoveItem(pItem,Quantity);
	
	((Creature *)pCaster)->Give(Item::Find(Item::GetFirst(),"Knockout Dust"),Quantity);

	//remove sand, add knockout dust
	return TRUE;
}

int SharpWeapon(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//only work on weapons
	Item *pItem;
	GameItem *pGI;
	Creature *pcCaster;
	int Skill;

	pcCaster = (Creature *)pCaster;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;

	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();
	if(pItem->GetData(INDEX_TYPE).Value != ITEM_TYPE_WEAPON)
	{
		Message("This spell only works on weapons","ok");
		return FALSE;
	}

	if(pGI->GetQuantity() > 1)
	{
		Message("You may only enchant a single weapon","ok");
		return FALSE;
	}

	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(396);

	
	int WepMod;
	int WepStat;
	int WepDuration;
	WepMod =  Skill / 7 + 1;
	WepStat = pItem->GetIndex("MAXDAMAGE");
	
	WepDuration = Skill * 30;

	pGI->SetModAmount(WepMod);
	pGI->SetModEnd(PreludeWorld->GetTotalTime() + WepDuration);
	pGI->SetModStat(WepStat);

	char blarg[128];
	sprintf(blarg,"The %s has been enchanted.",pGI->GetData(INDEX_NAME).String);
	Describe(blarg);

	//check for other ingredients
	//add a time based mod to target
	//after time revert
	return TRUE;
}

int FastWeapon(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//only work on weapons
	Item *pItem;
	GameItem *pGI;
	Creature *pcCaster;
	int Skill;

	pcCaster = (Creature *)pCaster;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;

	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();
	if(pItem->GetData(INDEX_TYPE).Value != ITEM_TYPE_WEAPON)
	{
		Message("This spell only works on weapons","ok");
		return FALSE;
	}

	if(pGI->GetQuantity() > 1)
	{
		Message("You may only enchant a single weapon","ok");
		return FALSE;
	}

	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(397);

	int WepMod;
	int WepStat;
	int WepDuration;
	WepMod =  - (Skill / 30 + 1);
	WepStat = pItem->GetIndex("SPEED");
	
	if(pGI->GetData(WepStat).Value + WepMod < 3)
	{
		WepMod = 3 - pGI->GetData(WepStat).Value;
	}

	WepDuration = Skill * 30;

	pGI->SetModAmount(WepMod);
	pGI->SetModEnd(PreludeWorld->GetTotalTime() + WepDuration);
	pGI->SetModStat(WepStat);

	char blarg[128];
	sprintf(blarg,"The %s has been enchanted.",pGI->GetData(INDEX_NAME).String);
	Describe(blarg);
	//check for other ingredients
	//add a time based mod to target
	//after time revert
	return TRUE;
}

int Golem(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//create golem, eliminate ingredients
	//check for ingredients
	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(398);

	
	ScriptArg *pSA;

	Push((Creature *)pCaster);
	Push((int)pTarget->GetPosition()->y);
	Push((int)pTarget->GetPosition()->x);

	PreludeEvents.RunEvent(375);

	pSA = Pop();
	
	Creature *pGolem;

	pGolem = pSA->GetCreature();

	pGolem->SetLastPlacedTime(-1);
	delete pSA;

	return TRUE;
}

int HardenArmour(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//only work on armour
	Item *pItem;
	GameItem *pGI;
	Creature *pcCaster;
	int Skill;

	pcCaster = (Creature *)pCaster;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;

	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();

	if(pItem->GetData(INDEX_TYPE).Value != ITEM_TYPE_ARMOR)
	{
		Message("This spell only works on Armour","ok");
		return FALSE;
	}

	if(pGI->GetQuantity() > 1)
	{
		Message("You may only enchant a single piece of armor","ok");
		return FALSE;
	}
	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(399);

	//Harden the armour
	int ArmorMod;
	int ArmorStat;
	int ArmorDuration;
	ArmorMod = Skill / 10;
	ArmorStat = pItem->GetIndex("ARMORMIN");
	ArmorDuration = Skill * 10;

	pGI->SetModAmount(ArmorMod);
	pGI->SetModEnd(PreludeWorld->GetTotalTime() + ArmorDuration);
	pGI->SetModStat(ArmorStat);

	char blarg[128];
	sprintf(blarg,"The %s has been enchanted.",pGI->GetData(INDEX_NAME).String);
	Describe(blarg);
	//add a time based mod to target
	//after time revert 
	return TRUE;
}

//now paralyze wepon
int FixEnchantment(Object *pCaster, Object *pTarget, int Level, int CallType, int Data)
{
	//confirm target is a stone
	Item *pItem;
	GameItem *pGI;
	Creature *pcCaster;
	int Skill;

	pcCaster = (Creature *)pCaster;
	Skill = pcCaster->GetData(INDEX_THAUMATURGY).Value;

	pGI = (GameItem *)pTarget;
	pItem = pGI->GetItem();
	if(pGI->GetData(INDEX_TYPE).Value != ITEM_TYPE_WEAPON &&
		pGI->GetData("SUBTYPE").Value != WEAPON_TYPE_MISSILE)
	{
		Message("This spell only works on and thrown melee weapons","ok");
		return FALSE;
	}

	if(pGI->GetQuantity() > 1)
	{
		Message("You may only enchant a single weapon","ok");
		return FALSE;
	}

	Push((Creature *)pCaster);
	PreludeEvents.RunEvent(400);

	int WepMod;
	int WepStat;
	int WepDuration;
	WepMod =  389; //lightning event
	WepStat = pItem->GetIndex("DAMAGEOVERRIDE");
	
	WepDuration = Skill * 30;

	pGI->SetModAmount(WepMod);
	pGI->SetModEnd(PreludeWorld->GetTotalTime() + WepDuration);
	pGI->SetModStat(WepStat);

	char blarg[128];
	sprintf(blarg,"The %s has been enchanted.",pGI->GetData(INDEX_NAME).String);
	Describe(blarg);

	return TRUE;

}

int ImmolationActivate(Object *pBase, int Level, int Damage)
{
	//damage everything around the in radius
	float Radius;

	if(Level >= 3)
	{
		Radius = 2.5;
	}
	else
	{
		Radius = 1.5;
	}

	//check all creatures around the caster;
	Object *pOb;
	pOb = PreludeWorld->GetCombat()->GetCombatants();

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			if(pOb != pBase && GetDistance((Creature *)pOb,(Creature *)pBase) < Radius)
			{
				if(Level == 1 && PreludeParty.IsMember((Creature *)pOb))
				{
				
				}
				else
				{
					((Creature *)pOb)->TakeDamage(NULL,Damage,DAMAGE_FIRE);
				}
			}
		}
		pOb = pOb->GetNextUpdate();
	}

	return TRUE;
}
