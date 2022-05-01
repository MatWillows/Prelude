//********************************************************************* 
//********************************************************************* 
//**************					creatures.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision: 7/25                                                     * 
//*Revisor: Mat Williams                                           
//*Purpose: Provide Creature functionality for Prelud                       
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        	Textures          
//*			
//********************************************************************* 
//*********************************************************************
#include "creatures.h"
#include "zsutilities.h"
#include <assert.h>
#include "missile.h"
#include "blood.h"
#include "zsfire.h"
#include "explosion.h"
#include "fireball.h"
#include "spells.h"
#include "spellbook.h"
#include "castaura.h"
#include "world.h"
#include "portals.h"
#include "regions.h"
#include "zsportrait.h"
#include "party.h"
#include "mainwindow.h"
#include "script.h"
#include "zstalk.h"
#include "mapwin.h"
#include "characterwin.h"
#include "combatmanager.h"
//#include "restwin.h"
#include "journal.h"
#include "gameitem.h"
#include "Locator.h"
#include "zsoptions.h"
#include "events.h"
#include "scriptfuncs.h"
#include "corpse.h"
#include "zsrest.h"
#include "MovePointer.h"
#include "zsmenubar.h"
#include "ZsContainer.h"
#include "entrance.h"
#include "spells.h"
#include "characterwin.h"
#include "zsconfirm.h"
#include "attacks.h"
#include "zsweapontrace.h"
#include "path.h"
#include "zssaychar.h"
#include "zsmessage.h"

#define WALK_DIVISOR 6.0f

#define BASE_ATTACK_POINTS	5
#define BASE_MIN_DAMAGE		0
#define BASE_MAX_DAMAGE		4
#define AVERAGE_SKILL_FACTOR 5070.0f

//path check time in game minutes	
#define PATH_CHECK_TIME		5

#define DISMISS_EVENT	60





//************** static Members *********************************

char Creature::CreatureFieldNames[160*32];
int Creature::NumCreatures = 0;
DATA_T *Creature::CreatureDataTypes = NULL;

Creature *Creature::pFirst = NULL; 
AnimRange Creature::Animations[12];

ScriptBlock *Creature::pSBImproveSkill = NULL;
RECT Creature::BodyDest[CHEST + 1];
int  Creature::BodyOverLayWidth[CHEST + 1];
int  Creature::BodyOverLayHeight[CHEST + 1];
int  Creature::BodyFileWidth[CHEST + 1];
int  Creature::BodyFileHeight[CHEST + 1];
RECT *Creature::BodySource[CHEST + 1];
int  Creature::NumBodyTextures[CHEST + 1];
LPDIRECTDRAWSURFACE7 Creature::EquipSource[CHEST + 1];
LPDIRECTDRAWSURFACE7 Creature::Skins[3][NUM_SKINS];
ZSModelEx *Creature::pIntersectBox;
int Creature::EquipHeadIndex = 0;
TextureGroup Creature::PeopleTextures(128,128);

LPDIRECTDRAWSURFACE7 TempSurf = NULL;

D3DLVERTEX Creature::ShadowVerts[4];
	
//************** Constructors  ****************************************

//simple constructor
Creature::Creature()
{
	FrameAdd = 0;
	LastPlacedTime = 0;
	DamageOverride = 0;
	SecondaryDamageOverride = 0;
	Acted = FALSE;

	Schedule = NULL;
	Created = FALSE;
	pRegionIn = NULL;
	LastChecked = 0;
	ZeroMemory(SkillImproved,sizeof(int) * MAX_NUM_SKILLS);
	ZeroMemory(KnownSpells,MAX_SPELLS);
	ZeroMemory(Equipment,sizeof(GameItem *) * MAX_EQUIPMENT);
	ReadySpell = -1;
	fWalkFrames = 6.0f;

	AreaIn = -1;

	pPrev = NULL;
	DataTypes = NULL;
	if(!NumCreatures)
	{
		InitTextures();

		TempSurf = Engine->Graphics()->CreateSurface(128,128,0,Engine->Graphics()->GetMask());

		if(!TempSurf)
		{
			SafeExit("Failed to create temporary creature surface");
		}
	}

	xoff = yoff = zoff = angleoff = 0.0f;

	pCurAction = NULL;
	//place a think action in the current action
	AddAction(ACTION_THINK,NULL,0);
	//increment the number of creatures in the game
	NumCreatures++;
	ActionCount = 0;
	pPortrait = NULL;
	Active = FALSE;
	pNext = pFirst;
	pFirst = this;
	if(pNext)
	{
		((Creature *)pNext)->SetPrev(this);
	}

	pTexture = NULL;
	
	pEORHand = NULL;
	pEOLHand = NULL;
	pEOBack = NULL;
	pEOHead = NULL;

	NumLocators = 0;
	Schedule = NULL;
	CurLocator = 0;
	pNextUpdate = NULL;
	pPrevUpdate = NULL;
	Large = 0;
	AmmoItemNumber = 0;


}

//copy constructor
Creature::Creature(Thing *pFromThing)
{
	FrameAdd = 0;
	LastPlacedTime = 0;
	AmmoItemNumber = 0;
	DamageOverride = 0;
	SecondaryDamageOverride = 0;

	Acted = FALSE;
	Created = FALSE;
	pRegionIn = NULL;
	Schedule = NULL;
	Large = 0;
	LastChecked = 0;
	ZeroMemory(SkillImproved,sizeof(int) * MAX_NUM_SKILLS);
	ZeroMemory(KnownSpells,MAX_SPELLS);
	ReadySpell = -1;
	AreaIn = -1;

	ZeroMemory(Equipment,sizeof(GameItem *) * MAX_EQUIPMENT);

	pPortrait = NULL;
	pPrev = NULL;
	pCurAction = NULL;
	Creature *pFrom;
	pFrom = (Creature *)pFromThing;
	fWalkFrames = pFrom->fWalkFrames;
	
	DataFieldNames = pFrom->DataFieldNames;
	NumFields = pFrom->NumFields;
	//assign member variables directly where possible
	DataFields = new DATA_FIELD_T[NumFields];
	
	if(pFrom->DataTypes != CreatureDataTypes)
	{
		DataTypes = new DATA_T[NumFields];
		memcpy(DataTypes,pFrom->DataTypes,sizeof(DATA_T)*NumFields);
	}
	else
	{
		DataTypes = CreatureDataTypes;
	}

	//create space for the data fields
	for(int n =0;n< NumFields;n++)
	{
		switch(DataTypes[n])
		{
		case DATA_INT:
		case DATA_FLOAT:
			DataFields[n] = pFrom->DataFields[n];
			break;
		case DATA_STRING:
			DataFields[n].String = new char[strlen(pFrom->DataFields[n].String) + 1];
			strcpy(DataFields[n].String,pFrom->DataFields[n].String);
			break;
		case DATA_VECTOR:
			DataFields[n].pVector = new D3DVECTOR;
			memcpy(DataFields[n].pVector, pFrom->DataFields[n].pVector,sizeof(D3DVECTOR));
			break;
		default:
			DataFields[n] = DataFields[n];
			break;
		}
	}
	if(pFrom->GetData(INDEX_BATTLEID).Value)
	{
		int UID;
		int ID;
		UID = -1;
		ID = pFrom->GetData(INDEX_ID).Value;
		Creature *pCreature;

		do
		{
			UID++;
			pCreature = (Creature *)Thing::Find(Creature::GetFirst(),ID,UID);
		}while(pCreature);
	
		this->SetData(INDEX_UID, UID);
	}

	memcpy(this->KnownSpells,pFrom->KnownSpells,MAX_SPELLS);
	this->ReadySpell = pFrom->ReadySpell;

	//copy the data fields from the source into the new object
	//set the unique ID (UID) to be equal to the next unique ID
	UniqueID = NextUniqueID;
	
	//increment the next unique ID
	NextUniqueID++;

	//done
	pCurAction = NULL;
	//place a think action in the current action
	AddAction(ACTION_THINK,NULL,0);
	//increment the number of creatures in the game
	NumCreatures++;
	ActionCount = 0;
	xoff = yoff = zoff = angleoff = 0.0f;
	Active = FALSE;
	pNext = pFirst;
	pFirst = this;
	if(pNext)
	{
		((Creature *)pNext)->SetPrev(this);
	}

	pMesh = pFrom->pMesh;
	Large = pFrom->Large;
	pTexture = pFrom->pTexture;
	
//	CreateTexture();

	pEORHand = NULL;
	pEOLHand = NULL;
	pEOBack = NULL;
	pEOHead = NULL;

	CreateInventory();
	this->ReEquip();

	SetEquipObjects();

//	SetData(INDEX_ID,UniqueID);

	//set up the starting values
	NumLocators = 0;
	Schedule = NULL;
	CurLocator = 0;
	pNextUpdate = NULL;
	pPrevUpdate = NULL;

	this->Scale = this->GetData(INDEX_SCALE).fValue;

//make it living
	this->SetData(INDEX_BATTLESTATUS,CREATURE_STATE_NONE);

}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************

Creature::~Creature()
{
	if(PreludeWorld)
	{
		if(AreaIn != -1)
			PreludeWorld->GetArea(AreaIn)->RemoveFromUpdate(this);
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			PreludeWorld->GetCombat()->RemoveFromCombat(this);
	}

	if(pFirst == this)
	{
		pFirst = (Creature *)this->GetNext();
	}

	if(pPrev)
	{
		pPrev->SetNext((Creature *)this->GetNext());
	}

	if(pNext)
	{
		((Creature *)pNext)->SetPrev(this->GetPrev());
	}
	//destroy the action stack
	//clear all the actions except the think
	KillActions();
	
	pCurAction = 0;
	//decrement the number of creatures in the game
	NumCreatures--;
	if(DataTypes != CreatureDataTypes)
	{
		delete[] DataTypes;
	}

	for(int n = 0; n < MAX_EQUIPMENT; n++)
	{
		if(Equipment[n])
		{
			delete Equipment[n];
			Equipment[n] = NULL;
		}
	}

	if(pTexture && !Engine->GetTextureNum(pTexture))
	{
		PeopleTextures.FreeTexture(pTexture);
	}

	pTexture = NULL;
	
	if(!NumCreatures)
	{
		DestroyFieldNames();
		int n;

		for(n = 0; n <= CHEST; n++)
		{
			delete BodySource[n];
			BodySource[n] = NULL;
			EquipSource[n]->Release();
			EquipSource[n] = 0;
		}
		TempSurf->Release();
		if(CreatureDataTypes)
			delete[] CreatureDataTypes;
		CreatureDataTypes = NULL;
	}

	if(Schedule && NumLocators)
	{
		delete[] Schedule;
	}

	
	if(PreludeWorld)
	{
		if(pEORHand)
		{
			PreludeWorld->RemoveMainObject(pEORHand);
			delete pEORHand;
			pEORHand = NULL;
		}

		if(pEOLHand)
		{
			PreludeWorld->RemoveMainObject(pEOLHand);
			delete pEOLHand;	
			pEOLHand = NULL;
		}

		if(pEOHead)
		{
			PreludeWorld->RemoveMainObject(pEOHead);
			delete pEOHead;	
			pEOHead = NULL;
		}

		if(pEOBack)
		{
			PreludeWorld->RemoveMainObject(pEOBack);
			delete pEOBack;	
			pEOBack = NULL;
		}
	}



}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************


//end: Accessors *******************************************************
void Creature::GetSpellMenu(Object *pActor, ActionMenu *pAMenu)
{
	char ActionString[64];
	int AP;
	AP = this->GetData(INDEX_ACTIONPOINTS).Value;

	for(int sn = 0; sn < PreludeSpells.GetNumSpells(); sn++)
	{
		if(SpellFunctions[sn] && KnownSpells[sn])
		{
			for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
			{
				if(PreludeSpells.Test(sn,n,(Object *)this,pActor))
				{
					int EncodedSpell;
					int SpellAP;
					SpellAP = PreludeSpells.GetSpell(sn)->GetLevel(n)->SpeedCost;
					EncodedSpell = sn * 10 + n;
					if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
					{
						sprintf(ActionString,"%s  [%i/%i]",PreludeSpells.GetSpell(sn)->GetLevel(n)->Name,SpellAP,AP);
					}
					else
					{
						sprintf(ActionString,"%s",PreludeSpells.GetSpell(sn)->GetLevel(n)->Name);
					}

					pAMenu->AddAction(ACTION_CAST, (void *)pActor, (void *)EncodedSpell, ActionString);
				}
			}
		}
	}
}

void Creature::GetValidSelfCombatActions(Object *pActor, ActionMenu *pAMenu)
{

}
void Creature::GetValidSelfNonCombatActions(Object *pActor, ActionMenu *pAMenu)
{
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_CHARACTER,(void *)pActor,"Character");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_MAP,NULL,"Map");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_JOURNAL,NULL,"Journal");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_REST,NULL,"Rest");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_OPTIONS,NULL,"Options");

	if(!Created)
		pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_DISMISS,(void *)pActor,"Dismiss");
	
}

void Creature::GetValidFriendCombatActions(Object *pActor, ActionMenu *pAMenu)
{

}

void Creature::GetValidFriendNonCombatActions(Object *pActor, ActionMenu *pAMenu)
{
	Creature *pCreature;
	pCreature = (Creature *)pActor;
	if(!pCreature->GetData(INDEX_BATTLESIDE).Value)
	{
		if(pCreature->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS &&
				pCreature->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_DEAD &&
				pCreature->GetRegionIn() == this->GetRegionIn() &&
				GetDistance(pCreature, this) < 12.0f)
		{
			pAMenu->AddAction(ACTION_TALK,(void *)pActor,NULL,"Talk");
		}		
		else
		{
			pAMenu->AddAction(ACTION_NONE,NULL, NULL,"Too Far to Converse");
		}
			
		if(GetData(INDEX_PICKPOCKET).Value)
		{
			pAMenu->AddAction(ACTION_PICKPOCKET,(void *)pActor,NULL,"PickPocket");
		}
	}
}

void Creature::GetValidPartyNonCombatActions(Object *pActor, ActionMenu *pAMenu)
{
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_SET_LEADER,(void *)pActor,"Make Leader");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_CHARACTER,(void *)pActor,"Character");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_MAP,NULL,"Map");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_JOURNAL,NULL,"Journal");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_REST,NULL,"Rest");
	pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_OPTIONS,NULL,"Options");
	if(!((Creature *)pActor)->WasCreated())
		pAMenu->AddAction(ACTION_GAME_COMMAND,(void *)ACTION_GAME_COMMAND_DISMISS,(void *)pActor,"Dismiss");
}

void Creature::GetValidPartyCombatActions(Object *pActor, ActionMenu *pAMenu)
{


}



void Creature::GetValidEnemyCombatActions(Object *pActor, ActionMenu *pAMenu)
{
	float Distance;
	float WeaponRange;
	Creature *pCreature;
	pCreature = (Creature *)pActor;
	char ActionString[64];
	Distance = GetDistance(this, pCreature);

	//add cast spell on if the actor can cast spells

	int AP;
	AP = GetData(INDEX_ACTIONPOINTS).Value;
	//if we are in combat
	BOOL Ammo;
	
	if(GetData(INDEX_BATTLESIDE).Value != pCreature->GetData(INDEX_BATTLESIDE).Value)
	{
		//enemy
		Ammo = this->HasAmmo();

		int mx, my, tx, ty;
		mx = this->GetPosition()->x;
		my = this->GetPosition()->y;
		tx = pCreature->GetPosition()->x;
		ty = pCreature->GetPosition()->y;

		WeaponRange = GetRange();
		if(WeaponRange >= Distance && Ammo &&
			PreludeWorld->GetCombat()->CheckLineOfSight(mx,my,tx,ty,(Object *)this, pActor))
		{
			float APCost;
			int FinalApCost;
		
			APCost = (float)GetData(INDEX_ATTACKPOINTS).Value;

			//check for passive bonuses to daggers and unarmed
			GameItem *pGI;
			int WeaponType;		
			pGI = this->GetEquipment("RIGHTHAND");
			if(pGI)
			{
				//dagger speed bonus
				if(pGI->GetData("TYPE").Value == 2 && 
					pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_DAGGER && 
					this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				{
					APCost -= 1;
				}
			}
			else
			{
				//unarmed speed bonus
				if(this->GetWeaponSkill() >= 20)
				{
					APCost -= 1;
				}
			}
			
			FinalApCost = (int)APCost;
				
			if(FinalApCost <= AP)
			{
				sprintf(ActionString,"Attack   [%i/%i]",FinalApCost,AP);
				pAMenu->AddAction(ACTION_ATTACK,(void *)pCreature,(void *)ATTACK_NORMAL,ActionString);
			}

			FinalApCost = (int)((APCost * STRONG_AP_MULTIPLIER)+ 0.5f);
			if(FinalApCost <= AP && WeaponRange < 2.5f)
			{	
				sprintf(ActionString,"Strong Attack   [%i/%i]",FinalApCost,AP);
				pAMenu->AddAction(ACTION_ATTACK,(void *)pCreature,(void *)ATTACK_STRONG,ActionString);
			}

			FinalApCost = (int)((APCost * QUICK_AP_MULTIPLIER)+ 0.5f);
			if(FinalApCost <= AP)
			{
				sprintf(ActionString,"Quick Attack   [%i/%i]",FinalApCost,AP);
				pAMenu->AddAction(ACTION_ATTACK,(void *)pCreature,(void *)ATTACK_RAPID,ActionString);
			}

			FinalApCost = (int)(1.0f + (APCost * AIMED_AP_MULTIPLIER)+ 0.5f);
			if(FinalApCost <= AP)
			{
				sprintf(ActionString,"Aimed Attack   [%i/%i]",FinalApCost,AP);
				pAMenu->AddAction(ACTION_ATTACK,(void *)pCreature,(void *)ATTACK_AIMED,ActionString);
			}

			//check for special skill
			BOOL CanFeint = TRUE;

			if(pGI)
			{
				WeaponType = pGI->GetData("SUBTYPE").Value;
			}
			else
			{
				WeaponType = WEAPON_TYPE_UNARMED;
			}

			switch(WeaponType)
			{
				case WEAPON_TYPE_MISSILE:
				case WEAPON_TYPE_THROWN:
					break;
				case WEAPON_TYPE_SWORD:
				case WEAPON_TYPE_DAGGER:
				case WEAPON_TYPE_AXE:
				case WEAPON_TYPE_BLUNT:
				case WEAPON_TYPE_POLEARM:
				case WEAPON_TYPE_UNARMED:
				default:
					FinalApCost = (int)((APCost * FEINT_AP_MULTIPLIER)+ 0.5f);
					if(FinalApCost <= AP)
					{
						sprintf(ActionString,"Feint   [%i/%i]",FinalApCost,AP);
						pAMenu->AddAction(ACTION_ATTACK,(void *)pCreature,(void *)ATTACK_FEINT,ActionString);
					}		
				break;
			}

			if(this->GetWeaponSkill() >= 40)
			{
				if(pGI)
				{
					WeaponType = pGI->GetData("SUBTYPE").Value;
				}
				else
				{
					WeaponType = WEAPON_TYPE_UNARMED;
				}

				FinalApCost = (int)((APCost * SPECIAL_AP_MULTIPLIER) + 0.5f);
				if(FinalApCost <= AP)
				{
					
					switch(WeaponType)
					{
						case WEAPON_TYPE_SWORD:
							sprintf(ActionString,"* Spin *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_DAGGER:
							sprintf(ActionString,"* Rapid *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_AXE:
							sprintf(ActionString,"* Rend *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_BLUNT:
							sprintf(ActionString,"* Brain *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_POLEARM:
							sprintf(ActionString,"* Sweep *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_MISSILE:
							sprintf(ActionString,"* Cripple *  [%i/%i]",FinalApCost,AP);
							break;
						default:
						case WEAPON_TYPE_UNARMED:
							sprintf(ActionString,"* Disarm *  [%i/%i]",FinalApCost,AP);
							break;
						case WEAPON_TYPE_THROWN:
							sprintf(ActionString,"* Target *  [%i/%i]",FinalApCost,AP);
							break;
					}

					pAMenu->AddAction(ACTION_SPECIAL_ATTACK,(void *)pCreature,(void *)ATTACK_FEINT,ActionString);
				}
			}
		}
		else
		{
			if(WeaponRange < Distance)
			{
				pAMenu->AddAction(ACTION_NONE,NULL,NULL,"Out of Attack Range");
			}
			
			if(!Ammo)
			{
				pAMenu->AddAction(ACTION_NONE,NULL,NULL,"Out of Ammo");
			}
			
			if(Ammo && WeaponRange >= Distance)
			{
				pAMenu->AddAction(ACTION_NONE,NULL,NULL,"Obstacle Interference");
			}
						
			pAMenu->AddAction(ACTION_APPROACH,(void *)pCreature,(void *)((int)GetRange() - 1),"Approach");
		}
	}
	else  //friend
	{

	}
}


int Creature::GetValidActions(Object *pActor, ActionMenu *pAMenu)
{
	if(!pActor)
	{
		pActor = this;
	}	
	else
	{
		//Top is always look at
		if(pActor->GetObjectType() != OBJECT_STATIC)
			pAMenu->AddAction(ACTION_LOOKAT,(void *)pActor,0,"Examine");
	}

	BOOL CanTargetCast = FALSE;
	int sn = 0; //spell number	
	char ActionString[64];
	int AP;
	AP = GetData(INDEX_ACTIONPOINTS).Value;
	Creature *pCreature;

	switch(pActor->GetObjectType())
	{
		case OBJECT_CREATURE:
			pCreature = (Creature *)pActor;
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				//combat
				if(pActor == this)
				{
					GetValidSelfCombatActions(pActor,pAMenu);
				}
				else
				if(PreludeParty.IsMember((Creature *)pActor))
				{
					GetValidPartyCombatActions(pActor,pAMenu);
				}
				else
				{
					GetValidEnemyCombatActions(pActor,pAMenu);
				}
				sprintf(ActionString,"Defend   [%i/%i]",AP,AP);
				pAMenu->AddAction(ACTION_DEFEND,(void *)this,(void *)0,ActionString);
				
				sprintf(ActionString,"Wait   [\?\?/%i]",AP);
				pAMenu->AddAction(ACTION_WAITACTION,(void *)NULL,(void *)0,ActionString);
			}
			else
			{
				//non-combat
				if(pActor == this)
				{
					GetValidSelfNonCombatActions(pActor,pAMenu);
				}
				else
				if(PreludeParty.IsMember((Creature *)pActor))
				{
					GetValidPartyNonCombatActions(pActor,pAMenu);
				}
				else
				{
					GetValidFriendNonCombatActions(pActor,pAMenu);
				}
			}
		
		
		//spell options
		if(GetData(INDEX_SPELLBOOK).Value)
		{
			int MinSpellAp;
			MinSpellAp = this->GetMinSpellAp();
						
			float fMinSpellRange;
			fMinSpellRange = this->GetMinSpellRange();

			float fMaxSpellRange;
			fMaxSpellRange = (int)this->GetMaxSpellRange() + 0.5f;

			Creature *pCreature;
			pCreature = (Creature *)pActor;
			float Distance;
			Distance = GetDistance(this, pCreature);

			if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
			{
				fMinSpellRange = 0.0f;
				fMaxSpellRange = 12.5f;
			}

			if(MinSpellAp > AP)
			{
				pAMenu->AddAction(ACTION_NONE, (void *)NULL, NULL, "Not Enough AP to Cast");
			}
			else
			if(Distance < fMinSpellRange || Distance > fMaxSpellRange)
			{
				pAMenu->AddAction(ACTION_NONE, (void *)NULL, NULL, "No Spell In Range");
			}
			else
			{
				sn = 0;
				while(!CanTargetCast && sn < MAX_SPELLS)
				{
					if(KnownSpells[sn])
					{
						for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
						{
							if(PreludeSpells.Test(sn,n,(Object *)this,pActor))
							{
								CanTargetCast =  TRUE;
								break;
							}
						}
					}
					sn++;
				}
				
				if(CanTargetCast)
				{
					pAMenu->AddAction(ACTION_CAST, (void *)pActor, NULL, "Cast");
				}
				else
				{
					pAMenu->AddAction(ACTION_NONE, NULL, NULL, "No Spells Valid for Target");
				}
			}
		}

		break; //object creature
	case OBJECT_ENTRANCE:
		pAMenu->AddAction(ACTION_OPEN, (void *)pActor,NULL,"Enter");
		break;	
	case OBJECT_PORTAL:
		Portal *pPortal;
		pPortal = (Portal *)pActor;
		
		if(pPortal->GetState() == PORTAL_CLOSED)
		{
			int LockValue;
			LockValue = pPortal->GetLock();
			if(LockValue)
			{
				if(LockValue < 100)
					pAMenu->AddAction(ACTION_OPEN,(void *)pPortal,(void *)1,"Pick");
	
				if(pPortal->GetKey())
				{
					Thing *pItem;
					pItem = Thing::Find((Thing *)Item::GetFirst(),pPortal->GetKey());
					if(PreludeParty.Has(pItem))
					{
						pAMenu->AddAction(ACTION_OPEN,(void *)pPortal,(void *)2,"Open");
					}
				}
			}
			else
			{
				pAMenu->AddAction(ACTION_OPEN,(void *)pPortal,(void *)0,"Open");
			}
		}
		else
		if(pPortal->GetState() == PORTAL_OPEN)
		{
			if(Valley->IsClear((int)pPortal->GetPosition()->x, (int)pPortal->GetPosition()->y))
				pAMenu->AddAction(ACTION_CLOSE,(void *)pPortal,(void *)0,"Close");
		}
		break;
	case OBJECT_ITEM:
		GameItem *pGI;
		pGI = (GameItem *)pActor;

		if(pGI->GetRegionIn() == this->GetRegionIn() &&
				GetDistance(pGI->GetPosition(), this->GetPosition()) < 12.0f)
		{
			if(pGI->GetItem()->GetData("CONTAINERSIZE").Value)
			{
				pAMenu->AddAction(ACTION_OPEN,(void *)pGI,(void *)0,"Open");
			}
		}
		else
		{
			if(pGI->GetItem()->GetData("CONTAINERSIZE").Value)
			{
				pAMenu->AddAction(ACTION_NONE,NULL, NULL,"Too Far to to Open.");
			}
		}
		
		if(pGI->GetLocation() == LOCATION_WORLD)
		{
			if(!pGI->GetItem()->GetData(INDEX_MOVEABLITY).Value)
				pAMenu->AddAction(ACTION_PICKUP,(void *)pGI,(void *)0,"PickUp");
		}

		if(pGI->GetItem()->GetData(INDEX_USE_OVERRIDE).Value)
		{
			if(pGI->GetLocation() == LOCATION_PERSON && !PreludeParty.IsMember((Creature *)pGI->GetOwner()))
			{
			
			}
			else
			{
				pAMenu->AddAction(ACTION_USEITEM,(void *)pGI,(void *)0, "Use");
			}
		}

	//spell options
		if(GetData(INDEX_SPELLBOOK).Value)
		{
			int MinSpellAp;
			MinSpellAp = this->GetMinSpellAp();
						
			int MinSpellRange;
			MinSpellRange = this->GetMinSpellRange();

			int MaxSpellRange;
			MaxSpellRange = this->GetMaxSpellRange();

			float Distance;
			Distance = 0.0f;

			if(MinSpellAp > AP && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				pAMenu->AddAction(ACTION_NONE, (void *)NULL, NULL, "Not Enough AP to Cast");
			}
			else
			{
				sn = 0;
				while(!CanTargetCast && sn < MAX_SPELLS)
				{
					if(KnownSpells[sn])
					{
						for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
						{
							if(PreludeSpells.Test(sn,n,(Object *)this,pGI))
							{
								CanTargetCast =  TRUE;
								break;
							}
						}
					}
					sn++;
				}
				
				if(CanTargetCast)
				{
					pAMenu->AddAction(ACTION_CAST, (void *)pGI, NULL, "Cast");
				}
				else
				{
					pAMenu->AddAction(ACTION_NONE, NULL, NULL, "No Spells Valid for Target");
				}
			}
		}

//		pAMenu->AddAction(ACTION_EQUIP,(void *)pGI,(void *)0,"Equip");
//		pAMenu->AddAction(ACTION_USE,(void *)pGI,(void *)0,"Use");

		break;
	case OBJECT_STATIC:
		//spell options
		if(GetData(INDEX_SPELLBOOK).Value)
		{
			int MinSpellAp;
			MinSpellAp = this->GetMinSpellAp();
						
			float fMinSpellRange;
			fMinSpellRange = (int)this->GetMinSpellRange();

			int fMaxSpellRange;
			fMaxSpellRange = (int)this->GetMaxSpellRange() + 0.5f;

			Creature *pCreature;
			pCreature = (Creature *)pActor;
			float Distance;
			Distance = GetDistance(this, pCreature);

			if(MinSpellAp > AP && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				pAMenu->AddAction(ACTION_NONE, (void *)NULL, NULL, "Not Enough AP to Cast");
			}
			else
			if(Distance < fMinSpellRange || Distance > fMaxSpellRange)
			{
				pAMenu->AddAction(ACTION_NONE, (void *)NULL, NULL, "No Spell In Range");
			}
			else
			{
				sn = 0;
				while(!CanTargetCast && sn < MAX_SPELLS)
				{
					if(KnownSpells[sn])
					{
						for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
						{
							if(PreludeSpells.Test(sn,n,(Object *)this,pActor))
							{
								CanTargetCast =  TRUE;
								break;
							}
						}
					}
					sn++;
				}
				
				if(CanTargetCast)
				{
					pAMenu->AddAction(ACTION_CAST, (void *)pActor, NULL, "Cast");
				}
				else
				{
					pAMenu->AddAction(ACTION_NONE, NULL, NULL, "No Spells Valid for Target");
				}
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}

//Evaluate Attack
//	Examine the likely hood of success of a type of attack and the amount of damage it will do
//	Should this key off intelligence?
int Creature::EvaluateAttack(Thing *pTarget, ATTACK_T AttackType, int &PercentSuccess, int &EstimateDamage)
{
	//make sure the target is in range
	//if not then there is no chance of success or damage

	return TRUE;
}

int Creature::GetDefaultAction(Object *pActor)
{
	//there is no default action if we're a PC
	//outside combat the default action is talk
	//in combat the default action is the last attack type taken by the action
	Creature *pCreature;
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		if(pActor == this)
		{
			return ACTION_DEFEND;
		}
		else
		if(pActor->GetObjectType() == OBJECT_CREATURE)
		{
			pCreature = (Creature *)pActor;
			if(pCreature->GetData(INDEX_BATTLESIDE).Value != GetData(INDEX_BATTLESIDE).Value)
			{
				if(pCreature->GetRange() >= GetDistance(this, pCreature))
				{	
					if(pCreature->GetData(INDEX_ATTACKPOINTS).Value <= pCreature->GetData(INDEX_ACTIONPOINTS).Value)
					{
						D3DVECTOR *vActor;
						D3DVECTOR *vPosition;
						vActor = pActor->GetPosition();
						vPosition = this->GetPosition();
						if(PreludeWorld->GetCombat()->CheckLineOfSight(vActor->x,vActor->y,vPosition->x,vPosition->y,pActor,this))
							return ACTION_ATTACK;
					}
				}
			}
		}
	}
	else
	{
		if(pActor == this)
		{
			return ACTION_NONE;
		}
		else
		if(pActor->GetObjectType() == OBJECT_CREATURE)
		{
			pCreature = (Creature *)pActor;
			if(pCreature->GetRegionIn() != this->GetRegionIn())
			{
				return ACTION_NONE;
			}

			if(PreludeParty.IsMember(pCreature)
				&& !PreludeParty.IsMember(this))
			{
				if(!GetData(INDEX_BATTLESIDE).Value)
					return ACTION_TALK;
				else
					return ACTION_NONE;
			}
		}
	}
	return ACTION_NONE;
}

Action *Creature::GetAction()
{
	return pCurAction;
}

int Creature::GetNumCreatures()
{
	return NumCreatures;
}

Creature *Creature::GetFirst()
{
	return pFirst;
}


int Creature::GetCombatRating()
{
	int AverageToHit;
	int AverageDamage;
	int BestWeaponSkill;
	int AttacksPerRound;
	int DamageRating;
	int AbsorbRating;
	int FlameRating;
	int RiverRating;

	AttacksPerRound = (float)(GetData(INDEX_ACTIONPOINTS).Value / (float)GetData(INDEX_ATTACKPOINTS).Value);
	
	BestWeaponSkill = GetData(INDEX_SWORD).Value;
	if(GetData(INDEX_AXE).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_AXE).Value;
	}
	if(GetData(INDEX_BLUNT).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_BLUNT).Value;
	}
	if(GetData(INDEX_UNARMED).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_UNARMED).Value;
	}
	if(GetData(INDEX_POLEARM).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_POLEARM).Value;
	}
	if(GetData(INDEX_THROWING).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_THROWING).Value;
	}
	if(GetData(INDEX_MISSILE).Value > BestWeaponSkill)
	{
		BestWeaponSkill = GetData(INDEX_MISSILE).Value;
	}
	AverageDamage = GetDamage(FALSE, TRUE);
	AverageToHit = (BestWeaponSkill + GetData(INDEX_DEXTERITY).Value*5)/2;
	DamageRating = (((AttacksPerRound * AverageDamage * BestWeaponSkill)/100)/3);
	if(GetRange() > 1.5f)
	{
		DamageRating *= 2;
	}

	FlameRating = GetData(INDEX_POWER_OF_FLAME).Value / 10;

	RiverRating = GetData(INDEX_GIFTS_OF_GODDESS).Value / 10;

	int Head;
	Thing *pHead;
	int Chest;
	Thing *pChest;
	int Legs;
	Thing *pLegs;

	int HeadAbsorb = 0;
	int ChestAbsorb = 0;
	int LegsAbsorb = 0;
	int AbsorbAverage = 0;

	Head = GetData(INDEX_HEAD).Value;
	if(Head && Item::FindItem(Head))
	{
		pHead = Item::FindItem(Head);

		HeadAbsorb = (pHead->GetData("ARMORMIN").Value + pHead->GetData("ARMORMAX").Value)/2;
	}

	Chest = GetData(INDEX_CHEST).Value;
	if(Chest && Item::FindItem(Chest))
	{
		pChest = Item::FindItem(Chest);

		ChestAbsorb = (pChest->GetData("ARMORMIN").Value + pChest->GetData("ARMORMAX").Value)/2;
	}

	Legs = GetData(INDEX_LEGS).Value;
	if(Legs && Item::FindItem(Legs))
	{
		pLegs = Item::FindItem(Legs);

		LegsAbsorb = (pLegs->GetData("ARMORMIN").Value + pLegs->GetData("ARMORMAX").Value)/2;
	}

	AbsorbAverage = ((HeadAbsorb * BLOW_LANDED_HEAD_CHANCE + 
						ChestAbsorb * (BLOW_LANDED_BODY_CHANCE - BLOW_LANDED_HEAD_CHANCE) +
						LegsAbsorb * (BLOW_LANDED_LEGS_CHANCE - BLOW_LANDED_BODY_CHANCE - BLOW_LANDED_HEAD_CHANCE))/100)/3;

	AbsorbRating = (GetData(INDEX_MAXHITPOINTS).Value * AbsorbAverage)/15;



	return (FlameRating + DamageRating + AbsorbRating + RiverRating);
}


//************  Display Functions  *************************************


//end: Display functions ***********************************************

//************ Mutators ************************************************
void Creature::AddToArea(Area *pArea)
{
	//create a portrait window for this creature
	//add it to the main window
	SetEquipObjects();
	InsertAction(ACTION_IDLE,NULL,NULL);
	fWalkFrames = Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->EndFrame - Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->StartFrame;
	
	GameItem *pGI;

	//if they are a person, then stat them initially from weapons and what-not
	if(!GetData(INDEX_TYPE).Value)
	{
		//base speed
		SetData(INDEX_ACTIONPOINTS,GetData(INDEX_SPEED).Value);
		SetData(INDEX_MAXACTIONPOINTS,GetData(INDEX_SPEED).Value);
	
		SetData(INDEX_HITPOINTS, GetData(INDEX_ENDURANCE).Value * 2 + GetData(INDEX_STRENGTH).Value);
		SetData(INDEX_MAXHITPOINTS, GetData(INDEX_ENDURANCE).Value * 2 + GetData(INDEX_STRENGTH).Value);

		SetData(INDEX_RESTPOINTS, GetData(INDEX_ENDURANCE).Value);
		SetData(INDEX_MAXRESTPOINTS, GetData(INDEX_ENDURANCE).Value);

		SetData(INDEX_WILLPOINTS, GetData(INDEX_WILLPOWER).Value);
		SetData(INDEX_MAXWILLPOINTS, GetData(INDEX_WILLPOWER).Value);

		//set the weapon skill
		pGI = this->GetEquipment("RIGHTHAND");
		if(pGI)
		{
			if(pGI->GetData(INDEX_TYPE).Value == 2)
			{
				//weapon
				SetData(INDEX_ATTACKPOINTS, pGI->GetData("SPEED").Value);
				//SetData(INDEX_MINDAMAGE, pGI->GetData("MINDAMAGE").Value);	
				//SetData(INDEX_MAXDAMAGE, pGI->GetData("MAXDAMAGE").Value);
				//set the weapon skill
				//SetData(INDEX_WEAPONSKILL, GetData(INDEX_SWORD + pGI->GetData("SUBTYPE").Value).Value);	
				SetData(INDEX_RANGE, 1.5f);//pGI->GetData("RANGE").fValue);
			}
			else
			{
				//weapon
				SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
				SetData(INDEX_MINDAMAGE, BASE_MIN_DAMAGE);	
				SetData(INDEX_MAXDAMAGE, BASE_MAX_DAMAGE);
				//set the weapon skill
				//SetData(INDEX_WEAPONSKILL, GetData(INDEX_UNARMED).Value);
				SetData(INDEX_RANGE, 1.5f);
			}
		}
		else
		{
			//weapon
			SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
			SetData(INDEX_MINDAMAGE, BASE_MIN_DAMAGE);	
			SetData(INDEX_MAXDAMAGE, BASE_MAX_DAMAGE);
			//set the weapon skill
			//SetData(INDEX_WEAPONSKILL, GetData(INDEX_UNARMED).Value);
			SetData(INDEX_RANGE, 1.5f);
		}
	}
	else
	{
		//base speed
		SetData(INDEX_ACTIONPOINTS,GetData(INDEX_MAXACTIONPOINTS).Value);
		SetData(INDEX_HITPOINTS, GetData(INDEX_MAXHITPOINTS).Value);
	}

	pRegionIn = pArea->GetRegion(this->GetPosition());

	pArea->AddToUpdate(this);

	this->SetAreaIn(PreludeWorld->GetAreaNum(pArea));

}

//************ Mutators ************************************************
void Creature::SetWalkFrames()
{
	fWalkFrames = Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->EndFrame - Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->StartFrame;
}
	
void Creature::AddToWorld()
{
	//create a portrait window for this creature
	//add it to the main window
	if(PreludeParty.IsMember(this))
	{

		ZSPortrait *pPortrait;
		pPortrait = ((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->GetPortrait(PreludeParty.GetMemberNum(this));
		pPortrait->SetTarget(this);
		pPortrait->Show();
		
		SetPortrait(pPortrait);

	}
	
	CreateTexture();
	SetEquipObjects();
	InsertAction(ACTION_IDLE,NULL,NULL);
	SetWalkFrames();
	

	GameItem *pGI;
	//if they are a person, then stat them initially from weapons and what-not
	if(!GetData(INDEX_TYPE).Value)
	{
		//base speed
		SetData(INDEX_ACTIONPOINTS,GetData(INDEX_SPEED).Value);
		SetData(INDEX_MAXACTIONPOINTS,GetData(INDEX_SPEED).Value);
	
		SetData(INDEX_HITPOINTS, GetData(INDEX_ENDURANCE).Value * 2 + GetData(INDEX_STRENGTH).Value);
		SetData(INDEX_MAXHITPOINTS, GetData(INDEX_ENDURANCE).Value * 2 + GetData(INDEX_STRENGTH).Value);

		SetData(INDEX_RESTPOINTS, GetData(INDEX_ENDURANCE).Value);
		SetData(INDEX_MAXRESTPOINTS, GetData(INDEX_ENDURANCE).Value);

		SetData(INDEX_WILLPOINTS, GetData(INDEX_WILLPOWER).Value);
		SetData(INDEX_MAXWILLPOINTS, GetData(INDEX_WILLPOWER).Value);

		//set the weapon skill
		pGI = this->GetEquipment("RIGHTHAND");
		if(pGI)
		{
			if(pGI->GetData(INDEX_TYPE).Value == 2)
			{
				//weapon
				SetData(INDEX_ATTACKPOINTS, pGI->GetData("SPEED").Value);
				//SetData(INDEX_MINDAMAGE, pGI->GetData("MINDAMAGE").Value);	
				//SetData(INDEX_MAXDAMAGE, pGI->GetData("MAXDAMAGE").Value);
				//set the weapon skill
				//SetData(INDEX_WEAPONSKILL, GetData(INDEX_SWORD + pGI->GetData("SUBTYPE").Value).Value);	
				SetData(INDEX_RANGE, 1.5f);//pGI->GetData("RANGE").fValue);
			}
			else
			{
				//weapon
				SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
				//SetData(INDEX_MINDAMAGE, BASE_MIN_DAMAGE);	
				//SetData(INDEX_MAXDAMAGE, BASE_MAX_DAMAGE);
				//set the weapon skill
				//SetData(INDEX_WEAPONSKILL, GetData(INDEX_UNARMED).Value);
				SetData(INDEX_RANGE, 1.5f);
			}
		}
		else
		{
			//weapon
			SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
			//SetData(INDEX_MINDAMAGE, BASE_MIN_DAMAGE);	
			//SetData(INDEX_MAXDAMAGE, BASE_MAX_DAMAGE);
			//set the weapon skill
			//SetData(INDEX_WEAPONSKILL, GetData(INDEX_UNARMED).Value);
			SetData(INDEX_RANGE, 1.5f);
		}
	}
	else
	{
		//base speed
		SetData(INDEX_ACTIONPOINTS,GetData(INDEX_MAXACTIONPOINTS).Value);
		SetData(INDEX_HITPOINTS, GetData(INDEX_MAXHITPOINTS).Value);
	}

	if(AreaIn >= 0)
	{
		pRegionIn = PreludeWorld->GetArea(AreaIn)->GetRegion(this->GetPosition());
		PreludeWorld->GetArea(AreaIn)->AddToUpdate(this);
	}
}	


int Creature::SetData(int fieldnum, int NewValue)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].Value = NewValue;
	//done
	if(pPortrait)
	{
		pPortrait->Dirty();
	}
	return TRUE;
}

int Creature::SetData(char *fieldname, int NewValue)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].Value = NewValue;
			if(pPortrait)
			{
				pPortrait->Dirty();
			}
			return TRUE;
		}
		fn += 32;
	}

	//display error
	
	//FATAL_ERROR(");

	return TRUE;
}

