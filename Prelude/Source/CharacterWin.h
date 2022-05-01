//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSCharacterwin.h           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Oct 30, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*********************************************************************
//*********************************************************************
#ifndef ZSCHARACTERWIN_H
#define ZSCHARACTERWIN_H

#include "zswindow.h"

//control ID's for the (C)haracter (S)creeen
typedef enum
{
	CS_PORTRAIT,
	CS_NAME,
	CS_SEX,
	CS_RACE,
	CS_ARMOR,
	CS_DAMAGE,
	CS_HP,
	CS_XP,
	CS_STAT1,
	CS_STAT2,
	CS_STAT3,
	CS_STAT4,
	CS_STAT5,
	CS_STAT6,
	CS_STAT7,
	CS_STAT8,
	CS_CLOSE,
	CS_GOSKILLS,
	CS_GOEQUIP,
	CS_SKILLS,
	CS_INVENTORY,
	CS_EQUIP,
	CS_GOSPELLS,
	CS_SPELLBOOK,
} CHARACTER_SCREEN_CONTROLS;

#define CHARACTER_WINDOW_ID		66666

class Creature; //prototype

//****************************CLASS************************************
//**************                                  *********************
//**                         CharacterWin                            **
//*********************************************************************
//*Purpose:    display characters information and allow expenditure of xp *
//*********************************************************************
//*Invariants:                                                                                                                    *
//*                                                                   *
//*********************************************************************

class CharacterWin : public ZSWindow
{
	protected:
	//***************MEMBERS****************************************************
	//**
		Creature *pTarget;
		int StartingValues[100];
		
	public:

//********************METHODS***************************************************
//**

//Display
//--------------------------------------------------------------------------	

//Accessors		
//------------------------------------------------------------------------------	
		Creature *GetTarget() { return pTarget; }
		

//Mutators
//------------------------------------------------------------------------
		int Command(int IDFrom, int Command, int Param);
		int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
		void SetTarget(Creature *NewTarget) { pTarget = NewTarget; }
		void ResetStats();
		void Show();
		BOOL RightButtonUp(int x, int y);
		
//Constructors
//-------------------------------------------------------------------------------
		CharacterWin(int NewID, int x, int y, int width, int height, Creature *pTarget);

//Destructor
//------------------------------------------------------------------------------

//debug
//-------------------------------------------------------------------------------

};

#endif