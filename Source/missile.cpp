#include "missile.h"

#include "zsutilities.h"



Missile::Missile(ZSModelEx *pNewMesh, ZSTexture *pNewTexture, D3DVECTOR *vStart, D3DVECTOR *vEnd, float Speed)
{
	Angle = 0.0f;
	xoff = 0.0f;
	yoff = 0.0f;
	zoff = 0.0f;
	Frame = 0;
	MoveFrame = 0;
	Scale = 1.0f;
	pNext = NULL;

	Position = *vStart;

	float Length = GetDistance(vStart,vEnd);

	NumMoveFrames = (int)((30.0f * Length) / Speed);

	xoff = (vEnd->x - vStart->x) / (float)NumMoveFrames;
	yoff = (vEnd->y - vStart->y) / (float)NumMoveFrames;
	zoff = (vEnd->z - vStart->z) / (float)NumMoveFrames;

	D3DVECTOR v1;
	D3DVECTOR v2;

	v1.x = 0;
	v1.y = 1;
	v1.z = 0;
	
	v2.z = 0;

	v2.x = vEnd->x - vStart->x;
	v2.y = vEnd->y - vStart->y;

	v2 = Normalize(v2);

	Angle = GetAngle(&v2,&v1) + PI;

	pMesh = pNewMesh;
	pTexture = pNewTexture;
}

BOOL Missile::AdvanceFrame()
{
	if(Frame++ <= pMesh->GetNumFrames())
	{
	
	}
	else
	{
		Frame = 0;
	}
	
	MoveFrame++;

	Position.x += xoff;
	Position.y += yoff;
	Position.z += zoff;

	if(MoveFrame == NumMoveFrames)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


