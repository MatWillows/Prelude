#include "spells.h"
#include "fireball.h"
#include "healaura.h"
#include "world.h"
#include <assert.h>
#include "creatures.h"
#include "spellfuncs.h"
#include "flags.h"
#include "script.h"
#include "scriptfuncs.h"
#include "events.h"



int (*SpellFunctions[MAX_SPELLS])(Object *pCaster, Object *pTarget, int Level, int CallType, int Data);
SpellList PreludeSpells;

BOOL SpellLevel::Load(FILE *fp)
{
	fread(Name,sizeof(char),32,fp);
	fread(&RestCost,sizeof(int),1,fp);
	fread(&HitCost,sizeof(int),1,fp);
	fread(&WillCost,sizeof(int),1,fp);
	fread(&SpeedCost,sizeof(int),1,fp);
	fread(&MinRange,sizeof(float),1,fp);
	fread(&MaxRange,sizeof(float),1,fp);
	fread(&SkillReq,sizeof(int),1,fp);
	fread(&Combat,sizeof(int),1,fp);
	fread(&TargetType,sizeof(int),1,fp);
	fread(&NeedBlood,sizeof(int),1,fp);
	fread(&NeedGoddess, sizeof(int),1,fp);
	fread(&ConfirmationEvent,sizeof(int),1,fp);

	int DLength;
	fread(&DLength,sizeof(int),1,fp);
	Description = new char[DLength];
	fread(Description,sizeof(char),DLength,fp);
	return TRUE;
}
	
BOOL SpellLevel::LoadText(FILE *fp)
{
	SeekTo(fp,"LEVELNAME:");
		GetString(fp,Name);
	SeekTo(fp,"RESTCOST:");
		RestCost = GetInt(fp);
	SeekTo(fp,"HITCOST:");
		HitCost = GetInt(fp);
	SeekTo(fp,"WILLCOST:");
		WillCost = GetInt(fp);
	SeekTo(fp,"SPEEDCOST:");
		SpeedCost = GetInt(fp);
	SeekTo(fp,"MINRANGE:");
		MinRange = GetFloat(fp);
	SeekTo(fp,"MAXRANGE:");
		MaxRange = GetFloat(fp);
	SeekTo(fp,"SKILLREQ:");
		SkillReq = GetInt(fp);
	SeekTo(fp,"COMBAT:");
		Combat = GetInt(fp);
	SeekTo(fp,"TARGET:");
		TargetType = GetInt(fp);
	SeekTo(fp,"BLOOD:");
		NeedBlood = GetInt(fp);
	SeekTo(fp,"GODDESS");
		NeedGoddess = GetInt(fp);
	SeekTo(fp,"EVENT");
		ConfirmationEvent = GetInt(fp);

	SeekTo(fp,"DESCRIPTION:");
	SeekTo(fp,"\"");
		Description = GetString(fp,'\"');
	return TRUE;
}

BOOL SpellLevel::Save(FILE *fp)
{
	fwrite(Name,sizeof(char),32,fp);
	fwrite(&RestCost,sizeof(int),1,fp);
	fwrite(&HitCost,sizeof(int),1,fp);
	fwrite(&WillCost,sizeof(int),1,fp);
	fwrite(&SpeedCost,sizeof(int),1,fp);
	fwrite(&MinRange,sizeof(float),1,fp);
	fwrite(&MaxRange,sizeof(float),1,fp);
	fwrite(&SkillReq,sizeof(int),1,fp);
	fwrite(&Combat,sizeof(int),1,fp);
	fwrite(&TargetType,sizeof(int),1,fp);
	fwrite(&NeedBlood,sizeof(int),1,fp);
	fwrite(&NeedGoddess, sizeof(int),1,fp);
	fwrite(&ConfirmationEvent,sizeof(int),1,fp);

	int DLength;
	DLength = strlen(Description) + 1;
	fwrite(&DLength,sizeof(int),1,fp);
	fwrite(Description,sizeof(char),DLength,fp);

	return TRUE;
}