int Creature::SetData(int fieldnum, float NewfValue)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].fValue = NewfValue;
	//done
	if(pPortrait)
	{
		pPortrait->Dirty();
	}
	return TRUE;
}

int Creature::SetData(char *fieldname, float NewfValue)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].fValue = NewfValue;
			if(pPortrait)
			{
				pPortrait->Dirty();
			}
			return TRUE;
		}
		fn += 32;
	}

	
//	FATAL_ERROR("SetData: Failed to find field: %s",fieldname);

	return TRUE;
}

int Creature::SetData(int fieldnum, char *NewString)
{
	//set the value at the index provide to be equal to the value passed
	if(DataFields[fieldnum].String)
		delete[] DataFields[fieldnum].String;
	DataFields[fieldnum].String = NewString;
	//done
	if(pPortrait)
	{
		pPortrait->Dirty();
	}
	return TRUE;
}

int Creature::SetData(char *fieldname, char *NewString)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			if(DataFields[n].String)
				delete[] DataFields[n].String;
			DataFields[n].String = NewString;
			if(pPortrait)
			{
				pPortrait->Dirty();
			}
			return TRUE;
		}
		fn += 32;
	}

//	FATAL_ERROR("Failed to find field: %s",fieldname);
	return TRUE;
}

int Creature::SetData(int fieldnum, D3DVECTOR *NewpVector)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].pVector = NewpVector;
	//done
	if(pPortrait)
	{
		pPortrait->Dirty();
	}
	return TRUE;
}

int Creature::SetData(char *fieldname, D3DVECTOR *NewpVector)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].pVector = NewpVector;
			if(pPortrait)
			{
				pPortrait->Dirty();
			}
			return TRUE;
		}
		fn += 32;
	}
	//display error

//	FATAL_ERROR("Failed to find field: %s",fieldname);

	return TRUE;
}

BOOL Creature::AdvanceFrame()
{
	if(Update() != ACTION_RESULT_REMOVE_FROM_GAME)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//offscreen updates for time passing or locations far from the player

void Creature::UpdateOffScreen()
{
	if(this->AreaIn == -1)
	{
	
	}

	if(pTexture)
	{
		if(!Engine->GetTextureNum(pTexture))
		{
			RECT rUpdate;
			rUpdate.left = (PreludeWorld->GetScreenX() * CHUNK_TILE_WIDTH) - 16;
			rUpdate.right = rUpdate.left + 32;
			rUpdate.top = (PreludeWorld->GetScreenY() * CHUNK_TILE_HEIGHT) - 16;
			rUpdate.bottom = rUpdate.top + 32;

			if(this->GetPosition()->x < rUpdate.left ||
				this->GetPosition()->y < rUpdate.top ||
				this->GetPosition()->x > rUpdate.right ||
				this->GetPosition()->y > rUpdate.bottom)
			{
				PeopleTextures.FreeTexture(pTexture);
				pTexture = NULL;
			}
		}
	}

	Locator *pLocator;
	if(NumLocators)
	{
		int Hour = PreludeWorld->GetHour();
		for(int n = 0; n < NumLocators; n++)
		{
			if((Schedule[n].GetStart() < Schedule[n].GetEnd() &&
				 (Schedule[n].GetStart() <= Hour && Schedule[n].GetEnd() >= Hour))
				 ||
				(Schedule[n].GetStart() > Schedule[n].GetEnd() &&
				 (Schedule[n].GetStart() <= Hour || Schedule[n].GetEnd() >= Hour)))
			{
				RECT rLoc;
				Schedule[n].GetBounds(&rLoc);
				D3DVECTOR *pPosition;
				pPosition = this->GetPosition();
				if(pPosition->x >= (float)rLoc.left &&
					pPosition->y >= (float)rLoc.top &&
					pPosition->x <= (float)rLoc.right &&
					pPosition->y <= (float)rLoc.bottom
					&& this->AreaIn == Schedule[n].GetArea())
				{
					//do nothing if
				}
				else
				{
					pLocator = &Schedule[n];
					int NewX;
					int NewY;
					NewX = (rand() % (rLoc.right - rLoc.left)) + rLoc.left;
					NewY = (rand() % (rLoc.bottom - rLoc.top)) + rLoc.top;
					BOOL Small;
					Small = (rLoc.right - rLoc.left == 1) && (rLoc.bottom - rLoc.top == 1);
					for(int nswitch = 0; nswitch < 20; nswitch++)
					{
						if(Small || (!Large && PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY))
									|| 
									(Large && PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX+1,NewY)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY+1)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX+1,NewY+1)))
						{
							if(this->AreaIn != -1)
								PreludeWorld->GetArea(this->AreaIn)->RemoveFromUpdate(this);
							pPosition->x = (float)NewX + 0.5f;
							pPosition->y = (float)NewY + 0.5f;
							pPosition->z = PreludeWorld->GetArea(pLocator->GetArea())->GetTileHeight(pPosition->x,pPosition->y);
							this->SetRegionIn(PreludeWorld->GetArea(pLocator->GetArea())->GetRegion(pPosition));
							this->SetAreaIn(PreludeWorld->GetAreaNum(PreludeWorld->GetArea(pLocator->GetArea())));
							PreludeWorld->GetArea(pLocator->GetArea())->AddToUpdate(this);
							this->ClearActions();
							RECT rUpdate;
							rUpdate.left = (PreludeWorld->GetScreenX() * CHUNK_TILE_WIDTH) - 16;
							rUpdate.right = rUpdate.left + 32;
							rUpdate.top = (PreludeWorld->GetScreenY() * CHUNK_TILE_HEIGHT) - 16;
							rUpdate.bottom = rUpdate.top + 32;

							if(this->GetPosition()->x > rUpdate.left &&
								this->GetPosition()->y > rUpdate.top &&
								this->GetPosition()->x < rUpdate.right &&
								this->GetPosition()->y < rUpdate.bottom)
							{
								this->CreateTexture();
							}
							break;
						}
						else
						{
							NewX = (rand() % (rLoc.right - rLoc.left)) + rLoc.left;
							NewY = (rand() % (rLoc.bottom - rLoc.top)) + rLoc.top;
						}	
					}
					
				}
				break;
			}
		}
	}

}


//Update
//	Update this creature based on its current action
ACTION_RESULT_T Creature::Update(void)
{
	if(!pTexture)
	{
		CreateTexture();
		SetEquipObjects();
	}	

	Action *pAction;
	int UpdateCount = 0;
	do
	{
		UpdateCount++;
		pAction = GetAction();
		//look at the current action and call the appropriate action function
		switch(pAction->GetType())
		{
			case ACTION_IDLE:
				LastResult = Idle();
				break;
			case ACTION_FORMATION:
				LastResult = Formation();
				break;
			//None
			case ACTION_NONE:
				LastResult = None();
				break;
			case ACTION_GOTO:
				LastResult = Goto();
				break;			
			case ACTION_MOVETO:
				LastResult = MoveTo();
				break;
			case ACTION_FOLLOWPATH:
				LastResult = FollowPath();
				break;
			case ACTION_MOVEIN:
				LastResult = MoveIn();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_MOVEINNOROTATE:
				LastResult = MoveNoRotate();
				break;
			case ACTION_ROTATE:
				LastResult = Rotate();
				break;
			case ACTION_ATTACK:
				LastResult = Attack();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_SECONDARY_ATTACK:
				LastResult = SecondaryAttack();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_SPECIAL_ATTACK:
				LastResult = SpecialAttack();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_KILL:
				LastResult = Kill();
				break;
			case ACTION_ANIMATE:
				LastResult = Animate();
				break;
			case ACTION_TALK:
				LastResult = TalkTo();
				break;
			case ACTION_SAY:
				LastResult = Say();
				break;
			case ACTION_TAKE:
				LastResult = Take();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_DROP:
				LastResult = Drop();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_GIVE:
				LastResult = Give();
				break;
			case ACTION_PICKUP:
				LastResult = PickUp();
				break;
			case ACTION_DIE:
				LastResult = Die();
				break;
			case ACTION_UNCONSCIOUS:
				LastResult = Unconscious();
				break;
			case ACTION_REACTTO:
				LastResult = ReactTo();
				break;
			case ACTION_DEFEND:
				LastResult = Defend();
				if(PreludeWorld->InCombat())
				{
					DoneOneAction();
				}
				break;
			case ACTION_SCRIPT:
				LastResult = Script();
				break;
			case ACTION_THINK:
				LastResult = Think();
				break;
			case ACTION_USER:
				LastResult = User();
				break;
			case ACTION_APPROACH:
				LastResult = Approach();
				break;
			case ACTION_WAITUNTIL:
				LastResult = WaitUntil();
				break;
			case ACTION_WAITFORSIGHT:
				LastResult = WaitForSight();
				break;
			case ACTION_WAITFORHEAR:
				LastResult = WaitForHear();
				break;
			case ACTION_WAITFORCLEAR:
				LastResult = WaitForClear();
				break;
			case ACTION_USEITEM:
				LastResult = UseItem();
				break;
			case ACTION_LOOKAT:
				LastResult = LookAt();
				break;
			case ACTION_WAITFOROTHERCREATURE:
				LastResult = WaitForOtherCreature();
				break;
			case ACTION_MISSILE:
				LastResult = WaitMissile();
				break;
			case ACTION_CAST:
				LastResult = Cast();
				break;
			case ACTION_WAITACTION:
				LastResult = WaitAction();
				break;
			case ACTION_LEAP:
				LastResult = Leap();
				break;
			case ACTION_OPEN:
				LastResult = Open();
				break;
			case ACTION_CLOSE:
				LastResult = Close();
				break;
			case ACTION_GAME_COMMAND:
				LastResult = GameCommand();
				break;
			case ACTION_PICKPOCKET:
				LastResult = PickPocket();
				break;
			case ACTION_SIT:
				LastResult = Sit();
				break;
			case ACTION_LAY:
				LastResult = Lay();
				break;
			default:
				//FATAL_ERROR("%s has an invalid action code",GetData("NAME").String);
				//display error
				SafeExit("Invalid Action Code");(1);
				break;
		}
		//if the action is finished remove it from the stack and start the next action
		if(LastResult == ACTION_RESULT_FINISHED || LastResult == ACTION_RESULT_REMOVE_FROM_GAME)
		{
			RemoveCurrentAction();
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT && Active)
			{
				return LastResult;
			}
	//		return LastResult;
		}
		//if it's unfished just return  normally
		if(LastResult == ACTION_RESULT_UNFINISHED)
		{
	//		return LastResult;
		}
		//if it inserted an action, start that action
		if(LastResult == ACTION_RESULT_ADDED_ACTION)
		{
	//		return ACTION_RESULT_ADDED_ACTION;
		}

		if(LastResult == ACTION_RESULT_OUT_OF_AP)
		{
			if(GetData(INDEX_ACTIONPOINTS).Value)
			{
				InsertAction(ACTION_DEFEND,NULL,NULL);
			}
			else
			{
				if(GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS &&
					GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_DEAD)
				{
					InsertAction(ACTION_IDLE,NULL,NULL);
				}
			}
		}

		if(LastResult == ACTION_RESULT_NOT_POSSIBLE)
		{
			if(pCurAction->IsSub())
			{
				if(pCurAction->GetType() == ACTION_APPROACH)
				{
					RemoveCurrentAction();
					InsertAction(ACTION_DEFEND, NULL, NULL);
					LastResult = ACTION_RESULT_ADDED_ACTION;
				}
				else
				{
					RemoveMinorAction();
					if(pCurAction->GetType() == ACTION_MOVETO)
					{
						RemoveCurrentAction();
						if(PreludeParty.IsMember(this))
						{
						
						}
					}
				}
			}
			else
			{
				RemoveCurrentAction();
			}
		}
	}
	while(LastResult != ACTION_RESULT_ADVANCE_FRAME &&
		LastResult != ACTION_RESULT_REMOVE_FROM_GAME &&
		UpdateCount < 10);

	return LastResult;
}
		
//actions that may be performed on creatures

//Use
//	Using a creature is the same as talking to it
int Creature::Use(Thing *pUser)
{

	return TRUE;
}

void GetDescription(int i, char *Text)
{
	switch(i)
	{
	case 0:  case 1: case 2: case 3: case 4: case 5:
		sprintf(Text, "Pathetic");
		break;
	case 6: case 7:
		sprintf(Text, "Awful");
		break;
	case 8: case 9:
		sprintf(Text, "Bad");
		break;
	case 10: case 11:
		sprintf(Text, "Below average");
		break;
	case 12:	case 13: case 14:
		sprintf(Text, "Average");
		break;
	case 15: case 16:
		sprintf(Text, "Above average");
		break;
	case 17: case 18:
		sprintf(Text, "Good");
		break;
	case 19: case 20:
		sprintf(Text, "Very Good");
		break;
	case 21: case 22:
		sprintf(Text, "Exceptional");
		break;
	case 23: case 24: case 25:
		sprintf(Text, "Near Perfection");
		break;
	default:
		sprintf(Text, "Perfect");
		break;
	}
}

//Lookat
//	Display information about a creature
int Creature::LookAt(Object *pLooker)
{
	//check for a lookat over-ride
	//display basic information about this creature
	//name
	char LookAtString[512];
	
	char *HealthString;
	
	HealthString = GetRangeDescriptor(this->GetData(INDEX_HITPOINTS).Value, 0, this->GetData(INDEX_MAXHITPOINTS).Value,RANGE_HEALTH);
	
	int Speed;
	Speed = this->GetData(INDEX_SPEED).Value;
	char SpeedString[16];

	GetDescription(Speed,SpeedString);	

	int Movement;
	Movement = Speed / this->GetData(INDEX_MOVEPOINTS).Value;

	int MaxHP;
	MaxHP = this->GetData(INDEX_MAXHITPOINTS).Value;

	int ArmorAbsorb = 0;
	char ArmorString[32];
	GameItem *pGI;
	pGI = this->GetEquipment("CHEST");
	if(pGI)
	{
		ArmorAbsorb = (pGI->GetData("ARMORMAX").Value + pGI->GetData("ARMORMIN").Value) / 2;
	}
	switch(ArmorAbsorb)
	{
	case 0:
	case 1:
		sprintf(ArmorString,"%s","Minimal");
		break;
	case 2:
	case 3:
		sprintf(ArmorString,"%s","Tough");
		break;
	case 4:
	case 5:
		sprintf(ArmorString,"%s","Armored");
		break;
	case 6:
	case 7:
		sprintf(ArmorString,"%s","Well Armored");
		break;
	case 8:
	case 9:
	case 10:
		sprintf(ArmorString,"%s","Superb");
		break;
	default:
		sprintf(ArmorString,"%s","Almost Impenetrable");
		break;
	}

	char AttackString[32];

	int AttackValue;

	AttackValue = this->GetData(INDEX_SPEED).Value;
	AttackValue += this->GetData(INDEX_DEXTERITY).Value ;
	AttackValue += (this->GetWeaponSkill() * 2) / 3;
	AttackValue /= 3;

	char DamageString[32];
	int DamageValue;
	DamageValue = GetDamage(FALSE, TRUE);

	//modify by strength
	float DamagePercent;
	DamagePercent = ((float)GetData(INDEX_STRENGTH).Value) / 13.0f;
	float FinDamage;
	FinDamage = DamagePercent * (float)DamageValue;
	DamageValue = (int)(FinDamage + 0.5f); 
	
	switch(DamageValue)
	{
	case 0:	case 1:
		sprintf(DamageString,"%s","Minimal");
		break;
	case 2:	case 3:
		sprintf(DamageString,"%s","Low");
		break;
	case 4:	case 5:
		sprintf(DamageString,"%s","Below Average");
		break;
	case 6:	case 7:
		sprintf(DamageString,"%s","Average");
		break;
	case 8: case 9:
		sprintf(DamageString,"%s","Above Average");
		break;
	case 10: case 11:
		sprintf(DamageString,"%s","High");
		break;
	case 12: case 13:
		sprintf(DamageString,"%s","Very High");
		break;
	case 14: case 15: case 16:
		sprintf(DamageString,"%s","Bone Crushing");
		break;
	default:
		sprintf(DamageString,"%s","Devastating");
		break;
	}
	
	GetDescription(AttackValue,AttackString);

	sprintf(LookAtString, "%s\n  HP:  %i/%i\n  Speed: %s\n  Armor: %s\n  Attack Ability: %s\n  Damage Potential: %s",
		this->GetData(INDEX_NAME).String,
		this->GetData(INDEX_HITPOINTS).Value,
		this->GetData(INDEX_MAXHITPOINTS).Value,
		SpeedString,
		ArmorString,
		AttackString,
		DamageString);
	
	ZSWindow *pWin;

	pWin = new ZSSayChar(-1, -1, -1, 0, 0, LookAtString, NULL);

	ZSWindow::GetMain()->AddTopChild(pWin);

	pWin->Show();

	pWin->SetFocus(pWin);

	pWin->GoModal();
	
	pWin->ReleaseFocus();

	ZSWindow::GetMain()->RemoveChild(pWin);
	return TRUE;
}

//TakeDamage
//	have the creature suffer some damage
int Creature::TakeDamage(Thing *pDamageSource, int DamageAmount, DAMAGE_T DamageType)
{
	int FinalDamage;
	D3DVECTOR Vector;
	Blood *pBlood;
	Explosion *pExplode;
							
	FinalDamage = DamageAmount;
	//check for a take damage over-ride
	if(GetData(INDEX_DAMAGE_OVERRIDE).Value)
	{
		return TRUE;
	}

	//subtract any resistances the creature has from the damage amount
	switch(DamageType)
	{
		case DAMAGE_NORMAL:
			FinalDamage = (FinalDamage * GetData("RESISTNORMAL").Value)/100;
			pBlood = new Blood(0.125f,this);
			PreludeWorld->AddMainObject((Object *)pBlood);
			break;
		case DAMAGE_FIRE:
			FinalDamage = (FinalDamage * GetData("RESISTFIRE").Value)/100;
			Vector = *GetData(INDEX_POSITION).pVector;
			Vector.z += 1;
			pExplode = new Explosion(Vector,0.6f,0,0,NULL);
			PreludeWorld->AddMainObject((Object *)pExplode);
			
			break;
		case DAMAGE_WATER:
			FinalDamage = (FinalDamage * GetData("RESISTWATER").Value)/100;
			break;
		case DAMAGE_AIR:
			FinalDamage = (FinalDamage * GetData("RESISTAIR").Value)/100;
			break;
		case DAMAGE_EARTH:
			FinalDamage = (FinalDamage * GetData("RESISTEARTH").Value)/100;
			break;
		case DAMAGE_DARK:
			FinalDamage = (FinalDamage * GetData("RESISTDARK").Value)/100;
			break;
		case DAMAGE_MENTAL:
			FinalDamage = (FinalDamage * GetData("RESISTMENTAL").Value)/100;
			break;
		case DAMAGE_LIGHT:
			FinalDamage = (FinalDamage * GetData("RESISTLIGHT").Value)/100;
			break;
		case DAMAGE_POISON:
			FinalDamage = (FinalDamage * GetData("RESISTPOISON").Value)/100;
			break;
		default:
			//display error
			break;
	}
	//damage always deals twice damage to rest points

	SetData(INDEX_HITPOINTS,GetData(INDEX_HITPOINTS).Value - (FinalDamage));

	char TempString[64];

	sprintf(TempString,"%s takes %i damage",GetData(INDEX_NAME).String,FinalDamage);

	Describe(TempString);
	
	//insert a die action if the damage is enough to kill the creature
	if(GetData(INDEX_HITPOINTS).Value <= 0)
	{
		if(!GetData(INDEX_TYPE).Value)
		{
			if(GetData(INDEX_SEX).Value)
			{
				if(rand() % 2)
					Engine->Sound()->PlayEffect(24);
				else
					Engine->Sound()->PlayEffect(25);
			}
			else
			{
				if(rand() % 2)
					Engine->Sound()->PlayEffect(20);
				else
					Engine->Sound()->PlayEffect(21);
			}
		}
		InsertAction(ACTION_UNCONSCIOUS,NULL,NULL);
		return TRUE;
	}

	//done
	int ChanceToExclaim;
	ChanceToExclaim = rand() % 100;
	if(FinalDamage && (ChanceToExclaim < 30 || FinalDamage > 10))
	{
		if(!GetData(INDEX_TYPE).Value)
		{
			if(GetData(INDEX_SEX).Value)
			{
				if(rand() % 2)
					Engine->Sound()->PlayEffect(24);
				else
					Engine->Sound()->PlayEffect(25);
			}
			else
			{
				if(rand() % 2)
					Engine->Sound()->PlayEffect(20);
				else
					Engine->Sound()->PlayEffect(21);
			}
		}
	}
	InsertAction(ACTION_ANIMATE,(void *)NULL,(void *)GET_HIT, TRUE);
	return TRUE;
}


int Creature::TakeDamage(int HitAmount, int RestAmount)
{
	//damage always deals twice damage to rest points

	SetData(INDEX_HITPOINTS, GetData(INDEX_HITPOINTS).Value - HitAmount);
	
   //insert a die action if the damage is enough to kill the creature
	if(GetData(INDEX_HITPOINTS).Value <= 0)
	{
		InsertAction(ACTION_UNCONSCIOUS,NULL,NULL);
		return TRUE;
	}

	//done
	return TRUE;
}


//InsertAction
//	Add an action to the bottom of the creature's action stack
int Creature::AddAction(ACTION_T ActionType, void *Target, void *Data, BOOL Sub)
{
	//place an action of the appropriate type on the bottom of the stack, but before any user or think actions
	Action *pAction = GetAction();

	Action *pLastAction;

	Action *pNewAction = Action::GetNewAction(ActionType, Target, Data, Sub);
	if(!pAction)
	{
		pCurAction = pNewAction;
		return TRUE;
	}
	if(pAction->GetType() == ACTION_THINK || pAction->GetType() == ACTION_USER)
	{	
		InsertAction(ActionType, Target, Data, Sub);
		return TRUE;
	}

	while(pAction && pAction->GetType() != ACTION_THINK && pAction->GetType() != ACTION_USER)
	{
		pLastAction = pAction;
		pAction = pAction->GetNext();
	}
	assert(pAction);
	assert(pLastAction);

	pNewAction->SetNext(pAction);
	pLastAction->SetNext(pNewAction);

	return TRUE;
}

//InsertAction
//	Insert an action on the top of the creatures action stack, effectively interrupting the current action
int Creature::InsertAction(ACTION_T ActionType, void *Target, void *Data, BOOL Sub)
{
	//check the actions to see if we have an identical one there.
	Action *TestAction;
	TestAction = pCurAction;
	
	while(TestAction)
	{
		if(TestAction->GetType() == ActionType &&
			TestAction->GetTarget() == Target &&
			TestAction->GetData() == Data)
		{
			return TRUE;
		}

		TestAction = TestAction->GetNext();
	}

	//place an action of the appropriate type on the top of the stack making it the current action
	if(pCurAction)
	{
		Action *pNewAction;
		if(pCurAction->GetType() == ACTION_ANIMATE)
		{
			if(ActionType == ACTION_ANIMATE)
			{
				RemoveCurrentAction();
				pNewAction = Action::GetNewAction(ActionType,Target,Data, Sub);
				pNewAction->SetNext(pCurAction);
				pCurAction = pNewAction;
			}
			else
			{
				pNewAction = Action::GetNewAction(ActionType,Target,Data, Sub);
				pNewAction->SetNext(pCurAction->GetNext());
				pCurAction->SetNext(pNewAction);
			}
		}
		else
		{
			pNewAction = Action::GetNewAction(ActionType,Target,Data, Sub);
			pNewAction->SetNext(pCurAction);
			pCurAction = pNewAction;
		}
	}
	else
	{
		pCurAction = Action::GetNewAction(ActionType,Target,Data, Sub);
	}
//	IncrementActionCount();
	return TRUE;
}

int Creature::RemoveAction(ACTION_T RemoveType)
{
	//remove all actions except for the think or user actions without finishing them
	Action *pAction, *pLastAction;
	
	pAction = pCurAction;
	
	if(pAction->GetType() == RemoveType)
	{
		pCurAction = pAction->GetNext();
		pAction->Release();
	//	DecrementActionCount();
		return TRUE;
	}
	else
	{
		while(pAction && pAction->GetType() != RemoveType)
		{	
			pLastAction = pAction;
			pAction = pAction->GetNext();
		}

		if(pAction)
		{
			pLastAction->SetNext(pAction->GetNext());
			pAction->Release();
			//DecrementActionCount();
			return TRUE;
		}
	}
	return FALSE;

}

//FinishAction
// Force a creature to immediately finish its current action
int Creature::FinishAction()
{
	//repeatedly advance the creature's current action until a result of action finished or action added 
	//is reached
	//should we do the following?
		//if the current action adds another action recursively call finish action
	return TRUE;
}
//KillActions
//  completely clear all actions
int Creature::KillActions()
{
	Action *pAction, *pStart;
	
	pStart = NULL;
	//leave the most recent animation
	pAction = pCurAction;

	while(pCurAction)
	{
		pAction = pCurAction;
		pCurAction = pCurAction->GetNext();
		pAction->Release();
	}

	xoff = 0.0f;
	yoff = 0.0f;
	zoff = 0.0f;
	angleoff = 0.0f;
	FrameAdd = 0;

	return TRUE;
}


//ClearActions
//	Clear out the list of actions the creature is performing
int Creature::ClearActions()
{
	//remove all actions except for the think or user actions without finishing them
	Action *pAction, *pStart;
	
	pStart = NULL;
	//leave the most recent animation
	pAction = pCurAction;
	if(pAction && pAction->GetType() == ACTION_ANIMATE)
	{
		pStart = pAction;
		pCurAction = pAction = pAction->GetNext();
	}

	while(pCurAction && pCurAction->GetType() != ACTION_DIE && pCurAction->GetType() != ACTION_UNCONSCIOUS)
	{
		pAction = pCurAction;
		pCurAction = pCurAction->GetNext();
		pAction->Release();
	}

	if(pCurAction)
	{

	}
	else
	{
		if(PreludeParty.IsMember(this))
		{
			this->InsertAction(ACTION_USER,NULL,NULL,FALSE);
		}
		else
		{
			this->InsertAction(ACTION_THINK,NULL,NULL,FALSE);
		}
	}
	if(pStart)
	{
		pStart->SetNext(pCurAction);
		pCurAction = pStart;
	}

	xoff = 0.0f;
	yoff = 0.0f;
	zoff = 0.0f;
	angleoff = 0.0f;
	FrameAdd = 0;

	return TRUE;
}

int Creature::RemoveCurrentAction()
{
	//remove all actions except for the think or user actions without finishing them
	
	Action *pAction;
	
	
	if(pCurAction->GetType() != ACTION_USER && pCurAction->GetType() != ACTION_THINK)
	{
		pAction = pCurAction;
		pCurAction = pCurAction->GetNext();
		pAction->Release();
	}
	return TRUE;
}

Action *Creature::GetMajorAction()
{
	Action *pAction;
	
	pAction = pCurAction;
	while(pAction->IsSub())
	{
		pAction = pAction->GetNext();
	}

	return pAction;
}


int Creature::RemoveMajorAction()
{
	Action *pAction;
	
	pAction = pCurAction;
	while(pAction->IsSub())
	{
		pCurAction = pCurAction->GetNext();
		pAction->Release();
		pAction = pCurAction;
	}
	pCurAction = pCurAction->GetNext();
	pAction->Release();
	
	return TRUE;
}

int Creature::RemoveMinorAction()
{
	Action *pAction, *pStart;
	
	pStart = NULL;
	//leave the most recent animation
	pAction = pCurAction;
	if(pAction->GetType() == ACTION_ANIMATE)
	{
		pStart = pAction;
		pCurAction = pAction = pAction->GetNext();
	}
	while(pAction->IsSub())
	{
		pCurAction = pCurAction->GetNext();
		pAction->Release();
		pAction = pCurAction;
	}

	if(pStart)
	{
		pStart->SetNext(pCurAction);
		pCurAction = pStart;
	}
	return TRUE;

}

//StartRound
//	Start a new combat round
int Creature::StartRound()
{
	//reset AP to maximum
	SetData(INDEX_ACTIONPOINTS,GetData(INDEX_ACTIONPOINTS).Value + GetData(INDEX_MAXACTIONPOINTS).Value);
	if(PreludeParty.IsMember(this))
	{
		int CurAct = GetData(INDEX_ACTIONPOINTS).Value;
		GameItem *pGI;
		pGI = GetEquipment("CHEST");
		if(pGI)
			SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value 
						- pGI->GetData("SPEED").Value);
		pGI = GetEquipment("LEGS");
		if(pGI)
			SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value 
						- pGI->GetData("SPEED").Value);
		int ArmorFactor;
		ArmorFactor = (GetData(INDEX_ARMOR).Value  - 10) / 10;
		if (ArmorFactor > 5) ArmorFactor = 5;
		if (ArmorFactor < 0) ArmorFactor = 0;
		if(CurAct != GetData(INDEX_ACTIONPOINTS).Value)
			SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value + 
				ArmorFactor); 
	}
	
	
	
	if(GetData(INDEX_ACTIONPOINTS).Value > GetData(INDEX_MAXACTIONPOINTS).Value)
	{
		SetData(INDEX_ACTIONPOINTS, GetData(INDEX_MAXACTIONPOINTS).Value);
	}
	
	
	//clear any defense bonuses
	if(GetData(INDEX_BLOCK).Value)
	{
		SetData(INDEX_ARMOR,GetData(INDEX_ARMOR).Value - GetData(INDEX_BLOCK).Value);
		SetData(INDEX_BLOCK,0);
	}
	if(GetData(INDEX_PARRY).Value)
	{
		SetData(INDEX_WEAPONSKILL,GetData(INDEX_WEAPONSKILL).Value - GetData(INDEX_PARRY).Value);
		SetData(INDEX_PARRY,0);
	}
	if(GetData(INDEX_DODGE).Value)
	{
		SetData(INDEX_SPEED,GetData(INDEX_SPEED).Value - GetData(INDEX_DODGE).Value);
		SetData(INDEX_DODGE,0);
	}

	//make sure can attack at least once per round
	GameItem *pGI;
	pGI = this->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		SetData(INDEX_ATTACKPOINTS, pGI->GetData("SPEED").Value);
	}
	else
	{
		SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
	}

	if(GetData(INDEX_ATTACKPOINTS).Value > GetData(INDEX_MAXACTIONPOINTS).Value)
	{
		SetData(INDEX_ATTACKPOINTS, GetData(INDEX_MAXACTIONPOINTS).Value);
	}

	//check for a start round over-ride and call it if it exists
	return TRUE;
}

//EndRound
// end a combat round
int Creature::EndRound()
{
	//check for an end round override and call it if it exists
	return TRUE;
}

//private action updaters

//None
//	do nothing
ACTION_RESULT_T Creature::None() 
{
	return ACTION_RESULT_UNFINISHED;
} 

//Goto
//	"teleport" to a specific location
ACTION_RESULT_T Creature::Goto() 
{
	//set position as pointed to by the data element of the current action
	//add this creature to any appropriate lists
	//remove them from the lists at their previous location
	return ACTION_RESULT_FINISHED;
}

