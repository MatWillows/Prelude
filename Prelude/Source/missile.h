#ifndef MISSILE_H
#define MISSILE_H

#include "defs.h"
#include "objects.h"

typedef enum
{
	MISSILE_STATE_NONE,
	MISSILE_STATE_DONE,
} MISSILE_STATE_T;

class Missile : public Object
{

private:
	float xoff;
	float yoff;
	float zoff;
	int MoveFrame;
	int NumMoveFrames;

public:
	BOOL AdvanceFrame();
	int GetNumMoveFrames() { return NumMoveFrames; }

	Missile(ZSModelEx *pNewMesh, ZSTexture *pNewTexture, D3DVECTOR *vStart, D3DVECTOR *vEnd, float Speed);

};




#endif