BOOL Spell::Load(FILE *fp)
{
	fread(&ID, sizeof(ID),1,fp);
	fread(&Type,sizeof(Type),1,fp);
	fread(&NumLevels,sizeof(NumLevels),1,fp);

	Levels = new SpellLevel[NumLevels];
	for(int n = 0; n < NumLevels; n++)
	{
		Levels[n].Load(fp);
	}

	return TRUE;

}

BOOL Spell::LoadText(FILE *fp)
{
	SeekTo(fp,"SPELLID:");
	ID = GetInt(fp);
	SeekTo(fp,"TYPE:");
	Type = GetInt(fp);
	SeekTo(fp,"NUMLEVELS:");
	NumLevels = GetInt(fp);

	Levels = new SpellLevel[NumLevels];
	for(int n = 0; n < NumLevels; n++)
	{
		Levels[n].LoadText(fp);
	}
	
	return TRUE;
}

BOOL Spell::Save(FILE *fp)
{
	fwrite(&ID, sizeof(ID),1,fp);
	fwrite(&Type,sizeof(Type),1,fp);
	fwrite(&NumLevels,sizeof(NumLevels),1,fp);

	for(int n = 0; n < NumLevels; n++)
	{
		Levels[n].Save(fp);
	}
	return TRUE;
}

BOOL SpellList::Load(const char *filename)
{
	FILE *fp;
	int n;
	char *cp;
	cp = strchr(filename,'.');
	int TempID;
	fpos_t fpos;

	fp = fopen(filename, "rb");

	if(!fp)
	{
		fp = SafeFileOpen("newspells.txt","rt");
		SeekTo(fp,"NUMSPELLS:");
		NumSpells = GetInt(fp);
		Spells = new Spell[NumSpells];
		
		while(SeekTo(fp,"*SPELL*"))
		{
			fgetpos(fp,&fpos);
			SeekTo(fp, "SPELLID:");
			TempID = GetInt(fp);
			fsetpos(fp,&fpos);
			Spells[TempID].LoadText(fp);
		}
		fclose(fp);
		Save("newspells.bin");
	}
	else
	{
		fread(&NumSpells,sizeof(NumSpells),1,fp);
		Spells = new Spell[NumSpells];
		for(n = 0; n < NumSpells; n++)
		{
			Spells[n].Load(fp);
		}
	}

	fclose(fp);

/*	SpellBlocks = new ScriptBlock[NumSpells];

	fp = SafeFileOpen("castspells.bin","rb");
	if(fp)
	{
		for(n = 0; n < NumSpells; n++)
		{
			SpellBlocks[n].Load(fp);
		}
		fclose(fp);
	}
	else
	{
		fp = SafeFileOpen("castspells.txt","rt");
		assert(fp);
		FILE *fpsave;
		fpsave = SafeFileOpen("castspells.bin","wb");

		for(n = 0; n < NumSpells; n++)
		{
			SpellBlocks[n].Import(fp);
			SpellBlocks[n].Save(fpsave);
		}
		fclose(fp);
		fclose(fpsave);
	}
*/
	return TRUE;
}

BOOL SpellList::Save(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wb");

	fwrite(&NumSpells,sizeof(NumSpells),1,fp);
	for(int n = 0; n < NumSpells; n++)
	{
		Spells[n].Save(fp);
	}
	return TRUE;
}
	
BOOL SpellList::Cast(int Num, int Level, Object *pCaster, Object *pTarget)
{
	//do setup and deduct points here
	Creature *pCreature;
	pCreature = (Creature *)pCaster;

	pCreature->SetData(INDEX_RESTPOINTS,pCreature->GetData(INDEX_RESTPOINTS).Value - Spells[Num].GetLevel(Level)->RestCost);
	pCreature->SetData(INDEX_HITPOINTS,pCreature->GetData(INDEX_HITPOINTS).Value - Spells[Num].GetLevel(Level)->HitCost);
	pCreature->SetData(INDEX_WILLPOINTS,pCreature->GetData(INDEX_WILLPOINTS).Value - Spells[Num].GetLevel(Level)->WillCost);

	SpellFunctions[Num](pCaster,pTarget,Level,SPELL_CALL_CAST,0);

	return TRUE;
}
	
