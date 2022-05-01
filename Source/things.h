//********************************************************************* 
//*                                                                                                                                    ** 
//**************				things.h          ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/16                                                  * 
//*Revisor:     mat williams                                          * 
//*Purpose:     define base class for objects in Prelude              * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*																						 * 
//********************************************************************* 
//*********************************************************************
#ifndef THINGS_H
#define THINGS_H

#include "defs.h"		//basic typedefs and constants used throughout game
#include "actionmenuclass.h"
#include "objects.h"


//preprocessor defs ***********************************************
//	These are used to rapidly de-ref certain data fields within
//  things that are speed critical and universal to all things
typedef enum
{
	INDEX_NAME = 0,				
	INDEX_ID,					
	INDEX_UID,					
	INDEX_TYPE,					

	//used in drawing
	INDEX_POSITION,			
	INDEX_ROTATION,			
	INDEX_SCALE,				
	INDEX_MESH,					
	INDEX_TEXTURE,
	INDEX_FRAME,
	INDEX_SOLIDITY,
	INDEX_MOVEABLITY,

	//universal action overrides 
	INDEX_LOOKAT_OVERRIDE,		
	INDEX_USE_OVERRIDE,			
	INDEX_DAMAGE_OVERRIDE,		

	//the following are not universal but are specific to individual types of things
	//they provide access to possible script overrides for specific functionality

	//items
	INDEX_EQUIP_OVERRIDE = INDEX_DAMAGE_OVERRIDE + 1,		
	INDEX_UNEQUIP_OVERRIDE,		
	INDEX_GRAB_OVERRIDE,			
	INDEX_PLACE_OVERRIDE,		
	INDEX_PICKUP_OVERRIDE,			
	INDEX_PUTDOWN_OVERRIDE,		
			
	//creatures
	INDEX_DIE_OVERRIDE = INDEX_DAMAGE_OVERRIDE + 1,			
	INDEX_PORTRAIT,
	INDEX_AICODE,
	INDEX_ACTIONPOINTS,
	INDEX_MAXACTIONPOINTS,
	INDEX_MAXRESTPOINTS,
	INDEX_RESTPOINTS,
	INDEX_MAXHITPOINTS,
	INDEX_HITPOINTS,
	INDEX_ATTACKPOINTS,
	INDEX_WILLPOINTS,
	INDEX_MAXWILLPOINTS,
	INDEX_MINDAMAGE,
	INDEX_MAXDAMAGE,
	INDEX_DAMAGETYPE,
	INDEX_RANGE,
	INDEX_WEAPONSKILL,
	INDEX_MOVEPOINTS,
	INDEX_BATTLESTATUS,
	INDEX_BATTLEID,
	INDEX_BATTLESIDE,
	INDEX_LASTATTACK,
	INDEX_FEINT,
	INDEX_PARRY,
	INDEX_BLOCK,
	INDEX_DODGE,
	INDEX_FLAMEBLOOD,
	INDEX_RIVERBLOOD,
	INDEX_VANBLOOD,
	INDEX_SEX,
	INDEX_SKIN,
	INDEX_HAIR,
	INDEX_FACIALHAIR,
	INDEX_AGE,
	INDEX_STRENGTH,
	INDEX_DEXTERITY,
	INDEX_CHARISMA,
	INDEX_INTELLIGENCE,
	INDEX_WILLPOWER,
	INDEX_ENDURANCE,
	INDEX_SPEED,
	INDEX_XP,
	INDEX_SWORD,
	INDEX_DAGGER,
	INDEX_AXE,
	INDEX_BLUNT,
	INDEX_POLEARM,
	INDEX_MISSILE,
	INDEX_UNARMED,
	INDEX_THROWING,
	INDEX_ARMOR,
	INDEX_STEALTH,
	INDEX_TINKER,
	INDEX_PICKPOCKET,
	INDEX_LITERACY_AND_LORE,
	INDEX_SPEECH,
	INDEX_BARTER,
	INDEX_MUSIC,
	INDEX_MEDICAL,
	INDEX_NATURE,
	INDEX_POWER_OF_FLAME,
	INDEX_GIFTS_OF_GODDESS,
	INDEX_THAUMATURGY,
	INDEX_SOUL_OF_WIND,
	INDEX_PICTURE,
	INDEX_RADIUS,
	INDEX_DESTINATION,
	INDEX_HEAD,
	INDEX_LEFTHAND,
	INDEX_RIGHTHAND,
	INDEX_CHEST,
	INDEX_LEGS,
	INDEX_RIGHTRING,
	INDEX_LEFTRING,
	INDEX_NECK,
	INDEX_FEET,
	INDEX_AMMO,
	INDEX_GLOVES,
	INDEX_SPELLBOOK,

	//switches
	INDEX_TINKER_OVERRIDE = INDEX_DAMAGE_OVERRIDE + 1,		
	INDEX_CLOSE_OVERRIDE,		
	INDEX_OPEN_OVERRIDE,		
	INDEX_BASH_OVERRIDE,		

	//events

	//spells
	INDEX_DESCRIPTOR1 = INDEX_ID + 1,
	INDEX_DESCRIPTOR2,
	INDEX_DESCRIPTOR3,
	INDEX_DESCRIPTOR4,

} THING_INDEX_T;

