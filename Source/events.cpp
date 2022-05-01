#include "events.h"
#include "scriptfuncs.h"
#include "zsutilities.h"
#include "world.h"
#include "party.h"
#include "mainwindow.h"
#include "script.h"
#include <assert.h>
#include "flags.h"
#include "zsmessage.h"

EventManager PreludeEvents;
#define EVENT_CHECK_TIME	20

BOOL Event::AdvanceFrame()
{
	//check out frequency versus the current time
	//get the total time
	if(Frequency)
	{
		if(PreludeWorld->GetTotalTime() % Frequency)
		{
			return TRUE;
		}
	}
	
	int n;
	BOOL TempInside = FALSE;
	for(n = 0; n < PreludeParty.GetNumMembers(); n ++)
	{
		float Distance;
		Distance = GetDistance(PreludeParty.GetMember(n)->GetPosition(),GetPosition());
		if(Distance < (Radius + 0.1f))
		{
			TempInside = TRUE;
			if(!Inside || (PreludeWorld->GetTotalTime() - LastChecked > EVENT_CHECK_TIME))
			{
				if(!ValidateKey(NULL))
				{
					if(PreludeWorld->GetCurAreaNum() == 0 && (GetPosition()->x < 1000 || GetPosition()->x > 1600 ||
						GetPosition()->y < 1000 || GetPosition()->y > 1900))
					{
						Message("Some events will not occur the Trial Version outside of Kellen.","Ok");
						return TRUE;
					}
				}
				//someone is inside the event's radius
				SetInside(TRUE);
				LastChecked = PreludeWorld->GetTotalTime();

				//run the event
				if(!EventNum)
				{
					return FALSE;
				}

				PreludeEvents.RunEvent(EventNum);
				ScriptArg *pSA;
				pSA = Pop();
				if(pSA)
				{
					int Result;
					Result = (int)pSA->GetValue();
					switch(Result)
					{
					case 2: //remove all of these events near this one
					case 3: //remove all of these events ever
						Object *pOb;
						int StartX;
						int StartY;
						int EndX;
						int EndY;
						int SegX;
						int SegY;
						StartX = (int)(this->GetPosition()->x) / UPDATE_SEGMENT_WIDTH - 2;
						StartY = (int)(this->GetPosition()->y) / UPDATE_SEGMENT_HEIGHT - 2;
						EndX = (int)(this->GetPosition()->x) / UPDATE_SEGMENT_WIDTH + 2;
						EndY = (int)(this->GetPosition()->x) / UPDATE_SEGMENT_HEIGHT + 2;
						
						for(SegY = StartY; SegY <= EndY; SegY++)
						for(SegX = StartX; SegX <= EndX; SegX++)
						{
							pOb = Valley->GetUpdateSegment(SegX, SegY);
							while(pOb)
							{
								if(pOb->GetObjectType() == OBJECT_EVENT)
								{
									Event *pEvent;
									pEvent = (Event *)pOb;
									if(pEvent->GetNum() == EventNum)
									{
										pEvent->SetNum(0);
									}
								}
								pOb = pOb->GetNext();
							}
						}
					case 1: //remove this event
						return FALSE;
					default:
						return TRUE;
					}
				}
			}
			else
			{
			}
		}
	}

	//we must be outside the events radius
	if(!TempInside)
		SetInside(FALSE);

	return TRUE;
}

void Event::DrawVisible()
{
	//this is the function actually used to show where an event is
	Engine->Graphics()->SetTexture(NULL);
	
	D3DVECTOR *pPosition;
	pPosition = GetPosition();

	ZSModelEx *pMesh;
	pMesh = Engine->GetMesh(0);
	pMesh->Draw(Engine->Graphics()->GetD3D(), pPosition->x, pPosition->y, pPosition->z, 1.f, 0);


	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

	if(eType == EVENT_RADIUS || eType == EVENT_RADIUS_TIME)
	{
		float CircleRadius;
		CircleRadius = Radius;
		
		float ScaleFactor;
		
		ZSModelEx *SphereMesh;
		SphereMesh = Engine->GetMesh("cylinder");

		ScaleFactor = CircleRadius * 2.0f;

		if(SphereMesh)
		{
			SphereMesh->Draw(Engine->Graphics()->GetD3D(), pPosition->x, pPosition->y, pPosition->z, 0.0f, ScaleFactor, ScaleFactor, 1.0f, 0.0f);
		}
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	}

}