BOOL SpellList::Test(int Num, int Level, Object *pCaster, Object *pTarget)
{
	Creature *pCreature;
	Creature *pcTarget;
	pCreature = (Creature *)pCaster;

	if(Spells[Num].Levels[Level].NeedBlood && !BloodKnown->Value)
	{
		return FALSE;
	}

	if(Spells[Num].Levels[Level].NeedGoddess && GoddessAngered->Value)
	{
		return FALSE;
	}

	switch(Spells[Num].Levels[Level].TargetType)
	{
	case SPELL_TARGET_ANYWHERE:
		if(pTarget && pTarget->GetObjectType() == OBJECT_ITEM)
			return FALSE;
		break;
	case SPELL_TARGET_CREATURE:
		if(pTarget && pTarget->GetObjectType() != OBJECT_CREATURE)
			return FALSE;
		break;
	case SPELL_TARGET_ITEM:
		if(pTarget && pTarget->GetObjectType() != OBJECT_ITEM)
			return FALSE;
		break;
	case SPELL_TARGET_SELF:
		if(pTarget != pCaster)
			return FALSE;
		break;
	case SPELL_TARGET_FRIEND:
		if(!pTarget || pTarget->GetObjectType() != OBJECT_CREATURE)
		{
			return FALSE;
		}
		pcTarget = (Creature *)pTarget;
		if(pcTarget->GetData(INDEX_BATTLESIDE).Value != pCreature->GetData(INDEX_BATTLESIDE).Value)
			return FALSE;
		break;
	case SPELL_TARGET_ENEMY:
		if(!pTarget || pTarget->GetObjectType() != OBJECT_CREATURE)
		{
			return FALSE;
		}
		pcTarget = (Creature *)pTarget;
		if(pcTarget->GetData("BATTLESIDE").Value == pCreature->GetData("BATTLESIDE").Value)
			return FALSE;
		break;
	case SPELL_TARGET_EMPTY:
		if(pTarget && pTarget->GetObjectType() != OBJECT_STATIC)
		{
			return FALSE;
		}
		break;
	default:
		break;
	}

	int SkillLevel;
	switch(Spells[Num].Type)
	{
	case SPELL_FLAME:
		SkillLevel = pCreature->GetData(INDEX_POWER_OF_FLAME).Value;
		break;
	case SPELL_RIVER:
		SkillLevel = pCreature->GetData(INDEX_GIFTS_OF_GODDESS).Value;
		break;
	case SPELL_THAUMATURGY:
		SkillLevel = pCreature->GetData(INDEX_THAUMATURGY).Value;
		break;
	default:
	case SPELL_DEATH:
		SkillLevel = pCreature->GetData(INDEX_POWER_OF_FLAME).Value;
		break;
	}

	if(SkillLevel < Spells[Num].Levels[Level].SkillReq)
	{
		return FALSE;
	}

	if(pCreature->GetData(INDEX_ACTIONPOINTS).Value < Spells[Num].Levels[Level].SpeedCost)
	{
		return FALSE;
	}
	
	if(pCreature->GetData(INDEX_WILLPOINTS).Value < Spells[Num].Levels[Level].WillCost)
	{
		return FALSE;
	}
	
	if(pCreature->GetData(INDEX_RESTPOINTS).Value < Spells[Num].Levels[Level].RestCost)
	{
		return FALSE;
	}
	
	if(pCreature->GetData(INDEX_HITPOINTS).Value < Spells[Num].Levels[Level].HitCost)
	{
		return FALSE;
	}

	if(PreludeWorld->InCombat())
	{
		D3DVECTOR vCaster, vTarget;
		vCaster = *(pCaster->GetPosition());
		vTarget = *(pTarget->GetPosition());
		vCaster.z = 0.0f;
		vTarget.z = 0.0f;

		float Distance;
		Distance = GetDistance(&vCaster,&vTarget);
		
		if(Distance < Spells[Num].Levels[Level].MinRange || 
			Distance > Spells[Num].Levels[Level].MaxRange)
		{
			return FALSE;
		}
	}	
	else
	{
		if(Spells[Num].Levels[Level].Combat)
			return FALSE;
	}

	if(Spells[Num].Levels[Level].ConfirmationEvent)
	{
		Push((Creature *)pCaster);
		PreludeEvents.RunEvent(Spells[Num].Levels[Level].ConfirmationEvent);
		ScriptArg *pSA;
		pSA = Pop();
		int Passed;
		Passed = (int)pSA->GetValue();
		delete pSA;
		if(!Passed)
		{
			return FALSE;
		}
	}

	return TRUE;
}