//MoveTo
// go to a specific place
ACTION_RESULT_T Creature::MoveTo()
{
	//if our current location is not onscreen/nearby or the destination is not onscreen/nearby
		//insert a Goto action and ignore intervening space
		//return ACTION_RESULT_ADDED_ACTION;
	//check to see if we're at our destination
	D3DVECTOR *pPosition = GetData(INDEX_POSITION).pVector;
	int StartX;
	StartX = (int)pPosition->x;
	int StartY;
	StartY = (int)pPosition->y;

	int EndX;
	int EndY;
	EndX = (int)pCurAction->GetTarget();
	EndY = (int)pCurAction->GetData();
	
	if(StartX == EndX && StartY == EndY)
	{
		return ACTION_RESULT_FINISHED;
	}

	if(abs(StartX - EndX) > 256 || abs(StartY - EndY) > 256)
	{
		Area *pArea;
		pArea = PreludeWorld->GetArea(this->GetAreaIn());
		if(pArea)
		{
			pArea->RemoveFromUpdate(this);
			SetPosition((float)EndX + 0.5f, (float)EndY + 0.5f, pArea->GetTileHeight(EndX,EndY));
			this->SetRegionIn(pArea->GetRegion(this->GetPosition()));
			pArea->AddToUpdate(this);
		}
		
		return ACTION_RESULT_FINISHED;
	}

	Path *pPath;
	pPath = new Path;

	if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
	{
		if(!Large)
			pPath->FindPath(StartX,StartY,EndX,EndY, 0.0f, this);
		else
			pPath->FindLargePath(StartX,StartY,EndX,EndY, 0.0f, this);
	}
	else
	{
		if(!Large)
			pPath->FindCombatPath(StartX,StartY,EndX,EndY, 0.0f, this);
		else
			pPath->FindLargeCombatPath(StartX,StartY,EndX,EndY, 0.0f, this);
	}

	//find a path the specified location
	
	if(pPath->GetLength() != 666)
	{
	//insert a FollowPath action to get there if it is possible
		//pPath->Show();
		if(pPath->GetLength() == 1)
		{
			InsertAction(ACTION_MOVEIN,NULL,(void *)pPath->Traverse(),TRUE);
			delete pPath;
		}
		else
		{
			InsertAction(ACTION_FOLLOWPATH,(void *)pPath,(void *)NULL,TRUE);
		}
		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{

/*/if it's not possible check the reason
	//if it's because a door is in the way, add a use action with the door as the target
		return ACTION_RESULT_ADDED_ACTION;
	//if it's because a creatuer is in the way get them to move??????
		return ACTION_RESULT_ADDED_ACTION;
	//otherwise
*/
	
		return ACTION_RESULT_NOT_POSSIBLE;
	}
}

//None
//	do nothing
ACTION_RESULT_T Creature::Formation() 
{
	//calculate our current position
	int xAt;
	int yAt;
	int xShould;
	int yShould;
	int xLead;
	int yLead;
	D3DVECTOR *pVector;
	D3DVECTOR LeaderAt;
	xAt = (int)GetPosition()->x;
	yAt = (int)GetPosition()->y;

	if(PreludeParty.IsLeader(this))
	{
		return ACTION_RESULT_FINISHED;
	}
	
	pVector = PreludeParty.GetLeader()->GetPosition();

	xLead = pVector->x;
	yLead = pVector->y;

	int LeaderFacing;

	LeaderFacing = FindFacing(PreludeParty.GetLeader()->GetData(INDEX_ROTATION).fValue);

	int MyPosition = 0;

	int n;
	for(n = 0; n < PreludeParty.GetNumMembers(); n ++)
	{
		if(PreludeParty.GetMember(n) == this)
		{
			MyPosition = n;
			break;
		}
	}

	int Distance = MyPosition;
	switch(LeaderFacing)
	{
		case NORTH:
			xShould = xLead;
			yShould = yLead + Distance;
			break;
		case SOUTH:
			xShould = xLead;
			yShould = yLead - Distance;
			break;
		case EAST:
			xShould = xLead - Distance;
			yShould = yLead;
			break;
		case WEST:
			xShould = xLead + Distance;
			yShould = yLead;
			break;
		case NORTHEAST:
			xShould = xLead - Distance;
			yShould = yLead + Distance;
			break;
		case NORTHWEST:
			xShould = xLead + Distance;
			yShould = yLead + Distance;
			break;
		case SOUTHEAST:
			xShould = xLead - Distance;
			yShould = yLead - Distance;
			break;
		case SOUTHWEST:
			xShould = xLead + Distance;
			yShould = yLead - Distance;
			break;
		default:
			break;
	}
	if(xAt != xShould || yAt != yShould)
	{
		InsertAction(ACTION_MOVETO,(void *)xShould,(void *)yShould,TRUE);
		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{
		return ACTION_RESULT_FINISHED;
	}
} 


//FollowPath
//	follow a path
ACTION_RESULT_T Creature::FollowPath()
{
	if(!pCurAction->IsFinished())
	{
		//check to see if we're at our destination
		Path *pPath;
		pPath = (Path *)pCurAction->GetTarget();
		
		D3DVECTOR *pPosition = GetData(INDEX_POSITION).pVector;
		int AtX;
		AtX = (int)pPosition->x;
		int AtY;
		AtY = (int)pPosition->y;

		int EndX;
		int EndY;

		int PathX = 0;
		int PathY = 0;

		pPath->GetEnd(&EndX, &EndY);

		int CurNode = pPath->GetCurNode();

		pPath->GetNodeXY(CurNode, &PathX, &PathY);

		if(!pCurAction->IsStarted())
		{
			pCurAction->Start();
		}



		if(AtX == EndX && AtY == EndY)
		{
			pCurAction->Finish();
			//return ACTION_RESULT_FINISHED;
			//check to see if our next action
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)STOP_WALK, TRUE);
			}
			else
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)NONCOMBAT_STOP_WALK, TRUE);
			}
			return ACTION_RESULT_ADDED_ACTION;
		}

		if(CurNode == pPath->GetLength())
		{
			PathX += 1;
		}
		
		
		//get the next node from the path pointed to in the data element of the current action
		//insert a move in action based on the direction of the node

		
		
		if(AtX != PathX || AtY != PathY)
		{
			//	DEBUG_INFO("not on path x/y?\n");
		}

		
		if(CurNode >= pPath->GetLength())
		{
			//DEBUG_INFO("Attempt to walk beyond path.  Forcing stop.\n");
			pCurAction->Finish();
			//return ACTION_RESULT_FINISHED;
			//check to see if our next action
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)STOP_WALK, TRUE);
			}
			else
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)NONCOMBAT_STOP_WALK, TRUE);
			}
			return ACTION_RESULT_ADDED_ACTION;
		}

		int Dest = pPath->Traverse();
		int x;
		int y;

		x = AtX;
		y = AtY;

		int CheckX = 0;
		int CheckY = 0;

		//move in the correct direction
		switch(Dest)
		{
			case NORTH:
				CheckX = x;
				CheckY = y - 1;
				break;
			case SOUTH:
				CheckX = x;
				CheckY = y + 1;
				break;
			case EAST:
				CheckX = x+1;
				CheckY = y;
				break;
			case WEST:
				CheckX = x-1;
				CheckY = y;
				break;
			case NORTHEAST:
				CheckX = x+1;
				CheckY = y - 1;
				break;
			case SOUTHEAST:
				CheckX = x+1;
				CheckY = y + 1;
				break;
			case NORTHWEST:
				CheckX = x-1;
				CheckY = y - 1;
				break;
			case SOUTHWEST:
				CheckX = x-1;
				CheckY = y+1;
				break;
			default:
				DEBUG_INFO("path w/o direction???\n");
				return ACTION_RESULT_FINISHED;
				break;
		}//switch

		if(!Large)
		{
			if(!Valley->IsClear(CheckX,CheckY, this))
			{
	 			Object *pOb;
				pOb = Valley->FindObject(CheckX,CheckY, OBJECT_PORTAL);
	
				if(pOb)
				{
					Portal *pPortal;
					pPortal = (Portal *)pOb;
					if(!pPortal->GetLock())
					{
						if(pPortal->GetState() != PORTAL_OPENNING && pPortal->GetState() != PORTAL_OPEN)
						{
							//check for an event
							if(pPortal->GetEvent() && PreludeParty.IsMember(this))
							{
								PreludeEvents.RunEvent(pPortal->GetEvent());
								ScriptArg *pSA;
								pSA = Pop();
								int Result;
								Result = (int)pSA->GetValue();
								switch(Result)
								{
									case 0:
										pPortal->SetState(PORTAL_CLOSED);
										break;
									case 1:
										pPortal->SetState(PORTAL_OPENNING);
										break;
									case 2:
										pPortal->SetState(PORTAL_CLOSED);
										pPortal->SetEvent(0);
										break;
									case 3:
										pPortal->SetState(PORTAL_OPENNING);
										pPortal->SetEvent(0);
										break;
								}
								delete pSA;
							}
							else
							{
								pPortal->SetState(PORTAL_OPENNING);
							}
						}

						if(pPortal->GetState() == PORTAL_OPENNING)
						{
							return ACTION_RESULT_ADVANCE_FRAME;
						}
						else
						{
							if(PreludeParty.IsMember(this))
								return ACTION_RESULT_NOT_POSSIBLE;
								
						}
					}
					else
					{
						if(PreludeWorld->InCombat() &&
							!PreludeParty.IsMember(this))
							{
								InsertAction(ACTION_MOVEIN,(void *)0,(void *)Dest, TRUE);
							}

						if(PreludeParty.IsMember(this))
							return ACTION_RESULT_NOT_POSSIBLE;
					}
				}

				if(CheckX == EndX && CheckY == EndY)
				{
					return ACTION_RESULT_FINISHED;
				}

				Path TempPath;

				pOb = Valley->FindObject(CheckX,CheckY, OBJECT_CREATURE);
				if(pOb)
				{
					TempPath.SetDepth(400);

					if(PreludeWorld->InCombat())
						TempPath.FindCombatPath(x,y,EndX,EndY, 0.0f, this);
					else
						TempPath.FindPathObjects(x,y,EndX,EndY, 0.0f, this);

					if(TempPath.GetLength() < 666)
					{
						delete pPath;
						pPath = new Path();
						memcpy(pPath, &TempPath,sizeof(Path));
						pCurAction->SetTarget((void *)pPath);
						return ACTION_RESULT_UNFINISHED;
					}
					else
					{
						DEBUG_INFO("Failed to find path around obstacle.\n");
					}

					Creature *pCreature;
					pCreature = (Creature *)pOb;

					if(pCreature->IsMoving())
					{
						//wait
						InsertAction(ACTION_IDLE,NULL,NULL);
						return ACTION_RESULT_ADVANCE_FRAME;
					}
					else
					{

					}
				}
				else
				{
					return ACTION_RESULT_FINISHED;
				}
			}
		}
		//large pathing
		//large things are only creatures.
		else
		{
			if(Large && (!Valley->IsClear(CheckX, CheckY, this)))
			{
				Object *pOb;
				pOb = Valley->FindObject(CheckX,CheckY, OBJECT_PORTAL);
				if(!pOb)
					pOb = Valley->FindObject(CheckX+1,CheckY, OBJECT_PORTAL);
				if(!pOb)
					pOb = Valley->FindObject(CheckX,CheckY+1, OBJECT_PORTAL);
				if(!pOb)
					pOb = Valley->FindObject(CheckX+1,CheckY+1, OBJECT_PORTAL);
				
				if(pOb)
				{
					Portal *pPortal;
					pPortal = (Portal *)pOb;
					if(pPortal->GetState() != PORTAL_OPENNING && pPortal->GetState() != PORTAL_OPEN)
					{
						pPortal->SetState(PORTAL_OPENNING);
					}

					if(pPortal->GetState() == PORTAL_OPENNING)
					{
						return ACTION_RESULT_ADVANCE_FRAME;
					}
				}
				else
				{
					pOb = Valley->FindObject(CheckX,CheckY,OBJECT_CREATURE);
					if(pOb && pOb->GetObjectType() == OBJECT_CREATURE)
					{
						if(((Thing *)pOb)->GetUniqueID() ==
							this->GetUniqueID())
						{

						}
						else
						{
							DEBUG_INFO("Large Creature could not traverse path.\n searching for new path\n");
							Path TempPath;
							TempPath.SetDepth(1500);
							
							if(PreludeWorld->InCombat())
								TempPath.FindLargeCombatPath(x,y,EndX,EndY, 0.0f, this);	
							else
								TempPath.FindLargePathObjects(x,y,EndX,EndY, 0.0f, this);

							if(TempPath.GetLength() < 666)
							{
								delete pPath;
								pPath = new Path();
								memcpy(pPath, &TempPath,sizeof(Path));
								pCurAction->SetTarget((void *)pPath);
								return ACTION_RESULT_UNFINISHED;
							}
							else
							{
								DEBUG_INFO("Failed to find path around obstacle.");
							}
						}
						return ACTION_RESULT_NOT_POSSIBLE;
					}
				}

			}
		}

		if(CurNode  > 0 && CurNode < pPath->GetLength() - 1)
		{
			float CurAngle;
			float DestAngle;
			float CWLength;
			float CCWLength;
			float RotationOffset;
			
			CurAngle = GetData(INDEX_ROTATION).fValue;
			
			DestAngle = DIRECTIONANGLES[Dest];

			if(CurAngle != DestAngle)
			{
				//place a rotate action to face in that direction
				//calculate the rotation perframe
				float StartRotation, EndRotation;

				StartRotation = CurAngle;

				EndRotation = DestAngle;
				
				CWLength = AngleDifCW(StartRotation,EndRotation);
				CCWLength = AngleDifCCW(StartRotation,EndRotation);

				//rotate in shorter angle
				if(CWLength > CCWLength)
				{
					RotationOffset = -1.0f * CCWLength/fWalkFrames;
					angleoff = RotationOffset;
				}
				else
				{
					RotationOffset = CWLength/fWalkFrames;
					angleoff = RotationOffset;
				}
			}	
			else
			{
				angleoff = 0.0f;
			}
		}

		if(CurNode == 0)
		{
			InsertAction(ACTION_MOVEIN,(void *)0,(void *)Dest, TRUE);
		}
		else
		{
			InsertAction(ACTION_MOVEINNOROTATE,(void *)0,(void *)Dest, TRUE);
		}
		return ACTION_RESULT_ADDED_ACTION;
	}

	angleoff = 0.0f;
	return ACTION_RESULT_FINISHED;
}

//MoveIn
//	move one tile in a specific direction
ACTION_RESULT_T Creature::MoveIn() 
{
	D3DVECTOR *pPosition;
	
	pPosition = GetData(INDEX_POSITION).pVector;

	//if not facing the direction specified in the data of the current action
	if(!pCurAction->IsStarted())
	{


		//if the action has not started yet check to see that the tile is clear
		//check to see that we have enough action points
	

		int x = (int)pPosition->x;
		int y = (int)pPosition->y;

		//BYTE Blocking = Valley->GetTileBlocking(x,y);
		float DestHeight;

		int CheckX;
		int CheckY;
		int MP;
		MP = this->GetData(INDEX_MOVEPOINTS).Value;

		//move in the correct direction
		switch((int)pCurAction->GetData())
		{
			case NORTH:
				CheckX = x;
				CheckY = y - 1;
				break;
			case SOUTH:
				CheckX = x;
				CheckY = y + 1;
				break;
			case EAST:
				CheckX = x+1;
				CheckY = y;
				break;
			case WEST:
				CheckX = x-1;
				CheckY = y;
				break;
			case NORTHEAST:
				CheckX = x+1;
				CheckY = y - 1;
				MP += MP /2;
				break;
			case SOUTHEAST:
				CheckX = x+1;
				CheckY = y + 1;
				MP += MP /2;
				break;
			case NORTHWEST:
				CheckX = x-1;
				CheckY = y - 1;
				MP += MP /2;
				break;
			case SOUTHWEST:
				CheckX = x-1;
				CheckY = y+1;
				MP += MP /2;
				break;
			default:
				//FATAL_ERROR("%s has no direction to MoveIn",GetData("NAME").String);
				break;
		}//switch

		if(GetData(INDEX_ACTIONPOINTS).Value < MP && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			return ACTION_RESULT_OUT_OF_AP;
		}

		float CurAngle;
		float DestAngle;
		CurAngle = GetData(INDEX_ROTATION).fValue;
		DestAngle = DIRECTIONANGLES[(int)pCurAction->GetData()];

		if(CurAngle < (DestAngle - 0.05f) || CurAngle > (DestAngle + 0.05f))
		{
			//place a rotate action to face in that direction
			InsertAction(ACTION_ROTATE,(void *)NULL,pCurAction->GetData(),TRUE);
			return ACTION_RESULT_ADDED_ACTION;
		}
				
		//first do a quick think
		if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT
			&& (this->GetData(INDEX_LOOKAT_OVERRIDE).Value ||
			this->GetData(INDEX_BATTLESIDE).Value))
		{
			//check my viewable distance
			//base viewable is 20 tiles.
			float CanSeeDistance = 15.0f;
			int BestStealth;
			BestStealth = PreludeParty.GetBest(INDEX_STEALTH)->GetData(INDEX_STEALTH).Value;
			
			CanSeeDistance -= ((float)BestStealth / 70.0f) * 10.0f; 
			
			float Distance;
			Distance = GetDistance(this,(Creature *)PreludeParty.GetLeader());
			if(Distance < CanSeeDistance)
			{
				ULONG TimeNow;
				TimeNow = timeGetTime();
				if(TimeNow - LastChecked > PATH_CHECK_TIME)
				{
					LastChecked = TimeNow;
					Path TempPath;
					TempPath.SetDepth(1500);
					if(TempPath.FindPathObjects(this->GetPosition()->x, this->GetPosition()->y,
									PreludeParty.GetLeader()->GetPosition()->x, PreludeParty.GetLeader()->GetPosition()->y, 1.5f, this)
						&&
						TempPath.GetLength() < CanSeeDistance + 5)
					{
						if(this->GetData(INDEX_LOOKAT_OVERRIDE).Value)
						{
							Push(this);
							PreludeEvents.RunEvent(GetData(INDEX_LOOKAT_OVERRIDE).Value);
							ScriptArg *pSA;
							pSA = Pop();
							BOOL Continue;
							Continue = (BOOL)pSA->GetValue();
							delete pSA;
							if(Continue)
							{		
								//do nothing, complete move
							}
							else
							{
								//a clear action necessitates this
								return ACTION_RESULT_ADVANCE_FRAME;
							}
						}
						else
						{
							PreludeWorld->GetCombat()->Start();
							return ACTION_RESULT_ADVANCE_FRAME;
						}
					}
				}
			}
		}

		DestHeight = Valley->GetTileHeight(CheckX, CheckY, DIR_NONE);
		
		if(Large)
		{
			float TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1, CheckY);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX , CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
		}


		if(DestHeight < 0.02f)
		{
			DestHeight = (GetData(INDEX_POSITION).pVector)->z;
		}

		float Length = fWalkFrames;

		//move in the correct direction
		switch((int)pCurAction->GetData())
		{
			case NORTH:
				yoff = -1.0f/Length;
				break;
			case SOUTH:
				yoff = 1.0f/Length;
				break;
			case EAST:
				xoff = 1.0f/Length;
				break;
			case WEST:
				xoff = -1.0f/Length;
				break;
			case NORTHEAST:
				yoff = -1.0f/Length;
				xoff = 1.0f/Length;
				break;
			case SOUTHEAST:
				yoff = 1.0f/Length;
				xoff = 1.0f/Length;
				break;
			case NORTHWEST:
				yoff = -1.0f/Length;
				xoff = -1.0f/Length;
				break;
			case SOUTHWEST:
				yoff = 1.0f/Length;
				xoff = -1.0f/Length;
				break;
			default:
				break;
		}

		zoff = (DestHeight - (GetData(INDEX_POSITION).pVector)->z)/Length;

		pCurAction->Start();
		
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			ActionCount++;
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)(WALK_LEFT + ActionCount % 2), TRUE);
		}
		else
		{
			ActionCount++;
			if(!PreludeParty.IsMember(this) && PreludeWorld->GetGameState() == GAME_STATE_NORMAL
				&& !this->GetData(INDEX_TYPE).Value)	
				FrameAdd = 1;
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)(NONCOMBAT_WALK_LEFT + ActionCount % 2), TRUE);
		}
		
		//subtract the action points
	//subtract the action points
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value - MP);
		else
		{
			Valley->RemoveFromUpdate(this,x,y);
			Valley->AddToUpdate(this, CheckX, CheckY);
		}

		return ACTION_RESULT_ADDED_ACTION;
	}

/*
#ifndef NDEBUG
	char temp[64];
	sprintf(temp,"%s at ( %f x %f )\n",GetData(INDEX_NAME).String,pPosition->x,pPosition->y);
	DEBUG_INFO(temp);
#endif
*/
	pPosition->y = ((float)((int)pPosition->y)) + 0.5f;
	pPosition->x = ((float)((int)pPosition->x)) + 0.5f;
	if(!Large)
		pPosition->z = Valley->GetTileHeight(pPosition->x,pPosition->y,DIR_NONE);
	else
	{
		int CheckX;
		int CheckY;
		CheckX = (int)pPosition->x;
		CheckY = (int)pPosition->y;
		float DestHeight;
		DestHeight = Valley->GetTileHeight(CheckX, CheckY, DIR_NONE);
		
		if(Large)
		{
			float TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX, CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
		}

		pPosition->z = DestHeight;
	}

	if(PreludeParty.IsMember(this))
	{
		PreludeParty.ChangedPosition();
	}

	SetData(INDEX_ROTATION,DIRECTIONANGLES[(int)pCurAction->GetData()]);

	pRegionIn = Valley->GetRegion(this->GetPosition());
	
	return ACTION_RESULT_FINISHED;
}

ACTION_RESULT_T Creature::MoveNoRotate()
{
	D3DVECTOR *pPosition;
	
	pPosition = GetData(INDEX_POSITION).pVector;

	//if not facing the direction specified in the data of the current action
	if(!pCurAction->IsStarted())
	{
		//first do a quick think
		if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT
			&& (this->GetData(INDEX_LOOKAT_OVERRIDE).Value ||
			this->GetData(INDEX_BATTLESIDE).Value))
		{
			//check my viewable distance
			//base viewable is 20 tiles.
			float CanSeeDistance = 15.0f;
			int BestStealth;
			BestStealth = PreludeParty.GetBest(INDEX_STEALTH)->GetData(INDEX_STEALTH).Value;
			
			CanSeeDistance -= ((float)BestStealth / 70.0f) * 10.0f; 
			
			float Distance;
			Distance = GetDistance(this,(Creature *)PreludeParty.GetLeader());
			if(Distance < CanSeeDistance)
			{
				ULONG TimeNow;
				TimeNow = timeGetTime();
				if(TimeNow - LastChecked > PATH_CHECK_TIME)
				{
					LastChecked = TimeNow;
					Path TempPath;
					TempPath.SetDepth(1500);
					if(TempPath.FindPathObjects(this->GetPosition()->x, this->GetPosition()->y,
									PreludeParty.GetLeader()->GetPosition()->x, PreludeParty.GetLeader()->GetPosition()->y, 1.5f, this)
						&&
						TempPath.GetLength() < CanSeeDistance + 5)
					{
						if(this->GetData(INDEX_LOOKAT_OVERRIDE).Value)
						{
							Push(this);
							PreludeEvents.RunEvent(GetData(INDEX_LOOKAT_OVERRIDE).Value);
							ScriptArg *pSA;
							pSA = Pop();
							BOOL Continue;
							Continue = (BOOL)pSA->GetValue();
							delete pSA;
							if(Continue)
							{		
								//do nothing, complete move
							}
							else
							{
								//a clear action necessitates this
								return ACTION_RESULT_ADVANCE_FRAME;
							}
						}
						else
						{
							PreludeWorld->GetCombat()->Start();
							return ACTION_RESULT_ADVANCE_FRAME;
						}
					}
				}
			}
		}

		//if the action has not started yet check to see that the tile is clear
		//check to see that we have enough action points
	

		int x = (int)pPosition->x;
		int y = (int)pPosition->y;

		BYTE Blocking = Valley->GetTileBlocking(x,y);
		float DestHeight;

		pCurAction->Start();

		int CheckX;
		int CheckY;
		int MP;
		MP = this->GetData(INDEX_MOVEPOINTS).Value;

		//move in the correct direction
		switch((int)pCurAction->GetData())
		{
			case NORTH:
				CheckX = x;
				CheckY = y - 1;
				break;
			case SOUTH:
				CheckX = x;
				CheckY = y + 1;
				break;
			case EAST:
				CheckX = x+1;
				CheckY = y;
				break;
			case WEST:
				CheckX = x-1;
				CheckY = y;
				break;
			case NORTHEAST:
				CheckX = x+1;
				CheckY = y - 1;
				MP += MP /2;
				break;
			case SOUTHEAST:
				CheckX = x+1;
				CheckY = y + 1;
				MP += MP /2;
				break;
			case NORTHWEST:
				CheckX = x-1;
				CheckY = y - 1;
				MP += MP /2;
				break;
			case SOUTHWEST:
				CheckX = x-1;
				CheckY = y+1;
				MP += MP /2;
				break;
			default:
				//FATAL_ERROR("%s has no direction to MoveIn",GetData("NAME").String);
				break;
		}//switch

		if(GetData(INDEX_ACTIONPOINTS).Value < MP && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			//if(GetData("ACTIONPOINTS").Value)
			//{
			//	return ACTION_RESULT_NOT_POSSIBLE;
			//}
			//else
			//{
				return ACTION_RESULT_OUT_OF_AP;
			//}
		}

		DestHeight = Valley->GetTileHeight(CheckX, CheckY, DIR_NONE);
		
		if(Large)
		{
			float TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1, CheckY);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX , CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
		}


		if(DestHeight < 0.02f)
		{
			DestHeight = (GetData(INDEX_POSITION).pVector)->z;
		}

		float Length = fWalkFrames;

		//move in the correct direction
		switch((int)pCurAction->GetData())
		{
			case NORTH:
				yoff = -1.0f/Length;
				break;
			case SOUTH:
				yoff = 1.0f/Length;
				break;
			case EAST:
				xoff = 1.0f/Length;
				break;
			case WEST:
				xoff = -1.0f/Length;
				break;
			case NORTHEAST:
				yoff = -1.0f/Length;
				xoff = 1.0f/Length;
				break;
			case SOUTHEAST:
				yoff = 1.0f/Length;
				xoff = 1.0f/Length;
				break;
			case NORTHWEST:
				yoff = -1.0f/Length;
				xoff = -1.0f/Length;
				break;
			case SOUTHWEST:
				yoff = 1.0f/Length;
				xoff = -1.0f/Length;
				break;
			default:
				break;
		}

		zoff = (DestHeight - (GetData(INDEX_POSITION).pVector)->z)/Length;

		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			ActionCount++;
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)(WALK_LEFT + ActionCount % 2), TRUE);
		}
		else
		{
			ActionCount++;
			if(!PreludeParty.IsMember(this) && PreludeWorld->GetGameState() == GAME_STATE_NORMAL 
				&& !this->GetData(INDEX_TYPE).Value)	
				FrameAdd = 1;
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)(NONCOMBAT_WALK_LEFT + ActionCount % 2), TRUE);
		}
		
		//subtract the action points
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value - MP);
		else
		{
			Valley->RemoveFromUpdate(this,x,y);
			Valley->AddToUpdate(this, CheckX, CheckY);
		}			


		return ACTION_RESULT_ADVANCE_FRAME;
	}

/*
#ifndef NDEBUG
	char temp[64];
	sprintf(temp,"%s at ( %f x %f )\n",GetData(INDEX_NAME).String,pPosition->x,pPosition->y);
	DEBUG_INFO(temp);
#endif
*/
	pPosition->y = ((float)((int)pPosition->y)) + 0.5f;
	pPosition->x = ((float)((int)pPosition->x)) + 0.5f;
	if(!Large)
		pPosition->z = Valley->GetTileHeight(pPosition->x,pPosition->y,DIR_NONE);
	else
	{
		int CheckX;
		int CheckY;
		CheckX = (int)pPosition->x;
		CheckY = (int)pPosition->y;
		float DestHeight;
		DestHeight = Valley->GetTileHeight(CheckX, CheckY, DIR_NONE);
		
		if(Large)
		{
			float TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX, CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
			TempHeight = Valley->GetTileHeight(CheckX + 1,CheckY + 1);
			if(TempHeight > DestHeight) DestHeight = TempHeight;
		}

		pPosition->z = DestHeight;
	}

	if(PreludeParty.IsMember(this))
	{
		PreludeParty.ChangedPosition();
	}

	SetData(INDEX_ROTATION,DIRECTIONANGLES[(int)pCurAction->GetData()]);

	pRegionIn = Valley->GetRegion(this->GetPosition());
	
	return ACTION_RESULT_FINISHED;
}


	


//Unconscious
// do nothing until rest points are greater than 0
ACTION_RESULT_T Creature::Unconscious()
{
	char blarg[64];

	if(pCurAction->IsFinished())
	{
		return ACTION_RESULT_FINISHED; 
	}
	//first animate the falling down sequence
	//if rest points are less than zero
	if((PreludeWorld->GetGameState() != GAME_STATE_COMBAT || GetData(INDEX_BATTLESIDE).Value) && GetData(INDEX_HITPOINTS).Value <= 0
		|| this->LastPlacedTime == -1)
	{
		pCurAction->Finish();
		
		RemoveCurrentAction();


		InsertAction(ACTION_DIE,NULL,NULL);
		
		return ACTION_RESULT_ADDED_ACTION;
	}

	if(!pCurAction->IsStarted())
	{

		if(PreludeParty.IsMember(this) && this->GetData(INDEX_DIE_OVERRIDE).Value)
		{
			pCurAction->Finish();
		
			PreludeEvents.RunEvent(this->GetData(INDEX_DIE_OVERRIDE).Value);
						
			return ACTION_RESULT_ADVANCE_FRAME;
			//should check to see if we need to continue
		}

		
		sprintf(blarg,"%s falls unconscious.", this->GetData(INDEX_NAME).String);
		Describe(blarg);

		if(GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS &&
			GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_DEAD)
		{
			pCurAction->Start();
			pCurAction->SetData((void *)0);
			
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)FALL_DOWN, TRUE);
			
			SetData(INDEX_BATTLESTATUS,CREATURE_STATE_UNCONSCIOUS);
			return ACTION_RESULT_ADDED_ACTION;
		}
		else
		{
			return ACTION_RESULT_FINISHED;
		}
	
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			PreludeWorld->GetCombat()->CheckForEnd();

	}

	if(GetData(INDEX_HITPOINTS).Value > 0)
	{
		pCurAction->Finish();
		SetData(INDEX_BATTLESTATUS,CREATURE_STATE_NONE);
		InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)GET_UP, TRUE);
		sprintf(blarg,"%s gets up.", this->GetData(INDEX_NAME).String);
		Describe(blarg);

		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{
		SetData(INDEX_ACTIONPOINTS,0);
	}
	
	return ACTION_RESULT_ADVANCE_FRAME;

}

//Rotate
//	Rotate to face a particular direction
ACTION_RESULT_T Creature::Rotate() 
{
	float StartRotation;
	float EndRotation;
	float CWLength;
	float CCWLength;
	float RotationOffset;

	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		pCurAction->SetLength(10);

		//calculate the rotation perframe
		StartRotation = GetData(INDEX_ROTATION).fValue;

		EndRotation = DIRECTIONANGLES[(int)pCurAction->GetData()];
		
		if(StartRotation < EndRotation + 0.05 && StartRotation > EndRotation - 0.05)
		{
			return ACTION_RESULT_FINISHED;
		}

		CWLength = AngleDifCW(StartRotation,EndRotation);
		CCWLength = AngleDifCCW(StartRotation,EndRotation);

		//rotate in shorter angle
		if(CWLength > CCWLength)
		{
			RotationOffset = -1.0f * CCWLength/(float)pCurAction->GetLength();
			
			angleoff = RotationOffset;

			//check to see if our next action
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)TURN_LEFT, TRUE);
		}
		else
		{
			InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)NONCOMBAT_TURN_LEFT, TRUE);
		}
			return ACTION_RESULT_ADDED_ACTION;
		}
		else
		{
			RotationOffset = CWLength/(float)pCurAction->GetLength();
		
			angleoff = RotationOffset;
			//check to see if our next action
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)TURN_RIGHT, TRUE);
			}
			else
			{
				InsertAction(ACTION_ANIMATE, (void *)NULL, (void *)NONCOMBAT_TURN_RIGHT, TRUE);
			}
			return ACTION_RESULT_ADDED_ACTION;
		}
	}

	angleoff = 0.0f;
	SetData(INDEX_ROTATION, DIRECTIONANGLES[(int)pCurAction->GetData()]);

	return ACTION_RESULT_FINISHED;
	
}

