//********************************************************************* 
//*                                                                                                                                    ** 
//**************				Creatures.h          ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/20                                                                                          * 
//*Revisor:     mat williams                                          * 
//*Purpose:     defines class for handling items in Prelude              * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*														              * 
//********************************************************************* 
//*********************************************************************
#ifndef CREATURES_H
#define CREATURES_H

#include "things.h"		//case thing class
#include "actions.h"	//action definition
#include "zstexture.h"
#include "equipobject.h"
#include "animrange.h"
#include "locator.h"
#include "texturemanager.h"

typedef enum
{
	CREATURE_STATE_NONE,
	CREATURE_STATE_UNCONSCIOUS,
	CREATURE_STATE_DEAD,
} CREATURE_STATE_T;

typedef enum
{
	BODY_NONE = -1,
	HAND = 0,	
	LOWLEG,	
	UPLEG,		
	HAIR,		
	LOWARM,	
	HEAD,		
	UPARM,
	BEARD,
	FACE,
	CHEST,		
} EQUIP_LOCATION_T;

#define NUM_SKINS			4
#define NUM_INVENTORY	24
#define NUM_ANIMATIONS	9
#define MAX_NUM_SKILLS	24
#define MAX_EQUIPMENT	16

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**						Item                                  ** 
//******************************************************************** 
//*Purpose:     class Item provides functionality for items in prelude
//* 
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************

class ZSPortrait;
class ScriptBlock;
class Region;
class WeaponTracer;


class Creature : public Thing
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 

	static ScriptBlock *pSBImproveSkill;

	static Creature *pFirst;
    static char CreatureFieldNames[160*32];
    static int NumCreatures;
	static AnimRange Animations[12];
	static DATA_T *CreatureDataTypes;

	//static members used for creating person textures
	static RECT BodyDest[CHEST + 1];
	static int  BodyOverLayWidth[CHEST + 1];
	static int  BodyOverLayHeight[CHEST + 1];
	static int  BodyFileWidth[CHEST + 1];
	static int  BodyFileHeight[CHEST + 1];
	static LPDIRECTDRAWSURFACE7 EquipSource[CHEST + 1];
	static RECT *BodySource[CHEST + 1];
	static int  NumBodyTextures[CHEST + 1];
	static LPDIRECTDRAWSURFACE7 Skins[3][NUM_SKINS];
	static ZSModelEx *pIntersectBox;
	static D3DLVERTEX ShadowVerts[4];
	static int EquipHeadIndex;
	static TextureGroup PeopleTextures;

	//a portrait window for the character
	ZSPortrait *pPortrait;

	//pointers to equipped object
	EquipObject *pEORHand;
	EquipObject *pEOLHand;
	EquipObject *pEOHead;
	EquipObject *pEOBack;

	//current action
	Action *pCurAction;
	ACTION_RESULT_T LastResult;

	//number of actions in the queue
	int ActionCount;
	BOOL Active;

	//keep track of the last time we did various things
	//generally to prevent too much pathing checks
	ULONG LastChecked;
		
	//last time a skill improved.  For Now, for party members only
	int SkillImproved[MAX_NUM_SKILLS];
	
	//spells casting stuff
	BYTE KnownSpells[MAX_SPELLS];
	char ReadySpell;

	//oversized (2x) or not
	BOOL Large;
	int FrameAdd;
	
	//private update functions
	ACTION_RESULT_T None();
	ACTION_RESULT_T Goto();
	ACTION_RESULT_T MoveTo();
	ACTION_RESULT_T FollowPath();
	ACTION_RESULT_T MoveIn();
	ACTION_RESULT_T MoveNoRotate();
	ACTION_RESULT_T Rotate();
	ACTION_RESULT_T Attack();
	ACTION_RESULT_T Kill();
	ACTION_RESULT_T Animate();
	ACTION_RESULT_T TalkTo();
	ACTION_RESULT_T Say();
	ACTION_RESULT_T Take();
	ACTION_RESULT_T Drop();
	ACTION_RESULT_T Give();
	ACTION_RESULT_T PickUp();
	ACTION_RESULT_T Die();
	ACTION_RESULT_T ReactTo();
	ACTION_RESULT_T Defend();
	ACTION_RESULT_T Script();
	ACTION_RESULT_T Think();
	ACTION_RESULT_T User();
	ACTION_RESULT_T UseItem();
	ACTION_RESULT_T Approach();
	ACTION_RESULT_T WaitUntil();
	ACTION_RESULT_T WaitForSight();
	ACTION_RESULT_T WaitForHear();
	ACTION_RESULT_T WaitForClear();
	ACTION_RESULT_T UseThing();
	ACTION_RESULT_T Unconscious();
	ACTION_RESULT_T LookAt();
	ACTION_RESULT_T WaitForOtherCreature();
	ACTION_RESULT_T WaitMissile();
	ACTION_RESULT_T Idle();
	ACTION_RESULT_T Cast();
	ACTION_RESULT_T WaitAction();
	ACTION_RESULT_T Leap();
	ACTION_RESULT_T Open();
	ACTION_RESULT_T Close();
	ACTION_RESULT_T Use();
	ACTION_RESULT_T GameCommand();
	ACTION_RESULT_T Formation();
    ACTION_RESULT_T PickPocket();
	ACTION_RESULT_T Sit();
	ACTION_RESULT_T Lay();
	ACTION_RESULT_T SecondaryAttack();
	ACTION_RESULT_T SpecialAttack();
	
	void GetValidSelfCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidSelfNonCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidFriendCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidFriendNonCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidPartyNonCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidPartyCombatActions(Object *pActor, ActionMenu *pAMenu);
	void GetValidEnemyCombatActions(Object *pActor, ActionMenu *pAMenu);

	//loads and initializes person texturing
	void InitTextures();

	//animation per frame offsets
	float xoff;
	float yoff;
	float zoff;
	float angleoff;

	//location information
	int NumLocators;
	int CurLocator;
	Locator *Schedule;
	int AreaIn;
	
	Creature *pPrev;

	Region *pRegionIn;

	//equipment stuff.  Necessary to make items unique.  Blarg
	GameItem *Equipment[MAX_EQUIPMENT];

	//walk divisor
	float fWalkFrames;

	//equipbestweapon
	BOOL EquipBestWeapon();
	BOOL Reload();

	//player created
	BOOL Created;

	//acted yet this round
	BOOL Acted;

	//attack damage override 
	int DamageOverride;
	int SecondaryDamageOverride;
	int AmmoItemNumber;
	
	//last time manually placed
	unsigned long LastPlacedTime;

	//last target location
	D3DVECTOR vLastTarget;

