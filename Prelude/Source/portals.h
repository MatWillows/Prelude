#include "objects.h"

class Thing;
class Region;

#define PORTAL_ANIMATION_FRAMES 15
typedef enum
{
	PORTAL_OPEN,
	PORTAL_CLOSED,
	PORTAL_OPENNING,
	PORTAL_CLOSING,
	PORTAL_LOCKED,
} PORTALSTATE_T;

class Portal : public Object
{
private:
	PORTALSTATE_T	State;
	float ClosedAngle;
	float OpenAngle;
	D3DVECTOR OpenLocation;
	D3DVECTOR ClosedLocation;
	Region *pRegionOne;
	int RegionOneNum;
	int RegionOneSubNum;
	Region *pRegionTwo;
	int RegionTwoNum;
	int RegionTwoSubNum;
	Region *pRegionParent;
	int ParentRegionNum;
	int KeyNum;
	int EventNum;
	int LockNum;


public:
	int GetKey() { return KeyNum; }
	int GetLock() { return LockNum; }
	int GetEvent() { return EventNum; }

	void SetKey(int n) { KeyNum = n; }
	void SetLock(int n) { LockNum = n; }
	void SetEvent(int n) { EventNum = n; }

	void SetRegionOne(Region *NewOne);
	void SetRegionTwo(Region *NewTwo);
	
	Region *GetRegionOne() { return pRegionOne; }
	Region *GetRegionTwo() { return pRegionTwo; }

	int GetRegionOneNum() { return RegionOneNum; }
	int GetRegionTwoNum() { return RegionTwoNum; }

	void SetRegionOneNum(int NewOne) { RegionOneNum = NewOne; }
	void SetRegionTwoNum(int NewTwo) { RegionTwoNum = NewTwo; }

	void SetRegionOneSubNum(int NewOne) { RegionOneSubNum = NewOne; }
	void SetRegionTwoSubNum(int NewTwo) { RegionTwoSubNum = NewTwo; }
	
	void SetState(PORTALSTATE_T NewState) { State = NewState; };
	PORTALSTATE_T GetState() { return State; };

	void SetClosedAngle(float NewAngle) { ClosedAngle = NewAngle; };
	float GetClosedAngle() { return ClosedAngle; };

	void SetOpenAngle(float NewAngle) { OpenAngle = NewAngle; };
	float GetOpenAngle() { return OpenAngle; }

	BOOL AdvanceFrame();

   int GetDefaultAction(Object *pactor);

	void Load(FILE *fp);
	void Save(FILE *fp);
	
	OBJECT_T GetObjectType() { return OBJECT_PORTAL; }

	int LookAt(Object *pLooker);

	int GetFrame() { return 0; }

	BOOL TileIntersect(int TileX, int TileY);
	BOOL TileBlock(int TileX, int TileY);

	int AltHighLight();

	void Draw();

	~Portal();
	Portal();

};