//Attack
//	Attempt to hit something with currently equipped weapon(s) if any
ACTION_RESULT_T Creature::Attack() 
{
	//Describe("Attack");
	Creature *pDefender;
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	
	pDefender = (Creature *)pCurAction->GetTarget();
	if(!pDefender || pCurAction->IsFinished())
	{
		return ACTION_RESULT_FINISHED;
	}
	int DistanceToTarget;

	//otherwise
	if(!pCurAction->IsStarted())
	{
		DistanceToTarget = GetDistance(this, pDefender);
		if(DistanceToTarget > GetRange())
		{
			//add an approach action
			InsertAction(ACTION_APPROACH, pCurAction->GetTarget(), (void *)(int)(GetRange() - 1), TRUE);
			RemoveAction(ACTION_ATTACK);
			return ACTION_RESULT_ADDED_ACTION;
		}
		else
		{
			float CurAngle;
			float DestAngle;
			CurAngle = GetMyAngle();
			DIRECTION_T facing;
			facing = FindFacing(GetData(INDEX_POSITION).pVector, ((Thing *)(pCurAction->GetTarget()))->GetData(INDEX_POSITION).pVector);
			DestAngle = DIRECTIONANGLES[facing];

			if(CurAngle < (DestAngle - 0.025f) || CurAngle > (DestAngle + 0.025f))
			{
				//Describe("aDDING ROTATION");
				//place a rotate action to face in that direction
				InsertAction(ACTION_ROTATE,(void *)NULL,(void *)facing, TRUE);
				return ACTION_RESULT_ADDED_ACTION;
			}		
			else
			{
				//Describe("Calculating cost");
				float APCost;
				APCost = (float)GetData(INDEX_ATTACKPOINTS).Value;
				pGI = this->GetEquipment("RIGHTHAND");
				if(pGI)
				{
					//dagger speed bonus
					if(pGI->GetData("TYPE").Value == 2 && 
						pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_DAGGER && 
						this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
					{
						APCost -= 1;
					}
				}
				else
				{
					//unarmed speed bonus
					if(this->GetWeaponSkill() >= 20)
					{
						APCost -= 1;
					}
				}

				switch((int)pCurAction->GetData())
				{
					case ATTACK_NORMAL:
						break;
					case ATTACK_STRONG:
						APCost *= STRONG_AP_MULTIPLIER;
						break;
					case ATTACK_RAPID:
						APCost *= QUICK_AP_MULTIPLIER;
						break;
					case ATTACK_FEINT:
						APCost *= FEINT_AP_MULTIPLIER;
						break;
					case ATTACK_AIMED:
						APCost *= AIMED_AP_MULTIPLIER;
						APCost += 1;
						break;
					default:
						//FATAL_ERROR("Attack for %s type not specified",GetData("NAME").String);
						break;
				}

				int FinalApCost = (int)(APCost + 0.5f);

				if(FinalApCost <= GetData(INDEX_ACTIONPOINTS).Value)
				{
					SetData(INDEX_ACTIONPOINTS,GetData(INDEX_ACTIONPOINTS).Value - FinalApCost);
				}
				else
				{
					//Describe("Not Enough AP");
					//check to see if we should do something else
					if(!PreludeParty.IsMember(this))
						InsertAction(ACTION_DEFEND,NULL,NULL);
					return ACTION_RESULT_OUT_OF_AP;
				}

				pCurAction->Start();
				pGI = this->GetEquipment("RIGHTHAND");
				if(pGI && (pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE ||
					(pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_THROWN && DistanceToTarget > 1.5f)))
				{
					if(pGI)
					{
						DamageOverride = pGI->GetData("DAMAGEOVERRIDE").Value;
						if(pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
						{
							//check for ammo
							int AmmoType;
							AmmoType = pGI->GetItem()->GetData("AMMOTYPE").Value;
							GameItem *pAmmo;
							ZSModelEx *pMissileMesh = NULL;
							
							pAmmo = this->GetEquipment("AMMO");
							if(AmmoType)
							{
								if(!pAmmo || pAmmo->GetItem()->GetData("AMMOTYPE").Value != AmmoType)
								{
									SetData(INDEX_ACTIONPOINTS,GetData(INDEX_ACTIONPOINTS).Value + FinalApCost);
									Describe("Not enough Ammo.");
									return ACTION_RESULT_FINISHED;
								}
		
								DamageOverride = pAmmo->GetData("DAMAGEOVERRIDE").Value;
		
								pMissileMesh = pAmmo->GetMesh();

								AmmoItemNumber = pAmmo->GetData(INDEX_ID).Value;

								if(pAmmo->GetQuantity() <= 1)
								{
									char dstring[64];
									sprintf(dstring,"%s is out of ammo.",this->GetData(INDEX_NAME).String);
									Describe(dstring);
									this->Unequip(pAmmo);
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


							D3DVECTOR vStart;

							vStart = *GetData(INDEX_POSITION).pVector;

							vStart.z += (GetMesh()->GetHeight() * 0.75f);

							D3DVECTOR vEnd;

							vEnd = *((Thing *)(pCurAction->GetTarget()))->GetData(INDEX_POSITION).pVector;

							vEnd.z += (((Thing *)pCurAction->GetTarget())->GetMesh()->GetHeight() * 0.75f);

							Missile *pMissile;


							pMissile = new Missile(pMissileMesh,Engine->GetTexture("items"),&vStart,&vEnd, 15.0f);
							
							PreludeWorld->AddMainObject((Object *)pMissile);
							//InsertAction(ACTION_MISSILE,(DWORD)pMissile,0);
							int NumWait;
							NumWait = pMissile->GetNumMoveFrames() - 15;
							if(NumWait > 0)
							{
							//	Describe ("adding wait");
								InsertAction(ACTION_WAITUNTIL,(void *)NumWait,0, TRUE);
							}
						}
						else
						if(pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_THROWN)
						{
							ZSModelEx *pMissileMesh = NULL;
							
							pMissileMesh = pGI->GetMesh();
							
							if(pGI->GetQuantity() - 1 <= 0)
							{
								char dstring[64];
								sprintf(dstring,"%s is out of %s.",this->GetData(INDEX_NAME).String,pGI->GetItem()->GetData(INDEX_NAME).String);
								Describe(dstring);
								this->Unequip(pGI);
							}
			
							pGI->SetQuantity(pGI->GetQuantity() - 1);


							if(!pMissileMesh)
							{
								DEBUG_INFO("thrown object does not have mesh\n");
								pMissileMesh = Engine->GetMesh("stonebullet");
							}


							D3DVECTOR vStart;

							vStart = *GetData(INDEX_POSITION).pVector;

							vStart.z += (GetMesh()->GetHeight() * 0.75f);

							D3DVECTOR vEnd;

							vEnd = *((Thing *)(pCurAction->GetTarget()))->GetData(INDEX_POSITION).pVector;

							vEnd.z += (((Thing *)pCurAction->GetTarget())->GetMesh()->GetHeight() * 0.75f);

							Missile *pMissile;


							pMissile = new Missile(pMissileMesh,Engine->GetTexture("items"),&vStart,&vEnd, 15.0f);
							
							PreludeWorld->AddMainObject((Object *)pMissile);
							//InsertAction(ACTION_MISSILE,(DWORD)pMissile,0);
							int NumWait;
							NumWait = pMissile->GetNumMoveFrames() - 15;
							if(NumWait > 0)
							{
							//	Describe ("adding wait");
								InsertAction(ACTION_WAITUNTIL,(void *)NumWait,0, TRUE);
							}
						}
					}
					else
					{
						DamageOverride = 0;
					}
				}
			
				pGI = this->GetEquipment("LEFTHAND");
				if(pGI && pGI->GetData("TYPE").Value == 2)
				{
					InsertAction(ACTION_SECONDARY_ATTACK, pCurAction->GetTarget(), pCurAction->GetData(), TRUE);
				}

				pGI = this->GetEquipment("RIGHTHAND");
				if(pGI)
				{
					DamageOverride = pGI->GetData("DAMAGEOVERRIDE").Value;
					int AnimRange;
					AnimRange = pGI->GetData("ANIMATIONRANGE").Value;
					if(AnimRange)
					{
						InsertAction(ACTION_ANIMATE,NULL, (void *)AnimRange, TRUE);
					}
					else
					{
						InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
					}
				}
				else
				{
					InsertAction(ACTION_ANIMATE,NULL, (void *)PUNCH_ATTACK, TRUE);
				}
				
				//Describe("Starting animation");

				return ACTION_RESULT_ADDED_ACTION;
			}
		}
	}

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(this, pDefender);

	//determine the basic type of attack (missile or melee)
	BOOL IsMelee = TRUE;


	pGI = this->GetEquipment("RIGHTHAND");
	int FieldIndex;
	double CriticalChance = 0;
	int ParryBonus = 0;
	int AbsorbModifier = 100;
	
	if(!pGI)
	{
		FieldIndex = this->GetIndex("UNARMED");
		IsMelee = TRUE;
	}
	else
	{
		switch(pGI->GetData("SUBTYPE").Value)
		{
		case WEAPON_TYPE_SWORD:
			IsMelee = TRUE;
			FieldIndex = INDEX_SWORD;
			if(this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				ParryBonus = 10;
			break;
		case WEAPON_TYPE_DAGGER:
			IsMelee = TRUE;
			FieldIndex = INDEX_DAGGER;
			break;
		case WEAPON_TYPE_AXE:
			IsMelee = TRUE;
			FieldIndex = INDEX_AXE;
			if(this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				CriticalChance = 5;
			break;
		case WEAPON_TYPE_BLUNT:
			IsMelee = TRUE;
			FieldIndex = INDEX_BLUNT;
			if(this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				AbsorbModifier = 75;
			break;
		case WEAPON_TYPE_POLEARM:
			IsMelee = TRUE;
			FieldIndex = INDEX_POLEARM;
			if(this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				ParryBonus = 10;
			break;
		case WEAPON_TYPE_MISSILE:
			IsMelee = TRUE;
			FieldIndex = INDEX_MISSILE;
			break;
		default:
		case WEAPON_TYPE_UNARMED:
			IsMelee = TRUE;
			FieldIndex = INDEX_UNARMED;
			break;
		case WEAPON_TYPE_THROWN:
			IsMelee = TRUE;
			FieldIndex = INDEX_THROWING;
			break;
		}
	}

	//modify to hit by type of attack
	switch((int)pCurAction->GetData())
	{
		case ATTACK_NORMAL:
			break;
		case ATTACK_STRONG:
			if(!IsMelee)
			{
				//FATAL_ERROR("%s attempted Strong Attack with Missile Weapon",GetData("NAME").String);
			}
			else
			{
				ToHit = ToHit + MELEE_STRONG_TOHIT_MODIFIER;
			}
			break;
		case ATTACK_RAPID:
			if(!IsMelee)
			{
				ToHit = ToHit + MISSILE_RAPID_TOHIT_MODIFIER;
			}
			else
			{
				ToHit = ToHit + MELEE_RAPID_TOHIT_MODIFIER;
			}
			break;
		case ATTACK_FEINT:
			if(!IsMelee)
			{
				//FATAL_ERROR("%s attempted to Feint with Missile Weapon",GetData("NAME").String);
			}
			else
			{
				ToHit = ToHit + MELEE_FEINT_TOHIT_MODIFIER;
			}
			break;
		case ATTACK_AIMED:
			if(!IsMelee)
			{
				ToHit = ToHit + MISSILE_AIMED_TOHIT_MODIFIER;
			}
			else
			{
				ToHit = ToHit + MELEE_AIMED_TOHIT_MODIFIER;
			}
			break;
		default:
			//FATAL_ERROR("Attack for %s type not specified",GetData("NAME").String);
			break;
	}

	if(!IsMelee)
	{
		//modify ToHit by range
		if(pGI && this->GetWeaponSkill() > pGI->GetData("PASSIVE").Value)
		{
			float fDistance;
			float fRange;
			fRange = this->GetRange();
			fDistance = GetDistance(this, pDefender);
			if(fDistance < fRange / 4.0f)
			{
				ToHit += 15;
			}
			else
			if(fDistance < fRange / 2.0f)
			{
				ToHit += 7;
			}
		}


	}


	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

/*
#ifndef NDEBUG
	char Blarg[64];
	sprintf(Blarg,"attack roll = %i\n", AttackRoll);
	DEBUG_INFO(Blarg);
#endif
*/
	if(PreludeParty.IsMember(this))
		this->ImproveSkill(FieldIndex);

	CriticalChance += ((float)ToHit * CRITICAL_NORMAL_MULTIPLIER);

	if(!PreludeParty.IsMember(this))
	{
		CriticalChance *= 0.66;
	}

	if(AttackRoll < ToHit)
	{
		if(AttackRoll < (int)CriticalChance)
		{
			IsCritical = TRUE;
			Describe("a critical!");
		}

		if(Parried(this,pDefender))
		{
			return ACTION_RESULT_FINISHED;
		}
		

		int BaseDamage = 0;
		int DamageRange = 1;
		int MinDamage = 0;
	
		if(AmmoItemNumber)
		{
			Item *pItem;
			pItem = (Item *)Thing::Find((Thing *)Item::GetFirst(),AmmoItemNumber);
			if(pItem)
			{
				DamageRange += (pItem->GetData("MAXDAMAGE").Value - pItem->GetData("MINDAMAGE").Value);
				MinDamage = pItem->GetData("MINDAMAGE").Value;
				BaseDamage = (rand() % DamageRange) + MinDamage;
			}

			SetAmmoItemNumber(0);
		}
		BaseDamage = GetDamage(FALSE);

	//modify to hit by type of attack
		switch((int)pCurAction->GetData())
		{
			case ATTACK_NORMAL:
				break;
			case ATTACK_STRONG:
				if(!IsMelee)
				{
					//FATAL_ERROR("%s attempted Strong Attack with Missile Weapon",GetData("NAME").String);
				}
				else
				{
					BaseDamage = (int)((double)BaseDamage * MELEE_STRONG_DAMAGE_MULTIPLIER);
				}
				break;
			case ATTACK_RAPID:
				if(!IsMelee)
				{
					BaseDamage = (int)((double)BaseDamage * MISSILE_RAPID_DAMAGE_MULTIPLIER);
				}
				else
				{
					BaseDamage = (int)((double)BaseDamage * MELEE_RAPID_DAMAGE_MULTIPLIER);
				}
				break;
			case ATTACK_FEINT:
				if(!IsMelee)
				{
					//FATAL_ERROR("%s attempted to Feint with Missile Weapon",GetData("NAME").String);
				}
				else
				{
					BaseDamage = (int)((double)BaseDamage * MELEE_FEINT_DAMAGE_MULTIPLIER);
				}
				break;
			case ATTACK_AIMED:
				if(!IsMelee)
				{
					BaseDamage = (int)((double)BaseDamage * MISSILE_AIMED_DAMAGE_MULTIPLIER);
				}
				else
				{
					BaseDamage = (int)((double)BaseDamage * MELEE_AIMED_DAMAGE_MULTIPLIER);
				}
				break;
			default:
				//FATAL_ERROR("Attack for %s type not specified",GetData("NAME").String);
				break;
		}

		//modify by strength

		if(IsMelee)
		{
			float DamagePercent;

			DamagePercent = ((float)GetData(INDEX_STRENGTH).Value) / 13.0f;

			float FinDamage;

			FinDamage = DamagePercent * (float)BaseDamage;

			BaseDamage = (int)(FinDamage + 0.5f); 
		}

		//modify by critical

		if(IsCritical)
		{
			if(BaseDamage < GetDamage(FALSE, TRUE))
			{
				BaseDamage = GetDamage(FALSE, TRUE);
			}
			BaseDamage *= CRITICAL_DAMAGE_MULTIPLIER;
		}

/*
#ifndef NDEBUG
	sprintf(Blarg,"Modified damage = %i\n",BaseDamage);
	DEBUG_INFO(Blarg);
#endif

*/
		if(!IsCritical)
			BaseDamage = AbsorbBlow(this, pDefender, BaseDamage, AbsorbModifier);
	
		if(BaseDamage <= 0)
			return ACTION_RESULT_FINISHED;

/*
#ifndef NDEBUG
	sprintf(Blarg,"Final Damage = %i\n",BaseDamage);
	DEBUG_INFO(Blarg);
#endif
*/
		//hit!
//		pGI = GetEquipment("RIGHTHAND");
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
			if(GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		
		//check for a damage override
		if(DamageOverride)
		{
			Push(BaseDamage);
			Push(pDefender);
			Push(this);
			PreludeEvents.RunEvent(DamageOverride);
			DamageOverride = 0;
		}
		else
		{
			((Creature *)pDefender)->TakeDamage(this,BaseDamage,(DAMAGE_T)GetData(INDEX_DAMAGETYPE).Value);
		}
		if(rand() % 3)
			pDefender->ImproveSkill(INDEX_ARMOR);

		pCurAction->Finish();
		InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);
		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{
		//do miss result
		Describe("Miss");
		Engine->Sound()->PlayEffect(10);
		return ACTION_RESULT_FINISHED;
	}

//	pCurAction->SetTarget(0);
//	nsertAction(ACTION_WAITFOROTHERCREATURE,(DWORD)pDefender,0);
	return ACTION_RESULT_FINISHED;//ADDED_ACTION;
}

//Attack
//	Attempt to hit something with currently equipped weapon(s) if any
ACTION_RESULT_T Creature::SpecialAttack() 
{
	if(pCurAction->IsFinished())
	{
		return ACTION_RESULT_FINISHED;
	}

	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		Action *pAction;
		pAction = pCurAction;
		GameItem *pGI;
		pGI = this->GetEquipment("RIGHTHAND");
		int WeaponType;

		if(pGI)
			WeaponType = pGI->GetData("SUBTYPE").Value;
		else
			WeaponType = WEAPON_TYPE_UNARMED;

		switch(WeaponType)
		{
			case WEAPON_TYPE_SWORD:
				SpinAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_DAGGER:
				RapidAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_AXE:
				RendAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_BLUNT:
				BrainAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_POLEARM:
				SweepAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_MISSILE:
				TargettedAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			default:
			case WEAPON_TYPE_UNARMED:
				DisarmAttack(this, (Creature *)pCurAction->GetTarget());
				break;
			case WEAPON_TYPE_THROWN:
				TargettedAttack(this, (Creature *)pCurAction->GetTarget());
				break;
		}
	
		pAction->Finish();
	
		if(pCurAction->GetType() == ACTION_WAITUNTIL)
		{
			return ACTION_RESULT_ADDED_ACTION;
		}

		return ACTION_RESULT_FINISHED;
	}
	else
	{
		return ACTION_RESULT_ADVANCE_FRAME;
	}
}

//Attack
//	Attempt to hit something with currently equipped weapon(s) if any
ACTION_RESULT_T Creature::SecondaryAttack() 
{
	Creature *pDefender;
	GameItem *pGI;
	int AttackSkill = 0;
	int DefenderSkill = 0;
	
	pDefender = (Creature *)pCurAction->GetTarget();
	if(!pDefender || pCurAction->IsFinished())
	{
		return ACTION_RESULT_FINISHED;
	}

	//otherwise
	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();

		InsertAction(ACTION_ANIMATE, NULL, (void *)TWO_HAND_STAB_ATTACK, TRUE);
			
		return ACTION_RESULT_ADDED_ACTION;
	}

	//perform the normal attack calculations as per the type of attack in the data
	int ToHit;
	//calculate base to hit
	ToHit = GetBaseToHit(this, pDefender);

	//determine the basic type of attack (missile or melee)
	BOOL IsMelee = TRUE;


	pGI = this->GetEquipment("LEFTHAND");
	int FieldIndex;

	int AbsorbModifier = 100;

	if(!pGI)
	{
		FieldIndex = this->GetIndex("UNARMED");
		IsMelee = TRUE;
	}
	else
	{
		switch(pGI->GetData("SUBTYPE").Value)
		{
		case WEAPON_TYPE_SWORD:
			IsMelee = TRUE;
			FieldIndex = INDEX_SWORD;
			break;
		case WEAPON_TYPE_DAGGER:
			IsMelee = TRUE;
			FieldIndex = INDEX_DAGGER;
			break;
		case WEAPON_TYPE_AXE:
			IsMelee = TRUE;
			FieldIndex = INDEX_AXE;
			break;
		case WEAPON_TYPE_BLUNT:
			IsMelee = TRUE;
			FieldIndex = INDEX_BLUNT;
			if(this->GetWeaponSkill() >= pGI->GetData("PASSIVE").Value)
				AbsorbModifier = 75;
			break;
		case WEAPON_TYPE_POLEARM:
			IsMelee = TRUE;
			FieldIndex = INDEX_POLEARM;
			break;
		case WEAPON_TYPE_MISSILE:
			IsMelee = TRUE;
			FieldIndex = INDEX_MISSILE;
			break;
		default:
		case WEAPON_TYPE_UNARMED:
			IsMelee = TRUE;
			FieldIndex = INDEX_UNARMED;
			break;
		case WEAPON_TYPE_THROWN:
			IsMelee = TRUE;
			FieldIndex = INDEX_THROWING;
			break;
		}
	}
	
	ToHit = (ToHit * this->GetData(FieldIndex).Value) / (pGI->GetData("SECONDARY").Value + 20);

	BOOL IsCritical = FALSE;

	int AttackRoll;

	AttackRoll = rand() % 100;

	if(pDefender->GetData("BATTLESTATUS").Value == CREATURE_STATE_UNCONSCIOUS)
	{
		AttackRoll = 0;
	}

/*
#ifndef NDEBUG
	char Blarg[64];
	sprintf(Blarg,"attack roll = %i\n", AttackRoll);
	DEBUG_INFO(Blarg);
#endif
*/
	
	if(PreludeParty.IsMember(this))
		this->ImproveSkill(FieldIndex);

	if(AttackRoll < ToHit)
	{
		if(AttackRoll < ((int)((double)ToHit * CRITICAL_NORMAL_MULTIPLIER)) / 2)
		{
			IsCritical = TRUE;
			Describe("Secondary critical!");
		}
		
		if(Parried(this,pDefender))
		{
			return ACTION_RESULT_FINISHED;
		}

		int BaseDamage;
		int DamageRange;
		
		BaseDamage = GetDamage(TRUE);
		BaseDamage = (BaseDamage * 2) / 3;
		
		//modify by strength

		float DamagePercent;

		DamagePercent = ((float)GetData(INDEX_STRENGTH).Value) / 13.0f;

		float FinDamage;

		FinDamage = DamagePercent * (float)BaseDamage;

		BaseDamage = (int)(FinDamage + 0.5f); 
	
		//modify by critical

		if(IsCritical)
		{
			if(BaseDamage < ((GetDamage(TRUE, TRUE) * 2) / 3))
			{
				BaseDamage = ((GetDamage(TRUE, TRUE) * 2) / 3);
			}
			BaseDamage *= CRITICAL_DAMAGE_MULTIPLIER;
		}

/*
#ifndef NDEBUG
	sprintf(Blarg,"Modified damage = %i\n",BaseDamage);
	DEBUG_INFO(Blarg);
#endif

*/
		if(!IsCritical)
		{
			BaseDamage = AbsorbBlow(this,pDefender,BaseDamage, AbsorbModifier);
			if(BaseDamage <= 0)
				return ACTION_RESULT_FINISHED;

		}
/*
#ifndef NDEBUG
	sprintf(Blarg,"Final Damage = %i\n",BaseDamage);
	DEBUG_INFO(Blarg);
#endif
*/
		//hit!
//		pGI = GetEquipment("RIGHTHAND");
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
			if(GetData(INDEX_TYPE).Value)
			{
				Engine->Sound()->PlayEffect(8);
			}
			else
			{
				Engine->Sound()->PlayEffect(6);
			}
		}
		
		((Creature *)pDefender)->TakeDamage(this,BaseDamage,(DAMAGE_T)GetData(INDEX_DAMAGETYPE).Value);

		if(rand() % 3)
			pDefender->ImproveSkill(INDEX_ARMOR);

		pCurAction->Finish();
		InsertAction(ACTION_WAITUNTIL,(void *)15,0, TRUE);
		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{
		//do miss result
		Describe("Miss");
		Engine->Sound()->PlayEffect(10);
		return ACTION_RESULT_FINISHED;
	}
//	pCurAction->SetTarget(0);
//	nsertAction(ACTION_WAITFOROTHERCREATURE,(DWORD)pDefender,0);
	return ACTION_RESULT_FINISHED;//ADDED_ACTION;
}

//Kill
//	Attack something until it is destroyed
ACTION_RESULT_T Creature::Kill() 
{
	Creature *pDefender;
	pDefender = (Creature *)pCurAction->GetTarget();
	//check to see if the target is dead
	if(pDefender->GetData(INDEX_HITPOINTS).Value <= 0)
	{
		//if they are then this action is done
		return ACTION_RESULT_FINISHED;
	}
	//if they are not then add an attack action with the 
	//same target as the kill action
	InsertAction(ACTION_ATTACK,pCurAction->GetTarget(),pCurAction->GetData());
	if(PreludeParty.IsMember(pDefender))
	{
		PreludeWorld->LookAt((Thing *)this);
	}
	return ACTION_RESULT_ADDED_ACTION;
}

//Animate
//	Run through a specific series of animations
ACTION_RESULT_T Creature::Animate() 
{
	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		
		ANIMATION_RANGE_T RangeNum = (ANIMATION_RANGE_T)(int)pCurAction->GetData();
		int Start;
		int End;
		Start = Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(RangeNum)->StartFrame;
		End = Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(RangeNum)->EndFrame;

		pCurAction->SetFrame(Start);
		pCurAction->SetLength(End);
	}

	if(this->FrameAdd && PreludeWorld->GetGameState() == GAME_STATE_NORMAL)
	{
		if(pCurAction->GetCurFrame() >= 1000)
		{
			pCurAction->SetFrame( (pCurAction->GetCurFrame() / 1000));
		}
		else
		{
			pCurAction->SetFrame( pCurAction->GetCurFrame() * 1000);
			return ACTION_RESULT_ADVANCE_FRAME;
		}
	}
	else
	{
		if(pCurAction->GetCurFrame() >= 1000)
		{
			pCurAction->SetFrame( (pCurAction->GetCurFrame() / 1000));
		}
	}

	D3DVECTOR *pV;

	pV = GetData(INDEX_POSITION).pVector;

	pV->x += xoff;
	pV->y += yoff;
	pV->z += zoff;

	SetData(INDEX_FRAME,pCurAction->GetCurFrame());
	
	float CurFacing;

	CurFacing = GetData(INDEX_ROTATION).fValue;
	
	SetData(INDEX_ROTATION, CurFacing + angleoff);
		
	pCurAction->SetFrame(pCurAction->GetCurFrame() + 1);

	if(pCurAction->GetCurFrame() > pCurAction->GetLength())
	{
		xoff = 0.0f;
		yoff = 0.0f;
		zoff = 0.0f;
		angleoff = 0.0f;
		FrameAdd = 0;
		return ACTION_RESULT_FINISHED;
	}
	return ACTION_RESULT_ADVANCE_FRAME;
}

typedef struct
{
	D3DVECTOR vStart;
	D3DVECTOR vEnd;
	D3DVECTOR vRotationPoint;
	D3DXMATRIX matRotate;
} LEAP_STRUCT_T;

//Leap
// A special form of attack available only to certain creatures
//
ACTION_RESULT_T Creature::Leap()
{
	LEAP_STRUCT_T *pLeap;

	if(!pCurAction->IsStarted())
	{
		//face the correct direction
		float CurAngle;
		float DestAngle;
		CurAngle = GetData(INDEX_ROTATION).fValue;
		DIRECTION_T facing;
		facing = FindFacing(GetData(INDEX_POSITION).pVector, ((Thing *)(pCurAction->GetTarget()))->GetData(INDEX_POSITION).pVector);
		DestAngle = DIRECTIONANGLES[facing];

		if(CurAngle != DestAngle)
		{
				//place a rotate action to face in that direction
			InsertAction(ACTION_ROTATE,NULL,(void *)facing, TRUE);
			return ACTION_RESULT_ADDED_ACTION;
		}
		
		pLeap = new LEAP_STRUCT_T;
		pLeap->vStart = *GetData(INDEX_POSITION).pVector;
		pLeap->vEnd = *((Creature *)pCurAction->GetTarget())->GetData(INDEX_POSITION).pVector;
			
		switch(facing)
		{
			case NORTH:
				pLeap->vEnd.y += 1.0f;
				break;
			case SOUTH:
				pLeap->vEnd.y -= 1.0f;
				break;
			case EAST:
				pLeap->vEnd.x -= 1.0f;
				break;
			case WEST:
				pLeap->vEnd.x += 1.0f;
				break;
			case NORTHEAST:
				pLeap->vEnd.y += 1.0f;
				pLeap->vEnd.x -= 1.0f;
				break;
			case SOUTHEAST:
				pLeap->vEnd.y -= 1.0f;
				pLeap->vEnd.x -= 1.0f;
				break;
			case SOUTHWEST:
				pLeap->vEnd.y -= 1.0f;
				pLeap->vEnd.x += 1.0f;
				break;
			case NORTHWEST:
				pLeap->vEnd.y += 1.0f;
				pLeap->vEnd.x += 1.0f;
				break;
			break;
		}

		pLeap->vEnd.z = Valley->GetZ(pLeap->vEnd.x,pLeap->vEnd.y);
		
		float Height;
		Height = 5.0f;
		
		if(Height < 0) Height = -Height;
		
		D3DVECTOR vRotationPoint;

		vRotationPoint.x = (pLeap->vStart.x + pLeap->vEnd.x) / 2.0;
		vRotationPoint.y = (pLeap->vStart.y + pLeap->vEnd.y) / 2.0;
		vRotationPoint.z = (pLeap->vStart.z + pLeap->vEnd.z) / 2.0;
		float RotationAngle = PI / 30.0f;
		
		D3DVECTOR vHigh;

		vHigh = vRotationPoint;
		vHigh.z += Height;
	
		D3DVECTOR vAxis;

		vAxis = CrossProduct((vHigh - vRotationPoint),(pLeap->vEnd - vRotationPoint));

		D3DXMatrixRotationAxis(&pLeap->matRotate, &(D3DXVECTOR3)vAxis, RotationAngle);
	
		pLeap->vRotationPoint = vRotationPoint;

		pCurAction->SetData((void *)(ULONG)pLeap);
		pCurAction->Start();
	
		pCurAction->SetLength(30);
		pCurAction->SetFrame(0);
	}
	else
	{
		pLeap = (LEAP_STRUCT_T *)pCurAction->GetData();
	}

	D3DVECTOR *pPosition;
		
	D3DXVECTOR4 vOut;
		
	pPosition = GetData(INDEX_POSITION).pVector;


	if(pCurAction->GetCurFrame() < 13)
	{
		SetData(INDEX_FRAME, Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->StartFrame + pCurAction->GetCurFrame());
		pCurAction->SetFrame(pCurAction->GetCurFrame() + 1);
	}
	
	if(pCurAction->GetCurFrame() < 30)
	{
		
		
		*pPosition -= pLeap->vRotationPoint;

		D3DXVec3Transform(&vOut,(D3DXVECTOR3 *)pPosition,&pLeap->matRotate);

		*pPosition = (D3DVECTOR)vOut;

		*pPosition += pLeap->vRotationPoint;
		
		pCurAction->SetFrame(pCurAction->GetCurFrame() + 1);
	}
	else
	{
		*pPosition = pLeap->vEnd;
	}

	if(pCurAction->GetCurFrame() >= 25)
	{
		if(pCurAction->GetCurFrame() < 40)
		{
			SetData(INDEX_FRAME, Animations[GetData(INDEX_TYPE).Value % 10].GetAnim(ONE_HAND_SLASH_ATTACK)->StartFrame + pCurAction->GetCurFrame() - 25);
			pCurAction->SetFrame(pCurAction->GetCurFrame() + 1);
		}
		else
		{
			delete pLeap;
			return ACTION_RESULT_FINISHED;
		}
	}
	return ACTION_RESULT_ADVANCE_FRAME;

	
}//end leap

//Talk
//	initiate conversation with another creature, usually a PC
ACTION_RESULT_T Creature::TalkTo() 
{
	if(pCurAction->IsStarted())
	{
		return ACTION_RESULT_ADVANCE_FRAME;
	}
	
	pCurAction->Start();

	Creature *pTarget;
	pTarget = (Creature *)pCurAction->GetTarget();
	
	if(pTarget->GetData(INDEX_USE_OVERRIDE).Value)
	{
		PreludeEvents.RunEvent(pTarget->GetData(INDEX_USE_OVERRIDE).Value);
		return ACTION_RESULT_FINISHED;
	}

	Talk(pTarget);
	
/*	char CharacterID[64];
	
	sprintf(CharacterID,"#%s#",pTarget->GetData(INDEX_NAME).String);

	FILE *fp;
	fp = fopen("people.txt","rt");
	
	if(fp && SeekTo(fp,CharacterID))
	{
		fseek(fp, -(strlen(CharacterID) + 1), SEEK_CUR);
		ScriptBlock SB;
	
		SB.Import(fp);	
		fclose(fp);		
	
		ZSWindow *pWin;
		pWin = new ZSTalkWin(78901, 125, 125, 550,350, &SB);
		((ZSTalkWin *)pWin)->SetPortrait(pTarget->GetData(INDEX_PORTRAIT).String);

		pWin->Show();
	
		ZSWindow::GetMain()->AddChild(pWin);
	
		pWin->SetFocus(pWin);

		pWin->GoModal();

		pWin->ReleaseFocus();
		pWin->Hide();
		
		ZSWindow::GetMain()->RemoveChild(pWin);
	}
	else
	{
		DEBUG_INFO("Failed to find character: ");
		DEBUG_INFO(CharacterID);
		DEBUG_INFO(" in people.txt\n");
		Describe("Failed to find character: ");
		Describe(CharacterID);
		Describe(" in people.txt");
		if(fp)
			fclose(fp);
	}
*/	
	//initiate dialog with the target creature
	return ACTION_RESULT_FINISHED;
}

//Say
// Say something out loud
ACTION_RESULT_T Creature::Say() 
{
	//use the generic message box to display some text
		return ACTION_RESULT_FINISHED;
}

//Take
//	Take an item from the possession of another Creature
ACTION_RESULT_T Creature::Take()
{
	//check the distance to where the item we are supposed to take is
	//if close enough take it
		return ACTION_RESULT_FINISHED;
	//if not move to a location closer to it by
	//inserting an approach action
		return ACTION_RESULT_ADDED_ACTION;
}

//Drop
// Drop an item on the ground
ACTION_RESULT_T Creature::Drop() 
{
	//place the target item on the ground
	D3DVECTOR vDropAt;
	GameItem *pGI, *pNewGI;
	pNewGI = new GameItem();
	pGI = (GameItem *)pCurAction->GetTarget();

	Unequip(pGI);
	
	pNewGI->SetItem(pGI->GetItem());
	pNewGI->SetQuantity(pGI->GetQuantity());
	pNewGI->SetOwner(pGI->GetOwner());
	pGI->SetItem(NULL,0);

	vDropAt = *pNewGI->GetOwner()->GetPosition();
	pNewGI->SetPosition(&vDropAt);
	pNewGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
	pNewGI->SetRegionIn(Valley->GetRegion(&vDropAt));
	Valley->AddToUpdate(pNewGI);
	if(pNewGI->GetOwner())
	{
		if(pNewGI->GetOwner()->GetObjectType() == OBJECT_CREATURE)
		if(!((Creature *)pNewGI->GetOwner())->Take(pNewGI->GetItem(),pNewGI->GetQuantity()))
		{
			SafeExit("Some attempted to drop something they didn't have!\n");
		}
	}
	pNewGI->SetLocation(LOCATION_WORLD,this);
	return ACTION_RESULT_FINISHED;
}

//Give
//	Give an item to another creature
ACTION_RESULT_T Creature::Give() 
{
	//if we're close enough place the item in the target's inventory
		return ACTION_RESULT_FINISHED;
	//otherwise add an approach action to get closer to the target
		return ACTION_RESULT_ADDED_ACTION;
}

//PickUp
//	Pick up an item from the ground
ACTION_RESULT_T Creature::PickUp() 
{
	//if we're close enough take the target item from the ground 
	GameItem *pGI;
	pGI = (GameItem *)pCurAction->GetTarget();

	D3DVECTOR *pPosition, *pItemPosition;
	pPosition = GetPosition();
	pItemPosition = pGI->GetPosition();
	D3DVECTOR Pos1;
	D3DVECTOR Pos2;
	Pos1 = *pPosition;
	Pos2 = *pItemPosition;
	Pos1.z = 0.0f;
	Pos2.z = 0.0f;

	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		if(GetDistance(&Pos1, &Pos2) > 3.6f)
		{
			InsertAction(ACTION_APPROACH, (void *)pGI, (void *)2, TRUE);
			return ACTION_RESULT_ADDED_ACTION;
		}
	}
	else
	{
		if(GetDistance(&Pos1, &Pos2) > 3.6f)
		{
			Describe("Can't reach.");
			return ACTION_RESULT_FINISHED;
		}
	}
			
	//check to see if anyone notices
	//if they do call the notice event
	//first check for a region;
	Region *pRegion = NULL;
	Object *pOb = NULL;
	Creature *pCreature = NULL;

	if(pGI->GetOwner() && PreludeParty.IsMember((Creature *)pGI->GetOwner()))
	{
		Valley->RemoveFromUpdate(pGI);
		Give(pGI->GetItem(),pGI->GetQuantity());
		delete pGI;
		((ZSMainWindow *)ZSWindow::GetMain())->GetTarget();
		return ACTION_RESULT_FINISHED;
	}

	pRegion = Valley->GetRegion(pGI->GetPosition());
	
	BOOL Noticed = FALSE;

	if(pRegion)
	{
		pOb = Valley->GetUpdateSegment(pGI->GetPosition()->x / UPDATE_SEGMENT_WIDTH, pGI->GetPosition()->y / UPDATE_SEGMENT_HEIGHT);

		while(pOb && !Noticed)
		{
			if(pOb->GetObjectType() == OBJECT_CREATURE)
			{
				pCreature = (Creature *)pOb;
				if(!PreludeParty.IsMember(pCreature) && Valley->GetRegion(pCreature->GetPosition()) == pRegion)
				{
					Push(pCreature);
					PreludeEvents.RunEvent(51);

					ScriptArg *pSA;
					pSA = Pop();

					Noticed = (BOOL)pSA->GetValue();
					delete pSA;
				}
			}
			pOb = pOb->GetNextUpdate();
		}
	}

	if(Noticed && pCreature)
	{
		Push(pCreature);
		PreludeEvents.RunEvent(52);

		ScriptArg *pSA;
		pSA = Pop();

		Noticed = (BOOL)pSA->GetValue();
		delete pSA;

	}

	if(!Noticed)
	{
		Valley->RemoveFromUpdate(pGI);
		Give(pGI->GetItem(),pGI->GetQuantity());
		((ZSMainWindow *)ZSWindow::GetMain())->GetTarget();
		delete pGI;
	}
	return ACTION_RESULT_FINISHED;
}

//die
//	cycle the die animation, then die
ACTION_RESULT_T Creature::Die() 
{
	//check for an on die override and call it if there is one
	//otherwise place everything being carried on the ground 
		//check each item to see if it's destructible 
		//if it is and a random check is passed destroy the item rather than drop it
	//place a dead body if possible
		//first animate the falling down sequence
	if(this->GetLastPlacedTime() == -1)
	{
		return ACTION_RESULT_REMOVE_FROM_GAME;
	}

	GameItem *pGI;
	if(!pCurAction->IsStarted())
	{
		//check to see if party member
		if(PreludeParty.IsMember(this))
		{
			PreludeParty.RemoveMember(this);
		}

		//this->OutputDebugInfo();
		char blarg[64];
		sprintf(blarg,"%s dies.",this->GetData(INDEX_NAME).String);
		Describe(blarg);
		//throw all the items in our inventory onto the ground
		//equipment
		
		pCurAction->Start();
		
		if(GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_DEAD &&
			GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS)
		{
			SetData(INDEX_BATTLESTATUS, CREATURE_STATE_DEAD);
			InsertAction(ACTION_ANIMATE, NULL, (void *)DIE, TRUE);
			return ACTION_RESULT_ADDED_ACTION;
		}
	}

	if(!pCurAction->IsFinished())
	{

		Object *pOb;
		GameItem *pNewGI;

		for(int n = 0; n < MAX_EQUIPMENT; n++)
		{
			pGI = GetEquipment(n);
			if(pGI && pGI->GetData("VALUE").Value)
			{
				BOOL Drop = FALSE;
				if(pGI->GetData(INDEX_TYPE).Value == ITEM_TYPE_ARMOR)
				{
					int pRand;
					pRand = rand() % 100;
					if(pRand < 33)
					{
						Drop = TRUE;
					}
				}
				else
				{
					Drop = TRUE;
				}

				if(Drop)
				{
					pNewGI = new GameItem;
					pNewGI->SetItem(pGI->GetCompressed());
					pNewGI->SetPosition(this->GetPosition());
					pNewGI->SetLocation(LOCATION_WORLD);
					pNewGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
					pNewGI->SetOwner(PreludeParty.GetLeader());
					Valley->AddToUpdate((Object *)pNewGI);
				}
			}
		}
	
		
		pOb = GetContents();
		while(pOb)
		{
			pGI = (GameItem *)pOb;
			if(pGI->GetData("VALUE").Value)
			{
				if(pGI->GetData("DROPOVERRIDE").Value)
				{
					PreludeEvents.RunEvent(pGI->GetData("DROPOVERRIDE").Value);
					ScriptArg *pSA;
					pSA = Pop();
					if(pSA->GetValue() && pSA->GetType() == ARG_ITEM)
					{
						pNewGI = new GameItem;
						pNewGI->SetItem((Item *)pSA->GetValue());
						pNewGI->SetPosition(this->GetPosition());
						pNewGI->SetLocation(LOCATION_WORLD);
						pNewGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
						pNewGI->SetOwner(PreludeParty.GetLeader());
						Valley->AddToUpdate((Object *)pNewGI);
					}
				}
				else
				{
					pNewGI = new GameItem;
					pNewGI->SetItem(pGI->GetCompressed());
					pNewGI->SetPosition(this->GetPosition());
					pNewGI->SetLocation(LOCATION_WORLD);
					pNewGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
					pNewGI->SetOwner(PreludeParty.GetLeader());
					Valley->AddToUpdate((Object *)pNewGI);
				}
			}
			pOb = pOb->GetNext();
		}
		
		Object *pBlood;
		//POSSESSED LEAVE NO CORPSES
		if(this->GetMesh() != Engine->GetMesh("majpossessed"))
		{
			pBlood = new Object;
			pBlood->SetPosition(this->GetPosition());
			pBlood->GetPosition()->z += 0.1f;
			pBlood->SetMesh(Engine->GetMesh("blood1"));
			pBlood->SetTexture(Engine->GetTexture("items"));
			pBlood->SetFrame(0);
			pBlood->SetAngle(((float)rand() / (float)RAND_MAX) * PI_MUL_2);
			pBlood->SetScale(((float)rand() / (float)RAND_MAX) * .2 + 0.9f);
			Corpse *pCorpse;
			pCorpse = new Corpse(this);
			Region *pRegion;
			
			pRegion = Valley->GetRegion(pBlood->GetPosition());
			if(pRegion)
			{
				pRegion->AddObject(pBlood);
				pRegion->AddObject(pCorpse);
			}
			else
			{
				Valley->AddObject(pBlood);
				Valley->AddObject(pCorpse);
			
			}
		}

		
		
		
	
		//remove equip objects just in case
		if(pEORHand)
		{
			PreludeWorld->RemoveMainObject(pEORHand);
			delete pEORHand;
			pEORHand = NULL;
		}

		if(pEOLHand)
		{
			PreludeWorld->RemoveMainObject(pEOLHand);
			delete pEOLHand;	
			pEOLHand = NULL;
		}

		if(pEOHead)
		{
			PreludeWorld->RemoveMainObject(pEOHead);
			delete pEOHead;	
			pEOHead = NULL;
		}

		if(pEOBack)
		{
			PreludeWorld->RemoveMainObject(pEOBack);
			delete pEOBack;	
			pEOBack = NULL;
		}
		
		pCurAction->Finish();

// moving check outside
//		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
//				PreludeWorld->GetCombat()->CheckForEnd();
		//check for a die override
	}

	while(this->GetNumLocators() > 0)
	{
		this->RemoveLocator(0);
	}

	this->NumLocators = 0;

//	PreludeWorld->GetArea(this->AreaIn)->RemoveFromUpdate(this);
	this->SetAreaIn(-1);
	this->SetRegionIn(NULL);
	if(PreludeParty.IsMember(this))
		PreludeParty.RemoveMember(this);

	if(this->GetData(INDEX_DIE_OVERRIDE).Value)
	{
		PreludeEvents.RunEvent(this->GetData(INDEX_DIE_OVERRIDE).Value);
		//should check to see if we need to continue
	}

	return ACTION_RESULT_REMOVE_FROM_GAME;

}

//ReactTo
//	
ACTION_RESULT_T Creature::ReactTo() 
{
	//check the react condition
	//if true
		return ACTION_RESULT_FINISHED;
	//otherwise
		return ACTION_RESULT_UNFINISHED;

}

//Defend
//	
ACTION_RESULT_T Creature::Defend() 
{
	int RemainingAP;
	int Modifier = 0;
	RemainingAP = GetData(INDEX_ACTIONPOINTS).Value;
	//based on the type of defense and number of AP left modify our stats by the appropriate amount
	//clear out all action points
	Modifier = RemainingAP * ((GetData(INDEX_ARMOR).Value / 25) + 1);
	SetData(INDEX_BLOCK, Modifier);
	SetData(INDEX_ARMOR, GetData(INDEX_ARMOR).Value + Modifier);
	Modifier = RemainingAP * ((this->GetWeaponSkill() / 25) + 1);
	SetData(INDEX_PARRY, Modifier);
	SetData(INDEX_WEAPONSKILL, GetData(INDEX_WEAPONSKILL).Value + Modifier);
	Modifier = RemainingAP / 3;
	SetData(INDEX_DODGE, Modifier);
	SetData(INDEX_SPEED, GetData(INDEX_SPEED).Value + Modifier);
	SetData(INDEX_ACTIONPOINTS,0);	
	return ACTION_RESULT_FINISHED;
}

ACTION_RESULT_T Creature::Script() 
{
	//call the script function specified 
	//return the result of the script function
	return ACTION_RESULT_FINISHED;

}

ACTION_RESULT_T Creature::Cast()
{ 
	int EncodedNum;
	int SpellNum;
	int PowerLevel;
	Object *pTarget;
	pTarget = (Object *)pCurAction->GetTarget();
	EncodedNum = (int)pCurAction->GetData();
	CastAura *pAura = NULL;

	SpellNum = EncodedNum / 10;
	PowerLevel = EncodedNum % 10;
				
	if(!pCurAction->IsStarted())
	{
		float CurAngle;
		float DestAngle;
		if(pTarget->GetObjectType() != OBJECT_ITEM)
		{
			CurAngle = GetData(INDEX_ROTATION).fValue;
			DIRECTION_T facing;
			facing = FindFacing(this->GetPosition(), pTarget->GetPosition());
			DestAngle = DIRECTIONANGLES[facing];


			if(CurAngle != DestAngle)
			{
				//place a rotate action to face in that direction
				InsertAction(ACTION_ROTATE,NULL,(void *)facing, TRUE);
				return ACTION_RESULT_ADDED_ACTION;
			}
		}

		pCurAction->Start();

		if(PreludeSpells.GetSpell(SpellNum)->GetType() == SPELL_FLAME)
		{
			pAura = new CastAura(this, 40, Engine->Graphics()->GetMaterial(COLOR_RED), Engine->Graphics()->GetMaterial(COLOR_YELLOW), Engine->Graphics()->GetMaterial(COLOR_ORANGE), "star");
			InsertAction(ACTION_ANIMATE, NULL, (void *)CAST_FLAME, TRUE);
		}
		else
		if(PreludeSpells.GetSpell(SpellNum)->GetType() == SPELL_FLAME)
		{
			pAura = new CastAura(this, 40, Engine->Graphics()->GetMaterial(COLOR_BLUE), Engine->Graphics()->GetMaterial(COLOR_WHITE), Engine->Graphics()->GetMaterial(COLOR_BLUE), "star");				
			InsertAction(ACTION_ANIMATE, NULL, (void *)CAST_RIVER, TRUE);
		}
		else
		{
			pAura = new CastAura(this, 40, Engine->Graphics()->GetMaterial(COLOR_PURPLE), Engine->Graphics()->GetMaterial(COLOR_GREEN), Engine->Graphics()->GetMaterial(COLOR_BLUE), "star");				
			InsertAction(ACTION_ANIMATE, NULL, (void *)CAST_FLAME, TRUE);
		}

		PreludeWorld->AddMainObject((Object *)pAura);
		
		return ACTION_RESULT_ADDED_ACTION;
	}


	int FieldIndex;

	if(PreludeSpells.GetSpell(SpellNum)->GetType() == SPELL_FLAME)
	{
		FieldIndex = INDEX_POWER_OF_FLAME;
	}
	else
	if(PreludeSpells.GetSpell(SpellNum)->GetType() == SPELL_RIVER)
	{
		FieldIndex = INDEX_GIFTS_OF_GODDESS;
	}
	else
	{
		FieldIndex = INDEX_THAUMATURGY;
	}

	if(!pCurAction->IsFinished())
	{
		pCurAction->Finish();
		
		PreludeSpells.Cast(SpellNum, PowerLevel, (Object *)this, (Object *)pTarget);
		
		InsertAction(ACTION_WAITUNTIL,(void *)15,(void *)0, TRUE);
		if(PreludeParty.IsMember(this))
			this->ImproveSkill(FieldIndex);

		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			SetData(INDEX_ACTIONPOINTS,GetData(INDEX_ACTIONPOINTS).Value - PreludeSpells.GetSpell(SpellNum)->GetLevel(PowerLevel)->SpeedCost);

		return ACTION_RESULT_ADDED_ACTION;
	}
				
	return ACTION_RESULT_FINISHED;
}


ACTION_RESULT_T Creature::Think() 
{
	//decide what to do next
	//find the nearest enemy;
	Creature *pCreature;
	GameItem *pGI;
	BOOL SwitchedEquipment = FALSE;
	ULONG TimeNow;
					
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		if(Active)
		{
			switch(GetData(INDEX_AICODE).Value)
			{
			case 0: //neutral
				//do nothing, unless attacked as handled in take damage
				InsertAction(ACTION_DEFEND,NULL,NULL);
				break;
			case 1://aggressive attack enemies
				pCreature = (Creature *)PreludeWorld->GetCombat()->FindNearestLiveOpponent(this);
				//check to see if I'm out of ammo
				pGI = this->GetEquipment("RIGHTHAND");
				if(pGI && pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
				{
					//check for ammo
					int AmmoType;
					AmmoType = pGI->GetItem()->GetData("AMMOTYPE").Value;
					GameItem *pAmmo;
					ZSModelEx *pMissileMesh = NULL;
					
					pAmmo = this->GetEquipment("AMMO");
					if(AmmoType)
					{
						if(!pAmmo || pAmmo->GetItem()->GetData("AMMOTYPE").Value != AmmoType)
						{
							SwitchedEquipment = this->Reload();
							if(!SwitchedEquipment)
							{
								SwitchedEquipment = this->EquipBestWeapon();
								if(SwitchedEquipment)
								{
									Describe("Switched Weapons");
								}
							}
							else
							{
								Describe("Reloaded");
							}

							if(SwitchedEquipment)
							{
								InsertAction(ACTION_WAITACTION,NULL,NULL);
								InsertAction(ACTION_WAITACTION,NULL,NULL);
								return ACTION_RESULT_ADVANCE_FRAME;
							}

						}
					}
				}
	
				
				if(GetData(INDEX_ACTIONPOINTS).Value <= 0)
				{
					return ACTION_RESULT_OUT_OF_AP;
				}
			
				if(pCreature)
				{
					Path TempPath;
					int TilesPerTurn;
					TilesPerTurn = this->GetData(INDEX_MAXACTIONPOINTS).Value / this->GetData(INDEX_MOVEPOINTS).Value;
					if(this->Large)
						TempPath.FindLargeCombatPath(this->GetPosition()->x,
											   this->GetPosition()->y,
											   pCreature->GetPosition()->x,
											   pCreature->GetPosition()->y,
											   this->GetRange(), this);
					else
						TempPath.FindCombatPath(this->GetPosition()->x,
										   this->GetPosition()->y,
										   pCreature->GetPosition()->x,
										   pCreature->GetPosition()->y,
										   this->GetRange(), this);
					if(TempPath.GetLength() <= 4 * TilesPerTurn || TempPath.GetLength() < (int)pCreature->GetRange())
					{
						InsertAction(ACTION_KILL,(void *)pCreature,NULL);
					}
					else
					{
						InsertAction(ACTION_DEFEND,NULL,NULL);
					}
				}
				else
				{
					InsertAction(ACTION_DEFEND,NULL,NULL);
				}
				break;
			case 2: //cowardly run away
				break;
			case 3: //berserk, attack anyone!
				break;
			default:
				//do nothing, unless attacked as handled in take damage
				InsertAction(ACTION_DEFEND,NULL,NULL);
				break;
			}
		}
		else
		{
			InsertAction(ACTION_IDLE,NULL,NULL,FALSE);
		}
	}
	else
	{
		//check for a look override
		if(GetData(INDEX_LOOKAT_OVERRIDE).Value)
		{
			//check my viewable distance
			//base viewable is 15 tiles.
			float CanSeeDistance = 15.0f;
			int BestStealth;
			BestStealth = PreludeParty.GetBest(INDEX_STEALTH)->GetData(INDEX_STEALTH).Value;
			
			CanSeeDistance -= ((float)BestStealth / 70.0f) * 10.0f; 
			
			float Distance;
			int n;
			for(n = 0;n < PreludeParty.GetNumMembers(); n++)
			{
				Distance = GetDistance(this->GetPosition(),PreludeParty.GetMember(n)->GetPosition());
				if(Distance < CanSeeDistance)
				{
					TimeNow = timeGetTime();
					if(TimeNow - LastChecked > PATH_CHECK_TIME)
					{
						LastChecked = TimeNow;

						Push(this);
						PreludeEvents.RunEvent(GetData(INDEX_LOOKAT_OVERRIDE).Value);
						BOOL Continue;
						ScriptArg *pSA;
						pSA = Pop();
						Continue = (BOOL)pSA->GetValue();
					
						delete pSA;
						
						if(!Continue)
						{
							return ACTION_RESULT_ADVANCE_FRAME;
						}
					}
				}
			}

		}
		else
		if(GetData(INDEX_BATTLESIDE).Value)
		{
			if(!this->GetRegionIn() || this->GetRegionIn()->IsOccupied())
			{
				float CanSeeDistance = 20.0f;
				int BestStealth;
				BestStealth = PreludeParty.GetBest(INDEX_STEALTH)->GetData(INDEX_STEALTH).Value;
				
				CanSeeDistance -= (float)BestStealth / 70.0f * 12.0f; 
				
				float Distance;
				Distance = GetDistance(this->GetPosition(),PreludeParty.GetLeader()->GetPosition());
				if(Distance < CanSeeDistance)
				{
					ULONG TimeNow;
					TimeNow = PreludeWorld->GetTotalTime();
					if(TimeNow - LastChecked > PATH_CHECK_TIME)
					{
						LastChecked = TimeNow;
						Path TempPath;
						TempPath.SetDepth(1500);

						TempPath.FindPath(this->GetPosition()->x,this->GetPosition()->y, PreludeParty.GetLeader()->GetPosition()->x, PreludeParty.GetLeader()->GetPosition()->y, 1.5f, this);
					
						Distance = TempPath.GetLength();

						//Distance = GetDistance(this->GetPosition(),PreludeParty.GetMember(n)->GetPosition());
						if(Distance < (CanSeeDistance + 2.0f))
						{
							PreludeWorld->GetCombat()->Start();
							return ACTION_RESULT_ADVANCE_FRAME;
						}
					}
				}
			}
		}

		D3DVECTOR *pPosition;
								
		if(PreludeWorld->GetGameState() != GAME_STATE_NORMAL || (PreludeWorld->GetTotalTime() - LastPlacedTime < 30))
		{
			return ACTION_RESULT_ADVANCE_FRAME;
		}
		else
		{
			if(NumLocators)
			{
				int Hour = PreludeWorld->GetHour();
				for(int n = 0; n < NumLocators; n++)
				{
					if((Schedule[n].GetStart() < Schedule[n].GetEnd() &&
						 (Schedule[n].GetStart() <= Hour && Schedule[n].GetEnd() >= Hour))
						 ||
						(Schedule[n].GetStart() > Schedule[n].GetEnd() &&
						 (Schedule[n].GetStart() <= Hour || Schedule[n].GetEnd() >= Hour)))
					{
						Locator *pLocator;
						pLocator = &Schedule[n];
						//we've found a locator
						RECT rLoc;
						Schedule[n].GetBounds(&rLoc);
						
						switch(Schedule[n].GetState())
						{
						case 1: //just like case 0, but with forced facing
							InsertAction(ACTION_ROTATE, NULL, (void *)(int)Schedule[n].GetAngle(), FALSE);
						default:
						case 0:
							if(!(rand() % 5))
							{
								int NewX;
								int NewY;
								NewX = (rand() % (rLoc.right - rLoc.left)) + rLoc.left;
								NewY = (rand() % (rLoc.bottom - rLoc.top)) + rLoc.top;
								BOOL Small;
								Small = (rLoc.right - rLoc.left == 1) && (rLoc.bottom - rLoc.top == 1);
								if(Small || (!Large && PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY))
									|| 
									(Large && PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX+1,NewY)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY+1)
										&& PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX+1,NewY+1)))
								{
									pPosition = this->GetPosition();
									if(PreludeWorld->GetCurAreaNum() != pLocator->GetArea())
									{
										PreludeWorld->GetArea(this->AreaIn)->RemoveFromUpdate(this);
										pPosition->x = (float)NewX + 0.5f;
										pPosition->y = (float)NewY + 0.5f;
										pPosition->z = PreludeWorld->GetArea(pLocator->GetArea())->GetTileHeight(pPosition->x,pPosition->y);
										PreludeWorld->GetArea(pLocator->GetArea())->AddToUpdate(this);
									}
									else
									{
										if(pPosition->x >= (float)rLoc.left &&
											pPosition->y >= (float)rLoc.top &&
											pPosition->x <= (float)rLoc.right &&
											pPosition->y <= (float)rLoc.bottom)
										{
											InsertAction(ACTION_MOVETO, (void *)NewX, (void *)NewY);
										}
										else
										{
											Path TempPath;

											if(TempPath.FindPath(pPosition->x,pPosition->y,NewX,NewY, 0.0f, this))
											{
												InsertAction(ACTION_MOVETO, (void *)NewX, (void *)NewY);
											}
											else
											{
												PreludeWorld->GetArea(this->AreaIn)->RemoveFromUpdate(this);
												pPosition->x = (float)NewX + 0.5f;
												pPosition->y = (float)NewY + 0.5f;
												pPosition->z = PreludeWorld->GetArea(pLocator->GetArea())->GetTileHeight(pPosition->x,pPosition->y);
												this->SetRegionIn(PreludeWorld->GetArea(pLocator->GetArea())->GetRegion(this->GetPosition()));
												PreludeWorld->GetArea(pLocator->GetArea())->AddToUpdate(this);
											}
										}
									}
								}
							}
							else
							{
								InsertAction(ACTION_IDLE,NULL,NULL);
							}
							break; //case 0,1
						case 2:
							pPosition = this->GetPosition();
							pPosition->x = (float)rLoc.left + 0.5f;
							pPosition->y = (float)rLoc.top + 0.5f;
							pPosition->z = Valley->GetTileHeight(pPosition->x,pPosition->y);
							this->SetAngle(DIRECTIONANGLES[Schedule[n].GetAngle()]);
							this->SetRegionIn(PreludeWorld->GetArea(pLocator->GetArea())->GetRegion(this->GetPosition()));
							InsertAction(ACTION_SIT,NULL,NULL);
							break;
						case 3:
							pPosition = this->GetPosition();
							pPosition->x = (float)rLoc.left + 0.5f;
							pPosition->y = (float)rLoc.top + 0.5f;
							pPosition->z = Valley->GetTileHeight(pPosition->x,pPosition->y);
							this->SetAngle(DIRECTIONANGLES[Schedule[n].GetAngle()]);
							this->SetRegionIn(PreludeWorld->GetArea(pLocator->GetArea())->GetRegion(this->GetPosition()));
							pPosition->z = pPosition->z + 0.1f;
							InsertAction(ACTION_LAY,NULL,NULL);
							break;
						}
						break;
					}
				}
			}
		}
	}
	return ACTION_RESULT_ADDED_ACTION;
}

ACTION_RESULT_T Creature::UseItem() 
{
	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		GameItem *pGI;
		pGI = (GameItem *)pCurAction->GetTarget();
		ZSWindow *pWin = NULL;
		
		if(pGI->GetLocation() == LOCATION_PERSON)
		{
			//we used the item from an inventory slot
			pWin = ZSWindow::GetMain()->GetChild(66666);
			if(pWin)
				pWin->Hide();
		}
		
		Push(this);
		
		PreludeEvents.RunEvent(pGI->GetItem()->GetData("USEOVERRIDE").Value);
			
		ScriptArg *pSAUsed;
		pSAUsed = Pop();

		if(pSAUsed->GetValue())
		{

		}
		else
		{

		}
		if(pWin)
		{
			pWin = ZSWindow::GetMain()->GetChild(66666);
			if(pWin)
				pWin->Show();
		}
		pWin = ZSWindow::GetMain()->GetChild(CHARACTER_WINDOW_ID);
		if(pWin)
		{
			CharacterWin *pCharWin;
			pCharWin = (CharacterWin *)pWin;
			pCharWin->ResetStats();
		}


		return ACTION_RESULT_FINISHED;
	}
	return ACTION_RESULT_ADVANCE_FRAME;
}

