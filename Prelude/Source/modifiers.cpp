#include "modifiers.h"
#include "world.h"
#include <assert.h>
#include "creatures.h"
#include "combatmanager.h"

void Modifier::SetUp(MOD_T mtype, unsigned long lStart, unsigned long lDuration, int prog, int modamount, Object *pNewSource, Object *pNewTarget, int NewStat)
{
	ModType = mtype;
	Start = lStart;
	Duration = lDuration;
	Progression = prog;
	Amount = modamount;
	pSource = pNewSource;
	pTarget = pNewTarget;
	Stat = NewStat;

}

Modifier::Modifier()
{
	//clear out just in case?
	pAura = NULL;
	pSource = NULL;
	pTarget = NULL;
	
	ModType = MOD_SPELL;  //type, 99% spell
	Combat = FALSE;  //combat or not
	Start = 0; //start round if combat, start minute if not
	Duration = 0; //duration in rounds of combat or in minutes
	Progression = 0; //progressive change to stat on a per round or per half hour basis
	Stat = 0; //the stat being modified
	Amount = 0;	//the amount of the modifier
	SpellNum = 0; //spell that activated it.
}


BOOL Modifier::AdvanceFrame()
{
	int CurRound;
	Creature *pCreature;
	pCreature = (Creature *)pTarget;
	if(Combat)
	{
		if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
		{
			this->Remove();	
			return FALSE;
		}
		else
		{
			CurRound = PreludeWorld->GetCombat()->GetRound();

			if(CurRound > GetStart() + GetDuration())
			{
				return FALSE;
			}
			else
			{
				//check for a progression
				if(Progression)
				{
					Amount += Progression;
					if(Stat != pCreature->GetIndex("HITPOINTS"))
					{
						pCreature->SetData(Stat, pCreature->GetData(Stat).Value + Progression);
					}
					else
					{
						if(Amount > 0)
						{
							pCreature->TakeDamage(NULL, Amount, DAMAGE_NORMAL);
						}
						else
						{
							pCreature->SetData(Stat, (pCreature->GetData(Stat).Value) + Amount);
							if(pCreature->GetData(INDEX_HITPOINTS).Value > 
								pCreature->GetData(INDEX_MAXHITPOINTS).Value)
							{
								pCreature->SetData(INDEX_HITPOINTS,pCreature->GetData(INDEX_MAXHITPOINTS).Value);
							}
						}
					}
				}
				return TRUE;
			}
		}
	}
	else
	{
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			this->Remove();
			return FALSE;
		}
		else
		{
			CurRound = PreludeWorld->GetTotalTime();
			if(CurRound > GetStart() + GetDuration())
			{
				this->Remove();
				return FALSE;
			}
			return TRUE;
		}
	}
	assert(0); //should never get here...
	return TRUE;
}

void Modifier::Remove()
{
	//undo the modifier
	if(pTarget->GetObjectType() != OBJECT_CREATURE)
	{
		return;
	}

	Creature *pCreature;
	pCreature = (Creature *)pTarget;

	if(Stat != pCreature->GetIndex("HITPOINTS"))
	{
		pCreature->SetData(Stat, pCreature->GetData(Stat).Value -Amount);
	}
	else
	{
		if(Amount > 0)
		{
			pCreature->TakeDamage(NULL, Amount, DAMAGE_NORMAL);
		}
		else
		{
			pCreature->SetData(Stat, (pCreature->GetData(Stat).Value) + Amount);
			if(pCreature->GetData(INDEX_HITPOINTS).Value > 
				pCreature->GetData(INDEX_MAXHITPOINTS).Value)
			{
				pCreature->SetData(INDEX_HITPOINTS,pCreature->GetData(INDEX_MAXHITPOINTS).Value);
			}
		}
	}

	if(pAura)
	{
		PreludeWorld->RemoveMainObject(pAura);
		delete pAura;
	}

	return;
}

