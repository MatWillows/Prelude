//********************************************************************* 
//*                                                                                                                                    ** 
//**************		Objects.h	        ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:        8/1                                                                                   * 
//*Revisor:         Mat Williams                                     
//*Purpose:                   
//********************************************************************* 
//*Outstanding issues:                                                 
//* get rid of mesh and texture num
//* bettter integration with regions                                  * 
//*																						 * 
//********************************************************************* 
//*********************************************************************
#ifndef OBJECTS_H
#define OBJECTS_H

#include "defs.h"
#include "zstexture.h"
#include "ZSModelEx.h"
#include "zsengine.h"

//preprocessor defs ***********************************************

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**					            Object                       
//******************************************************************** 
//*Purpose:																			 * 
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************

typedef enum
{
	OBJECT_NONE = 0,
	OBJECT_STATIC,
	OBJECT_THING,
	OBJECT_PORTAL,
	OBJECT_CREATURE,
	OBJECT_ITEM,
	OBJECT_SWITCH,
	OBJECT_CONTAINER,
	OBJECT_EVENT,
	OBJECT_WATER,
	OBJECT_FOUNTAIN,
	OBJECT_PARTICLESYSTEM,
	OBJECT_FIRE,
	OBJECT_EQUIP,
	OBJECT_SIGN,
	OBJECT_BARRIER,
	OBJECT_ENTRANCE,
	OBJECT_MINIMAP,
	OBJECT_MODIFIER,
} OBJECT_T;

class Item;
class GameItem;
class Area;

class Object
{
protected:
//************************************************************************************** 
//                             MEMBER VARIABLES 
	D3DVECTOR Position;
	int Frame;
	long Data;
	float Angle;
	float BlockingRadius;
	float Scale;
	int MeshNum;
	ZSModelEx *pMesh;
	Object *pNext;
	int TextureNum;
	ZSTexture *pTexture;
	Object *pNextUpdate;
	Object *pPrevUpdate;
	Object *pContents;
	
//************************************************************************************** 

public:

// Display Functions -------------------------------
	virtual void Draw();
	void Draw(HDC hdc);

	virtual int HighLight(COLOR_T Color);
	virtual int AltHighLight();

	virtual void Shadow() { return; };

// Accessors ---------------------------------------	-
	virtual D3DVECTOR *GetPosition();
	virtual int GetFrame() { return Frame; };
	virtual float GetMyAngle();
	virtual float GetRadius();
	virtual float GetScale();

	inline ZSModelEx *GetMesh() { return pMesh; }
	inline Object *GetNext() { return pNext; }
	int GetTextureNum();
	ZSTexture *GetTexture();

	virtual BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	virtual BOOL RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	virtual BOOL LineIntersect(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);
	virtual OBJECT_T GetObjectType() { return OBJECT_STATIC; }
   virtual int GetDefaultAction(Object *pactor) { return 0; };
	virtual int LookAt(Object *looker) { return 0; };
	Object *GetNextUpdate() { return pNextUpdate; }
	Object *GetPrevUpdate() { return pPrevUpdate; }
	D3DVECTOR GetCenter();
	float GetCurrentRadius();
	long GetData() { return Data; }
	virtual Object *GetContents() { return pContents; }

// Mutators -----------------------------------------
	int SetPosition(D3DVECTOR *pNewPosition);
	int SetPosition(float x, float y, float z);
	virtual int SetFrame(int NewFrame);
	virtual int SetAngle(float NewAngle);
	int SetRadius(float NewRadius);
	virtual int SetScale(float NewScale);
	int SetMesh(ZSModelEx *pNewMesh);
	void SetMeshNum(int NewNum) { MeshNum = NewNum; }
	int SetNext(Object *pNewNext);
	int SetTextureNum(int NewNum);
	int SetTexture(ZSTexture *pNewTexture);
	void SetData(long NewData) { Data = NewData; }
	virtual BOOL AdvanceFrame();
	virtual void AdjustCamera();
	virtual void Load(FILE *fp);
	virtual void AddToWorld() { return; }
	virtual void AddToArea(Area *pArea);

	void SetNextUpdate(Object *pNew) { pNextUpdate = pNew; }
	void SetPrevUpdate(Object *pNew) { pPrevUpdate = pNew; }

	virtual void Move(D3DVECTOR *vDirection);


	virtual BOOL TileIntersect(int TileX, int TileY);
	virtual BOOL TileBlock(int TileX, int TileY);
	BOOL AddItem(GameItem *pGameItem);
	BOOL AddItem(Item *pItem, int Quantity);
	
	BOOL RemoveItem(GameItem *pGameItem);
	BOOL RemoveItem(Item *pItem, int Quantity);
	
// Output ---------------------------------------------
	virtual void Save(FILE *fp);
  
// Constructors ---------------------------------------
	Object();
 
// Destructor -----------------------------------------
	virtual ~Object();

// Debug ----------------------------------------------
	friend Object *LoadObject(FILE *fp);

// Operators ------------------------------------------
};

Object *LoadObject(FILE *fp);

#endif