ACTION_RESULT_T Creature::User() 
{
	
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT && 
		this->GetData(INDEX_FEINT).Value < 0)
	{
		return Think();
	}

#ifdef AUTOTEST
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		return Think();
	}
#endif

	//do nothing
	if(PreludeWorld->GetGameState() != GAME_STATE_TEXT)
	{
		InsertAction(ACTION_IDLE,NULL,NULL,TRUE);
		return ACTION_RESULT_WAIT_FOR_USER;
	}
	else
	{
		return ACTION_RESULT_ADVANCE_FRAME;
	}
}


ACTION_RESULT_T Creature::Approach() 
{
	//if we're close enough to the target we're finished
	Object *pTarget;

	pTarget = (Object *)pCurAction->GetTarget();
	
	float CurDistance;
	float NeededDistance;

	NeededDistance = (float)(int)pCurAction->GetData() + 0.5f;

	if(NeededDistance < 1.5f) 
		NeededDistance = 1.5f;

//	NeededDistance = GetData("RANGE").fValue;
	CurDistance = GetDistance(this, (Creature *)pTarget);

	if(CurDistance <= NeededDistance)
	{
		return ACTION_RESULT_FINISHED;
	}

	Path TempPath;
	TempPath.SetTraveller(this);
	if(Large)
		TempPath.SetTravellerSize(2);

	TempPath.SetTarget(pTarget);

	if(pTarget->GetObjectType() == OBJECT_CREATURE && ((Creature *)pTarget)->Large)
		TempPath.SetTargetSize(2);
	else
		TempPath.SetTargetSize(1);


	D3DVECTOR vDest;
	D3DVECTOR vStart;
	D3DVECTOR vCurNode;
	int xTarget = 0;
	int yTarget = 0;
	float fMinDistance = 256.0f;
	BOOL LargeSearch = FALSE;
	
	vDest.x = pTarget->GetPosition()->x;
	vDest.y = pTarget->GetPosition()->y;
	vDest.z = 0.0f;
	vStart.x = this->GetPosition()->x;
	vStart.y = this->GetPosition()->y;
	vStart.z = 0.0f;

	Path LargePaths[4];
	Path *BestPath;

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		if(Large)
		{
			if(pTarget->GetObjectType() == OBJECT_CREATURE &&
				((Creature *)pTarget)->IsLarge())
			{
				LargePaths[0].FindLargeCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y, NeededDistance, this);
				LargePaths[1].FindLargeCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x+1, (int)vDest.y, NeededDistance, this);
				LargePaths[2].FindLargeCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y+1, NeededDistance, this);
				LargePaths[3].FindLargeCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x+1, (int)vDest.y+1, NeededDistance, this);
			
				BestPath = &LargePaths[0];
				if(BestPath->GetLength() > LargePaths[1].GetLength())
				{
					BestPath = &LargePaths[1];
				}
				if(BestPath->GetLength() > LargePaths[2].GetLength())
				{
					BestPath = &LargePaths[2];
				}
				if(BestPath->GetLength() > LargePaths[3].GetLength())
				{
					BestPath = &LargePaths[3];
				}
				TempPath = *BestPath;
				
			}
			else
			{
				TempPath.FindLargeCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y, NeededDistance, this);
			}
		}	
		else
		{
			if(pTarget->GetObjectType() == OBJECT_CREATURE &&
				((Creature *)pTarget)->IsLarge())
			{
				LargePaths[0].FindCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y, NeededDistance, this);
				LargePaths[1].FindCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x+1, (int)vDest.y, NeededDistance, this);
				LargePaths[2].FindCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y+1, NeededDistance, this);
				LargePaths[3].FindCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x+1, (int)vDest.y+1, NeededDistance, this);
			
				BestPath = &LargePaths[0];
				if(BestPath->GetLength() > LargePaths[1].GetLength())
				{
					BestPath = &LargePaths[1];
				}
				if(BestPath->GetLength() > LargePaths[2].GetLength())
				{
					BestPath = &LargePaths[2];
				}
				if(BestPath->GetLength() > LargePaths[3].GetLength())
				{
					BestPath = &LargePaths[3];
				}
				TempPath = *BestPath;
			}
			else
			{
				TempPath.FindCombatPath((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y,  NeededDistance, this);
			}
		}
	}
	else
	{
		if(Large)
		{
			TempPath.FindLargePathObjects((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y, NeededDistance, this);
		
		}	
		else
		{

			TempPath.FindPathObjects((int)vStart.x, (int)vStart.y, (int)vDest.x, (int)vDest.y,  NeededDistance, this);
		}	
	}

	if(TempPath.GetLength() < 660)
	{
//		InsertAction(ACTION_MOVEIN,NULL,(void *)TempPath.Traverse(), TRUE);
//		return ACTION_RESULT_ADDED_ACTION;
	}
	else
	{
		return ACTION_RESULT_NOT_POSSIBLE;
	}

	//figure out how far down the path we must travel to be close enough
	Path *FollowPath;
	
	FollowPath = new Path;

	memcpy(FollowPath, &TempPath, sizeof(Path));

	InsertAction(ACTION_FOLLOWPATH,(void *)FollowPath,(void *)NULL, TRUE);

	return ACTION_RESULT_ADDED_ACTION;
}

ACTION_RESULT_T Creature::WaitUntil() 
{
	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		pCurAction->SetLength((int)pCurAction->GetTarget());
		pCurAction->SetFrame(0);
		return ACTION_RESULT_ADVANCE_FRAME;
	}

	pCurAction->SetFrame(pCurAction->GetCurFrame() + 1);

	if(pCurAction->GetCurFrame() > pCurAction->GetLength())
	{
		return ACTION_RESULT_FINISHED;
	}
	else
	{
		return ACTION_RESULT_ADVANCE_FRAME;
	}	
}
ACTION_RESULT_T Creature::WaitForSight() 
{
	//check to see if a PC is in sight range.
	//if they are 
		return ACTION_RESULT_FINISHED;
	//otherwise
		return ACTION_RESULT_UNFINISHED;
}
ACTION_RESULT_T Creature::WaitForHear() 
{
	//check for a PC in audio range
	//if there is one
		return ACTION_RESULT_FINISHED;
	//otherwise
		return ACTION_RESULT_UNFINISHED;
}
ACTION_RESULT_T Creature::WaitForClear() 
{
	//check to see if the target space is clear yet.
	//if it is 
		return ACTION_RESULT_FINISHED;
	//if it isn't 
		return ACTION_RESULT_UNFINISHED;
}

ACTION_RESULT_T Creature::WaitForOtherCreature()
{
	Creature *pCreature;

	pCreature = (Creature *)pCurAction->GetTarget();
	ACTION_RESULT_T ReturnType;
	ReturnType = pCreature->Update();
	if(ReturnType == ACTION_RESULT_OUT_OF_AP)
	{
		return ACTION_RESULT_FINISHED;
	}

	return ReturnType;
}

ACTION_RESULT_T Creature::WaitMissile()
{
/*	Missile *pMissile;

	pMissile = (Missile *)pCurAction->GetTarget();

	if(pMissile->Update() == MISSILE_STATE_DONE)
	{
		delete pMissile;
		return ACTION_RESULT_FINISHED;
	}
	else
	{
		return ACTION_RESULT_ADVANCE_FRAME;
	}
*/
	return ACTION_RESULT_FINISHED;
}

ACTION_RESULT_T Creature::UseThing() 
{
	//attempt to use a thing, possibly on another thing if data is not empty
	//if successfult
		return ACTION_RESULT_FINISHED;
	//if unable to use
		return ACTION_RESULT_UNFINISHED;
}

ACTION_RESULT_T Creature::Open()
{
	Object *pThing = (Object *)pCurAction->GetTarget();
	Region *pRegion;
	Region *pRegion1;
	Region *pRegion2;
	Creature *pPicker;
	Object *pOb;
	Creature *pCreature;
	int Noticed = FALSE;
	char blarg[128];
	if(!ValidateKey(NULL))
	{
		if(PreludeWorld->GetCurAreaNum() == 0 && (GetPosition()->x < 1000 || GetPosition()->x > 1600 ||
			GetPosition()->y < 1000 || GetPosition()->y > 1900))
		{
			Message("Some actions can not be taken outside of Kellen in the Trial Version","Ok");
			return ACTION_RESULT_FINISHED;
		}
	}
																	
	switch(pThing->GetObjectType())
	{
		case OBJECT_PORTAL:
			Portal *pPortal;
			pPortal = (Portal *)pThing;
			int opentype;
			opentype = (int)pCurAction->GetData();
			switch(opentype)
			{
				case 0: //basic unlocked door;
					if(!pPortal->GetLock())
						pPortal->SetState(PORTAL_OPENNING);
					break;		
				case 1: //locked  

					if(Confirm(ZSWindow::GetMain(), "Pick the Lock?","yes","no"))
					{
						if(!(rand() % 3))
							this->ImproveSkill(INDEX_TINKER);

						//check to see if anyone notices
						//if they do call the notice event
						//first check for a region;
						pRegion1 = pPortal->GetRegionOne();
						pRegion2 = pPortal->GetRegionTwo();
					
						pOb = NULL;
						pCreature = NULL;

					
						pOb = Valley->GetUpdateSegment(pPortal->GetPosition()->x / UPDATE_SEGMENT_WIDTH, pPortal->GetPosition()->y / UPDATE_SEGMENT_HEIGHT);

						while(pOb && !Noticed)
						{
							if(pOb->GetObjectType() == OBJECT_CREATURE)
							{
								pCreature = (Creature *)pOb;
								if(!PreludeParty.IsMember(pCreature) && (Valley->GetRegion(pCreature->GetPosition()) == pRegion1 
																			||
																		Valley->GetRegion(pCreature->GetPosition()) == pRegion2))
								{
									Push(pCreature);
									PreludeEvents.RunEvent(51);

									ScriptArg *pSA;
									pSA = Pop();

									Noticed = (BOOL)pSA->GetValue();
									delete pSA;
								}
							}
							pOb = pOb->GetNextUpdate();
						}

						if(Noticed && pCreature)
						{
							Push(pCreature);
							PreludeEvents.RunEvent(53);

							ScriptArg *pSA;
							pSA = Pop();

							Noticed = (BOOL)pSA->GetValue();
							delete pSA;

						}

						if(!Noticed)
						{
							DEBUG_INFO("attempting to pick lock\n");
							pPicker = PreludeParty.GetBest(INDEX_TINKER);
							int TinkerSkill;
							int LockValue;
							int TinkerChance;
							TinkerSkill = pPicker->GetData(INDEX_TINKER).Value;
							LockValue = pPortal->GetLock();
							if(TinkerSkill <= LockValue)
							{
								Describe("No one has high enough tinker skill to pick the lock.");
							}
							else
							{
								TinkerChance = (TinkerSkill - LockValue) * 10;
								int nrand;
								nrand = rand() % 100;
								if(nrand < TinkerChance)
								{
									sprintf(blarg,"%s picks the lock.", pPicker->GetData(INDEX_NAME).String);
									Describe(blarg);
									pPortal->SetState(PORTAL_OPENNING);
									pPortal->SetLock(0);
								}
								else
								{
									sprintf(blarg,"%s fails to pick the lock.", pPicker->GetData(INDEX_NAME).String);
									Describe(blarg);
								}
							}
						}
						else
						if(Noticed == 2)
						{
							Valley->IllegalActivity(this);
						}
					}
					break;
				case 2: //locked, but has key
					pPortal->SetState(PORTAL_OPENNING);
					Thing *pItem;
					pItem = Thing::Find((Thing *)Item::GetFirst(),pPortal->GetKey());
					//PreludeParty.Take(pItem);
					DEBUG_INFO("Door opened with key...\n");
					pPortal->SetLock(0);
					break;
				default:
					break;
			}
			//check for an event
			if(pPortal->GetEvent())
			{
				PreludeEvents.RunEvent(pPortal->GetEvent());
				ScriptArg *pSA;
				pSA = Pop();
				int Result;
				Result = (int)pSA->GetValue();
				switch(Result)
				{
				case 0:
					pPortal->SetState(PORTAL_CLOSED);
					break;
				case 1:
					pPortal->SetState(PORTAL_OPENNING);
					break;
				case 2:
					pPortal->SetState(PORTAL_CLOSED);
					pPortal->SetEvent(0);
					break;
				case 3:
					pPortal->SetState(PORTAL_OPENNING);
					pPortal->SetEvent(0);
					break;
				}
				delete pSA;
			}
			break;
		case OBJECT_ITEM:
		case OBJECT_CONTAINER:
				GameItem *pGI;
			pGI = (GameItem *)pThing;
			//check to see if anyone notices
			//if they do call the notice event
			//first check for a region;
			pRegion = Valley->GetRegion(pGI->GetPosition());
	
			if(pRegion)
			{
				int xn;
				int yn;
				int StartX;
				int EndX;
				int StartY;
				int EndY;
				StartX = ((int)pGI->GetPosition()->x - 32) / UPDATE_SEGMENT_WIDTH;
				if(StartX < 0) StartX = 0;
				StartY = ((int)pGI->GetPosition()->y - 32) / UPDATE_SEGMENT_HEIGHT;
				if(StartY < 0) StartY = 0;
				
				EndX = ((int)pGI->GetPosition()->x + 32) / UPDATE_SEGMENT_WIDTH;
				if(EndX >= (Valley->GetWidth() / UPDATE_SEGMENT_WIDTH)) EndX = Valley->GetWidth() / UPDATE_SEGMENT_WIDTH - 1;
				
				EndY = ((int)pGI->GetPosition()->y + 32) / UPDATE_SEGMENT_HEIGHT;
				if(EndY >= (Valley->GetWidth() / UPDATE_SEGMENT_WIDTH)) EndY = Valley->GetWidth() / UPDATE_SEGMENT_WIDTH - 1;
				
				for(yn = StartY; yn <= EndY; yn++)
				for(xn = StartX; xn <= EndX; xn++)
				{
					pOb = Valley->GetUpdateSegment(xn,yn);

					while(pOb && !Noticed)
					{
						if(pOb->GetObjectType() == OBJECT_CREATURE)
						{
							pCreature = (Creature *)pOb;
							if(!PreludeParty.IsMember(pCreature) && Valley->GetRegion(pCreature->GetPosition()) == pRegion)
							{
								Push(pCreature);
								PreludeEvents.RunEvent(51);

								ScriptArg *pSA;
								pSA = Pop();

								Noticed = (BOOL)pSA->GetValue();
								delete pSA;
							}
						}
						pOb = pOb->GetNextUpdate();
					}
					if(Noticed)
					{
						break;
					}
				}
			}

			if(Noticed && pCreature)
			{
				Push(pCreature);
				PreludeEvents.RunEvent(53);

				ScriptArg *pSA;
				pSA = Pop();

				Noticed = (BOOL)pSA->GetValue();
				delete pSA;
				return ACTION_RESULT_FINISHED;

			}

		if(!Noticed)
		{
			if(pThing->GetData())
			{		
				if(Confirm(ZSWindow::GetMain(), "Pick the Lock?","yes","no"))
				{
					DEBUG_INFO("attempting to pick lock\n");
					pPicker = PreludeParty.GetBest(INDEX_TINKER);
					int TinkerSkill;
					int LockValue;
					int TinkerChance;
					TinkerSkill = pPicker->GetData(INDEX_TINKER).Value;
					LockValue = pThing->GetData();
					if(TinkerSkill <= LockValue)
					{
						Describe("No one has high enough tinker skill to pick the lock.");
						return ACTION_RESULT_FINISHED;
					}
					else
					{
						TinkerChance = (TinkerSkill - LockValue) * 10;
						int nrand;
						nrand = rand() % 100;
						if(TinkerChance > 0 && nrand < TinkerChance)
						{
							sprintf(blarg,"%s picks the lock.", pPicker->GetData(INDEX_NAME).String);
							Describe(blarg);
							pThing->SetData(0);	
						}
						else
						{
							sprintf(blarg,"%s fails to pick the lock.", pPicker->GetData(INDEX_NAME).String);
							Describe(blarg);
							return ACTION_RESULT_FINISHED;
						}
					}
				}
				else
				{
					return ACTION_RESULT_FINISHED;
				}
			}
		
			ZSContainerWin *pCWin;
			pCWin = new ZSContainerWin(-1,100,100,600,400,pThing);
			pCWin->Show();
			pCWin->GetMain()->AddChild(pCWin);
			pCWin->SetFocus(pCWin);
			pCWin->GoModal();
			pCWin->ReleaseFocus();
			ZSWindow::GetMain()->RemoveChild(pCWin);
		}	
		else
		if(Noticed == 2)
		{
			Valley->IllegalActivity((Object *)this);
		}
		break;
	case OBJECT_ENTRANCE:
		Entrance *pEntrance;
		pEntrance = (Entrance *)pThing;
		pEntrance->Go();
		if(pCurAction && pCurAction->GetType() == ACTION_OPEN)
		{
			return ACTION_RESULT_FINISHED;
		}
		else
		{
			return ACTION_RESULT_ADVANCE_FRAME;
		}
		default:
			break;
	}
	return ACTION_RESULT_FINISHED;
}

ACTION_RESULT_T Creature::Close()
{
	Thing *pThing = (Thing *)pCurAction->GetTarget();

	switch(pThing->GetObjectType())
	{
		case OBJECT_PORTAL:
			Portal *pPortal;
			pPortal = (Portal *)pThing;
			pPortal->SetState(PORTAL_CLOSING);
			break;
		case OBJECT_ITEM:

		default:
			break;
	}
	return ACTION_RESULT_FINISHED;
}

ACTION_RESULT_T Creature::Use()
{

	return ACTION_RESULT_FINISHED;
}


ACTION_RESULT_T Creature::LookAt() 
{
	Object *pThing;
	pThing = (Object *)pCurAction->GetTarget();
	pThing->LookAt(this);
	return ACTION_RESULT_FINISHED;
}


