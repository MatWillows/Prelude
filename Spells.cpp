#include "spells.h"
#include "fireball.h"
#include "healaura.h"
#include "world.h"
#include <assert.h>
#include "creatures.h"

char Spell::SpellFieldNames[64*32];
Spell *Spell::pFirst;
int Spell::NumSpells;


Spell::Spell()
{
	NumSpells++;

}

Spell::~Spell()
{
	NumSpells--;
	if(!NumSpells)
	{
		DestroyFieldNames();
	}
}

void LoadSpells(FILE *fp)
{
	Spell *pSpell, *pNextSpell;

	assert(fp);

	pSpell = new Spell;
	pSpell->SetFirst((Thing *)pSpell);

	pSpell->LoadFieldNames(fp,Spell::SpellFieldNames);

	char c;

	while(TRUE)
	{
		pSpell->LoadData(fp);
		c = (char)fgetc(fp);
		if(feof(fp) || c == THING_SEPERATION_CHARACTER)
		{
			break;
		}
		else
		{
			fseek(fp,-1,1);
			pNextSpell = new Spell;
			pNextSpell->SetFieldNames(pSpell->GetFieldNames());
			pSpell->SetNext(pNextSpell);
			pSpell->GetNext()->SetNumFields(pSpell->GetNumFields());
			pSpell = pNextSpell;
		}
	}

	return;
}


void SaveSpells(FILE *fp)
{
	assert(fp);

	Spell *pSpell;

	pSpell = (Spell *)Spell::GetFirst();

	pSpell->SaveFieldNames(fp);

	while(pSpell)
	{
		pSpell->SaveData(fp);
		pSpell = (Spell *)pSpell->GetNext();
	}
	return;
}

void DeleteSpells()
{
	Spell *pSpell, *pToDelete;

	//get the first Spell
	pToDelete = pSpell = (Spell *)Spell::GetFirst();

	while(pSpell)
	{
		pToDelete = pSpell;
		pSpell = (Spell *)pSpell->GetNext();
		delete pToDelete;
	}

	//GOT_HERE("Deleted Spells",NULL);
	return;
}

void Spell::Cast(int PowerLevel, Thing *pCaster, Object *pTarget)
{
	if(GetData(INDEX_ID).Value < 10)
	{
		FireBall *pFireBall;
		
		D3DVECTOR vStart =  *pCaster->GetPosition();
		D3DVECTOR vEnd = *pTarget->GetPosition();
		vStart.z += 1.0f;
		vEnd.z += 1.0f;

		int MinDamage;
		int MaxDamage;
		int HPDown = 0;
		int RPDown = 0;
		float Radius;

		switch(PowerLevel)
		{
			case 0:
				MinDamage = 3;
				MaxDamage = 6;
				Radius = 1.5f;
				RPDown = GetData("RESTCOST").Value;
				break;
			case 1:
				MinDamage = 4;
				MaxDamage = 8;
				Radius = 2.0f;
				RPDown = GetData("RESTCOST").Value;
				break;
			case 2:
				MinDamage = 2;
				MaxDamage = 5;
				Radius = 0.5f;
				RPDown = GetData("RESTCOST").Value;
				break;
			case 3:
				MinDamage = 6;
				MaxDamage = 12;
				Radius = 2.5f;
				RPDown = GetData("RESTCOST").Value;
				HPDown = GetData("RESTCOST").Value;
				break;
			default:
				MinDamage = 3;
				MaxDamage = 6;
				Radius = 2.5f;
				break;
		}
					
				
		pFireBall = new FireBall(vStart,
										 vEnd,
										 4.0f, 
										 0.9f, 
										 Radius,
										 MinDamage,
										 MaxDamage,
										 pCaster);	
		
		Valley->AddMainObject((Object *)pFireBall);
		pCaster->TakeDamage(HPDown, RPDown);
		
		int NumWait;
		
		NumWait = pFireBall->GetNumMoveFrames();
		
		if(NumWait > 0)
		{
			((Creature *)pCaster)->InsertAction(ACTION_WAITUNTIL, (void *)NumWait, (void *)0);
		}
	}
	else
	{
		if(pTarget->GetObjectType() == OBJECT_CREATURE)
		{
			int RestHeal = 0;
			int HitHeal = 0;

			switch(PowerLevel)
			{
				case 0:
					HitHeal = 10;
					break;
				case 1:
					HitHeal = 5;
					break;
				case 2:
					break;
				case 3:
					break;
				default:
					HitHeal = 5;
					break;
			}

			HealAura *pHealAura;

			pHealAura = new HealAura((Creature *)pTarget, RestHeal, HitHeal);

			int CurHP = ((Creature *)pTarget)->GetData("HITPOINTS").Value;
			CurHP += HitHeal;
			char blarg[64];
			
			if(CurHP <= ((Creature *)pTarget)->GetData("MAXHITPOINTS").Value)
			{
				((Creature *)pTarget)->SetData("HITPOINTS",CurHP);
				sprintf(blarg,"%s regains %i health.",((Creature *)pTarget)->GetData(INDEX_NAME).String, HitHeal);
				Describe(blarg);
			}
			else
			{
				((Creature *)pTarget)->SetData("HITPOINTS",((Creature *)pTarget)->GetData("MAXHITPOINTS").Value);
				sprintf(blarg,"%s is fully healed.",((Creature *)pTarget)->GetData(INDEX_NAME).String);
				Describe(blarg);
			}

			Valley->AddMainObject((Object *)pHealAura);
		}
	}

}

float Spell::GetRange(int PowerLevel, Thing *pCaster)
{
	float Range;
	float BaseRange;
	BaseRange = (float)GetData("RANGE").Value;
	switch(PowerLevel)
	{
		case 0:	//balanced
			return BaseRange;
		case 1:	//chaotic
			return BaseRange / 2;
		case 2:	//focussed
			return BaseRange * 1.5;
		case 3:
			return BaseRange * 1.5;
		default:
			return BaseRange;
	}


	return BaseRange;
}