void Event::Draw()
{
	//is there any reason to do anything here?
	//all events are invisible
	if(PreludeWorld->GetGameState() == GAME_STATE_EDIT)
	{
		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
		DrawVisible();
	}

}
	
void Event::Load(FILE *fp)
{
	//load the event	
	fread(&Position,	sizeof(Position),1,fp);
	fread(&rBounds,	sizeof(rBounds),1,fp);
	fread(&vCenter,	sizeof(vCenter),1,fp);
	fread(&Radius,		sizeof(Radius),1,fp);
	fread(&eType,		sizeof(eType),1,fp);
	fread(&EventNum,	sizeof(EventNum),1,fp);
	fread(&HourBegin,	sizeof(HourBegin),1,fp);
	fread(&HourEnd,	sizeof(HourEnd),1,fp);
	fread(&TimeStart,	sizeof(TimeStart),1,fp);
	fread(&Frequency,	sizeof(Frequency),1,fp);

	pMesh = NULL;
	pTexture = NULL;

}
	
void Event::Save(FILE *fp)
{
	//output the type
	//then the rest of the data
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);

	fwrite(&Position,	sizeof(Position),1,fp);
	fwrite(&rBounds,	sizeof(rBounds),1,fp);
	fwrite(&vCenter,	sizeof(vCenter),1,fp);
	fwrite(&Radius,		sizeof(Radius),1,fp);
	fwrite(&eType,		sizeof(eType),1,fp);
	fwrite(&EventNum,	sizeof(EventNum),1,fp);
	fwrite(&HourBegin,	sizeof(HourBegin),1,fp);
	fwrite(&HourEnd,	sizeof(HourEnd),1,fp);
	fwrite(&TimeStart,	sizeof(TimeStart),1,fp);
	fwrite(&Frequency,	sizeof(Frequency),1,fp);
}

ScriptBlock *EventManager::GetEvent(int num)
{
	return &SBEvents[num];
}

void EventManager::RunEvent(int Num)
{
	char blarg[64];
	sprintf(blarg, "\nRunning event: %i\n", Num);
	DEBUG_INFO(blarg);
	
	GAME_STATE_T OldState;
	OldState = PreludeWorld->GetGameState();

	PreludeWorld->SetGameState(GAME_STATE_TEXT);

	ScriptBlock *OldContext;

	OldContext = ScriptContextBlock;
	
	ScriptContextBlock = &SBEvents[Num];

	DEBUG_INFO("Processing Event\n");

	SBEvents[Num].Process();
	
	DEBUG_INFO("Done Processing Event\n");

	ScriptContextBlock = OldContext;

	if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
	{
		if(OldState == GAME_STATE_COMBAT && PreludeWorld->GetGameState() == GAME_STATE_NORMAL)
		{}
		else
		{
			PreludeWorld->SetGameState(OldState);
		}
	}
	((ZSMainWindow *)ZSWindow::GetMain())->SetHighLightNonStatic(FALSE);
}

void EventManager::LoadEvents(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"rb");

	fread(&NumEvents,sizeof(NumEvents),1,fp);

	if(SBEvents)
	{
		delete[] SBEvents;
	}
	
	SBEvents = new ScriptBlock[NumEvents];

	for(int n = 0; n < NumEvents; n++)
	{
		SBEvents[n].Load(fp);
	}

	fclose(fp);
}

void EventManager::SaveEvents(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wb");

	fwrite(&NumEvents,sizeof(NumEvents),1,fp);
	for(int n = 0; n < NumEvents; n++)
	{
		SBEvents[n].Save(fp);
	}

	fclose(fp);
}

void EventManager::ImportEvents(const char *filename)
{
	//count the number of events
	FILE *fp;
	fp = SafeFileOpen(filename,"rt");

	int TwiceNum = 0;
	while(SeekTo(fp,"#"))
	{
		TwiceNum++;
	}
	NumEvents = TwiceNum /2;
	
	SBEvents = new ScriptBlock[NumEvents];

	//return to the beginning of the file
	fseek(fp,0,0);

	for(int n = 0; n < NumEvents; n++)
	{
		if(!SeekTo(fp,"#"))
		{
			char blarg[64];
			sprintf(blarg, "bad event #%i\n",n);
			DEBUG_INFO(blarg);
			SafeExit("failed to import events");
		}

		SeekTo(fp,"#");
		SBEvents[n].Import(fp);
	}

	fclose(fp);
}