//************************************************************************************** 

public:

// Display Functions --------------------------------
	void Draw();
	void DrawEquipment();
	int DrawData();
	void ShowRange();

// Accessors ----------------------------------------
	D3DVECTOR GetLastTarget() { return vLastTarget; };

	unsigned long GetLastPlacedTime() { return LastPlacedTime; }

	void SetAmmoItemNumber(int n) { AmmoItemNumber = n; }
	BOOL HasAmmo();
	int GetWeaponSkill();

	AnimRange *GetAnimRange(int Num) { return &Animations[Num]; }
	BOOL WasCreated() { return Created; }
	void GetSpellMenu(Object *pActor, ActionMenu *pAMenu);
	Region *GetRegionIn() { return pRegionIn; }
	void SetRegionIn(Region *pNewRegion) { pRegionIn = pNewRegion; }
	BOOL IsLarge() { return Large; }


	GameItem *GetEquipment(int n) { return Equipment[n]; };
	GameItem *GetEquipment(char *psLocation) { return Equipment[GetIndex(psLocation) - EquipHeadIndex]; };	
	char GetReadySpell() { return ReadySpell; }
	void SetReadySpell(char nReady) { ReadySpell = nReady; }

	int *GetLastSkillImproves() { return SkillImproved; }
	int ImproveSkill(int FieldIndex);
	int GetLastImprove(int Index);
	int SetLastImprove(int Index, int Amount);
	BOOL AddSpell(int Num);

	int GetAreaIn() { return AreaIn; }
	
	Creature *GetPrev() { return pPrev; }

   int GetValidActions(Object *pActor, ActionMenu *pAMenu);
   int GetDefaultAction(Object *pActor);
   int GetNumCreatures();

	int EvaluateAttack(Thing *pTarget, ATTACK_T AttackType, int &PercentSuccess, int &EstimateDamage);
	static Creature *GetFirst();

	int GetCombatRating();

	static Creature *FindCreature(const char *Name);
	static Creature *FindCreature(int UID);

	inline int GetActionCount()
	{
		return ActionCount;
	}

	ZSPortrait *GetPortrait() { return pPortrait; }

	ACTION_RESULT_T GetLastResult() { return LastResult; }
	BOOL IsActive() { return Active; }
	OBJECT_T GetObjectType() { return OBJECT_CREATURE; }

	void Save(FILE *fp);

	BOOL TileIntersect(int TileX, int TileY);
	BOOL TileBlock(int TileX, int TileY) { return TileIntersect(TileX, TileY); }

	BOOL IsMoving();

	int GetPowerLevel();

	int GetMinSpellAp();
	int GetMinSpellRange();
	int GetMaxSpellRange();

	int GetDamage(BOOL OffHand, BOOL Average = FALSE);
	float GetRange();