ACTION_RESULT_T Creature::PickPocket()
{
	char blarg[128];
	
	if(!pCurAction->IsStarted())
	{
		pCurAction->Start();
		//open the steal window
		Push(this); //picker
		Push((Creature *)pCurAction->GetTarget()); //target
		
		PreludeEvents.RunEvent(25); //generic pick pocket event

		this->ImproveSkill(INDEX_PICKPOCKET);

		ScriptArg *pSA;
		pSA = Pop();
		int result;
		result = (int)pSA->GetValue();
		delete pSA;

		switch(result)
		{
		case 0:
			//do nothing
			sprintf(blarg,"%s fails to pilfer anything from %s.",this->GetData(INDEX_NAME).String, ((Creature *)pCurAction->GetTarget())->GetData(INDEX_NAME).String);
			Describe(blarg);
			break;
		case 1:
			//success, get random item from target
			int GrabbedChance;
			int MaxWeight;

			MaxWeight = 5 * (((this->GetData(INDEX_PICKPOCKET).Value) / 20) + 1);

			GrabbedChance = 10;
			GameItem *pGI;
			pGI = NULL;
			Creature *pTarget;
			pTarget = (Creature *)pCurAction->GetTarget();
			int numattempts;
			numattempts = 0;

			pGI = (GameItem *)pTarget->GetContents();
			if(!pGI)
			{
				sprintf(blarg,"%s has nothing to steal.", ((Creature *)pCurAction->GetTarget())->GetData(INDEX_NAME).String);
				Describe(blarg);
				return ACTION_RESULT_FINISHED;
			}

			while(pGI)
			{
				if(pGI->GetData("VALUE").Value == -1)//autosucceed for quest items.
				{
					this->Give(pGI->GetItem(),pGI->GetQuantity());
					pTarget->Take(pGI->GetItem(),pGI->GetQuantity());
					sprintf(blarg,"%s lifts an item from %s.",this->GetData(INDEX_NAME).String, ((Creature *)pCurAction->GetTarget())->GetData(INDEX_NAME).String);
					Describe(blarg);			
					DEBUG_INFO("Lifted quest item.\n");
					return ACTION_RESULT_FINISHED;
				}
				pGI = (GameItem *)pGI->GetNext();
			}

			while(TRUE && numattempts < 20)
			{
				if(!pGI)
					pGI = (GameItem *)pTarget->GetContents();
				
				if(pGI && (rand() % 100) < GrabbedChance)
				{
					Item *pItem;
					pItem = pGI->GetItem();
					if(pItem->GetData("WEIGHT").Value < MaxWeight)
					{
						int Quantity;
						Quantity = (rand() % pGI->GetQuantity()) + 1;

						if(pGI->GetData("DROPOVERRIDE").Value)
						{
							PreludeEvents.RunEvent(pGI->GetData("DROPOVERRIDE").Value);
							ScriptArg *pSA;
							pSA = Pop();
							if(pSA->GetValue() && pSA->GetType() == ARG_ITEM)
							{
								this->Give(pItem, Quantity);
								sprintf(blarg,"%s lifts something from %s.",this->GetData(INDEX_NAME).String, ((Creature *)pCurAction->GetTarget())->GetData(INDEX_NAME).String);
								Describe(blarg);
							}
						}
						else
						{
							sprintf(blarg,"%s lifts something from %s.",this->GetData(INDEX_NAME).String, ((Creature *)pCurAction->GetTarget())->GetData(INDEX_NAME).String);
							Describe(blarg);
							this->Give(pItem, Quantity);
							pTarget->Take(pItem, Quantity);
						}
						break;
					}
				}
				if(pGI)
					pGI = (GameItem *)pGI->GetNext();
				numattempts++;
			}
			break;
		case 2:
		default:
			Push((Creature *)pCurAction->GetTarget()); //target
			//party was seend doing illegal!
			PreludeEvents.RunEvent(53);
			int Noticed;
			ScriptArg *pResult;
			pResult = Pop();
			Noticed = (int)pResult->GetValue();
			delete pResult;
			if(Noticed == 2)
			{
				Valley->IllegalActivity(this);
			}
			break;
		}
	}
		
	return ACTION_RESULT_FINISHED;
}

int Creature::EnterCombat()
{

	KillActions();

	if(PreludeParty.IsMember(this))
	{
		InsertAction(ACTION_USER,NULL,NULL);
	}
	else
	{
		InsertAction(ACTION_THINK,NULL,NULL);
	}
	
	if(PreludeParty.IsMember(this))
	{
		int xAt;
		int yAt;
		Object *pFound;
	
		xAt = (int)this->GetPosition()->x;
		yAt = (int)this->GetPosition()->y;

		pFound = Valley->FindOtherObject(xAt,yAt,OBJECT_CREATURE, (Object *)this);

		if(pFound)
		{
			int xOff;
			int yOff;
			int xn; 
			int yn;
			xOff = 0;
			yOff = 0;
			BOOL Placed = FALSE;
			Path TempPath;

			while(!Placed)
			{
				xOff++;
				yOff++;
				for(xn = xAt - xOff; xn <= xAt + xOff; xn++)
				for(yn = yAt - yOff; yn <= yAt + yOff; yn++)
				{
					if(TempPath.FindPathObjects(xAt,yAt,xn,yn,0.0f, (Object *)this))
					{
						D3DVECTOR *pV;
						pV = this->GetPosition();
						pV->x = (float)xn + 0.5f;
						pV->y = (float)yn + 0.5f;
						pV->z = Valley->GetTileHeight(xn,yn);
	
						Placed = TRUE;
						break;
					}
				}
			}
		}
	}
	InsertAction(ACTION_ANIMATE,NULL,(void *)ENTER_CROUCH,TRUE);
	return TRUE;
}

int Creature::SetFirst(Creature *NewFirst)
{
	pFirst = NewFirst;
	return TRUE;
}

/*
int Creature::AddItem(Thing *pAddThing)
{
	//cycle through all the inventory slots and if one is empty place the thing in it
	//otherwise return false
	int InventoryIndex = GetIndex("INVENTORY1");

	for(int n = 0; n < NUM_INVENTORY; n++)
	{
		if(!GetData(InventoryIndex).Value)
		{
			SetData(InventoryIndex,pAddThing->GetData("UID").Value);
			SetData(InventoryIndex + 1, GetData(InventoryIndex + 1).Value + 1);
			return TRUE;
		}

		if(GetData(InventoryIndex).Value == pAddThing->GetData("ID").Value)
		{
			SetData(InventoryIndex + 1, GetData(InventoryIndex + 1).Value + 1);
			return TRUE;
		}

		InventoryIndex += 2;
	}
	return FALSE;
}


int Creature::RemoveItem(Thing *pRemoveThing)
{
	//return true if the thing is in the creature's inventory and may be removed
	//cycle through all the inventory slots and if one is empty place the thing in it
	//otherwise return false
	int InventoryIndex = GetIndex("INVENTORY1");

	for(int n = 0; n < NUM_INVENTORY; n++)
	{
		if(GetData(InventoryIndex).Value == pRemoveThing->GetData("ID").Value)
		{
			SetData(InventoryIndex + 1, GetData(InventoryIndex + 1).Value - 1);
			if(GetData(InventoryIndex +1).Value == 0)
			{
				SetData(InventoryIndex, 0);
			}
			return TRUE;
		}

		InventoryIndex += 2;
	}
	return FALSE;
}
*/
int Creature::CreateTexture()
{
	//allocate space for the texture
	//paste in the base skin color
	//examine each equipment slot and paste in the appropriate texture
	//done
	
	//do nothing if we're dealing with a monster.
	if(GetData(INDEX_TYPE).Value)
	{
		return TRUE;
	}

	if(pTexture)
	{
		if(Engine->GetTextureNum(pTexture))
		{
			return TRUE;
		}
	}
	else
	{
		pTexture = PeopleTextures.GetTexture();
	}

	int Age = GetData(INDEX_AGE).Value;
	int RandEquip;
	int n;
	int EquipN;

	BOOL Robed;
	Robed = FALSE;

	HRESULT hr;
	
	if(Age >= 13)
	{
		hr = TempSurf->Blt(NULL,Skins[GetData(INDEX_SEX).Value][GetData(INDEX_SKIN).Value],NULL,DDBLT_WAIT,NULL);
		if(hr != DD_OK)
		{
			Engine->ReportError(hr);
			SafeExit("Bad Skin\n");
		}

		n = FACE;
		RandEquip = GetData(INDEX_MOVEABLITY).Value;
		if(RandEquip)
		{
			RandEquip -= 1;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				SafeExit("bad blit in creature create texture");
			}
		}

		n = BEARD;
		RandEquip = GetData(INDEX_FACIALHAIR).Value;
		if(RandEquip)
		{
			RandEquip -= 1;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				SafeExit("bad blit in creature create texture");
			}
		}
	}
	else
	{
		hr = TempSurf->Blt(NULL,Skins[2][GetData(INDEX_SKIN).Value],NULL,DDBLT_WAIT,NULL);
		if(hr != DD_OK)
		{
			Engine->ReportError(hr);
			SafeExit("bad blit in creature create texture");
		}
	}

	
	n = HAIR;
	RandEquip = GetData(INDEX_HAIR).Value;
	hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
	if(hr != DD_OK)
	{
		Engine->ReportError(hr);
		SafeExit("bad blit in creature create texture");
	}

	GameItem *pGI;

	for(EquipN = MAX_EQUIPMENT - 1; EquipN >= 0; EquipN--)
	{
		pGI = GetEquipment(EquipN);
	if(pGI)
	{
	
		n = HAND;
		RandEquip = pGI->GetData("HANDTEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Hand\n");
				SafeExit("bad blit in creature create texture");
			}
		}

		n = LOWLEG;
		RandEquip = pGI->GetData("LOWERLEGTEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Low Leg\n");
				SafeExit("bad blit in creature create texture");
			}
		}

		n = UPLEG;
		RandEquip = pGI->GetData("UPPERLEGTEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Up Leg\n");
				SafeExit("bad blit in creature create texture");
			}
		}
		n = LOWARM;
		RandEquip = pGI->GetData("LOWERARMTEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Low Arm\n");
				SafeExit("bad blit in creature create texture");
			}
		}

		n = UPARM;
		RandEquip = pGI->GetData("UPPERARMTEXTURE").Value;;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Up Arm\n");
				SafeExit("bad blit in creature create texture");
			}
		}
		n = CHEST;
		RandEquip = pGI->GetData("TORSOTEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Torso\n");
				SafeExit("bad blit in creature create texture");
			}
		}

		n =	FACE;
		RandEquip = pGI->GetData("FACETEXTURE").Value;
		if(RandEquip)
		{
			RandEquip--;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				DEBUG_INFO("Bad Face\n");
				SafeExit("bad blit in creature create texture");
			}
		}

		if(pGI->GetData("CHANGEMESH").Value)
		{
			Robed = TRUE;
		}

	}
	}

/*	if(!GetData("SEX").Value)
	{
		n = HEAD;
		RandEquip = GetData("FACIALHAIR").Value;
		if(RandEquip)
		{
			RandEquip = GetData("HAIR").Value;
			hr = TempSurf->Blt(&BodyDest[n],EquipSource[n],&BodySource[n][RandEquip],DDBLT_WAIT | DDBLT_KEYSRC,NULL);
			
			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
				SafeExit("bad blit in creature create texture");
			}
		}
	}	
*/	

	if(Age < 13)
	{
		pMesh = Engine->GetMesh("child");
	}
	else
	if(Age > 50)
	{
		if(Robed)
		{
			pMesh = Engine->GetMesh("robedoldperson");
		}
		else
		{
			pMesh = Engine->GetMesh("oldperson");
		}
	}
	else
	if(GetData(INDEX_SOLIDITY).Value)
	{
		if(GetData(INDEX_SEX).Value)
		{
			pMesh = Engine->GetMesh("fatwoman");
		}
		else
		{
			pMesh = Engine->GetMesh("fatman");
		}
	}
	else
	if(Robed)
	{
	
		if(GetData(INDEX_SEX).Value)
		{
			pMesh = Engine->GetMesh("robedwoman");
		}
		else
		{
			pMesh = Engine->GetMesh("robedman");
		}
	}
	else
	if(GetData(INDEX_SEX).Value)
	{
		pMesh = Engine->GetMesh("woman");
	}
	else
	{
		pMesh = Engine->GetMesh("man");
	}

	//pTexture = new ZSTexture(TempSurf,Engine->Graphics()->GetD3D(),128,128);
	//hr = pTexture->GetSurface()->Blt(NULL, TempSurf, NULL, DDBLT_WAIT, NULL);
	hr = D3DXLoadTextureFromSurface(Engine->Graphics()->GetD3D(), pTexture->GetSurface(), NULL, TempSurf, NULL,NULL,D3DX_FT_POINT);
	
	if(hr != DD_OK)
	{
		Engine->ReportError(hr);
		DEBUG_INFO("Bad Texture Blt\n");
		SafeExit("bad blit in creature create texture");
	}

	return true;
}

int Creature::ClearSubActions()
{
	if(pCurAction)
	{
		while(pCurAction &&
				pCurAction->GetType() != ACTION_THINK &&
				pCurAction->GetType() != ACTION_USER)
		{
			RemoveMajorAction();
		}
	}	
	return TRUE;
}

ACTION_RESULT_T Creature::Idle()
{
	if(pCurAction->IsStarted())
	{
		return ACTION_RESULT_FINISHED;
	}
	else
	{
		pCurAction->Start();
		
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			InsertAction(ACTION_ANIMATE, NULL, (void *)IDLE_BREATHING, TRUE);
		else
			InsertAction(ACTION_ANIMATE, NULL, (void *)NONCOMBAT_IDLE, TRUE);
	}
	return ACTION_RESULT_ADDED_ACTION;	
}
//end: Mutators ********************************************************



//************ Outputs *************************************************

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************


int LoadBinCreatures(FILE *fp)
{
	Creature::NumCreatures = 0;

	int NewNumCreatures;
	unsigned long CheckSum;
	char blarg[64];

	Creature *pCreature, *pNextCreature;

	assert(fp);

	fread(&NewNumCreatures,sizeof(int),1,fp);
	sprintf(blarg,"NewNumCreatures? = %i", NewNumCreatures);
	DEBUG_INFO(blarg);

	pCreature = new Creature;
	fread(&pCreature->NumFields,sizeof(int),1,fp);
	
	pCreature->DataTypes = Creature::CreatureDataTypes = new DATA_T[pCreature->NumFields];
	fread(pCreature->DataTypes,sizeof(DATA_T),pCreature->NumFields,fp);

	pCreature->LoadFieldNames(fp,Creature::CreatureFieldNames);
	pCreature->EquipHeadIndex = pCreature->GetIndex("HEAD");

	CheckSum = ftell(fp);

	for(int n = 0; n < NewNumCreatures -1; n++)
	{
		pCreature->LoadBin(fp);
		fread(&pCreature->Created,sizeof(BOOL),1,fp);

		Object *pOb;
		pOb = pCreature->GetContents();
		while(pOb)
		{
			((GameItem *)pOb)->SetLocation(LOCATION_PERSON,(Thing *)pCreature);
			pOb = pOb->GetNext();
		}
		fread(&pCreature->NumLocators,sizeof(int),1,fp);
		fread(&pCreature->CurLocator,sizeof(int),1,fp);	
		if(pCreature->NumLocators)
		{
			pCreature->Schedule = new Locator[pCreature->NumLocators];
			for(int ln = 0; ln < pCreature->NumLocators; ln++)
			{
				pCreature->Schedule[ln].Load(fp);
			}
		}

		//save skill info
		//last time a skill improved.  For Now, for party members only
		fread(pCreature->SkillImproved,sizeof(int),MAX_NUM_SKILLS,fp);

		//spells casting stuff
		fread(pCreature->KnownSpells,sizeof(BYTE),MAX_SPELLS,fp);
		fread(&pCreature->ReadySpell,sizeof(BYTE),1,fp);
	
		//equipment
		BOOL bEquipAt;
		for(int en = 0; en < MAX_EQUIPMENT; en++)
		{
			fread(&bEquipAt,sizeof(BOOL),1,fp);
			if(bEquipAt)
			{
				pCreature->Equipment[en] = (GameItem *)LoadObject(fp);
				pCreature->Equipment[en]->SetLocation(LOCATION_EQUIPPED, pCreature);
			}
			else
			{
				pCreature->Equipment[en] = NULL;
			}
		}

		pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);

		pNextCreature = new Creature;
		pNextCreature->SetFieldNames(pCreature->GetFieldNames());
		pNextCreature->DataTypes = pCreature->DataTypes;
		pNextCreature->NumFields = pCreature->NumFields;
		pCreature = pNextCreature;
	}

	pCreature->LoadBin(fp);
	fread(&pCreature->Created,sizeof(BOOL),1,fp);

	fread(&pCreature->NumLocators,sizeof(int),1,fp);
	fread(&pCreature->CurLocator,sizeof(int),1,fp);	
	if(pCreature->NumLocators)
	{
		pCreature->Schedule = new Locator[pCreature->NumLocators];
		for(int ln = 0; ln < pCreature->NumLocators; ln++)
		{
			pCreature->Schedule[ln].Load(fp);
		}
	}
	//save skill info
	//last time a skill improved.  For Now, for party members only
	fread(pCreature->SkillImproved,sizeof(int),MAX_NUM_SKILLS,fp);

	//spells casting stuff
	fread(pCreature->KnownSpells,sizeof(BYTE),MAX_SPELLS,fp);
	fread(&pCreature->ReadySpell,sizeof(BYTE),1,fp);

	//equipment
	BOOL bEquipAt;
	for(int en = 0; en < MAX_EQUIPMENT; en++)
	{
		fread(&bEquipAt,sizeof(BOOL),1,fp);
		if(bEquipAt)
		{
			pCreature->Equipment[en] = (GameItem *)LoadObject(fp);
			pCreature->Equipment[en]->SetLocation(LOCATION_EQUIPPED, pCreature);
		}
		else
		{
			pCreature->Equipment[en] = NULL;
		}
	}

	pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);

	CheckSum = ftell(fp);

	FILE *afp;
	afp = SafeFileOpen("anim.txt","rt");

	for(n = 0; n < NUM_ANIMATIONS; n++)
	{
		Creature::Animations[n].Load(afp);

		DEBUG_INFO("Loaded animation range\n");
	}	
	
	fclose(afp);


	//walkassistant
	pCreature = Creature::GetFirst();
	while(pCreature)
	{
		pCreature->fWalkFrames = Creature::Animations[pCreature->GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->EndFrame - Creature::Animations[pCreature->GetData(INDEX_TYPE).Value % 10].GetAnim(WALK_LEFT)->StartFrame;
		pCreature = (Creature *)pCreature->GetNext();
	}

	DEBUG_INFO("Loaded Binary Creatures\n");

	DEBUG_INFO("Setting Creature Sizes\n");

	pCreature = Creature::GetFirst();
	while(pCreature)
	{
		ZSModelEx *pMesh;
		pMesh = pCreature->GetMesh();
		if(pMesh)
		{
			if(pMesh->GetWidth() * pCreature->GetScale() > 1.6f || pMesh->GetDepth() * pCreature->GetScale() > 1.6f)
			{
				pCreature->Large = TRUE;
			}
			else
			{
				pCreature->Large = FALSE;
			}
		}
		pCreature = (Creature *)pCreature->GetNext();
	}

	sprintf(blarg,"There are %i Creatures\n",Creature::NumCreatures);
	DEBUG_INFO(blarg);

	return TRUE;
}


//set up locations and frames and what not.
void ReImportCreatures(FILE *fp)
{
	char blarg[32];
	DEBUG_INFO("reimporting creatures");
	sprintf(blarg,"%i creatures.\n",Creature::NumCreatures);
	DEBUG_INFO(blarg);

	D3DVECTOR *vParty;
	vParty = PreludeParty.GetLeader()->GetPosition();
	assert(fp);

    Creature *pCreature, *pOldCreature;
	
	pCreature = new Creature;
	
	pCreature->pFirst = (Creature *)pCreature->pNext;
	pCreature->pNext = NULL;
	
	pCreature->SetNumFields(pCreature->GetFirst()->GetNumFields());
	pCreature->LoadFieldNames(fp,Creature::CreatureFieldNames);
	pCreature->DataTypes = pCreature->CreatureDataTypes;

	char c;

	while(TRUE)
	{
		pCreature->SetNumFields(pCreature->GetFirst()->GetNumFields());
		pCreature->DataTypes = pCreature->CreatureDataTypes;
		pCreature->LoadData(fp);

		pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);
		
		if(!pCreature->GetData(INDEX_ID).Value)
		{
			//base speed
			pCreature->SetData(INDEX_ACTIONPOINTS,pCreature->GetData(INDEX_SPEED).Value);
			pCreature->SetData(INDEX_MAXACTIONPOINTS,pCreature->GetData(INDEX_SPEED).Value);
	
			pCreature->SetData(INDEX_HITPOINTS, pCreature->GetData(INDEX_ENDURANCE).Value * 2 + pCreature->GetData("STRENGTH").Value);
			pCreature->SetData(INDEX_MAXHITPOINTS, pCreature->GetData(INDEX_ENDURANCE).Value * 2 + pCreature->GetData("STRENGTH").Value);
		}
		
		pOldCreature = (Creature *)Thing::Find(Creature::GetFirst(), pCreature->GetData(INDEX_NAME).String);

		if(!pOldCreature)
		{
			DEBUG_INFO("adding new creature\n\n");
			DEBUG_INFO(pCreature->GetData(INDEX_NAME).String);
			DEBUG_INFO("\n\n");
			
			pCreature->SetNext(Creature::GetFirst());
			pCreature->SetFirst(pCreature);
		
			c = (char)fgetc(fp);
			
			if(feof(fp) || c == THING_SEPERATION_CHARACTER)
			{
				break;
			}
			else
			{
				fseek(fp,-1,1);
				pCreature = new Creature;
				pCreature->pFirst = (Creature *)pCreature->pNext;
				pCreature->pNext = NULL;
				pCreature->SetFieldNames(Creature::CreatureFieldNames);
			}
		}
		else
		{
			int UID;
			D3DVECTOR pVector;
			DATA_FIELD_T *pOldFields;
			
			while(pOldCreature)
			{
				pOldCreature->ClearActions();
				pOldFields = pOldCreature->DataFields;
				pVector = *pOldCreature->GetPosition();
				UID = pOldCreature->GetData(INDEX_UID).Value;

				pOldCreature->DataFields = new DATA_FIELD_T[pCreature->NumFields];
				*pOldCreature = *pCreature;
				memcpy(pCreature->DataFields[INDEX_POSITION].pVector, &pVector, sizeof(D3DVECTOR));
				pOldCreature->SetData(INDEX_UID, UID);
						
				pOldCreature->SetScale(pCreature->GetScale());

				if(pOldCreature->GetMesh() != pCreature->GetMesh() &&
					pOldCreature->GetData(INDEX_TYPE).Value)
				{
					DEBUG_INFO(pOldCreature->GetData(INDEX_NAME).String);
					DEBUG_INFO(": updating mesh in re-import");
					pOldCreature->SetMesh(pCreature->GetMesh());
				}

				if(pOldCreature->GetTexture() != pCreature->GetTexture() &&
					pOldCreature->GetData(INDEX_TYPE).Value)
				{
					DEBUG_INFO(pOldCreature->GetData(INDEX_NAME).String);
					DEBUG_INFO(": updating texture in re-import");
					pOldCreature->SetTexture(pCreature->GetTexture());
				}

				pOldCreature = (Creature *)Thing::Find((Thing *)pOldCreature->GetNext(),pCreature->GetData(INDEX_NAME).String);
				if(pOldCreature)
				{
					delete[] pOldFields;
					pOldFields = NULL;
				}
			}
			
			pCreature->DataFields = pOldFields;
			delete pCreature;
				
			c = (char)fgetc(fp);
			if(feof(fp) || c == THING_SEPERATION_CHARACTER)
			{
				break;
			}
			else
			{
				fseek(fp,-1,1);
				pCreature = new Creature;
				pCreature->pFirst = (Creature *)pCreature->pNext;
				pCreature->pNext = NULL;	
				pCreature->SetFieldNames(Creature::CreatureFieldNames);
			}
		}
	}
	DEBUG_INFO("Setting Creature Sizes\n");

	pCreature = Creature::GetFirst();
	while(pCreature)
	{
		pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);
		ZSModelEx *pMesh;
		pMesh = pCreature->GetMesh();
		if(pMesh)
		{
			if(pMesh->GetWidth() > 1.6f || pMesh->GetDepth() > 1.6f)
			{
				pCreature->Large = TRUE;
			}
			else
			{
				pCreature->Large = FALSE;
			}
		}
	
		pCreature = (Creature *)pCreature->GetNext();
	}

	DEBUG_INFO("done reimporting creatures");
	sprintf(blarg,"%i creatures.\n",Creature::NumCreatures);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Re-Equipping and re-stating Creatures....");
	pCreature = Creature::GetFirst();
	int numdone = 0;
	while(pCreature)
	{
		pCreature->CreateInventory();
		pCreature->ReEquip();
		pCreature->SetData(INDEX_ACTIONPOINTS,pCreature->GetData(INDEX_MAXACTIONPOINTS).Value);
		if(!pCreature->GetData(INDEX_ACTIONPOINTS).Value)
		{
			DEBUG_INFO("no ap to creature: ");
			DEBUG_INFO(pCreature->GetData(INDEX_NAME).String);
			DEBUG_INFO("\n");
		}

		pCreature->SetData(INDEX_HITPOINTS,pCreature->GetData(INDEX_MAXHITPOINTS).Value);

		pCreature = (Creature *)pCreature->GetNext();
		numdone++;
	}

	sprintf(blarg,"%i equipped creatures.\n",numdone);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Done re-equip\n");

	PreludeParty.Teleport((int)vParty->x,(int)vParty->y);

	DEBUG_INFO("Party re-placed\n");
}

int LoadCreatures(FILE *fp)
{
	Creature *pCreature, *pNextCreature;

	assert(fp);

	pCreature = new Creature;
	
	pCreature->LoadFieldNames(fp,Creature::CreatureFieldNames);
	pCreature->EquipHeadIndex = pCreature->GetIndex("HEAD");

	char c;

	while(TRUE)
	{
		pCreature->LoadData(fp);
		
		pCreature->SetData(INDEX_MAXACTIONPOINTS,pCreature->GetData(INDEX_SPEED).Value);
		pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);

//		pCreature->CreateTexture();
		//if the creature is not a human, assign it's texture

		c = (char)fgetc(fp);
		if(feof(fp) || c == THING_SEPERATION_CHARACTER)
		{
			break;
		}
		else
		{
			fseek(fp,-1,1);
			pNextCreature = new Creature;
			pNextCreature->SetFieldNames(pCreature->GetFieldNames());
			pNextCreature->SetNumFields(pCreature->GetNumFields());
			pCreature = pNextCreature;
		}
	}

	FILE *afp;
	afp = SafeFileOpen("anim.txt","rt");

	for(int n = 0; n < NUM_ANIMATIONS; n++)
	{
		Creature::Animations[n].Load(afp);

		DEBUG_INFO("Loaded animation range\n");
	}	
	
	fclose(afp);

	DEBUG_INFO("Loaded Creatures\n");
	char blarg[64];
	sprintf(blarg,"There are %i Creatures\n",Creature::NumCreatures);
	DEBUG_INFO(blarg);

	return TRUE;
}

int SaveCreatures(FILE *fp)
{
	Creature *pCreature;

	pCreature = Creature::GetFirst();

	pCreature->SaveFieldNames(fp);

	while(pCreature)
	{
		pCreature->SaveData(fp);


		pCreature = (Creature *)pCreature->GetNext();
	}
	DEBUG_INFO("Saved Creatures");
	return TRUE;
}

int SaveBinCreatures(FILE *fp)
{
	unsigned long CheckSum;
	char blarg[64];

	Creature *pCreature;

	pCreature = Creature::GetFirst();

	int OldNumCreatures;
	OldNumCreatures = Creature::NumCreatures - PreludeParty.GetNumMembers();

	fwrite(&OldNumCreatures, sizeof(int),1,fp);
	sprintf(blarg,"NumCreatures? = %i", OldNumCreatures);
	DEBUG_INFO(blarg);

	fwrite(&pCreature->NumFields,sizeof(int),1,fp);
	
	fwrite(pCreature->DataTypes,sizeof(DATA_T),pCreature->NumFields,fp);
	
	pCreature->SaveFieldNames(fp);

	CheckSum = ftell(fp);

	BOOL bTrue = TRUE;
	BOOL bFalse = FALSE;
	while(pCreature)
	{
		if(!PreludeParty.IsMember(pCreature))
		{
			pCreature->SaveBin(fp);
			fwrite(&pCreature->Created,sizeof(BOOL),1,fp);

			fwrite(&pCreature->NumLocators,sizeof(int),1,fp);
			fwrite(&pCreature->CurLocator,sizeof(int),1,fp);	
			for(int n = 0; n < pCreature->NumLocators; n++)
			{
				pCreature->Schedule[n].Save(fp);
			}

			//save skill info
			//last time a skill improved.  For Now, for party members only
			fwrite(pCreature->SkillImproved,sizeof(int),MAX_NUM_SKILLS,fp);

			//spells casting stuff
			fwrite(pCreature->KnownSpells,sizeof(BYTE),MAX_SPELLS,fp);
			fwrite(&pCreature->ReadySpell,sizeof(BYTE),1,fp);

			for(int en = 0; en < MAX_EQUIPMENT; en++)
			{
				if(pCreature->Equipment[en])
				{
					fwrite(&bTrue,sizeof(BOOL),1,fp);
					pCreature->Equipment[en]->Save(fp);
				}
				else
				{
					fwrite(&bFalse,sizeof(BOOL),1,fp);
				}
			}
		
		}
		pCreature = (Creature *)pCreature->GetNext();
	}

	CheckSum = ftell(fp);

	DEBUG_INFO("Saved Bin Creatures");
	return TRUE;
}


int DeleteCreatures()
{
	DEBUG_INFO("\nDeleting Creatures\n");


	Creature *pCreature, *pToDelete;
	char blarg[64];

	
	//get the first creature
	pToDelete = pCreature = Creature::pFirst;

	while(pCreature)
	{
		pToDelete = pCreature;
		pCreature = (Creature *)pCreature->GetNext();
		delete pToDelete;
	}
	//GOT_HERE("Deleted Creatures",0);
	DEBUG_INFO("Creatures remaining: ");
	sprintf(blarg,"%i\n",Creature::NumCreatures);
	DEBUG_INFO(blarg);

	if(Creature::pSBImproveSkill)
	{
		delete Creature::pSBImproveSkill;
		Creature::pSBImproveSkill = NULL;
	}

	Creature::pFirst = NULL;


	return TRUE;
}

Creature *Creature::FindCreature(const char *Name)
{
	Creature *pCreature;
	pCreature = pFirst;

	while(pCreature)
	{
		if(!strcmp(pCreature->GetData(INDEX_NAME).String,Name))
		{
			return pCreature;
		}
		else
		{
			pCreature = (Creature *)pCreature->GetNext();
		}
	}

	//GOT_HERE("unable to find creature name %s",Name);

	return FALSE;
}

Creature *Creature::FindCreature(int UID)
{
	Creature *pCreature;
	pCreature = pFirst;

	while(pCreature)
	{
		if(pCreature->GetData(INDEX_ID).Value == UID)
		{
			return pCreature;
		}
		else
		{
			pCreature = (Creature *)pCreature->GetNext();
		}
	}

//	GOT_HERE("unable to find creature with ID %i",UID);

	return FALSE;
}

void Creature::InitTextures()
{
	DEBUG_INFO("initializing people textures");
	char New[256];
	
	strcpy(New,Engine->GetRootDirectory());

	strcat(New,"\\PeopleTextures\\");

	SetCurrentDirectory(New);

	FILE *fp;
	fp = SafeFileOpen("peopletextures.ini","rt");
	
	char *FileName;

	int n, sn;

	int Width; 
	int Height;

	for(n = 0; n <= CHEST; n++)
	{
		SeekTo(fp,"file:");
		FileName = GetStringNoWhite(fp);
		BodyFileWidth[n]		= GetInt(fp);
	   BodyFileHeight[n]		= GetInt(fp);
		BodyOverLayWidth[n]	= GetInt(fp);
		BodyOverLayHeight[n] = GetInt(fp);
	  
		BodyDest[n].left		= GetInt(fp);
		BodyDest[n].top		= GetInt(fp);
		BodyDest[n].right		= GetInt(fp);
		BodyDest[n].bottom	= GetInt(fp);

		EquipSource[n] = Engine->Graphics()->CreateSurfaceFromFile(FileName, BodyFileWidth[n],BodyFileHeight[n],0,COLOR_KEY_FROM_FILE);
		
		Width		= (BodyFileWidth[n] / BodyOverLayWidth[n]);
		Height	=  (BodyFileHeight[n] / BodyOverLayHeight[n]);

		NumBodyTextures[n] = Width * Height;

		BodySource[n] = new RECT[NumBodyTextures[n]];

		for(sn = 0; sn < NumBodyTextures[n]; sn++)
		{
			BodySource[n][sn].left	 = (sn % Width) * BodyOverLayWidth[n];
			BodySource[n][sn].right	 =	((sn % Width) + 1) * BodyOverLayWidth[n];
			BodySource[n][sn].top	 = (sn / Width) * BodyOverLayHeight[n];
			BodySource[n][sn].bottom =	((sn / Width) + 1) * BodyOverLayHeight[n];
		}

		delete[] FileName;
	}

	char SkinName[32];
	for(n = 0; n < NUM_SKINS; n++)
	{
		sprintf(SkinName,"mskin%i.bmp",n+1);
		Skins[0][n] = Engine->Graphics()->CreateSurfaceFromFile(SkinName,128,128,0,COLOR_KEY_FROM_FILE);
	}

	for(n = 0; n < NUM_SKINS; n++)
	{
		sprintf(SkinName,"fskin%i.bmp",n+1);
		Skins[1][n] = Engine->Graphics()->CreateSurfaceFromFile(SkinName,128,128,0,COLOR_KEY_FROM_FILE);
	}

	for(n = 0; n < NUM_SKINS; n++)
	{
		sprintf(SkinName,"childskin%i.bmp",n+1);
		Skins[2][n] = Engine->Graphics()->CreateSurfaceFromFile(SkinName,128,128,0,COLOR_KEY_FROM_FILE);
	}

	fclose(fp);

	SetCurrentDirectory(Engine->GetRootDirectory());

	DEBUG_INFO(" Done init people textures\n");

	pIntersectBox = Engine->GetMesh("selectbox");

	DEBUG_INFO(" setting up shadows\n");

	ZeroMemory(ShadowVerts, 4 * sizeof(D3DLVERTEX));

	ShadowVerts[0].tu = 0.0f;
	ShadowVerts[0].tv = 0.0f;

	ShadowVerts[1].tu = 1.0f;
	ShadowVerts[1].tv = 0.0f;

	ShadowVerts[2].tu = 0.0f;
	ShadowVerts[2].tv = 1.0f;

	ShadowVerts[3].tu = 1.0f;
	ShadowVerts[3].tv = 1.0f;

	ShadowVerts[0].color = D3DRGBA(1.0f,1.0f,1.0f,0.5f);
	ShadowVerts[1].color = D3DRGBA(1.0f,1.0f,1.0f,0.5f);
	ShadowVerts[2].color = D3DRGBA(1.0f,1.0f,1.0f,0.5f);
	ShadowVerts[3].color = D3DRGBA(1.0f,1.0f,1.0f,0.5f);




}

void Creature::ShowRange()
{
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

	D3DVECTOR At;
	float CircleRadius;
	At = *GetData(INDEX_POSITION).pVector;
	CircleRadius = GetRange() - 0.75f;
	
	float ScaleFactor;
	
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));

	ZSModelEx *SphereMesh;
	SphereMesh = Engine->GetMesh("cylinder");

	ScaleFactor = CircleRadius * 2.0f;;

	At.z += 1.0f;
	
	if(SphereMesh)
	{
		SphereMesh->Draw(Engine->Graphics()->GetD3D(),At.x,At.y,At.z, 0.0f, ScaleFactor, ScaleFactor, 1.0f, 0.0f);
	}
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);

//	Engine->Graphics()->Circle(&At, COLOR_PURPLE, CircleRadius);
//	Engine->Graphics()->Circle(&At, COLOR_PURPLE, CircleRadius);
}

void Creature::DrawEquipment()
{
	if(pEORHand)
	{
		Engine->Graphics()->SetTexture(pEORHand->GetTexture());
		pEORHand->Draw();
	}
	if(pEOLHand)
	{
		Engine->Graphics()->SetTexture(pEOLHand->GetTexture());
		pEOLHand->Draw();
	}
	if(pEOHead)
	{
		Engine->Graphics()->SetTexture(pEOHead->GetTexture());
		pEOHead->Draw();
	}
}

void Creature::Draw()
{
	if(pRegionIn)
	{
		if(!pRegionIn->IsOccupied())
		{
			return;
		}
	}
	else
	{
		if(PreludeParty.Inside())
		{
			return;
		}
	}

	//get a pointer to the mesh indicated by the things mesh field
	D3DVECTOR *pPosition;

	pPosition = GetPosition();

	float Rotation;

	Rotation = GetMyAngle();

	int Frame;

	Frame = GetFrame();

	Engine->Graphics()->SetTexture(pTexture);

	if(pMesh)
	{
		if(Large)
			pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x + 0.5f,pPosition->y + 0.5f, pPosition->z,Rotation,Scale,Scale,Scale,Frame);
		else
			pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x, pPosition->y, pPosition->z,Rotation,Scale,Scale,Scale,Frame);
/*
	//cartoon rendering
	//should write seperate function and call for all objects in a scene to avoid state changes.
	//don't need a texture
		Engine->Graphics()->ClearTexture();
	//set to black outine
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLACK));
	//turn off lighting
	 	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	//reverse culling
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
	//set to draw wireframe
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3WFILL_WIREFRAME);
	//increase line width...Possible?  DX8 only?
		if(Large)
			pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x + 0.5f,pPosition->y + 0.5f, pPosition->z,Rotation,Scale,Scale,Scale,Frame);
		else
			pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x, pPosition->y, pPosition->z,Rotation,Scale,Scale,Scale,Frame);
	//undo our state changes to return everything to normal
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3WFILL_SOLID);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
*/	
	//draw equipment
		DrawEquipment();
	}		


	return;
}