void EventManager::DoTimed(unsigned long CurTime)
{
	Event *pE, *pENext, *pEToRemove;
	int Num;
			
	pE = epTimed;
	
	BOOL RanEvent = FALSE;

	while(pE)
	{
		pENext = (Event *)pE->GetNext();
		//check the time
		//if the event range is in the appropriate hour, call the event
		if(CurTime>= pE->GetStart())
		{
			if(pE->GetBegin() < pE->GetEnd()) 
			{
				if(PreludeWorld->GetHour() >= pE->GetBegin() && PreludeWorld->GetHour() <= pE->GetEnd())
				{
					if(!pE->IsInside() || (pE->GetBegin() != 0 || pE->GetEnd() != 23))
					{
						pE->SetInside(TRUE);
						Num = pE->GetNum();
						RunEvent(pE->GetNum());
						RanEvent = TRUE;
						pEToRemove = pE;
						pE = (Event *)pE->GetNext();
						ScriptArg *SA;
						SA = Pop();
						if(SA->GetValue())
						{
							RemoveEvent(&epTimed, pEToRemove);
							delete pEToRemove;
						//	RemoveTimed(Num);
						}
					}
				}
				else
				{
					pE->SetInside(FALSE);
				}
			}
			else
			if(PreludeWorld->GetHour() >= pE->GetBegin() || PreludeWorld->GetHour() <= pE->GetEnd())
			{
				if(!pE->IsInside() || (pE->GetBegin() != 0 || pE->GetEnd() != 23))
				{
					pE->SetInside(TRUE);
					Num = pE->GetNum();
					RunEvent(pE->GetNum());
					RanEvent = TRUE;
					pE = (Event *)pE->GetNext();
					ScriptArg *SA;
					SA = Pop();
					if(SA->GetValue())
					{
						RemoveTimed(Num);
					}
				}
			}
			else
			{
				pE->SetInside(FALSE);
			}
		}
		pE = pENext;
	}

	if(!RanEvent)
	{
		RanEvent = CheckForRandomEvent();
	}
}

void EventManager::DoStartCombat()
{
	Event *pE;
	int Num;
			
	pE = epStartCombat;

	while(pE)
	{
		Num = pE->GetNum();
		RunEvent(pE->GetNum());
		pE = (Event *)pE->GetNext();
	
		ScriptArg *SA;
		SA = Pop();
		if(SA->GetValue())
		{
			RemoveStartCombat(Num);
		}
	}
}

void EventManager::DoEndCombat()
{
	Event *pE;
	int Num;
			
	pE = epEndCombat;

	while(pE)
	{
		Num = pE->GetNum();
		RunEvent(pE->GetNum());
		pE = (Event *)pE->GetNext();
	
		ScriptArg *SA;
		SA = Pop();
		if(SA->GetValue())
		{
			RemoveEndCombat(Num);
		}
	}
}

void EventManager::DoCombatRound()
{
	Event *pE;
	int Num;
			
	pE = epCombatRound;

	while(pE)
	{
		Num = pE->GetNum();
		RunEvent(pE->GetNum());
		pE = (Event *)pE->GetNext();
	
		ScriptArg *SA;
		SA = Pop();
		if(SA->GetValue())
		{
			RemoveCombatRound(Num);
		}
	}
}

void EventManager::DoRest()
{
	Event *pE;
	int Num;
			
	pE = epRest;

	while(pE)
	{
		Num = pE->GetNum();
		RunEvent(pE->GetNum());
		pE = (Event *)pE->GetNext();
	
		ScriptArg *SA;
		SA = Pop();
		if(SA->GetValue())
		{
			RemoveRest(Num);
		}
	}

}

void EventManager::AddTimed(int Num, int NewHourBegin, int NewHourEnd, unsigned long NewTimeStart, int NewFrequency)
{
/*
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"adding timed: %i\n",Num);
	DEBUG_INFO(blarg);
#endif
*/
	Event *pE;

	pE = new Event();
	pE->SetNum(Num);
	pE->SetBegin(NewHourBegin);
	pE->SetEnd(NewHourEnd);
	pE->SetStart(NewTimeStart);
	pE->SetFrequency(NewFrequency);
	pE->SetInside(FALSE);
	pE->SetNext((Object *)epTimed);
	epTimed = pE;
}