int SpellList::AdvanceSpell(int Num, int Level, int Tick, Object *pCaster, Object *pTarget)
{



	return 0;
}

SpellList::SpellList()
{



}

void SpellList::Init()
{
	//load the spells;
	//assign the function pointers
	for(int n = 0; n < MAX_SPELLS; n++)
	{
		SpellFunctions[n] = NULL;
	}

	//FlameSpells
	SpellFunctions[0] = FlameFinger;
	SpellFunctions[1] = FireBallSpell;
	SpellFunctions[2] = FlamingHands;
	SpellFunctions[3] = Berserk;
	SpellFunctions[4] = Haste;
	SpellFunctions[5] = Flash;
	SpellFunctions[6] = Immolation;
	SpellFunctions[7] = Spear;
	SpellFunctions[8] = Sentinel;
	

	//RiverSpells
	SpellFunctions[10] = MothersHeal;
	SpellFunctions[11] = Slow;
	SpellFunctions[12] = Shield;
	SpellFunctions[13] = Growth;
	SpellFunctions[14] = Visage;
	SpellFunctions[15] = Weaken;
	SpellFunctions[16] = Strength;
	SpellFunctions[17] = Root;
	SpellFunctions[18] = Cycle;

	//Thaumaturgy
	SpellFunctions[20] = LightningStone;
	SpellFunctions[21] = KnockoutDust;
	SpellFunctions[22] = SharpWeapon;
	SpellFunctions[23] = FastWeapon;
	SpellFunctions[24] = Golem;
	SpellFunctions[25] = HardenArmour;
	SpellFunctions[26] = FixEnchantment;

	//this should be better handled, preferably with dynamic linking
	GoddessAngered = PreludeFlags.Get("GODDESSANGERED");
	BloodKnown = PreludeFlags.Get("BLOODMAGIC");

}

SpellTracker::SpellTracker(Object *pCast, Object *pTarg, int SNum, int LNum)
{
	pCaster = pCast;
	pTarg = pTarget;
	CurTick = 0;
	SpellNum = SNum;
	SpellLevel = LNum;
	Duration = PreludeSpells.GetSpell(SpellNum)->GetLevel(LNum)->Duration;

}

BOOL SpellTracker::AdvanceFrame()
{
	ULONG CurTime;
	CurTime = PreludeWorld->GetTotalTime();

	if(CurTime > LastCall)
	{
		LastCall = CurTime;
		CurTick++;
		
		SpellFunctions[SpellNum](pCaster,pTarget, SpellLevel, SPELL_CALL_ADVANCE, CurTick);

		if(CurTick >= Duration)
		{
			SpellFunctions[SpellNum](pCaster,pTarget,SpellLevel, SPELL_CALL_END, 0);
			return FALSE;
		}
	}

	return TRUE;
}