int Creature::DrawData()
{
	D3DVECTOR vBase(0.0f, 0.0f, 0.0f);
	D3DVECTOR vOffset(-0.4f, 0.0f, 0.0f);

	PreludeWorld->ConvertToWorld(&vBase);
	PreludeWorld->ConvertToWorld(&vOffset);
	vOffset = vOffset - vBase;

	float HPRatio;
	float RPRatio;
	float WPRatio;

	HPRatio = (float)(GetData(INDEX_HITPOINTS).Value) / (float)(GetData(INDEX_MAXHITPOINTS).Value);

	D3DVECTOR *pVect;

	pVect = GetData(INDEX_POSITION).pVector;

	float x;
	float y;
	float z;

	x = pVect->x;
	y = pVect->y;
	z = pVect->z + (pMesh->GetHeight() * 1.1f);

	D3DVECTOR vPosition(x,y,z);
	vPosition -= vOffset;
	
	D3DCOLORVALUE White;
	White.r = 1.0f;
	White.g = 1.0f;
	White.b = 1.0f;
	White.a = 0.33f;
	
	D3DCOLORVALUE Red;
	Red.r = 1.0f;
	Red.g = 0.0f;
	Red.b = 0.0f;
	Red.a = 0.6f;
	
	D3DCOLORVALUE Green;
	Green.r = 0.0f;
	Green.g = 1.0f;
	Green.b = 0.0f;
	Green.a = 0.6f;

	D3DCOLORVALUE Blue;
	Blue.r = 0.0f;
	Blue.g = 0.0f;
	Blue.b = 1.0f;
	Blue.a = 0.6f;
	
	Engine->Graphics()->ClearTexture();

	D3DMATERIAL7 TempMat;
	ZeroMemory(&TempMat,sizeof(TempMat));

	float Width = Engine->GetMesh(0)->GetWidth();

	if(HPRatio > 0.0f)
	{
		TempMat.diffuse = Red; 
		TempMat.ambient = Red;
		
		Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

		Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 1.0f, 1.0f, HPRatio, 0);
	}

	TempMat.diffuse = White; 
	TempMat.ambient = White;
	TempMat.power = 1.0f;
	
	Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

	Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 0);

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

	if(!PreludeParty.IsMember(this))
	{
		return TRUE;
	}

	if(GetData(INDEX_MAXRESTPOINTS).Value && GetData(INDEX_POWER_OF_FLAME).Value)
	{
		vPosition += vOffset;
		RPRatio = (float)(GetData(INDEX_RESTPOINTS).Value) / (float)(GetData(INDEX_MAXRESTPOINTS).Value);

		if(RPRatio > 0.0f)
		{
			TempMat.diffuse = Blue; 
			TempMat.ambient = Blue;
			
			Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

			Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 1.0f, 1.0f, RPRatio, 0);
		}

		TempMat.diffuse = White; 
		TempMat.ambient = White;
		TempMat.power = 1.0f;
		
		Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

		Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 0);

		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	}

	if(GetData(INDEX_MAXWILLPOINTS).Value && (GetData(INDEX_THAUMATURGY).Value || GetData(INDEX_GIFTS_OF_GODDESS).Value))
	{
		vPosition += vOffset;
		WPRatio = (float)(GetData(INDEX_WILLPOINTS).Value) / (float)(GetData(INDEX_MAXWILLPOINTS).Value);

		if(WPRatio > 0.0f)
		{
			TempMat.diffuse = Green; 
			TempMat.ambient = Green;
			
			Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

			Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 1.0f, 1.0f, WPRatio, 0);
		}

		TempMat.diffuse = White; 
		TempMat.ambient = White;
		TempMat.power = 1.0f;
		
		Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

		Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 0, 0);

		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	}

	//display action points....
	int MaxAp;
	MaxAp = GetData(INDEX_ACTIONPOINTS).Value;
	if(MaxAp &&
		PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		ZSModelEx *pMesh;
		pMesh = Engine->GetMesh("selectbox");
		vPosition += vOffset;
		for(int n = 0; n < MaxAp; n++)
		{
			pMesh->Draw(Engine->Graphics()->GetD3D(), vPosition.x, vPosition.y, vPosition.z, 1.0f, 0.11f,0.11f,0.09f,0);
			vPosition.z += 0.12f;
		}


	}
	return TRUE;
}

void Creature::OutputDebugInfo()
{
	char ds[128];

	sprintf(ds,"Name: %s\n",GetData(INDEX_NAME).String);
	
	DEBUG_INFO(ds);

	Action *pAction = pCurAction;

	while(pAction)
	{
		pAction->OutputDebugInfo();
		pAction = pAction->GetNext();
	}

	sprintf(ds,"AP: %i\n,",GetData(INDEX_ACTIONPOINTS).Value);

	DEBUG_INFO(ds);
}

void Creature::SetEquipObjects()
{
	//RightHand
	GameItem *pGI;
	Item *pItem = NULL;
	pGI = GetEquipment("RIGHTHAND");
	if(pGI && pGI->GetData("VALUE").Value)
	{
			if(pGI->GetData("NUMBERHANDS").Value > 1 &&
				pGI->GetData("ANIMATIONRANGE").Value == 4)
			{
				if(!pEORHand)
				{
					pEORHand = new EquipObject((Thing *)this, pGI->GetItem(), EQUIP_POS_SCABBARD);				
				//	PreludeWorld->AddMainObject(pEORHand);
				}
				else
				{
					pEORHand->SetItem(pGI->GetItem());
					pEORHand->SetPosition(EQUIP_POS_SCABBARD);
				}
			}
			else
			{
				if(!pEORHand)
				{
					pEORHand = new EquipObject((Thing *)this, pGI->GetItem(), EQUIP_POS_RHAND);
			//		PreludeWorld->AddMainObject(pEORHand);
				}
				else
				{
					pEORHand->SetItem(pGI->GetItem());
					pEORHand->SetPosition(EQUIP_POS_RHAND);
				}
			}
	}
	else
	{
		if(pEORHand)
		{
			PreludeWorld->RemoveMainObject(pEORHand);
			pEORHand = NULL;
		}
	}
//Shield
	pGI = GetEquipment("LEFTHAND");
	if(pGI && pGI->GetData("VALUE").Value)
	{
		pItem = pGI->GetItem();
		if(pItem)
		{
			if(pItem->GetData("TYPE").Value != 2)
			{
				if(!pEOLHand)
				{
					pEOLHand = new EquipObject((Thing *)this, pItem, EQUIP_POS_LEFTARM);
			//		PreludeWorld->AddMainObject(pEOLHand);
				}
				else
				{
					pEOLHand->SetItem(pItem);
					pEOLHand->SetPosition(EQUIP_POS_LEFTARM);
				}
			}
			else
			{
				if(!pEOLHand)
				{
					pEOLHand = new EquipObject((Thing *)this, pItem, EQUIP_POS_LHAND);
			//		PreludeWorld->AddMainObject(pEOLHand);
				}
				else
				{
					pEOLHand->SetItem(pItem);
					pEOLHand->SetPosition(EQUIP_POS_LHAND);
				}
			
			}
		}
	}
	else
	{
		if(pEOLHand)
		{
			PreludeWorld->RemoveMainObject(pEOLHand);
			pEOLHand = NULL;
		}
	}
/*


	//LeftHand
	if(GetData("RIGHTHAND").Value)
	{
		pItem = Thing::FindItem(Item::GetFirst(),GetData("RIGHTHAND").Value);
		if(pItem)
		{
			if(!pEORHand)
			{
				pEORHand = new EquipObject((Thing *)this, pItem, EQUIP_POS_RHAND);
				PreludeWorld->AddMainObject(pEORHand);
			}
			else
			{
				pEORHand->SetItem(pItem);
			}
		}
	}
*/
	//Head
	pGI = GetEquipment("HEAD");
	if(pGI && pGI->GetData("VALUE").Value)
	{
		pItem = pGI->GetItem();
		if(pItem)
		{
			if(!pEOHead)
			{
				pEOHead = new EquipObject((Thing *)this, pItem, EQUIP_POS_HEAD);
		//		PreludeWorld->AddMainObject(pEOHead);
			}
			else
			{
				pEOHead->SetItem(pItem);
				pEOHead->SetPosition(EQUIP_POS_HEAD);
			}
		}
	}
	else
	{
		if(pEOHead)
		{
			PreludeWorld->RemoveMainObject(pEOHead);
			pEOHead = NULL;
		}
	}
/*
	

	//Back
	if(GetData("RIGHTHAND").Value)
	{
		pItem = Thing::FindItem(Item::GetFirst(),GetData("RIGHTHAND").Value);
		if(pItem)
		{
			if(!pEORHand)
			{
				pEORHand = new EquipObject((Thing *)this, pItem, EQUIP_POS_RHAND);
				PreludeWorld->AddMainObject(pEORHand);
			}
			else
			{
				pEORHand->SetItem(pItem);
			}
		}
	}
*/	
}

ACTION_RESULT_T Creature::WaitAction()
{
	if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
	{
		return ACTION_RESULT_FINISHED;
	}

	if(GetData(INDEX_ACTIONPOINTS).Value)
	{
		SetData(INDEX_ACTIONPOINTS, GetData(INDEX_ACTIONPOINTS).Value - 1);
		Creature *pCreature;
		Creature *pNext = NULL;
		Object *pOb;
		int MostAp = 0;
		pOb = PreludeWorld->GetCombat()->GetCombatants();
		while(pOb)
		{
			pCreature = (Creature *)pOb;
			if(pCreature != this && pCreature->GetData(INDEX_ACTIONPOINTS).Value > MostAp)
			{
				MostAp = pCreature->GetData(INDEX_ACTIONPOINTS).Value;
				pNext = pCreature;
			}
			pOb = pOb->GetNextUpdate();
		}

		if(pNext)
		{
			if(MostAp && MostAp <= this->GetData(INDEX_ACTIONPOINTS).Value)
			{
				this->SetData(INDEX_ACTIONPOINTS, MostAp - 1);
			}
		}
		else
		{

		}
		
		char blarg[128];
		sprintf(blarg,"%s waits.", GetData(INDEX_NAME).String);
		Describe(blarg);
	}

	return ACTION_RESULT_FINISHED;
}

int Creature::Equip(GameItem *ToEquip, int NumEquip)
{
	int EquipSlot;
	char *SlotName;
	GameItem *pGI;
	BOOL Secondary = FALSE;
	char blarg[128];

	if(PreludeParty.IsMember(this))
	{

		if(ToEquip->GetData("MINSTR").Value > this->GetData(INDEX_STRENGTH).Value)
		{
			sprintf(blarg, "%s is not strong enough to equip that.",this->GetData(INDEX_NAME).Value);
			Describe(blarg);
			return FALSE;
		}

		if(ToEquip->GetData("MINDEX").Value > this->GetData(INDEX_DEXTERITY).Value)
		{
			sprintf(blarg, "%s is not coordinated enough to equip that.",this->GetData(INDEX_NAME).Value);
			Describe(blarg);
			return FALSE;
		}

		if(PreludeWorld->InCombat())
		{
			if(this->GetData(INDEX_ACTIONPOINTS).Value > 3)
			{
				if(Confirm(ZSWindow::GetMain(),"Spend 3 AP to equip?","Yes","No"))
				{
					SetData(INDEX_ACTIONPOINTS,GetData(INDEX_ACTIONPOINTS).Value - 3);
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				sprintf(blarg, "%s needs 3 AP to equip during combat.",this->GetData(INDEX_NAME).Value);
				Describe(blarg);
				return FALSE;
			}
		}
	
	
	}

	SlotName = ToEquip->GetData("EQUIPLOCATION").String;
	
	if(!strcmp(SlotName,"HAND"))
	{
		if(ToEquip->GetData("NUMBERHANDS").Value == 2)
		{
			pGI = GetEquipment("LEFTHAND");
			if(pGI)
			{
				return FALSE;
			}
			pGI = GetEquipment("RIGHTHAND");
			if(pGI)
			{
				return FALSE;
			}
		}
	
		EquipSlot = GetIndex("RIGHTHAND");
		if(GetData(EquipSlot).Value && ToEquip->GetData("NUMBERHANDS").Value < 2)
		{
			//check out skills;
			if(ToEquip->GetData(INDEX_TYPE).Value == 2)
			{
				int Skill, SkillNeeded;
				SkillNeeded = ToEquip->GetData("SECONDARY").Value;
				if(SkillNeeded && SkillNeeded < 100)
				{
						
					switch(ToEquip->GetData("SUBTYPE").Value)
					{
					case WEAPON_TYPE_SWORD:
						Skill = this->GetData(INDEX_SWORD).Value;
						break;
					case WEAPON_TYPE_DAGGER:
						Skill = this->GetData(INDEX_DAGGER).Value;
						break;
					case WEAPON_TYPE_AXE:
						Skill = this->GetData(INDEX_AXE).Value;
						break;
					case WEAPON_TYPE_BLUNT:
						Skill = this->GetData(INDEX_BLUNT).Value;
						break;
					case WEAPON_TYPE_POLEARM:
						Skill = this->GetData(INDEX_POLEARM).Value;
						break;
					case WEAPON_TYPE_MISSILE:
						Skill = this->GetData(INDEX_MISSILE).Value;
						break;
					default:
					case WEAPON_TYPE_UNARMED:
						Skill = this->GetData(INDEX_UNARMED).Value;
						break;
					case WEAPON_TYPE_THROWN:
						Skill = this->GetData(INDEX_THROWING).Value;
						break;
					}
					if(Skill >= SkillNeeded)
					{
						EquipSlot = GetIndex("LEFTHAND");
						Secondary = TRUE;
					}
					else
					{
						if(PreludeParty.IsMember(this))
						{
							char blarg[64];
							sprintf(blarg, "Requires %i skill to wield in off hand.",SkillNeeded);
							Describe(blarg);
						}
					}
				}
			}
		}
	}
	else
	if(!strcmp(SlotName,"RING"))
	{
		EquipSlot = GetIndex("RIGHTRING");
		if(GetData(EquipSlot).Value)
		{
			EquipSlot = GetIndex("LEFTRING");
		}
	}
	else
	{
		EquipSlot = GetIndex(SlotName);
	}

	if(!strcmp(SlotName,"LEFTHAND"))
	{
		pGI = GetEquipment("RIGHTHAND");
		if(pGI && pGI->GetData("NUMBERHANDS").Value == 2)
		{
	
			return FALSE;
		}
	}

	EquipSlot -= GetIndex("HEAD");

	pGI = GetEquipment(EquipSlot);
	
	if(pGI)
	{
		if(!strcmp(SlotName,"AMMO") && pGI->GetItem()->GetData(INDEX_ID).Value == ToEquip->GetData(INDEX_ID).Value)
		{
			pGI->SetQuantity(pGI->GetQuantity() + ToEquip->GetQuantity());
			return TRUE;
		}
		return FALSE;
	}
	Equipment[EquipSlot] = new GameItem(ToEquip);
	if(NumEquip)
	{
		Equipment[EquipSlot]->SetQuantity(NumEquip);
	}

	Equipment[EquipSlot]->SetLocation(LOCATION_EQUIPPED,this);
	EquipSlot += GetIndex("HEAD");

	SetData(EquipSlot, ToEquip->GetData(INDEX_ID).Value + 1000 * NumEquip);
		
	CreateTexture();
	SetEquipObjects();

	switch(ToEquip->GetData(INDEX_TYPE).Value)
	{
	case 0:
		break;
	case 1:
		break;
	case 2: //weapon
		if(!strcmp(SlotName,"RIGHTHAND"))
		{
			SetData(INDEX_ATTACKPOINTS, GetData(INDEX_ATTACKPOINTS).Value + ToEquip->GetData("SPEED").Value - BASE_ATTACK_POINTS);
			SetData(INDEX_RANGE,1.5f);//ToEquip->GetData("RANGE").fValue);
		}	
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	default:
		break;
	}

	int EOverride;
	EOverride = ToEquip->GetData("EQUIPOVERRIDE").Value;
	if(EOverride)
	{
		Push(this);
		PreludeEvents.RunEvent(EOverride);
	}

	int Index = ToEquip->GetItem()->GetIndex("MOD1");
	int ModAmount;
	for(int n = 0; n < 5; n++)
	{
		ModAmount = ToEquip->GetData(Index).Value;
		if(ModAmount)
		{
			DEBUG_INFO("Modding due to equipment being equipped.\n");
			if(this->GetData(ToEquip->GetData(Index - 1).String).Value >= 1)
				this->SetData(ToEquip->GetData(Index - 1).String, this->GetData(ToEquip->GetData(Index - 1).String).Value + ModAmount);
		}

		Index += 2;
	}

	if(PreludeParty.IsMember(this))
	{
		ZSWindow *pWin;
		pWin = ZSWindow::GetMain()->GetChild(CHARACTER_WINDOW_ID);
		if(pWin)
		{
			CharacterWin *pCharWin;
			pCharWin = (CharacterWin *)pWin;
			pCharWin->ResetStats();
		}
	}
//	SetData("ARMORMIN", GetData("ARMORMIN").Value + ToEquip->GetData("ARMORMIN").Value);
//	SetData("ARMORMAX", GetData("ARMORMAX").Value + ToEquip->GetData("ARMORMAX").Value);
	if(PreludeWorld->InCombat())
	{
		DoneOneAction();
	}
	
	return TRUE;
}

ACTION_RESULT_T Creature::GameCommand()
{
	ACTION_GAME_COMMAND_T Command;
	if(pCurAction->IsStarted())
	{
		return ACTION_RESULT_FINISHED;
	}
	pCurAction->Start();
	Action *pStartAction;
	pStartAction = pCurAction;
	
	Command = (ACTION_GAME_COMMAND_T)((int)pCurAction->GetTarget());
	switch(Command)
	{
	case ACTION_GAME_COMMAND_DISMISS:
		Creature *pToDismiss;
		pToDismiss = (Creature *)pCurAction->GetData();
		Push(pToDismiss);
		PreludeEvents.RunEvent(DISMISS_EVENT);

		ScriptArg *pSA;
		pSA = Pop();

		int Dismissed;
		Dismissed = (int)pSA->GetValue();
		delete pSA;

		if(Dismissed)
		{
			PreludeParty.RemoveMember(pToDismiss);
			if(PreludeParty.GetLeader() == pToDismiss)
			{
				PreludeParty.SetLeader(PreludeParty.GetMember(0));
			}
		}
		break;	

	case ACTION_GAME_COMMAND_REST:

		//open map screen
		ZSRest *pRest;
		pRest = new ZSRest(66666, 100, 100, 600, 400);
		pRest->Show();
		ZSWindow::GetMain()->AddTopChild(pRest);
		ZSWindow::GetMain()->SetFocus(pRest);
		pRest->GoModal();
		pRest->ReleaseFocus();
		ZSWindow::GetMain()->RemoveChild(pRest);
		break;

	case ACTION_GAME_COMMAND_JOURNAL:
		//open journal screen
		if(PreludeParty.GetBest(INDEX_LITERACY_AND_LORE)->GetData(INDEX_LITERACY_AND_LORE).Value)
		{
			JournalWin *pJW;
			pJW = new JournalWin(66666,100,100,600,400);
			pJW->Show();
			ZSWindow::GetMain()->AddTopChild(pJW);

			pJW->SetFocus(pJW);
			pJW->GoModal();
			pJW->ReleaseFocus();

			ZSWindow::GetMain()->RemoveChild(pJW);
		}
		break;
	case ACTION_GAME_COMMAND_CHARACTER:
		CharacterWin *pCWin;
		pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, (Creature *)pCurAction->GetData());
		pCWin->Show();
		ZSWindow::GetMain()->AddTopChild(pCWin);

		pCWin->SetFocus(pCWin);
		pCWin->GoModal();
		pCWin->ReleaseFocus();

		ZSWindow::GetMain()->RemoveChild(pCWin);
		break;
	case ACTION_GAME_COMMAND_SET_LEADER:
		PreludeParty.SetLeader((Creature *)pCurAction->GetData());
		break;
	case ACTION_GAME_COMMAND_MAP:
		//open map screen
		if(PreludeWorld->GetGameState() == GAME_STATE_NORMAL
				&&
				Valley == PreludeWorld->GetArea(0))
		{
			MapWin *pMap;
			pMap = new MapWin;
			pMap->Show();
			ZSWindow::GetMain()->AddTopChild(pMap);
			ZSWindow::GetMain()->SetFocus(pMap);
			pMap->GoModal();
			pMap->ReleaseFocus();
			ZSWindow::GetMain()->RemoveChild(pMap);
		}
		break;
	case ACTION_GAME_COMMAND_OPTIONS:
		//open map screen
		ZSMainOptionsMenu *pOpMenu;
		pOpMenu = new ZSMainOptionsMenu(66666);
		pOpMenu->Show();
		ZSWindow::GetMain()->AddTopChild(pOpMenu);
		ZSWindow::GetMain()->SetFocus(pOpMenu);
		pOpMenu->GoModal();
		pOpMenu->ReleaseFocus();
		ZSWindow::GetMain()->RemoveChild(pOpMenu);
		break;

	default:
		break;
	}
	if(pStartAction == pCurAction)
		return ACTION_RESULT_FINISHED;
	else
		return ACTION_RESULT_ADVANCE_FRAME;
}


int Creature::Unequip(GameItem *ToUnEquip)
{
	int EquipSlot;
	GameItem *pGI;
	char *SlotName;
	SlotName = ToUnEquip->GetData("EQUIPLOCATION").String;

	int n;
	BOOL Found;
	for (n = 0; n < MAX_EQUIPMENT; n++)
	{
		if(this->GetEquipment(n) == ToUnEquip)
		{
			Found = TRUE;
		}
	}
	if (!Found) return TRUE;

	if(!strcmp(SlotName,"HAND"))
	{
		pGI = GetEquipment("RIGHTHAND");
		if(pGI == ToUnEquip)
		{
			EquipSlot = GetIndex("RIGHTHAND");
		}
		else
		{
			EquipSlot = GetIndex("LEFTHAND");
		}
	}
	else
	if(!strcmp(SlotName,"RING"))
	{
		pGI = GetEquipment("RIGHTRING");
		if(pGI == ToUnEquip)
		{
			EquipSlot = GetIndex("RIGHTRING");
		}
		else
		{
			EquipSlot = GetIndex("LEFTRING");
		}
	}
	else
	{
		EquipSlot = GetIndex(SlotName);
	}

	pGI = GetEquipment(EquipSlot - GetIndex("HEAD"));

	if(pGI && pGI == ToUnEquip)
	{
		SetEquipment(EquipSlot - GetIndex("HEAD"), NULL);
		SetData(EquipSlot,0);
	}
	else
	{
		return FALSE;
	}

	CreateTexture();
	SetEquipObjects();

	switch(ToUnEquip->GetData(INDEX_TYPE).Value)
	{
	case 0:
		break;
	case 1:
		break;
	case 2: //weapon
		if(!strcmp(SlotName,"RIGHTHAND"))
		{
			SetData(INDEX_ATTACKPOINTS, GetData(INDEX_ATTACKPOINTS).Value - ToUnEquip->GetData("SPEED").Value + BASE_ATTACK_POINTS);
			SetData(INDEX_RANGE,1.5f);
		}
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	default:
		break;
	}

	int EOverride;
	EOverride = ToUnEquip->GetData("UNEQUIPOVERRIDE").Value;
	if(EOverride)
	{
		Push(this);
		PreludeEvents.RunEvent(EOverride);
	}

	int Index = ToUnEquip->GetItem()->GetIndex("MOD1");
	int ModAmount;
	for(n = 0; n < 5; n++)
	{
		ModAmount = ToUnEquip->GetData(Index).Value;
		if(ModAmount)
		{
			DEBUG_INFO("Modding due to equipment being removed\n");
			if(this->GetData(ToUnEquip->GetData(Index - 1).String).Value - ModAmount >= 1)
				this->SetData(ToUnEquip->GetData(Index - 1).String, this->GetData(ToUnEquip->GetData(Index - 1).String).Value - ModAmount);
			//check for an open character window
		}

		Index += 2;
	}

	ZSWindow *pWin;
	pWin = ZSWindow::GetMain()->GetChild(CHARACTER_WINDOW_ID);
	if(pWin)
	{
		CharacterWin *pCharWin;
		pCharWin = (CharacterWin *)pWin;
		pCharWin->ResetStats();
	}

	if(PreludeWorld->InCombat())
	{
		DoneOneAction();
	}

	return TRUE;
}

int Creature::Has(Thing *ToTest)
{
	Object *pOb;
	GameItem *pGI;
	pOb = GetContents();
	int Total = 0;
	while(pOb)
	{
		pGI = (GameItem *)pOb;
		if(pGI->GetItem() == (Item *)ToTest)
		{
			Total += pGI->GetQuantity();
		}
		pOb = pOb->GetNext();
	}

	
	for(int n = 0; n < MAX_EQUIPMENT; n++)
	{
		pGI = this->Equipment[n];
		if(pGI && pGI->GetItem() == (Item *)ToTest)
		{
			Total += pGI->GetQuantity();
		}
	}
	
	return Total;
/*	int Start;
	int End;
	int n;
	int ItemNum;
	Start = GetIndex("INVENTORY1");
	End = GetIndex("INVENTORY24");
	ItemNum = ToTest->GetData(INDEX_ID).Value;
	int Total = 0;

	for(n = Start; n <= End; n++)
	{
		if((GetData(n).Value % 1000) == ItemNum)
		{
			Total += (GetData(n).Value / 1000);
		}
	}
	return Total;
*/
}

void Creature::ReEquip()
{
	int Index;
	int StartIndex;
	int MaxIndex;
	StartIndex = INDEX_HEAD;
	MaxIndex = INDEX_GLOVES;

	GameItem *pGI;
	Item *pItem;

	for(Index = StartIndex;Index <= MaxIndex; Index++)
	{
		pItem = NULL;

		if(GetData(Index).Value)
		{
			pItem = Item::FindItem(GetData(Index).Value % 1000);
			if(pItem)
			{
				if(!Equipment[Index - StartIndex])
				{
					Equipment[Index - StartIndex] = new GameItem(GetData(Index).Value);
					Equipment[Index - StartIndex]->SetLocation(LOCATION_EQUIPPED, this);
				}
				else
				{
					Equipment[Index - StartIndex]->SetItem(GetData(Index).Value);
					Equipment[Index - StartIndex]->SetLocation(LOCATION_EQUIPPED, this);
				}

				pGI = Equipment[Index - StartIndex];

				if(pGI)
				{
					switch(pGI->GetData(INDEX_TYPE).Value)
					{
					case 0:
						break;
					case 1:
						break;
					case 2: //weapon
						if(Index == INDEX_RIGHTHAND)
						{
							SetData(INDEX_ATTACKPOINTS, GetData(INDEX_ATTACKPOINTS).Value + pGI->GetData("SPEED").Value - BASE_ATTACK_POINTS);
							SetData(INDEX_RANGE, 1.5f);//pGI->GetData("RANGE").fValue);
						}
						break;
					case 3:
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;
					case 8:
						break;
					default:
						break;
					}
				}
			}
		}
		else
		{
			if(Equipment[Index - StartIndex])
			{
				DEBUG_INFO("killing false equipment\n");
				delete Equipment[Index - StartIndex];
			}
			Equipment[Index - StartIndex] = NULL;
		}
	}

}

int Creature::Take(Thing *ToTake, int Quantity)
{
	return RemoveItem((Item *)ToTake,Quantity);
/*

	int Start;
	int End;
	int n;
	int ItemNum;

	int Total;
	Total = Has(ToTake);
	if(Quantity > Total)
	{
		DEBUG_INFO("Attempted to take more than someone had\n");
		return FALSE;
	}

	Total = Quantity;

	Start = GetIndex("INVENTORY1");
	End = GetIndex("INVENTORY24");
	
	ItemNum = ToTake->GetData(INDEX_ID).Value;

	int HereTotal;
	
	for(n = Start; n <= End; n++)
	{
		if((GetData(n).Value % 1000) == ItemNum)
		{
			HereTotal = (GetData(n).Value / 1000);
			if(HereTotal > Total)
			{
				HereTotal -= Total;
				SetData(n,HereTotal * 1000 + ItemNum);
				return TRUE;
			}
			else
			if(HereTotal == Total)
			{
				SetData(n,0);
				return TRUE;
			}
			else
			{
				Total -= HereTotal;
				SetData(n,0);
			}
		}
	}
	DEBUG_INFO("Something wrong in creture take\n");
	SafeExit("Something wrong in creture take\n");
	return FALSE;
*/
}

int Creature::Give(Thing *ToReceive, int Quantity)
{
	if(PreludeParty.IsMember(this) && ToReceive->GetData(INDEX_ID).Value == 100)
	{
		Flag *pFlag;
		pFlag = PreludeFlags.Get("PARTYDRACHS");
		pFlag->Value = (void *) ((int)pFlag->Value + Quantity);
		char blarg[64];
		sprintf(blarg,"The party receives %i drachs.",Quantity);
		Engine->Sound()->PlayEffect(27);
		Describe(blarg);
	}
	else
	{
		AddItem((Item *)ToReceive, Quantity);
	}
	return true;
/*	
	int Start;
	int End;
	int n;
	int ItemNum;

	Start = GetIndex("INVENTORY1");
	End = GetIndex("INVENTORY24");
	
	ItemNum = ToReceive->GetData(INDEX_ID).Value;

	//see if we already have any of that item and if so add to that quantity
	for(n = Start; n <= End; n++)
	{
		if((GetData(n).Value % 1000) == ItemNum)
		{
			int HereTotal = GetData(n).Value / 1000;
			SetData(n, 1000 * (HereTotal + Quantity) + ItemNum);
			return TRUE;
		}
	}

	for(n = Start; n <= End; n++)
	{
		if(!GetData(n).Value)
		{
			SetData(n, 1000 * Quantity + ItemNum);
			return TRUE;
		}
	}
	return FALSE;
*/
}

Creature::operator = (Creature &OtherThing)
{
	for(int n = 0; n < NumFields; n++)
	{
		switch(DataTypes[n])
		{
		case DATA_NONE:
			break;
		case DATA_INT:
			DataFields[n].Value = OtherThing.GetData(n).Value;
			break;
		case DATA_FLOAT:
			DataFields[n].fValue = OtherThing.GetData(n).fValue;
			break;
		case DATA_VECTOR:
			
			DataFields[n].pVector = new D3DVECTOR;
			memcpy(DataFields[n].pVector,OtherThing.GetData(n).pVector,sizeof(D3DVECTOR));
			break;
		case DATA_STRING:
			
			DataFields[n].String = new char[strlen(OtherThing.GetData(n).String) + 1];
			memcpy(DataFields[n].String,OtherThing.GetData(n).String,strlen(OtherThing.GetData(n).String) + 1);
			break;
		}
	}
}

void Creature::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	//uniqueness
	//are we unique?
	int Unique;
	Unique = GetData(INDEX_BATTLEID).Value;
	fwrite(&Unique,sizeof(Unique),1,fp);
	
	//id of creature here
	int ID;
	ID = GetData(INDEX_ID).Value;
	fwrite(&ID,sizeof(ID),1,fp);
	
	if(Unique)
	{
		ID = GetData(INDEX_UID).Value;
		fwrite(&ID, sizeof(ID), 1, fp);
		//and any other unique data associated with this creature
		//well, all data I guess
	}		

	MeshNum = Engine->GetMeshNum(pMesh);
	fwrite(GetPosition(),	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	fwrite(&MeshNum, sizeof(MeshNum),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);

//	fwrite(&Created, sizeof(BOOL),1,fp);
	//save the actions on the current stack
	
}

void Creature::Load(FILE *fp)
{


}

Creature *LoadCreature(FILE *fp)
{
	Creature *pCreature;
	//uniqueness
	//are we unique?
	int Unique;
	fread(&Unique,sizeof(Unique),1,fp);
	
	//id of creature here
	int ID = 0;
	int UID = 0;
	fread(&ID,sizeof(ID),1,fp);
	
	if(Unique)
	{
		fread(&UID, sizeof(UID),1,fp);
		//pCreature = new Creature((Creature *)Thing::Find(Creature::GetFirst(),ID, UID));
		//char blarg[64];
		//sprintf(blarg,"Nonunique: %s\n",pCreature->GetData(INDEX_NAME).String);
//		DEBUG_INFO("Loading Non Unique\n");
	}	
//	else
//	{
		pCreature = (Creature *)Thing::Find(Creature::GetFirst(),ID, UID);
//	}	
	//pCreature->CreateTexture();
	//pCreature->SetEquipObjects();
	//pCreature->InsertAction(ACTION_IDLE,NULL,NULL);
	//pCreature->SetData("ACTIONPOINTS", pCreature->GetData("MAXACTIONPOINTS").Value);
	
if(pCreature)
{
	fread(pCreature->GetPosition(),	sizeof(D3DVECTOR),1,fp);
	fread(&pCreature->Frame, sizeof(pCreature->Frame),1,fp);
	fread(&pCreature->Data, sizeof(pCreature->Data),1,fp);
	fread(&pCreature->Angle, sizeof(pCreature->Angle),1,fp);
	fread(&pCreature->BlockingRadius, sizeof(pCreature->BlockingRadius),1,fp);
	fread(&pCreature->Scale, sizeof(pCreature->Scale),1,fp);
	fread(&pCreature->MeshNum, sizeof(pCreature->MeshNum),1,fp);
	fread(&pCreature->TextureNum, sizeof(pCreature->TextureNum),1,fp);
	pCreature->SetScale(pCreature->GetData(INDEX_SCALE).fValue);
}
else
{
	DEBUG_INFO("ID, UID combo Not Found\n");
	char blarg [32];
	sprintf(blarg, "ID: %i UID: %i\n", ID, UID);
	DEBUG_INFO(blarg);

	pCreature = (Creature *)Thing::Find(Creature::GetFirst(),ID);

	if(!pCreature)
	{
		DEBUG_INFO("Could Not find Creature from dynamic. Attempting re-import\n");

		pCreature = (Creature *)Thing::Find(Creature::GetFirst(),ID);
	}
	if(pCreature)
	{
		Creature *pNew;
		pNew = new Creature(pCreature);
		pNew->SetData(INDEX_UID,UID);

		fread(pNew->GetPosition(),	sizeof(D3DVECTOR),1,fp);
		fread(&pNew->Frame, sizeof(pCreature->Frame),1,fp);
		fread(&pNew->Data, sizeof(pCreature->Data),1,fp);
		fread(&pNew->Angle, sizeof(pCreature->Angle),1,fp);
		fread(&pNew->BlockingRadius, sizeof(pCreature->BlockingRadius),1,fp);
		fread(&pNew->Scale, sizeof(pCreature->Scale),1,fp);
		fread(&pNew->MeshNum, sizeof(pCreature->MeshNum),1,fp);
		fread(&pNew->TextureNum, sizeof(pCreature->TextureNum),1,fp);
		pNew->SetScale(pNew->GetData(INDEX_SCALE).fValue);

		pCreature = pNew;
		
		pCreature->AddLocator();
		Locator *pLoc;
		pLoc = pCreature->GetLocator(0);
		RECT rBounds;
		rBounds.left = pCreature->GetPosition()->x;
		rBounds.right = rBounds.left + 1;
		rBounds.top = pCreature->GetPosition()->y;
		rBounds.bottom = rBounds.top + 1;

		pLoc->SetBounds(&rBounds);
		pLoc->SetStart(0);
		pLoc->SetEnd(23);
	}
	else
	{	
		DEBUG_INFO("Base Not Found\n");
		D3DVECTOR vBlarg;
		int TempInt;
		long TempLong;
		float TempFloat;


		fread(&vBlarg,	sizeof(D3DVECTOR),1,fp);
		fread(&TempInt, sizeof(int),1,fp);
		fread(&TempLong, sizeof(long),1,fp);
		fread(&TempFloat, sizeof(float),1,fp);
		fread(&TempFloat, sizeof(float),1,fp);
		fread(&TempFloat, sizeof(float),1,fp);
		fread(&TempInt, sizeof(int),1,fp);
		fread(&TempInt, sizeof(int),1,fp);
		
		return NULL;
	}
}
	//load inventory/contents;

	return pCreature;
}

void Creature::AddLocator()
{
	NumLocators++;

	Locator *pNewSchedule;
	pNewSchedule = new Locator[NumLocators];

	for(int n = 0; n < NumLocators-1; n++)
	{
		pNewSchedule[n] = Schedule[n];
	}

	if(Schedule)
	{
		delete[] Schedule;
	}

	Schedule = pNewSchedule;
}

void Creature::RemoveLocator(int Num)
{
	if(NumLocators  <= 0 || Schedule == NULL)
	{	
		DEBUG_INFO("Removing Locator w/o shedule\n");
		return;
	}
	DEBUG_INFO("Removing Locator\n");
	Locator *pNewSchedule = NULL;
	NumLocators--;
	if(NumLocators)
	{
		pNewSchedule = new Locator[NumLocators];

		int n;

		for(n = 0; n < Num; n++)
		{
			pNewSchedule[n] = Schedule[n];
		}

		for(n = Num; n < NumLocators; n++)
		{
			pNewSchedule[n] = Schedule[n+1];
		}
	}

	if(Schedule)
		delete[] Schedule;

	Schedule = pNewSchedule;
}

void Creature::RemoveLocator(Locator *pToRemove)
{
	int n;
	for(n = 0; n < NumLocators; n++)
	{
		if(&Schedule[n] == pToRemove)
		{
			RemoveLocator(n);
			return;
		}
	}

	DEBUG_INFO("attempt to remove locator not on list\n");
	return;

}