void Modifier::Apply()
{
	if(!pTarget || pTarget->GetObjectType() != OBJECT_CREATURE)
	{
		return;
	}

	Object *pOb;
	Modifier *pMod;
	pOb = PreludeWorld->GetMainObjects();
	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_MODIFIER && pOb != this)
		{
			pMod = (Modifier *)pOb;
			pOb = pOb->GetNext();	
			if(pMod->GetTarget() == this->GetTarget() && pMod->GetStat() == this->GetStat())
			{
				DEBUG_INFO("Overriding mod\n");
				pMod->Remove();
				PreludeWorld->RemoveMainObject(pMod);
				delete pMod;
			}
			else
			{

			}
		}
		else
		{
			pOb = pOb->GetNext();
		}
	}

	Creature *pCreature;
	pCreature = (Creature *)pTarget;


	if(Stat != pCreature->GetIndex("HITPOINTS"))
	{
		pCreature->SetData(Stat, pCreature->GetData(Stat).Value + Amount);
	}
	else
	{
		if(Amount > 0)
		{
			pCreature->TakeDamage(NULL, Amount, DAMAGE_NORMAL);
		}
		else
		{
			pCreature->SetData(Stat, (pCreature->GetData(Stat).Value) + Amount);
			if(pCreature->GetData(INDEX_HITPOINTS).Value > 
				pCreature->GetData(INDEX_MAXHITPOINTS).Value)
			{
				pCreature->SetData(INDEX_HITPOINTS,pCreature->GetData(INDEX_MAXHITPOINTS).Value);
			}
		}
	}

	return;
}


void Modifier::Save(FILE *fp)
{
	OBJECT_T obMod = OBJECT_MODIFIER;
	fwrite(&obMod, sizeof(OBJECT_T),1,fp);
	//type of modifier
	fwrite(&ModType,sizeof(MOD_T),1,fp);  //type, 99% spell
	fwrite(&Combat, sizeof(BOOL),1,fp);//combat or not
	fwrite(&Start, sizeof(unsigned long),1,fp); //start round if combat, start minute if not
	fwrite(&Duration, sizeof(unsigned long),1,fp); //duration in rounds of combat or in minutes
	fwrite(&Progression, sizeof(int),1,fp); //progressive change to stat on a per round or per half hour basis
	fwrite(&Stat, sizeof(int),1,fp); //the stat being modified
	fwrite(&Amount, sizeof(int),1,fp);	//the amount of the modifier
	assert(!pAura);
	
	int cID;
	int cUID;
	BOOL Test;
	if(pSource)
	{
		cID = ((Creature *)pSource)->GetData(INDEX_ID).Value;
		cUID = ((Creature *)pSource)->GetData(INDEX_UID).Value;
		Test = TRUE;
		fwrite(&Test, sizeof(BOOL), 1, fp);
		fwrite(&cID, sizeof(int), 1, fp);
		fwrite(&cUID, sizeof(int), 1, fp);
	}
	else
	{
		Test = FALSE;
		fwrite(&Test, sizeof(BOOL), 1, fp);
	}

	//fwrite(&Object *pTarget; //creature being affected
	if(pTarget)
	{
		cID = ((Creature *)pTarget)->GetData(INDEX_ID).Value;
		cUID = ((Creature *)pTarget)->GetData(INDEX_UID).Value;
		Test = TRUE;
		fwrite(&Test, sizeof(BOOL), 1, fp);
		fwrite(&cID, sizeof(int), 1, fp);
		fwrite(&cUID, sizeof(int), 1, fp);
	}
	else
	{
		Test = FALSE;
		fwrite(&Test, sizeof(BOOL), 1, fp);
	}
		
	fwrite(&SpellNum, sizeof(int), 1, fp); //spell that activated it.
}

void Modifier::Load(FILE *fp)
{
	//type of modifier
	fread(&ModType,sizeof(MOD_T),1,fp);  //type, 99% spell
	fread(&Combat, sizeof(BOOL),1,fp);//combat or not
	fread(&Start, sizeof(unsigned long),1,fp); //start round if combat, start minute if not
	fread(&Duration, sizeof(unsigned long),1,fp); //duration in rounds of combat or in minutes
	fread(&Progression, sizeof(int),1,fp); //progressive change to stat on a per round or per half hour basis
	fread(&Stat, sizeof(int),1,fp); //the stat being modified
	fread(&Amount, sizeof(int),1,fp);	//the amount of the modifier
	//fread(&Object *pAura; //magical effect displayed, usually on the target
	pAura = NULL;
	//fread(&Object *pSource; //source of the modifier
	int cID;
	int cUID;
	BOOL Test;
	
	fread(&Test, sizeof(BOOL), 1, fp);
	
	if(Test)
	{
		fread(&cID, sizeof(int), 1, fp);
		fread(&cUID, sizeof(int), 1, fp);
		pSource = Creature::Find(Creature::GetFirst(),cID, cUID);
	}
	else
	{
		pSource = NULL;
	}

	fread(&Test, sizeof(BOOL), 1, fp);
	
	if(Test)
	{
		fread(&cID, sizeof(int), 1, fp);
		fread(&cUID, sizeof(int), 1, fp);
		pTarget = Creature::Find(Creature::GetFirst(),cID, cUID);
	}
	else
	{
		pTarget = NULL;
	}
		
	fread(&SpellNum, sizeof(int),1,fp); //spell that activated it.
}
