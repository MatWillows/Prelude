//get target
//a special window used to get a particular type of target
//types of target are:
//creature
//creature friend
//creature enemy
//item
//location

#include "zsWindow.h"

typedef enum
{
	TARGET_NONE,
	TARGET_LOCATION,
	TARGET_CREATURE,
	TARGET_ITEM,
	TARGET_DIRECTION,
} TARGET_T;

//function which creates target window
//places target in Target
//returns True is successful
//false if not
BOOL GetTarget(void **Target, TARGET_T Type, D3DVECTOR vSource, float frange, char *Message = NULL);

class ZSTargetWin : public ZSWindow
{
private:
	
	void *pTarget;
	D3DVECTOR vSource;
	float frange;
	TARGET_T TType;
	D3DVECTOR vMouse;
	BOOL ValidTarget;
	int MouseScreenX;
	int MouseScreenY;
	BOOL LBDown;
	
	void GetMouseTarget();

public:

	int Draw();

	int GoModal();

	void *GetTarget() { return pTarget; }
	void SetTarget(void *NewTarget) { pTarget = NewTarget; }

	TARGET_T GetTargetType() { return TType; }
	void SetTargetType(TARGET_T NewType) { TType = NewType; }

	int MoveMouse(long *x, long *y, long *z);

	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonUp(int x, int y);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSTargetWin(int NewID, TARGET_T TargetType, D3DVECTOR vTargetSource, float fTargetRange, char *Message = NULL);

};