void Creature::PlaceByLocator()
{
	DEBUG_INFO("placing creatures by time and locator\n");

	Creature *pCreature;
	Locator *pLocator;
	pCreature = GetFirst();
	D3DVECTOR *pPosition;
	int Hour;
	Hour = PreludeWorld->GetHour();
	Area *pArea;
	while(pCreature)
	{
		int newangle;
		newangle = rand() % NORTHWEST + 1;
		pCreature->SetAngle(DIRECTIONANGLES[newangle]);
		if(pCreature->GetNumLocators())
		{	
			pCreature->ClearActions();
			pArea = PreludeWorld->GetArea(pCreature->GetAreaIn());
			if(pArea) pArea->RemoveFromUpdate(pCreature);
			for(int n = 0; n < pCreature->GetNumLocators(); n++)
			{
				pLocator = pCreature->GetLocator(n);
				if((pLocator->GetStart() < pLocator->GetEnd() &&
					 (pLocator->GetStart() <= Hour && pLocator->GetEnd() >= Hour))
					 ||
					(pLocator->GetStart() > pLocator->GetEnd() &&
					 (pLocator->GetStart() <= Hour || pLocator->GetEnd() >= Hour)))
				{
					RECT rLoc;
					pLocator->GetBounds(&rLoc);
					int NewX;
					int NewY;
					if((rLoc.right - rLoc.left) >= 1 || (rLoc.bottom - rLoc.top) >= 1)
					{
						for(int nn = 0; nn < 20; nn++)
						{
							NewX = (rand() % (rLoc.right - rLoc.left)) + rLoc.left;
							NewY = (rand() % (rLoc.bottom - rLoc.top)) + rLoc.top;
							if(PreludeWorld->GetArea(pLocator->GetArea())->IsClear(NewX,NewY))
							{
								break;
							}
						}
					}
					else
					{
						NewX = rLoc.left;
						NewY = rLoc.top;
					}
					pPosition = pCreature->GetPosition();
					pPosition->x = (float)NewX + 0.5f;
					pPosition->y = (float)NewY + 0.5f;
					pPosition->z = PreludeWorld->GetArea(pLocator->GetArea())->GetTileHeight(pPosition->x,pPosition->y);
					PreludeWorld->GetArea(pLocator->GetArea())->AddToUpdate(pCreature);
					pCreature->pRegionIn = PreludeWorld->GetArea(pLocator->GetArea())->GetRegion(pPosition);
					break;
				}
			}
		}
		pCreature = (Creature *)pCreature->GetNext();
	}
	
	DEBUG_INFO("done\n");
}

void Creature::Sort()
{
	Creature *pCreature, *pLast, *pMove, *pNF, *pCPrev;
	pNF = NULL;
	pCPrev = NULL;
	pMove = NULL;
	pCreature = NULL;
	pLast = NULL;

	while(pFirst)
	{
		pCPrev = NULL;
		pLast = NULL;
		pMove = pCreature = pFirst;
		while(pCreature)
		{
			if(strcmp(pCreature->GetData(INDEX_NAME).String, pMove->GetData(INDEX_NAME).String) < 0)
			{
				pCPrev = pLast;
				pMove = pCreature;
			}
			pLast = pCreature;
			pCreature = (Creature *)pCreature->GetNext();
		}
		if(pMove == pFirst)
		{
			pFirst = (Creature *)pFirst->GetNext();
		}
		else
		if(pCPrev)
		{
			pCPrev->SetNext(pMove->GetNext());
		}
		pMove->SetNext(pNF);
		pNF = pMove;
		pLast = NULL;
	}

	pFirst = pNF;

	pCreature = pCreature->GetFirst();
	pCreature->SetPrev(NULL);
	while(pCreature)
	{
		if(pCreature->GetNext())
		{
			((Creature *)pCreature->GetNext())->SetPrev(pCreature);
		}
		pCreature = (Creature *)pCreature->GetNext();
	}

	DEBUG_INFO("Creatures area sorted\n");

}

BOOL Creature::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(!pMesh) 
		return FALSE;

	if(this->pRegionIn != ((Creature *)PreludeWorld->GetActive())->GetRegionIn())
	{
		return FALSE;
	}

if(!PreludeWorld->FullSelect())
{
	//select by base
	int MyX;
	int MyY;
	int RayX;
	int RayY;
	RayX = (int)vRayEnd->x;
	RayY = (int)vRayEnd->y;
	MyX = (int)GetPosition()->x;
	MyY = (int)GetPosition()->y;
	BOOL RetVal;

	if(Large)
	{
		if((RayX == MyX || RayX == MyX + 1)
			&&
			(RayY == MyY || RayY == MyY + 1))
		{
			RetVal = TRUE;
		}
		else
		{
			RetVal =  FALSE;
		}
	}
	else
	{
		if(RayX == MyX && RayY == MyY)
		{
			RetVal =  TRUE;
		}
		else
		{
			RetVal =  FALSE;
		}
	}
	
	if(RetVal && (this->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_UNCONSCIOUS ||
					this->GetData(INDEX_BATTLESTATUS).Value == CREATURE_STATE_DEAD))
	{
		Object *pOb;
		pOb = this->GetNextUpdate();
		while(pOb)
		{
			if(pOb->RayIntersect(vRayStart,vRayEnd))
			{
				return FALSE;
			}
			pOb = pOb->GetNextUpdate();
		}
	}

	return RetVal;
}
	//transform the ray into object coordinates;
	//first check if ray is close enough to matter
	//point to line distance

	float Distance;
	Distance = GetDistance(GetPosition(),vRayEnd);
	
	float Scale;
	Scale = GetScale();

	float InverseHeight;
	float InverseWidth = 1.0f;
	float InverseDepth = 1.0f;

	InverseHeight = 1.0f/pMesh->GetHeight();
	if(Large)
	{	
		InverseWidth = 0.5f;
		InverseDepth = 0.5f;
	}

	//convert the ray to object coordinates
	//first translate then rotate
	D3DXMATRIX matRotate, matScale, matTransform;
	D3DXMatrixRotationZ( &matRotate, -(GetMyAngle() + PI));
	D3DXMatrixScaling(&matScale, InverseWidth, InverseDepth, InverseHeight);
	D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
	D3DXVECTOR4 vTransStart, vTransEnd;
	D3DVECTOR vMoveStart, vMoveEnd, vMoveTemp;
	if(this->IsLarge())
	{
		vMoveTemp = *GetPosition();
		vMoveTemp.x += 0.5f;
		vMoveTemp.y += 0.5f;
		vMoveStart = *vRayStart - vMoveTemp;
		vMoveEnd = *vRayEnd - vMoveTemp;
	}
	else
	{
		vMoveStart = *vRayStart - *GetPosition();
		vMoveEnd = *vRayEnd - *GetPosition();
	}
	D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
	D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
	return pIntersectBox->Intersect(0, (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
}

BOOL Creature::TileIntersect(int TileX, int TileY)
{
	int pX;
	int pY;
	D3DVECTOR *pPosition;
	pPosition = this->GetPosition();
	pX = (int)pPosition->x;
	pY = (int)pPosition->y;

	int Status;
	
	Status = this->GetData(INDEX_BATTLESTATUS).Value;

	if(Status == CREATURE_STATE_UNCONSCIOUS ||
			Status == CREATURE_STATE_DEAD)
	{
		return FALSE;
	}
	

	if(pX == TileX && pY == TileY)
	{
		
		return TRUE;
	}

	if(Large)
	{
		if(pX + 1== TileX && pY == TileY)
		{
			return TRUE;
		}
		if(pX == TileX && pY + 1== TileY)
		{
			return TRUE;
		}
		if(pX + 1 == TileX && pY +1 == TileY)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL Creature::IsMoving()
{
	ACTION_T CurType;
	CurType = pCurAction->GetType();
	if(CurType == ACTION_MOVEIN ||
		CurType == ACTION_MOVETO ||
		CurType == ACTION_MOVEINNOROTATE ||
		CurType == ACTION_FOLLOWPATH ||
		CurType == ACTION_ROTATE)
	{
		return TRUE;
	}

	if(xoff > 0.0001f || xoff < -0.0001f)
		return TRUE;
	if(yoff > 0.0001f || yoff < -0.0001f)
		return TRUE;
	if(angleoff > 0.0001f || angleoff < -0.0001f)
		return TRUE;
	
	return FALSE;

}

void Creature::CreateInventory()
{
	Object *pOb;
	GameItem *pGI;

	while(pContents)
	{
		pOb = GetContents();
		pGI = (GameItem *)pOb;
		pContents = pOb->GetNext();
		delete pGI;	
	}

	int StartIndex;
	int EndIndex;
	StartIndex = GetIndex("INVENTORY1");
	EndIndex = GetIndex("INVENTORY24");

	int n;

	for(n = StartIndex; n <= EndIndex; n++)
	{
		if(GetData(n).Value)
		{
			pGI = new GameItem(GetData(n).Value);
			pGI->SetNext(pContents);
			pGI->SetLocation(LOCATION_PERSON,this);
			pContents = pGI;
		}
	}

//	ReEquip();

}

void Creature::Shadow()
{
	D3DVECTOR *pPosition;
	pPosition = GetPosition();
	
	ShadowVerts[0].z = 
	ShadowVerts[1].z = 
	ShadowVerts[2].z = 
	ShadowVerts[3].z = pPosition->z + 0.01f;

	float MeshWidth = 1.0f;
	float MeshDepth = 1.0f;

	if(pMesh->GetWidth() > 1.0f)
	{
		MeshWidth = pMesh->GetWidth();
	}

	if(pMesh->GetDepth() > 1.0f)
	{
		MeshDepth = pMesh->GetDepth();
	}

	MeshWidth /= 2.0f;
	MeshDepth /= 2.0f;

	ShadowVerts[0].x = pPosition->x - MeshWidth;
	ShadowVerts[0].y = pPosition->y - MeshDepth;
	

	ShadowVerts[1].x = pPosition->x + MeshWidth;
	ShadowVerts[1].y = pPosition->y - MeshWidth;

	ShadowVerts[2].x = pPosition->x - MeshWidth;
	ShadowVerts[2].y = pPosition->y + MeshWidth;

	ShadowVerts[3].x = pPosition->x + MeshWidth;
	ShadowVerts[3].y = pPosition->y + MeshWidth;

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	if(FAILED(Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, ShadowVerts, 4, NULL)))
	{
		SafeExit("problem in drawing shadow.\n");
	}
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
}

ACTION_RESULT_T Creature::Sit()
{
	if(pCurAction->IsStarted())
	{
		return ACTION_RESULT_FINISHED;
	}
	
	pCurAction->Start();

	this->SetFrame(pMesh->GetNumFrames() - 2);

	return ACTION_RESULT_ADVANCE_FRAME;
}

ACTION_RESULT_T Creature::Lay()
{
	if(pCurAction->IsStarted())
	{
		return ACTION_RESULT_FINISHED;
	}
	
	pCurAction->Start();

	this->SetFrame(pMesh->GetNumFrames() - 1);

	D3DVECTOR *pLocation;
	pLocation = GetPosition();


	return ACTION_RESULT_ADVANCE_FRAME;
}

int Creature::GetPowerLevel()
{
	float Damage;
	float AttacksPerRound;
	float SkillFactor;
	float RangeFactor;
	float SurvivalTime;
	float AC;
	Item *Armor = NULL;

	Damage = (float)(GetData(INDEX_MINDAMAGE).Value + GetData(INDEX_MAXDAMAGE).Value) / 2.0f;
	
	AttacksPerRound = (float)(int)(GetData(INDEX_MAXACTIONPOINTS).Value / GetData(INDEX_ATTACKPOINTS).Value);
	if(AttacksPerRound < 1.0f)
	{
		AttacksPerRound = 1.0f;
	}

	SkillFactor = (float)((GetWeaponSkill() + 1) * GetData(INDEX_SPEED).Value * GetData("DEXTERITY").Value) / AVERAGE_SKILL_FACTOR;

	SurvivalTime = (float)GetData(INDEX_HITPOINTS).Value / 8.0f;

	if(GetData(INDEX_CHEST).Value)
	{
		Armor = (Item *)Thing::Find((Thing *)Item::GetFirst(),GetData(INDEX_CHEST).Value % 1000);
	}

	if(Armor)
	{
		AC = (float)(Armor->GetData("ARMORMIN").Value + Armor->GetData("ARMORMAX").Value) / 2.0f;
		SurvivalTime += AC / 2.0f;
	}
	else
	{
		AC = 0.0f;
	}

	if(GetRange() > 1.5f)
	{
		RangeFactor = Damage * ((GetRange() - 1.5f) / 5.0f) * SkillFactor;
	}
	else
	{
		RangeFactor = 0.0f;
	}

	float PowerLevel;

	PowerLevel = (Damage * (AttacksPerRound * SkillFactor)) * SurvivalTime + RangeFactor;

	return (int)PowerLevel;
}

void Creature::FixSchedules()
{
	Creature *pCreature;
	pCreature = GetFirst();
	char blarg[32];
	Locator *pLoc;
	RECT rLoc;
	int temp, n;
	Area *pArea;
	
	FILE *fp;
	fp = fopen("creatures.txt","rt");
	
	while(pCreature)
	{
		if(fp)
		{
			fseek(fp,0,0); //start at beginning
			if(!SeekTo(fp,pCreature->GetData(INDEX_NAME).String))
			{
				Describe("Creature not in Creatures.txt:");
				Describe(pCreature->GetData(INDEX_NAME).String);
			}
		}

		if(!pCreature->GetNumLocators())
		{
			if(pCreature->Schedule)
			{
				pCreature->Schedule = NULL;
				Describe("found schedule w/o locator????");
				Describe(pCreature->GetData(INDEX_NAME).String);
			}
		}

		for(n = 0; n < pCreature->GetNumLocators(); n++)
		{
			pLoc = pCreature->GetLocator(n);

			pLoc->GetBounds(&rLoc);

			if(rLoc.left > rLoc.right)
			{
				temp = rLoc.left;
				rLoc.left = rLoc.right;
				rLoc.right = temp;
				Describe("bad left fixed");
				Describe(pCreature->GetData(INDEX_NAME).String);
				pLoc->SetBounds(&rLoc);
			}

			if(rLoc.left == rLoc.right)
			{
				rLoc.right++;
				Describe("bad left-right equal fixed");
				Describe(pCreature->GetData(INDEX_NAME).String);
				pLoc->SetBounds(&rLoc);
			}

			if(rLoc.top > rLoc.bottom)
			{
				temp = rLoc.top;
				rLoc.top = rLoc.bottom;
				rLoc.bottom = temp;
				Describe("bad top fixed");
				Describe(pCreature->GetData(INDEX_NAME).String);
				pLoc->SetBounds(&rLoc);
			}

			if(rLoc.top == rLoc.bottom)
			{
				rLoc.bottom++;
				Describe("bad top-bottom equal fixed");
				Describe(pCreature->GetData(INDEX_NAME).String);
				pLoc->SetBounds(&rLoc);
			}

			pArea = PreludeWorld->GetArea(pLoc->GetArea());

			if(!pArea)
			{
				Describe(" ");
				Describe("Bad Area!");
				Describe("Removing:");
				Describe(pCreature->GetData(INDEX_NAME).String);
				Describe(" ");
				delete pCreature;
				pCreature = pCreature->GetFirst();
				break;
			}
			else
			{
				if(rLoc.right >= pArea->GetWidth())
				{ 
					rLoc.right = pArea->GetWidth() - 2;
					Describe("bad right fixed");
				}
				if(rLoc.bottom >= pArea->GetHeight())
				{
					rLoc.bottom = pArea->GetHeight() - 2;
					Describe("bad bottom fixed");
				}
			}

			if((rLoc.right - rLoc.left == 1) &&
				(rLoc.bottom - rLoc.top == 1))
			{
				if(pArea && pArea->GetBlocking(rLoc.left,rLoc.top))
				{
					Describe(" ");
					Describe("Bad Blocking on Location:");
					Describe(pCreature->GetData(INDEX_NAME).String);
					sprintf(blarg,"At Time: %i - %i",pLoc->GetStart(),pLoc->GetEnd());
					Describe(blarg);
					Describe(" ");
				}
			}

	
		}

		pCreature = (Creature *)pCreature->GetNext();
	}

}


int Creature::ImproveSkill(int FieldIndex)
{
	if(!PreludeParty.IsMember(this) || this->GetData(FieldIndex).Value >= 99)
		return FALSE;


	//convert the fieldindex to a skill index
	int SkillIndex;
	SkillIndex = FieldIndex - this->GetIndex("SWORD");

	//check to see when it was last improved
	int LastImproved;
	LastImproved = SkillImproved[SkillIndex];

	//check to see whether or not it should improve
	Push(LastImproved);
	Push(FieldIndex);
	Push(this);

	PreludeEvents.RunEvent(91);
//	pSBImproveSkill->Process();

	ScriptArg *pSA;

	pSA = Pop();
	int ImproveIt;
	ImproveIt = (int)pSA->GetValue();

	delete pSA;


	//if so bump up the skill by one point
	if(ImproveIt)
	{
		//return true
		Push(1);
		Push(FieldIndex);
		Push(this);
		CallScript("BumpSkill","getskills.txt");
		char blarg[128];
		
		//should add a sound here
		sprintf(blarg,"%s has improved %s skill.",this->GetData(INDEX_NAME).String,this->GetName(FieldIndex));
		Describe(blarg);

		Engine->Sound()->PlayEffect(29);

		SkillImproved[SkillIndex] = 0;
		return TRUE;
	}
	else
	{
		//else 
		//return false
		SkillImproved[SkillIndex]++;
		return FALSE;
	}
}
int Creature::GetLastImprove(int Index)
{
	int FieldIndex;
	FieldIndex = Index - INDEX_SWORD;
	return SkillImproved[FieldIndex];
}

int Creature::SetLastImprove(int Index, int Amount)
{
	int FieldIndex;
	FieldIndex = Index - INDEX_SWORD;
	SkillImproved[FieldIndex] = Amount;
	return Amount;
}


BOOL Creature::AddSpell(int Num)
{
	assert(Num < MAX_SPELLS);
	switch(PreludeSpells.GetSpell(Num)->GetType())
	{
	case SPELL_FLAME:
		if(!this->GetData(INDEX_POWER_OF_FLAME).Value)
		{
			return FALSE;
		}
		break;
	case SPELL_RIVER:
		if(!this->GetData(INDEX_GIFTS_OF_GODDESS).Value)
		{
			return FALSE;
		}
		break;
	case SPELL_THAUMATURGY:
		if(!this->GetData(INDEX_THAUMATURGY).Value)
		{
			return FALSE;
		}
		break;
	}
	
	this->SetData(INDEX_SPELLBOOK,1);
	KnownSpells[Num] = TRUE;
	return TRUE;
}

//return the distance between two creatures, accounting for creature size
float GetDistance(Object *cA, int ixA, int iyA, Object *cB, int ixB, int iyB)
{
	float xA, yA, xB, yB;
	BOOL ALarge = TRUE;
	BOOL BLarge = TRUE;


	xA = ixA + 0.5f;
	xB = ixB + 0.5f;
	yA = iyA + 0.5f;
	yB = iyB + 0.5f;
	Creature *pCA = NULL;
	Creature *pCB = NULL;
	pCA = (Creature *)cA;
	pCB = (Creature *)cB;

	if(cA->GetObjectType() != OBJECT_CREATURE || !pCA->IsLarge())
	{
		ALarge = FALSE;
	}

	if(cB->GetObjectType() != OBJECT_CREATURE || !pCB->IsLarge())
	{
		BLarge = FALSE;
	}


	D3DVECTOR vA, vB;
	vA.z = 0.0f;
	vB.z = 0.0f;
	
	if(!ALarge && !BLarge)
	{
		vA.x = xA;
		vA.y = yA;
		vB.x = xB;
		vB.y = yB;
		return GetDistance(&vA, &vB);
	}

	if(ALarge && BLarge)
	{	
		if(xA == xB)
		{
			if(yA < yB)
			{
				//N
				vA.x = xA;
				vA.y = yA + 1.0f;
				vB.x = xB;
				vB.y = yB;
				return GetDistance(&vA, &vB);
			}
			else
			{
				//S
				vA.x = xA;
				vA.y = yA;
				vB.x = xB;
				vB.y = yB + 1.0f;
				return GetDistance(&vA, &vB);
			}
		}
		else
		if(yA == yB)
		{
			if(xA < xB)
			{
				//W
				vA.x = xA + 1.0f;
				vA.y = yA;
				vB.x = xB;
				vB.y = yB;
				return GetDistance(&vA, &vB);
			}
			else
			{
				//E
				vA.x = xA;
				vA.y = yA;
				vB.x = xB + 1.0f;
				vB.y = yB;
				return GetDistance(&vA, &vB);
			}
		}
		else
		if(xA < xB)
		{
			if(yA < yB)
			{
				//NW
				vA.x = xA + 1.0f;
				vA.y = yA + 1.0f;
				vB.x = xB;
				vB.y = yB;
				return GetDistance(&vA, &vB);
			}
			else
			{
				//SW
				vA.x = xA + 1.0f;
				vA.y = yA;
				vB.x = xB;
				vB.y = yB + 1.0f;
				return GetDistance(&vA, &vB);
			}
		}
		else
		{
			if(yA < yB)
			{
				//NE
				vA.x = xA;
				vA.y = yA + 1.0f;
				vB.x = xB + 1.0f;
				vB.y = yB;
				return GetDistance(&vA, &vB);
			}
			else
			{
				//SE
				vA.x = xA;
				vA.y = yA;
				vB.x = xB+1;
				vB.y = yB+1;
				return GetDistance(&vA, &vB);

			}
		}
	}

	Object *pLarge;
	Object *pSmall;
	float xS, yS, xL, yL;
	if(ALarge)
	{
		pLarge = cA;
		xL = xA;
		yL = yA;
		pSmall = cB;
		xS = xB;
		yS = yB;
	}
	else
	{
		pLarge = cB;
		xL = xB;
		yL = yB;
		pSmall = cA;
		xS = xA;
		yS = yA;
	}

	if(xS == xL)
	{
		if(yS < yL)
		{
			//N
			vA.x = xS;
			vA.y = yS;
			vB.x = xL;
			vB.y = yL;
			return GetDistance(&vA, &vB);
		}
		else
		{
			//S
			vA.x = xS;
			vA.y = yS;
			vB.x = xL;
			vB.y = yL + 1.0f;
			return GetDistance(&vA, &vB);
		}
	}
	else
	if(yS == yL)
	{
		if(xS < xL)
		{
			//W
			vA.x = xS;
			vA.y = yS;
			vB.x = xL;
			vB.y = yL;
			return GetDistance(&vA, &vB);
		}
		else
		{
			//E
			vA.x = xS;
			vA.y = yS;
			vB.x = xL + 1.0f;
			vB.y = yL;
			return GetDistance(&vA, &vB);
		}
	}
	else
	if(xS < xL)
	{
		if(yS < yL)
		{
			//NW
			vA.x = xS;
			vA.y = yS;
			vB.x = xL;
			vB.y = yL;
			return GetDistance(&vA, &vB);
		}
		else
		{
			//SW
			vA.x = xS;
			vA.y = yS;
			vB.x = xL;
			vB.y = yL + 1.0f;
			return GetDistance(&vA, &vB);
		}
	}
	else
	{
		if(yS < yL)
		{
			//NE
			vA.x = xS;
			vA.y = yS;
			vB.x = xL + 1.0f;
			vB.y = yL;
			return GetDistance(&vA, &vB);
		}
		else
		{
			//SE
			vA.x = xS;
			vA.y = yS;
			vB.x = xL + 1.0f;
			vB.y = yL + 1.0f;
			return GetDistance(&vA, &vB);

		}
	}
	DEBUG_INFO("Should never get here!\n");
	exit(999);
	return 0.0f;
}

float GetDistance(Creature *cA, Creature *cB)
{
	return GetDistance(cA, cA->GetPosition()->x, cA->GetPosition()->y, cB, cB->GetPosition()->x, cB->GetPosition()->y);
}

void Creature::FixIDs()
{
	Creature *pCreature, *pSearchCreature;

	pCreature = Creature::GetFirst();
	int UID, ID;

	while(pCreature)
	{
		if(pCreature->GetData(INDEX_BATTLEID).Value)
		{
			ID = pCreature->GetData(INDEX_ID).Value;
			UID = pCreature->GetData(INDEX_UID).Value;
			pSearchCreature = Creature::GetFirst();
			while(pSearchCreature)
			{
				if(pSearchCreature->GetData(INDEX_ID).Value == ID)
				{
					if(pSearchCreature != pCreature && pSearchCreature->GetData(INDEX_UID).Value == UID)
					{
					   Describe("duplicate UID found");
					   pSearchCreature->SetData(INDEX_UID, UID + 1);
					}
				}
				pSearchCreature = (Creature *)pSearchCreature->GetNext();
			}
		}
		pCreature = (Creature *)pCreature->GetNext();
	}
}

BOOL Creature::EquipBestWeapon()
{
	GameItem *pGI = NULL;

	GameItem *pWeapon =  NULL;
	GameItem *pBestWeapon = NULL;
	BOOL HasAmmo;
	int Skill, BestSkill;
	Skill = 0, BestSkill = 0;

	pGI = this->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		this->Unequip(pGI);
		this->AddItem(pGI);
	}
	
	pWeapon = (GameItem *)this->GetContents();
	while(pWeapon)
	{
		if(pWeapon->GetData(INDEX_TYPE).Value == ITEM_TYPE_WEAPON)
		{
			//if it's a missile weapon check to see if we have ammo for it.
			if(pWeapon->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE
				&&
				pWeapon->GetData("AMMOTYPE").Value)
			{
				HasAmmo = FALSE;
				pGI = this->GetEquipment("AMMO");
				if(pGI && pGI->GetData(INDEX_TYPE).Value == ITEM_TYPE_AMMO &&
						pGI->GetData("AMMOTYPE").Value == pWeapon->GetData("AMMOTYPE").Value)
				{
					HasAmmo = TRUE;
				}
			
				pGI = (GameItem *)this->GetContents();
				while(pGI && !HasAmmo)
				{
					if(pGI->GetData(INDEX_TYPE).Value == ITEM_TYPE_AMMO &&
						pGI->GetData("AMMOTYPE").Value == pWeapon->GetData("AMMOTYPE").Value)
					{
						HasAmmo = TRUE;
					}
					pGI = (GameItem *)pGI->GetNext();
				}
			}
			else
			{
				HasAmmo = TRUE;
			}

			if(HasAmmo) //hasammo automatically true for non missile items.
			{
				Skill = this->GetData(INDEX_WEAPONSKILL).Value + this->GetData(INDEX_SWORD + pWeapon->GetData("SUBTYPE").Value).Value; 
				if(Skill >= BestSkill)
				{
					BestSkill = Skill;
					pBestWeapon = pWeapon;
				}
			}
		}
		
		pWeapon = (GameItem *)pWeapon->GetNext();
	}

	if(pBestWeapon)
	{
		GameItem *pNGI;
		pNGI = new GameItem(pBestWeapon);
		this->RemoveItem(pBestWeapon);
		this->Equip(pNGI);
		this->Reload();
		return TRUE;
	}
	this->SetEquipObjects();
	return FALSE;
}



BOOL Creature::Reload()
{
	GameItem *pGI;
	pGI = this->GetEquipment("RIGHTHAND");
	BOOL FoundAmmo = FALSE;
	if(pGI)
	{
		if(pGI->GetItem()->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
		{
			//check for ammo
			int AmmoType;
			AmmoType = pGI->GetItem()->GetData("AMMOTYPE").Value;
			GameItem *pAmmo;
			//I'm out of ammo
			pAmmo = this->GetEquipment("AMMO");
			if(pAmmo && pAmmo->GetData("AMMOTYPE").Value == AmmoType)
			{
				//I already have ammo for this weapon equipped
				DEBUG_INFO("Weapon already loaded");
				return TRUE;
			}
			//check to see if I have more
			pGI = (GameItem *)this->GetContents();
			while(pGI)
			{
				if(pGI->GetData(INDEX_TYPE).Value == ITEM_TYPE_AMMO &&
					pGI->GetData("AMMOTYPE").Value == AmmoType)
				{
					this->Equip(pGI);
					this->RemoveItem(pGI);
					FoundAmmo = TRUE;
					break;
				}
				pGI = (GameItem *)pGI->GetNext();
			}
		}
		else
		{
			DEBUG_INFO("Reloading non missile weapon");
			return TRUE;
		}

	}

	return FoundAmmo;
}

WeaponTracer *Creature::CreateWeaponParticleSystem(Creature *pAttacker, COLOR_T Color)
{	
	WeaponTracer *pSys = NULL;

	pSys = new WeaponTracer;

	pSys->pLink = pEORHand;
	pSys->Material = *Engine->Graphics()->GetMaterial(Color);

	return pSys;
}

BOOL Creature::HasAmmo()
{
	GameItem *pGI;
	pGI = this->GetEquipment("RIGHTHAND");

	if(pGI && pGI->GetData("SUBTYPE").Value == WEAPON_TYPE_MISSILE)
	{	//check for ammo
		int AmmoType;
		AmmoType = pGI->GetItem()->GetData("AMMOTYPE").Value;
		GameItem *pAmmo;
		
		pAmmo = this->GetEquipment("AMMO");
		if(AmmoType)
		{
			if(!pAmmo || pAmmo->GetItem()->GetData("AMMOTYPE").Value != AmmoType)
			{
				return FALSE;
			}
		}
	}


	return TRUE;
}

int Creature::GetWeaponSkill()
{
	int BaseSkill;
	BaseSkill = this->GetData(INDEX_WEAPONSKILL).Value;

	int WeaponType;
	GameItem *pGI;
	pGI = this->GetEquipment("RIGHTHAND");

	if(pGI)
	{
		WeaponType = pGI->GetData("SUBTYPE").Value;
		switch(WeaponType)
		{
		case WEAPON_TYPE_SWORD:
			BaseSkill += this->GetData(INDEX_SWORD).Value;
			break;
		case WEAPON_TYPE_AXE:
			BaseSkill += this->GetData(INDEX_AXE).Value;
			break;
		case WEAPON_TYPE_BLUNT:
			BaseSkill += this->GetData(INDEX_BLUNT).Value;
			break;
		case WEAPON_TYPE_DAGGER:
			BaseSkill += this->GetData(INDEX_DAGGER).Value;
			break;
		case WEAPON_TYPE_POLEARM:
			BaseSkill += this->GetData(INDEX_POLEARM).Value;
			break;
		case WEAPON_TYPE_THROWN:
			BaseSkill += this->GetData(INDEX_THROWING).Value;
			break;
		case WEAPON_TYPE_MISSILE:
			BaseSkill += this->GetData(INDEX_MISSILE).Value;
			break;
		case WEAPON_TYPE_UNARMED:
		default:
			BaseSkill += this->GetData(INDEX_UNARMED).Value;	
			break;
		}
	}
	else
	{
		BaseSkill += this->GetData(INDEX_UNARMED).Value;
	}

	return BaseSkill;

}

int Creature::GetMinSpellAp()
{
	int CurAP;
	int MinAP;

	MinAP = 50;

	int sn;
	
	sn = 0;
	int GoddessSkill;
	GoddessSkill = this->GetData(INDEX_GIFTS_OF_GODDESS).Value;
	int FlameSkill;
	FlameSkill = this->GetData(INDEX_POWER_OF_FLAME).Value;

	for(sn = 0; sn < MAX_SPELLS; sn++)
	{
		if(KnownSpells[sn])
		{
			for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
			{
				if((PreludeSpells.GetSpell(sn)->GetType() == SPELL_RIVER && GoddessSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					(PreludeSpells.GetSpell(sn)->GetType() == SPELL_FLAME && FlameSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					PreludeSpells.GetSpell(sn)->GetType() == SPELL_THAUMATURGY)
				{
					CurAP = PreludeSpells.GetSpell(sn)->GetLevel(n)->SpeedCost;
					if(CurAP < MinAP)
					{
						MinAP = CurAP;
					}
				}
			}
		}
	}

	return MinAP;
}

int Creature::GetMinSpellRange()
{
	int CurRange;
	int MinRange;

	MinRange = 100;

	int sn;
	
	sn = 0;
	int GoddessSkill;
	GoddessSkill = this->GetData(INDEX_GIFTS_OF_GODDESS).Value;
	int FlameSkill;
	FlameSkill = this->GetData(INDEX_POWER_OF_FLAME).Value;

	for(sn = 0; sn < MAX_SPELLS; sn++)
	{
		if(KnownSpells[sn])
		{
			for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
			{
				if((PreludeSpells.GetSpell(sn)->GetType() == SPELL_RIVER && GoddessSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					(PreludeSpells.GetSpell(sn)->GetType() == SPELL_FLAME && FlameSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					PreludeSpells.GetSpell(sn)->GetType() == SPELL_THAUMATURGY)
				{
					CurRange = PreludeSpells.GetSpell(sn)->GetLevel(n)->MinRange;
					if(CurRange < MinRange)
					{
						MinRange = CurRange;
					}
				}
			}
		}
	}

	return MinRange;

}

int Creature::GetMaxSpellRange()
{
	int CurRange;
	int MaxRange;

	MaxRange = 0;

	int sn;
	
	sn = 0;
	int GoddessSkill;
	GoddessSkill = this->GetData(INDEX_GIFTS_OF_GODDESS).Value;
	int FlameSkill;
	FlameSkill = this->GetData(INDEX_POWER_OF_FLAME).Value;

	for(sn = 0; sn < MAX_SPELLS; sn++)
	{
		if(KnownSpells[sn])
		{
			for(int n = 0; n < PreludeSpells.GetSpell(sn)->GetNumLevels(); n++)
			{
				if((PreludeSpells.GetSpell(sn)->GetType() == SPELL_RIVER && GoddessSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					(PreludeSpells.GetSpell(sn)->GetType() == SPELL_FLAME && FlameSkill >= PreludeSpells.GetSpell(sn)->GetLevel(n)->SkillReq) ||
					PreludeSpells.GetSpell(sn)->GetType() == SPELL_THAUMATURGY)
				{
					CurRange = PreludeSpells.GetSpell(sn)->GetLevel(n)->MaxRange;
					if(CurRange > MaxRange)
					{
						MaxRange = CurRange;
					}
				}
			}
		}
	}

	return MaxRange;

}

void Creature::OutputSitStand()
{
	Creature *pCreature;
	pCreature = GetFirst();

	DEBUG_INFO("begin sit\n\n");
	
	while(pCreature)
	{
		Locator *pLoc;
		for(int n = 0; n < pCreature->GetNumLocators(); n++)
		{
			pLoc = pCreature->GetLocator(n);
			if(pLoc->GetState() == LOC_STATE_SIT)
			{
				DEBUG_INFO(pCreature->GetData(INDEX_NAME).String);
				DEBUG_INFO("\n");
				break;
			}
		}
		pCreature = (Creature *)pCreature->GetNext();

	}

	DEBUG_INFO("\n end sit \n\n");

	DEBUG_INFO("begin lay\n\n");
	
	while(pCreature)
	{
		Locator *pLoc;
		for(int n = 0; n < pCreature->GetNumLocators(); n++)
		{
			pLoc = pCreature->GetLocator(n);
			if(pLoc->GetState() == LOC_STATE_LAY)
			{
				DEBUG_INFO(pCreature->GetData(INDEX_NAME).String);
				DEBUG_INFO("\n");
				break;
			}
		}
		pCreature = (Creature *)pCreature->GetNext();

	}

	DEBUG_INFO("\n end lay \n\n");
}

int Creature::GetDamage(BOOL OffHand, BOOL Average)
{
	GameItem *pGI;

	if(OffHand)
	{
		pGI = GetEquipment("LEFTHAND");
	}
	else
	{
		pGI = GetEquipment("RIGHTHAND");
	}

	int MinDamage;
	int MaxDamage;
	int DamageRange;
	
	if(pGI)
	{
		MinDamage = pGI->GetData("MINDAMAGE").Value;
		MaxDamage = pGI->GetData("MAXDAMAGE").Value;
		DamageRange = (MaxDamage - MinDamage) + 1;
	}
	else
	{
		MinDamage = GetData(INDEX_MINDAMAGE).Value;
		MaxDamage = GetData(INDEX_MAXDAMAGE).Value;
		DamageRange = (MaxDamage - MinDamage) + 1;
		if(!GetData(INDEX_TYPE).Value)
		{
			MinDamage += GetData(INDEX_UNARMED).Value / 6;
		}
	}
	int Damage;
	if(Average)
	{
		Damage = MinDamage + (DamageRange/2);
	}
	else
	{
		Damage = MinDamage + (rand() % DamageRange);
	}

	//easy
	if(PreludeWorld->GetDifficulty() == 0)
	{
		if(!PreludeParty.IsMember(this))
		{
			Damage = Damage * 85;
			Damage = Damage / 100;
		}
		else
		{
			Damage = Damage * 115;
			Damage = Damage / 100;
		}

	}

	//hard
	if(PreludeWorld->GetDifficulty() == 2)
	{
		if(!PreludeParty.IsMember(this))
		{
			Damage = Damage * 115;
			Damage = Damage / 100;
		}
		else
		{
			Damage = Damage * 85;
			Damage = Damage / 100;
		}
	}

	return Damage;
}

float Creature::GetRange()
{
	GameItem *pGI;

	pGI = this->GetEquipment("RIGHTHAND");

	if(pGI)
	{
		return pGI->GetData("RANGE").fValue;
	}
	else
	{
		return this->GetData(INDEX_RANGE).fValue;
	}
}

void Creature::DoneOneAction()
{
	GameItem *pGI;
	pGI = this->GetEquipment("RIGHTHAND");
	if(pGI)
	{
		SetData(INDEX_ATTACKPOINTS, pGI->GetData("SPEED").Value);
	}
	else
	{
		SetData(INDEX_ATTACKPOINTS, BASE_ATTACK_POINTS);
	}
}