void EventManager::AddStartCombat(int Num)
{

#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"adding sc: %i\n",Num);
	DEBUG_INFO(blarg);
#endif


	Event *pE;

	pE = new Event();
	pE->SetNum(Num);
	
	pE->SetNext((Object *)epStartCombat);
	epStartCombat = pE;

}

void EventManager::AddEndCombat(int Num)
{

#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"adding ec: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE;

	pE = new Event();
	pE->SetNum(Num);
	
	pE->SetNext((Object *)epEndCombat);
	epEndCombat = pE;
}

void EventManager::AddCombatRound(int Num)
{
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"adding ecr: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE;

	pE = new Event();
	pE->SetNum(Num);
	
	pE->SetNext((Object *)epCombatRound);
	epCombatRound = pE;
}

void EventManager::AddRest(int Num)
{
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"adding rest: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE;

	pE = new Event();
	pE->SetNum(Num);
	
	pE->SetNext((Object *)epRest);
	epRest = pE;
}


void EventManager::RemoveTimed(int Num)
{
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"removing timed: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE, *pLE;

	pE = epTimed;

	while(pE && pE->GetNum() == Num)
	{
		epTimed = (Event *)epTimed->GetNext();
		delete pE;
		pE = epTimed;
	}
	if(!pE) return;
	pLE = pE;
	pE = (Event *)pE->GetNext();
	while(pE)
	{
		if(pE->GetNum() == Num)
		{
			pLE->SetNext(pE->GetNext());
			delete pE;
			return;
		}
		pLE = pE;
		pE = (Event *)pE->GetNext();		
	}
}

void EventManager::RemoveStartCombat(int Num)
{
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"removing sc: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE, *pLE;

	pE = epStartCombat;

	if(pE->GetNum() == Num)
	{
		epStartCombat = (Event *)epStartCombat->GetNext();
		delete pE;
	}
	pLE = pE;
	pE = (Event *)pE->GetNext();
	while(pE)
	{
		if(pE->GetNum() == Num)
		{
			pLE->SetNext(pE->GetNext());
			delete pE;
			return;
		}
		pLE = pE;
		pE = (Event *)pE->GetNext();		
	}


}

void EventManager::RemoveEndCombat(int Num)
{
#ifndef NDEBUG
	char blarg[32];
	sprintf(blarg,"removing ec: %i\n",Num);
	DEBUG_INFO(blarg);
#endif

	Event *pE, *pLE;

	pE = epEndCombat;

	if(!pE)
	{
		return;
	}

	if(pE->GetNum() == Num)
	{
		epEndCombat = (Event *)epEndCombat->GetNext();
		delete pE;
		pE = epEndCombat;
		return;
	}
	pLE = pE;
	pE = (Event *)pE->GetNext();
	while(pE)
	{
		if(pE->GetNum() == Num)
		{
			pLE->SetNext(pE->GetNext());
			delete pE;
			return;
		}
		pLE = pE;
		pE = (Event *)pE->GetNext();		
	}


}

void EventManager::RemoveCombatRound(int Num)
{
	Event *pE, *pLE;

	pE = epCombatRound;

	if(pE->GetNum() == Num)
	{
		epCombatRound = (Event *)epCombatRound->GetNext();
		delete pE;
	}
	pLE = pE;
	pE = (Event *)pE->GetNext();
	while(pE)
	{
		if(pE->GetNum() == Num)
		{
			pLE->SetNext(pE->GetNext());
			delete pE;
			return;
		}
		pLE = pE;
		pE = (Event *)pE->GetNext();		
	}


}

void EventManager::RemoveRest(int Num)
{
	Event *pE, *pLE;

	pE = epRest;

	if(pE->GetNum() == Num)
	{
		epRest = (Event *)epRest->GetNext();
		delete pE;
	}
	pLE = pE;
	pE = (Event *)pE->GetNext();
	while(pE)
	{
		if(pE->GetNum() == Num)
		{
			pLE->SetNext(pE->GetNext());
			delete pE;
			return;
		}
		pLE = pE;
		pE = (Event *)pE->GetNext();		
	}
}