// Mutators -----------------------------------------
	void SetLastTarget(D3DVECTOR vNewLast) { vLastTarget = vNewLast; }

	void SetLastPlacedTime(unsigned long NewTime) { LastPlacedTime = NewTime; }

	int GetAmmoItemNumber() { return AmmoItemNumber; }
	void SetAngleOff(float NewOff) { angleoff = NewOff; }
	//inventory creation
	void SetCreated(BOOL NewVal) { Created = NewVal; }
	void SetWalkFrames(); 
	void CreateInventory();
	
	void SetEquipment(int n, GameItem *pToEquip) { Equipment[n] = pToEquip; }
	void SetAreaIn(int NewArea) { AreaIn = NewArea; }
	
	void SetPrev(Creature *pNewPrev) { pPrev = pNewPrev; }
	
	void SetActive(BOOL bnew) { Active = bnew; }
	
	void SetPortrait(ZSPortrait *pNew) { pPortrait = pNew; }

   ACTION_RESULT_T Update(void);				
	void UpdateOffScreen(void);	

   //actions that may be performed on creatures
   int Use(Thing *pUser);
   int LookAt(Object *pLooker);
   int TakeDamage(Thing *pDamageSource, int DamageAmount, DAMAGE_T DamageType);
   int TakeDamage(int HitAmount, int RestAmount);

   int AddAction(ACTION_T ActionType, void *Target, void *Data, BOOL Sub = FALSE);
   int InsertAction(ACTION_T ActionType, void *Target, void *Data, BOOL Sub = FALSE);
   int FinishAction();
   int ClearActions();
   int KillActions();

   Action *GetAction();
   void SetAction(Action *NewAction) {pCurAction = NewAction; };

   int StartRound();
   int EndRound(); 
	int EnterCombat();
	int SetFirst(Creature *NewFirst);

	friend int LoadCreatures(FILE *fp);
	friend int SaveCreatures(FILE *fp);
	friend int LoadBinCreatures(FILE *fp);
	friend int SaveBinCreatures(FILE *fp);
	friend int DeleteCreatures();
	friend void ReImportCreatures(FILE *fp);
	friend Creature *LoadCreature(FILE *fp);
	friend void InitCreatures();
	friend class ZSActionWin;
	friend class Corpse;


//	int AddItem(Thing *pAddThing);
//	int RemoveItem(Thing *pRemoveThing);

	int CreateTexture();
	int RemoveCurrentAction();
	int RemoveMajorAction();
	int RemoveMinorAction();
	Action *GetMajorAction();
	int RemoveAction(ACTION_T RemoveType);

	inline void IncrementActionCount()
	{
		ActionCount++;
	}
	inline void DecrementActionCount()
	{
		ActionCount--;
	}
	inline void ClearActionCount()
	{
		ActionCount = 0;
	}

	int ClearSubActions();

	int SetData(int fieldnum, int NewValue);	
	int SetData(int fieldnum, float NewfValue);
	int SetData(int fieldnum, char *NewString);
	int SetData(int fieldnum, D3DVECTOR *NewpVector);
	int SetData(char *fieldname, int NewValue);	
	int SetData(char *fieldname, float NewfValue);
	int SetData(char *fieldname, char *NewString);
	int SetData(char *fieldname, D3DVECTOR *NewpVector);

	void SetLastResult(ACTION_RESULT_T NewRes) { LastResult = NewRes; }

	void SetEquipObjects();
	
	void ReEquip(); //initialize equipment values and modifiers
	

	int Has(Thing *ToTest);
	int Take(Thing *ToTake, int Quantity = 1);
	int Give(Thing *ToReceive, int Quantity = 1);
	int Equip(GameItem *ToEquip, int NumEquip = 1);
	int Unequip(GameItem *ToUnEquip);

	void AddToWorld();
	void AddToArea(Area *pArea);

	BOOL AdvanceFrame();

	void Load(FILE *fp);

	Locator *GetLocator(int Num) { return &Schedule[Num]; }
	int GetNumLocators() { return NumLocators; }
	int GetCurLocator() { return CurLocator; }
	
	void RemoveLocator(int Num);
	void RemoveLocator(Locator *pToRemove);
	void AddLocator();

	static void PlaceByLocator();

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

	void Shadow();

	BOOL KnowsSpell(int Num) { return KnownSpells[Num]; }

	void DoneOneAction();

// Output ---------------------------------------------
	WeaponTracer *CreateWeaponParticleSystem(Creature *pAttacker, COLOR_T Color);


// Constructors ---------------------------------------

   Creature();
   Creature(Thing *pThingFrom);

// Destructor -----------------------------------------

   ~Creature();

// Debug ----------------------------------------------
	void OutputDebugInfo();

	operator = (Creature &OtherThing);

	static void Sort();
	static void FixSchedules();
	static void FixIDs();

	static void OutputSitStand();

	friend float GetDistance(Object *cA, int xA, int yA, Object *cB, int xB, int yB); 

	friend class Party;
};

int LoadCreatures(FILE *fp);
int SaveCreatures(FILE *fp);
int DeleteCreatures();
Creature *LoadCreature(FILE *fp);
void ReImportCreatures(FILE *fp);
void InitCreatures();

float GetDistance(Object *cA, int xA, int yA, Object *cB, int xB, int yB); 
float GetDistance(Creature *cA, Creature *cB);


#endif
