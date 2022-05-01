//********************************************************************* 
//********************************************************************* 
//**************                nothing.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:                                                                                        * 
//*Revisor:                                               
//*Purpose:                        
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        			               
//*			
//********************************************************************* 
//*********************************************************************
#include "actionmenuclass.h"

//************** static Members *********************************

//************** Constructors  ****************************************

//simple constructor
ActionMenu::ActionMenu()
{
	NumActions = 0;
}

//copy constructor


//end:  Constructors ***************************************************



//*************** Destructor *******************************************


//end:  Destructor *****************************************************



//************  Accessors  *********************************************
int ActionMenu::GetNumActions()
{
	return NumActions;
}
	
Action *ActionMenu::GetAction(int ActionNum)
{
	return &Actions[ActionNum];
}

char *ActionMenu::GetString(int ActionNum)
{
	return &Strings[ActionNum][0];
}

//end: Accessors *******************************************************



//************  Display Functions  *************************************


//end: Display functions ***********************************************



//************ Mutators ************************************************
int ActionMenu::AddAction(ACTION_T NewType, void *NewTarget, void *NewData, char *NewString)
{
	Actions[NumActions].SetType(NewType);
	Actions[NumActions].SetTarget((void *)NewTarget);
	Actions[NumActions].SetData((void *)NewData);
	strcpy(&Strings[NumActions][0],NewString);
	NumActions++;
	return TRUE;
}



//end: Mutators ********************************************************



//************ Outputs *************************************************

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************