void EventManager::Clear()
{
	Event *pE;

	pE = epTimed;
	while(pE)
	{
		epTimed = (Event *)epTimed->GetNext();
		delete pE;
		pE = epTimed;
	}
	
	epTimed = NULL;
	
	pE = epStartCombat;
	while(pE)
	{
		epStartCombat = (Event *)epStartCombat->GetNext();
		delete pE;
		pE = epStartCombat;
	}
	epStartCombat = NULL;

	pE = epEndCombat;
	while(pE)
	{
		epEndCombat = (Event *)epEndCombat->GetNext();
		delete pE;
		pE = epEndCombat;
	}
	epEndCombat = NULL;

	pE = epCombatRound;
	while(pE)
	{
		epCombatRound = (Event *)epCombatRound->GetNext();
		delete pE;
		pE = epCombatRound;
	} 
	epCombatRound = NULL;

	pE = epRest;
	while(pE)
	{
		epRest = (Event *)epRest->GetNext();
		delete pE;
		pE = epRest;
	}
	epRest = NULL;
}


void EventManager::Load(FILE *fp)
{

//	LoadEvents("events.bin");
	Clear();

	BOOL AnotherEvent = TRUE;

	Event *pE;
	OBJECT_T Type;

	fread(&AnotherEvent, sizeof(BOOL),1,fp);
	while(AnotherEvent)
	{
		pE = new Event();
		fread(&Type,sizeof(OBJECT_T),1,fp);
		pE->Load(fp);
		pE->SetNext(epTimed);
		epTimed = pE;
		fread(&AnotherEvent, sizeof(BOOL),1,fp);
	}

	fread(&AnotherEvent, sizeof(BOOL),1,fp);
	while(AnotherEvent)
	{
		pE = new Event();
		fread(&Type,sizeof(OBJECT_T),1,fp);
		pE->Load(fp);
		pE->SetNext(epStartCombat);
		epStartCombat = pE;
		fread(&AnotherEvent, sizeof(BOOL),1,fp);
	}

	
	fread(&AnotherEvent, sizeof(BOOL),1,fp);
	while(AnotherEvent)
	{
		pE = new Event();
		fread(&Type,sizeof(OBJECT_T),1,fp);
		pE->Load(fp);
		pE->SetNext(epEndCombat);
		epEndCombat = pE;
		fread(&AnotherEvent, sizeof(BOOL),1,fp);
	}

	
	fread(&AnotherEvent, sizeof(BOOL),1,fp);
	while(AnotherEvent)
	{
		pE = new Event();
		fread(&Type,sizeof(OBJECT_T),1,fp);
		pE->Load(fp);
		pE->SetNext(epCombatRound);
		epCombatRound = pE;
		fread(&AnotherEvent, sizeof(BOOL),1,fp);
	}

	
	fread(&AnotherEvent, sizeof(BOOL),1,fp);
	while(AnotherEvent)
	{
		pE = new Event();
		fread(&Type,sizeof(OBJECT_T),1,fp);
		pE->Load(fp);
		pE->SetNext(epRest);
		epRest = pE;
		fread(&AnotherEvent, sizeof(BOOL),1,fp);
	}
}

void EventManager::Save(FILE *fp)
{
	BOOL AnotherEvent = TRUE;
	BOOL NoOtherEvent = FALSE;

	Event *pE;

	pE = epTimed;

	while(pE)
	{
		fwrite(&AnotherEvent,sizeof(BOOL),1,fp);
		pE->Save(fp);
		pE = (Event *)pE->GetNext();
	}
	fwrite(&NoOtherEvent,sizeof(BOOL),1,fp);

	pE = epStartCombat;

	while(pE)
	{
		fwrite(&AnotherEvent,sizeof(BOOL),1,fp);
		pE->Save(fp);
		pE = (Event *)pE->GetNext();
	}
	fwrite(&NoOtherEvent,sizeof(BOOL),1,fp);
	
	pE = epEndCombat;

	while(pE)
	{
		fwrite(&AnotherEvent,sizeof(BOOL),1,fp);
		pE->Save(fp);
		pE = (Event *)pE->GetNext();
	}
	fwrite(&NoOtherEvent,sizeof(BOOL),1,fp);
	
	pE = epCombatRound;

	while(pE)
	{
		fwrite(&AnotherEvent,sizeof(BOOL),1,fp);
		pE->Save(fp);
		pE = (Event *)pE->GetNext();
	}
	fwrite(&NoOtherEvent,sizeof(BOOL),1,fp);

	pE = epRest;

	while(pE)
	{
		fwrite(&AnotherEvent,sizeof(BOOL),1,fp);
		pE->Save(fp);
		pE = (Event *)pE->GetNext();
	}
	fwrite(&NoOtherEvent,sizeof(BOOL),1,fp);
}

