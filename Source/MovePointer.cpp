#include "MovePointer.h"
#include "party.h"
#include "world.h"


BOOL MovePointer::AdvanceFrame()
{
	if(PreludeWorld->InCombat())
	{
		PreludeParty.RemoveMarker(this);
		return FALSE;
	}

	if(!((Creature *)pLink)->IsMoving())
	{
		NotMoveCount++;
		if(NotMoveCount > 15 || *pLink->GetPosition() == Position)
		{
			PreludeParty.RemoveMarker(this);
			return FALSE;
		}
	}
	else
	{
		NotMoveCount = 0;
	}

	SetAngle(GetMyAngle() + ROTATION_ANGLE);
	if(GetMyAngle() > PI_MUL_2)
	{
		SetAngle(0.0f);
	}
	return TRUE;
}

MovePointer::MovePointer(D3DVECTOR *pDest, Object *pNewLink)
{
	Position = *pDest;
	pLink = pNewLink;
	pMesh = Engine->GetMesh(1);
	pTexture = NULL;
	Scale = 1.0f;
	pNextUpdate = NULL;
	pNext = NULL;
	pPrevUpdate = NULL;
	Frame = 0;
	Angle = 0.0f;
	TextureNum = 0;
	NotMoveCount = 0;
}
