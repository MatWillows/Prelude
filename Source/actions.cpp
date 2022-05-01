//********************************************************************* 
//********************************************************************* 
//**************                actions.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision: 7/24/00	                                                 * 
//*Revisor: Mat Williams                                           
//*Purpose:	Manage creature action stack                
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*                    
//*			
//********************************************************************* 
//*********************************************************************
#include "actions.h"
#include "path.h"

//************** static Members *********************************

//************** Constructors  ****************************************
Action *Action::pFreeList = NULL;

//simple constructor
Action::Action()
{
	Type = ACTION_NONE;
	pNext = NULL;
	Target = NULL;
	Data = NULL;
	Length = 0;
	CurFrame = 0;
	Started = FALSE;
	Finished = FALSE;
	Sub = FALSE;
	pNextFree = NULL;
	pPrevFree = NULL;
}

Action::Action(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub)
{
	Type = NewType;
	pNext = NULL;
	Target = NewTarget;
	Data = NewData;
	Length = 0;
	CurFrame = 0;
	Started = FALSE;
	Finished = FALSE;
	Sub = NewSub;
	pNextFree = NULL;
	pPrevFree = NULL;
}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************
Action::~Action()
{
	if(Type == ACTION_FOLLOWPATH)
	{
		Path *pPath;
		pPath = (Path *)Target;
		if(pPath)
			delete pPath;
	}

}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************
void *Action::GetTarget()
{
	return Target;
}

void *Action::GetData()
{
	return Data;
}

BOOL Action::IsFinished()
{
	return Finished;
}

BOOL Action::IsStarted()
{
	return Started;
}

int Action::GetLength()
{
	return Length;
}

int Action::GetCurFrame()
{
	return CurFrame;
}

ACTION_T Action::GetType()
{
	return Type;
}

Action *Action::GetNext()
{
	return pNext;
}

//end: Accessors *******************************************************



//************  Display Functions  *************************************


//end: Display functions ***********************************************



//************ Mutators ************************************************
int Action::SetTarget(void *NewTarget)
{
	Target = NewTarget;
	return TRUE;

}

int Action::SetData(void *NewData)
{

	Data = NewData;
	return TRUE;
}

int Action::SetFrame(int NewFrame)
{
	CurFrame = NewFrame;
	return TRUE;

}

int Action::SetType(ACTION_T NewType)
{
	Type = NewType;
	return TRUE;

}

int Action::SetLength(int NewLength)
{
	Length = NewLength;
	return TRUE;

}

int Action::SetNext(Action *pNewNext)
{
	pNext = pNewNext;
	return TRUE;
}

int Action::Start()
{
	Started = TRUE;
	return TRUE;
}

int Action::Finish()
{
	Finished = TRUE;
	return TRUE;
}

//end: Mutators ********************************************************


//************ Outputs *************************************************
void Action::OutputDebugInfo()
{
	switch(Type)
	{
			case ACTION_NONE:
				DEBUG_INFO("Action: NONE\n");
				break;
			case ACTION_GOTO:
				DEBUG_INFO("Action: Goto\n");
				break;
			case ACTION_MOVETO: 
				DEBUG_INFO("Action: Moveto\n");
				break;
			case ACTION_FOLLOWPATH:
				DEBUG_INFO("Action: Follow Path\n");
				break;
			case ACTION_MOVEIN: 
				DEBUG_INFO("Action: Move In\n");
				break;
			case ACTION_ROTATE: 
				DEBUG_INFO("Action: Rotate\n");
				break;
			case ACTION_ATTACK: 
				DEBUG_INFO("Action: Attack\n");
				break;
			case ACTION_KILL: 
				DEBUG_INFO("Action: Kill\n");
				break;
			case ACTION_ANIMATE: 
				DEBUG_INFO("Action: Animate\n");
				break;
			case ACTION_TALK: 
				DEBUG_INFO("Action: Talk\n");
				break;
			case ACTION_SAY: 
				DEBUG_INFO("Action: Say\n");
				break;
			case ACTION_TAKE: 
				DEBUG_INFO("Action: Take\n");
				break;
			case ACTION_DROP: 
				DEBUG_INFO("Action: Drop\n");
				break;
			case ACTION_GIVE: 
				DEBUG_INFO("Action: Give\n");
				break;
			case ACTION_PICKUP: 
				DEBUG_INFO("Action: Pickup\n");
				break;
			case ACTION_DIE: 
				DEBUG_INFO("Action: Die\n");
				break;
			case ACTION_UNCONSCIOUS:
				DEBUG_INFO("Action: Unconscious\n");
				break;
			case ACTION_REACTTO:
				DEBUG_INFO("Action: ReactTo\n");
				break;
			case ACTION_DEFEND:
				DEBUG_INFO("Action: Defend\n");
				break;
			case ACTION_SCRIPT: 
				DEBUG_INFO("Action: Script\n");
				break;
			case ACTION_THINK:
				DEBUG_INFO("Action: Think\n");
				break;
			case ACTION_USER: 
				DEBUG_INFO("Action: User\n");
				break;
			case ACTION_APPROACH: 
				DEBUG_INFO("Action: Approach\n");
				break;
			case ACTION_WAITUNTIL: 
				DEBUG_INFO("Action: WaintUntil\n");
				break;
			case ACTION_WAITFORSIGHT: 
				DEBUG_INFO("Action: WaitForSight\n");
				break;
			case ACTION_WAITFORHEAR: 
				DEBUG_INFO("Action: WaitForHear\n");
				break;
			case ACTION_WAITFORCLEAR: 
				DEBUG_INFO("Action: WaitForClear\n");
				break;
			case ACTION_USEITEM:
				DEBUG_INFO("Action: UseItem\n");
				break;
			case ACTION_LOOKAT: 
				DEBUG_INFO("Action: LookAt\n");
				break;
			case ACTION_MISSILE: 
				DEBUG_INFO("Action: Missile\n");
				break;
			case ACTION_WAITFOROTHERCREATURE: 
				DEBUG_INFO("Action: Waitforothecreature\n");
				break;
			case ACTION_IDLE:
				DEBUG_INFO("Action: Idle\n");
				break;
			case ACTION_CAST:
				DEBUG_INFO("Action: Cast\n");
				break;
			default:
				break;
	}
/*	
	char blarg[64];
	sprintf(blarg,"Target: %f\n",Target);
	
	sprintf(blarg,"Data: %f\n",Data);
*/
}