Event::Event()
{
	Inside = FALSE;
	LastChecked = 0;
}

EventManager::EventManager()
{
	ForceRandom = FALSE;
	SBEvents = NULL;
	NumEvents = 0;
	epTimed = NULL;
	epStartCombat = NULL;
	epEndCombat = NULL;
	epCombatRound = NULL;
	epRest = NULL;

	LastUpdateTime = 0;
	LastSleepTime = 0;

	DEBUG_INFO("setting up valley random events\n");

	FILE *fp;

	fp = SafeFileOpen("randevents.txt","rt");

	int xn, yn;
	if(fp)
	{
		SeekTo(fp,"EventFactor:");
		EventFactor = GetInt(fp);
		if(!EventFactor)
			EventFactor = 10;
		SeekTo(fp,"CaveEvent:");
		CaveEvent = GetInt(fp);
		SeekTo(fp,"SleepEvent:");
		SleepEvent = GetInt(fp);
		for(yn = 0; yn < RANDOM_EVENT_DIM; yn++)
		for(xn = 0; xn < RANDOM_EVENT_DIM; xn++)
		{
			ValleyEventTypes[xn][yn] = GetInt(fp);
		}
		fclose(fp);
	}
	else
	{
		EventFactor = 5;
		CaveEvent = 0;
		SleepEvent = 0;
		for(yn = 0; yn < RANDOM_EVENT_DIM; yn++)
		for(xn = 0; xn < RANDOM_EVENT_DIM; xn++)
		{
			ValleyEventTypes[xn][yn] = 0;
		}
	}

	LastRandomTime = 0;
	RandomOverride = 0;
	TimesSinceSleepCalled = 0;

	DEBUG_INFO("Done Setting up random events\n");

}

EventManager::~EventManager()
{
	if(SBEvents)
	{
		delete[] SBEvents;
	}
	//delete the lists



}

int EventManager::GetRandomEvent(int x, int y)
{
	if(RandomOverride)
	{
		int Override;
		Override = RandomOverride;
		RandomOverride = 0;
		return Override;
	}

	int xn;
	xn = x;
	int yn;
	yn = y;

	int EventNum;
	xn = xn / (3200 / RANDOM_EVENT_DIM);
	yn = yn / (3200 / RANDOM_EVENT_DIM);

	if(xn >= RANDOM_EVENT_DIM || xn < 0 || yn >= RANDOM_EVENT_DIM || yn < 0)
	{
		DEBUG_INFO("event random out of bounds\n");
		return 0;
	}

	EventNum = ValleyEventTypes[xn][yn];

	return EventNum;
}

