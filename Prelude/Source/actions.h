 
//********************************************************************* 
//*                                                                                                                                    ** 
//**************				actions.h          ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/20                                                                                          * 
//*Revisor:     mat williams                                          * 
//*Purpose:     defines class for creature actions            * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*														              * 
//********************************************************************* 
//*********************************************************************
#ifndef ACTIONS_H
#define ACTIONS_H

#include "defs.h"

typedef enum
{
	ACTION_NONE = 0,
	ACTION_GOTO,
	ACTION_MOVETO,
	ACTION_FOLLOWPATH,
	ACTION_MOVEIN,
	ACTION_MOVEINNOROTATE,
	ACTION_ROTATE,
	ACTION_ATTACK,
	ACTION_KILL,
	ACTION_ANIMATE,
	ACTION_TALK,
	ACTION_SAY,
	ACTION_TAKE,
	ACTION_DROP,
	ACTION_GIVE,
	ACTION_PICKUP,
	ACTION_DIE,
	ACTION_UNCONSCIOUS,
	ACTION_REACTTO,
	ACTION_DEFEND,
	ACTION_SCRIPT,
	ACTION_THINK,
	ACTION_USER,
	ACTION_APPROACH,
	ACTION_WAITUNTIL,
	ACTION_WAITFORSIGHT,
	ACTION_WAITFORHEAR,
	ACTION_WAITFORCLEAR,
	ACTION_USEITEM,
	ACTION_LOOKAT,
	ACTION_MISSILE,
	ACTION_WAITFOROTHERCREATURE,
	ACTION_IDLE,
	ACTION_CAST,
	ACTION_WAITACTION,
	ACTION_LEAP,
	ACTION_USE,
	ACTION_OPEN,
	ACTION_CLOSE,
	ACTION_GAME_COMMAND,
	ACTION_PICKPOCKET,
	ACTION_FORMATION,
	ACTION_SIT,
	ACTION_LAY,
	ACTION_SECONDARY_ATTACK,
	ACTION_SPECIAL_ATTACK,
} ACTION_T;

typedef enum
{
	ACTION_RESULT_NONE = 0,
	ACTION_RESULT_FINISHED,
	ACTION_RESULT_UNFINISHED,
	ACTION_RESULT_ADDED_ACTION,
	ACTION_RESULT_NOT_POSSIBLE,
	ACTION_RESULT_OUT_OF_AP,
	ACTION_RESULT_ADVANCE_FRAME,
	ACTION_RESULT_WAIT_FOR_USER,
	ACTION_RESULT_REMOVE_FROM_GAME,
} ACTION_RESULT_T;

typedef enum
{
	ACTION_GAME_COMMAND_REST,
	ACTION_GAME_COMMAND_JOURNAL,
	ACTION_GAME_COMMAND_CHARACTER,
	ACTION_GAME_COMMAND_SET_LEADER,
	ACTION_GAME_COMMAND_MAP,
	ACTION_GAME_COMMAND_OPTIONS,
	ACTION_GAME_COMMAND_DISMISS,
} ACTION_GAME_COMMAND_T;


typedef struct
{
	int StartFrame;
	int EndFrame;
	float XPerFrame;
	float YPerFrame;
	float ZPerFrame;
	float RotatePerFrame;
} ANIM_STRUCT_T;

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**						Action                                  ** 
//******************************************************************** 
//*Purpose:     class Action represents the possible actions a character might take
//*
//* 
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************


class Action
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 
   ACTION_T	Type;
   void *Target;
   void *Data;
   BOOL		Finished;
   BOOL		Started;
   int		Length;
   int		CurFrame;
	BOOL		Sub;
   Action	*pNext;

   Action *pNextFree;
   Action *pPrevFree;

   static Action *pFreeList;

//************************************************************************************** 

public:

// Display Functions --------------------------------

// Accessors ----------------------------------------
   static void AddAction();	
   static void AddAction(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub = FALSE);

   static Action *GetNewAction();
   static Action *GetNewAction(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub = FALSE);
   static void ReleaseAll();
   void Release();

   void *GetTarget();
   void *GetData();
   BOOL IsFinished();
   BOOL IsStarted();
   int GetLength();
   int GetCurFrame();
   ACTION_T GetType();
	Action *GetNext();
	BOOL IsSub() { return Sub; }

// Mutators -----------------------------------------
   int SetTarget(void *NewTarget);
   int SetData(void *NewData);
   int SetFrame(int NewFrame);
   int SetType(ACTION_T NewType);
   int SetLength(int NewLength);
	int SetNext(Action *pNewNext);
	int Start();
	int Finish();
	void SetSub(BOOL NewSub) { Sub = NewSub; }

// Output ---------------------------------------------

// Constructors ---------------------------------------

   Action();

   Action(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub = FALSE);

// Destructor -----------------------------------------

   ~Action();

// Debug ----------------------------------------------
	void OutputDebugInfo();

};

#endif