#define THING_SEPERATION_CHARACTER	'@'


//*******************************CLASS******************************** 
//**************                                 ********************* 
//**						Thing                                   ** 
//******************************************************************** 
//*Purpose:     class Thing provides the basic data references and 
//*				function calls for all non-static "objects" in the game                                                 * 
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************


class Thing : public Object
{
protected:
//************************************************************************************** 
//                             MEMBER VARIABLES 
   static int NextUniqueID;
	static int NumThings;
	
	int UniqueID;
   
	char *DataFieldNames;
   
	DATA_FIELD_T *DataFields;
   
	int NumFields;
   
//	Thing *pNext;
	
	DATA_T *DataTypes;
	
//************************************************************************************** 

public:

// Display Functions -------------------------------

   virtual void Draw(void);

	int AltHighLight();

	virtual int Arrow(D3DCOLORVALUE Color);
	void WordBalloon();

	int Circle(COLOR_T Color);
	virtual void Shadow() { return; };

// Accessors ----------------------------------------

	static Thing *Find(Thing *SearchStart, int ID, int UID = 0);
	static Thing *Find(Thing *SearchStart, const char *ThingName);

   virtual DATA_FIELD_T GetData(int fieldnum);			
   virtual DATA_FIELD_T GetData(char *fieldname);	
	
//	Thing *GetNext();
	char *GetFieldNames();
	int GetNumFields();
	int GetIndex(char *FieldName);
	char *GetName(int FieldNum);
	DATA_T GetType(int FieldNum);
	DATA_T *GetDataTypes() { return DataTypes; }
	int GetUniqueID() { return UniqueID; }

   virtual int GetValidActions(Object *pactor, ActionMenu *pAMenu) { return 0; };
   virtual int GetDefaultAction(Object *pactor) { return 0; };
	virtual D3DVECTOR *GetPosition();
	virtual int GetFrame();
	virtual int SetFrame(int NewFrame);
	virtual float GetMyAngle();
	virtual int SetAngle(float NewAngle);
	virtual float GetRadius();
	virtual float GetScale();
   
	void GetXY(int *X, int *Y);
	virtual OBJECT_T GetObjectType() { return OBJECT_THING; }

//	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	
// Mutators -----------------------------------------

	virtual int TakeDamage(Thing *pDamageSource, int DamageAmount, DAMAGE_T DamageType)
	{	return 1; }

  	virtual int TakeDamage(int HitAmount, int RestAmount)
	{	return 1; }


//	int SetNext(Thing *NewNext);
	int SetFieldNames(char *NewNames);
	int SetNumFields(int NewNumFields);

	void SetDataTypes(DATA_T *NewTypes) { DataTypes = NewTypes; };
	
   virtual int SetData(int fieldnum, int NewValue);	
	virtual int SetData(int fieldnum, float NewfValue);
	virtual int SetData(int fieldnum, char *NewString);
	virtual int SetData(int fieldnum, D3DVECTOR *NewpVector);
	virtual int SetData(char *fieldname, int NewValue);	
	virtual int SetData(char *fieldname, float NewfValue);
	virtual int SetData(char *fieldname, char *NewString);
	virtual int SetData(char *fieldname, D3DVECTOR *NewpVector);

   int LoadFieldNames(FILE *fp, char *destination);

	int DestroyFieldNames();
 
   int LoadData(FILE *fp);
	int LoadBin(FILE *fp);

	virtual BOOL AdvanceFrame();

	virtual int Has(Thing *ToTest);
	virtual int Take(Thing *ToTake, int Quantity = 1);
	virtual int Give(Thing *ToReceive, int Quantity = 1);

// Output ---------------------------------------------

   int SaveFieldNames(FILE *fp);
   int SaveData(FILE *fp);
	int SaveBin(FILE *fp);

// Constructors ---------------------------------------

   Thing();
   Thing(Thing *pFrom);

// Destructor -----------------------------------------

   virtual ~Thing();

// Debug ----------------------------------------------

   int OutputDebugInfo(const char *filename);
   int OutputDebugInfo(FILE *fp);

// Operators ------------------------------------------

   operator = (Thing& otherThing);		//assignment operator
	bool operator == (Thing& ThingOne);

};

#endif
