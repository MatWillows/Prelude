#ifndef  ENTRANCE_H
#define ENTRANCE_H

#include "objects.h"

class Entrance : public Object
{
private:
	char DestinationName[32];
	int  EventNum;
	int	 DestinationX;
	int  DestinationY;

public:
	char *GetDestinationName() { return DestinationName; }
	int GetDestX() { return DestinationX; }
	int GetDestY() { return DestinationY; }
	int GetEvent() { return EventNum; }

	void SetDestX(int NewX) { DestinationX = NewX; }
	void SetDestY(int NewY) { DestinationY = NewY; }
	void SetEvent(int NewEvent) { EventNum = NewEvent; }

	void Save(FILE *fp);

	void Load(FILE *fp);

	OBJECT_T GetObjectType() { return OBJECT_ENTRANCE; }

	int GetDefaultAction(Object *pactor);

	BOOL Go();

	void Draw();

	Entrance();

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);

};




#endif