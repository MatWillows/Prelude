#ifndef EVENTS_H
#define EVENTS_H

#include "objects.h"

class ScriptBlock;

typedef enum
{
	EVENT_NONE,
	EVENT_RADIUS,
	EVENT_BOUND,
	EVENT_TIME,
	EVENT_BOUND_TIME,
	EVENT_RADIUS_TIME,
} EVENT_T;

class Event : public Object
{
private:
	BOOL Inside;
	unsigned long LastChecked;
	RECT rBounds;
	D3DVECTOR vCenter;
	float Radius;
	EVENT_T eType;
	int EventNum;
	int HourBegin;
	int HourEnd;
	unsigned long TimeStart;
	int Frequency;

public:

	unsigned long GetLastChecked() { return LastChecked; }
	void SetLastChecked(unsigned long NewCheck) { LastChecked = NewCheck; }
	
	BOOL IsInside() { return Inside; }
	void SetInside(BOOL NewVal) { Inside = NewVal; }

	RECT *GetBounds() { return &rBounds; }
	D3DVECTOR *GetCenter() { return &vCenter; }
	float GetRadius() { return Radius; }
	void SetRadius(float NewRadius) { Radius = NewRadius; }

	EVENT_T GetEventType() { return eType; }
	void SetEventType(EVENT_T eNewType) { eType = eNewType; }

	OBJECT_T GetObjectType() { return OBJECT_EVENT; }

	void SetBegin(int NewBegin) { HourBegin = NewBegin; }
	void SetEnd(int NewEnd) { HourEnd = NewEnd; }
	void SetStart(unsigned long NewStart) { TimeStart = NewStart; }
	void SetNum(int NewNum) { EventNum = NewNum; }
	void SetFrequency(int NewFreq) { Frequency = NewFreq; }

	int GetBegin() { return HourBegin; }
	int GetEnd() { return HourEnd; }
	unsigned long GetStart() { return TimeStart; }
	int GetNum() { return EventNum; }
	int GetFrequency() { return Frequency; }

	BOOL AdvanceFrame();

	void Draw();

	void DrawVisible();
	
	void Load(FILE *fp);
	
	void Save(FILE *fp);

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd) { return FALSE; }

	Event();
};


#define RANDOM_EVENT_DIM 16
class EventManager
{
private:
	
	//how many events are in the game
	int NumEvents;

	//when did we last do a timed update
	unsigned long LastUpdateTime;

	//stuff for random events
	unsigned long LastRandomTime;
	int RandomOverride;
	int ValleyEventTypes[RANDOM_EVENT_DIM][RANDOM_EVENT_DIM];
	int EventFactor;
	int CaveEvent;
	int SleepEvent;
	int TimesSinceSleepCalled;
	int LastSleepTime;

	//the master list of events
	ScriptBlock *SBEvents;

	//events that are not area based
	Event *epTimed;
	Event *epStartCombat;
	Event *epEndCombat;
	Event *epCombatRound;
	Event *epRest;

	BOOL ForceRandom;

	//helper functions to add and remove the envents from their lists
	void AddEvent(Event **ListStart, Event *ToAdd);
	void RemoveEvent(Event **ListStart, Event *ToAdd);

public:

	void Clear();

	ScriptBlock *GetEvent(int num);

	void RunEvent(int Num);

	void LoadEvents(const char *filename);
	void SaveEvents(const char *filename);
	void ImportEvents(const char *filename);

	void DoTimed(unsigned long CurTime);
	void DoStartCombat();
	void DoEndCombat();
	void DoCombatRound();
	void DoRest();

	void AddTimed(int Num, int NewHourBegin, int NewHourEnd, unsigned long NewTimeStart, int NewFrequency);
	void AddStartCombat(int Num);
	void AddEndCombat(int Num);
	void AddCombatRound(int Num);
	void AddRest(int Num);

	void RemoveTimed(int Num);
	void RemoveStartCombat(int Num);
	void RemoveEndCombat(int Num);
	void RemoveCombatRound(int Num);
	void RemoveRest(int Num);

	int RandomSleep();

	void Load(FILE *fp);
	void Save(FILE *fp);

	EventManager();

	~EventManager();

	void SetRandomOverride(int NewOV) { RandomOverride = NewOV; }

	BOOL CheckForRandomEvent();

	int GetRandomFactor() { return EventFactor; }

	int GetRandomEvent(int x, int y);

	int GetLastSleepTime() { return LastSleepTime; }
	void SetLastSleepTime(int newtime) { LastSleepTime = newtime; }

	void SetForce() { ForceRandom = TRUE; }
	void UnSetForce() { ForceRandom = FALSE; }

	void SetLastRandomTime(unsigned long newlast) { LastRandomTime = newlast; }
	unsigned long GetLastRandomTime() { return LastRandomTime; }

	void UnsetCreaturePointers();

	void SetCreaturePointers();

};

extern EventManager PreludeEvents;




#endif