BOOL EventManager::CheckForRandomEvent()
{
	int TimeDif;
	TimeDif = PreludeWorld->GetTotalTime() - LastRandomTime;

	Flag *pCanTravel;
	int ChanceForEvent = 0;
	
	//chance for event increases as time passes
	ChanceForEvent = TimeDif / EventFactor;
	if(ChanceForEvent > 66)
		ChanceForEvent = 66;


	int EventRoll;
	EventRoll = (rand() % 100) + 2;
	
	if(!ForceRandom && EventRoll > ChanceForEvent)
	{
//		DEBUG_INFO("no force or chance of random\n");
		return FALSE;
	}
	UnSetForce();
	
	LastRandomTime = PreludeWorld->GetTotalTime();

	Creature *pCreature;
	pCreature = PreludeParty.GetLeader();
	if(!pCreature) 
	{
		DEBUG_INFO("bad party leader in random\n");
		return FALSE;
	}
		
	int xn;
	int yn;
	xn = (int)pCreature->GetPosition()->x;
	yn = (int)pCreature->GetPosition()->y;

	//don't activate random events in town
	if(PreludeWorld->GetTown(xn,yn))
	{
		DEBUG_INFO("Can't call event in town");
		return FALSE;
	}
	for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		if(PreludeParty.GetMember(n)->GetRegionIn())
		{
			DEBUG_INFO("Can't call random in building");
			return FALSE;
		}
	}
	
	Object *pOb;
	pOb = Valley->GetUpdateSegment(xn / UPDATE_SEGMENT_WIDTH, yn / UPDATE_SEGMENT_HEIGHT);

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			if(!PreludeParty.IsMember((Creature *)pOb))
			{
				DEBUG_INFO("Not Calling Random Event due to creature in area.\n");
				return FALSE;
			}
		}
		pOb = pOb->GetNextUpdate();
	}

	if(RandomOverride)
	{
		DEBUG_INFO("calling random override\n");
		PreludeEvents.RunEvent(RandomOverride);
		RandomOverride = 0;
		return TRUE;
	}

	if(PreludeWorld->GetCurAreaNum())
	{
		if(CaveEvent)
		{
			DEBUG_INFO("Calling Cave Events\n");
			PreludeEvents.RunEvent(CaveEvent);
			pCanTravel = PreludeFlags.Get("TRAVELEVENT");
			pCanTravel->Value = (void *)0;		
			return TRUE;
		}

		DEBUG_INFO("can't call random events in caverns\n");
		return FALSE;

	}

	int EventNum;
	xn = xn / (3200 / RANDOM_EVENT_DIM);
	yn = yn / (3200 / RANDOM_EVENT_DIM);

	if(xn >= RANDOM_EVENT_DIM || xn < 0 || yn >= RANDOM_EVENT_DIM || yn < 0)
	{
		DEBUG_INFO("event random out of bounds\n");
		pCanTravel = PreludeFlags.Get("TRAVELEVENT");
		pCanTravel->Value = (void *)0;		
		return FALSE;
	}

	EventNum = ValleyEventTypes[xn][yn];

	if(EventNum)
	{
		DEBUG_INFO("Calling random event\n");
		PreludeEvents.RunEvent(EventNum);
		pCanTravel = PreludeFlags.Get("TRAVELEVENT");
		pCanTravel->Value = (void *)0;		
		return TRUE;
	}
	DEBUG_INFO("No Random Events in area\n");
	return FALSE;
}

int EventManager::RandomSleep()
{
	//should resting be more dangerous away from towns?
	TimesSinceSleepCalled++;

	int CurTime;

	CurTime = PreludeWorld->GetTotalTime();
	int ChanceMod = 8;

	if(PreludeFlags.Get("GuardAwake")->Value)
	{
		ChanceMod += 5 * (int)PreludeFlags.Get("GuardAwake")->Value;
	}

	if(LastSleepTime - CurTime < 720)
	{
		ChanceMod = 4;
		if(TimesSinceSleepCalled < 1)
			TimesSinceSleepCalled = 1;

	}

	LastSleepTime = PreludeWorld->GetTotalTime();

	int Chance;
	Chance = rand() % ChanceMod;

	if(Chance <= TimesSinceSleepCalled)
	{
		TimesSinceSleepCalled = -1;
		return SleepEvent;
	}
	return 0;
}

void EventManager::AddEvent(Event **ListStart, Event *pToAdd)
{

	return;
}

void EventManager::RemoveEvent(Event **ListStart, Event *pToRemove)
{
	Event *pE;
	if(!pToRemove || !ListStart)
	{
		return;
	}

	if(*ListStart == pToRemove)
	{
		*ListStart = (Event *)pToRemove->GetNext();
		return;
	}

	pE = *ListStart;

	while(pE)
	{
		if(pE->GetNext() == pToRemove)
		{
			pE->SetNext(pToRemove->GetNext());
			return;
		}
		pE = (Event *)pE->GetNext();
	}

	return;
}

void EventManager::UnsetCreaturePointers()
{
	for (int n = 0; n < NumEvents; n++)
	{
		SBEvents[n].UnsetCreatures();
	}
}

void EventManager::SetCreaturePointers()
{
	for (int n = 0; n < NumEvents; n++)
	{
		SBEvents[n].SetCreatures();
	}
}