//end: Outputs ********************************************************





//************ Debug ***************************************************
void Action::AddAction()
{
	Action *pAction;
	pAction = new Action;
	if(pFreeList)
	{
		pFreeList->pNextFree = pFreeList;
		pFreeList->pPrevFree = pAction;
	}
	pFreeList = pAction;
}

void Action::AddAction(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub)
{
	Action *pAction;
	pAction = new Action(NewType,NewTarget,NewData,NewSub);
	if(pFreeList)
	{
		pFreeList->pNextFree = pFreeList;
		pFreeList->pPrevFree = pAction;
	}
	pFreeList = pAction;
}


Action *Action::GetNewAction()
{
	Action *pAction;
	if(!pFreeList)
	{
		AddAction();
	}

	pAction = pFreeList;
	
	pFreeList = pAction->pNextFree;

	if(pFreeList)
		pFreeList->pPrevFree = NULL;
	
	pAction->pNextFree = NULL;
	pAction->pPrevFree = NULL;
	pAction->Type = ACTION_NONE;
	pAction->pNext = NULL;
	pAction->Target = NULL;
	pAction->Data = NULL;
	pAction->Length = 0;
	pAction->CurFrame = 0;
	pAction->Started = FALSE;
	pAction->Finished = FALSE;
	pAction->Sub = FALSE;
	
	return pAction;
}

Action *Action::GetNewAction(ACTION_T NewType,void *NewTarget, void *NewData, BOOL NewSub)
{
	Action *pAction;
	if(!pFreeList)
	{
		AddAction();
	}

	pAction = pFreeList;
	
	pFreeList = pAction->pNextFree;

	if(pFreeList)
		pFreeList->pPrevFree = NULL;
	
	pAction->pNextFree = NULL;
	pAction->pPrevFree = NULL;

	pAction->Type = NewType;
	pAction->Target = NewTarget;
	pAction->Data = NewData;
	pAction->Sub = NewSub;
	pAction->pNext = NULL;
	pAction->Length = 0;
	pAction->CurFrame = 0;
	pAction->Started = FALSE;
	pAction->Finished = FALSE;
	
	return pAction;
}

void Action::Release()
{
	if(Type == ACTION_FOLLOWPATH)
	{
		Path *pPath;
		pPath = (Path *)Target;
		delete pPath;
		Target = NULL;
	}

	this->pNext = NULL;
	this->pNextFree = pFreeList;
	this->pPrevFree = NULL;
	if(pFreeList) pFreeList->pPrevFree = this;
	pFreeList = this;
}

void Action::ReleaseAll()
{
	Action *pAction;
	Action *pDel;
	pAction = pFreeList;

	while(pAction)
	{
		pDel = pAction;
		pAction = pAction->pNextFree;
		delete pDel;
	}

}


















//end: Debug ***********************************************************