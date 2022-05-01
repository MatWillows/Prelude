#ifndef MOVEPOINTER_H
#define MOVEPOINTER_H


#include "objects.h"


class MovePointer : public Object
{
private:
	Object *pLink;

public:
	BOOL NotMoveCount;

	BOOL AdvanceFrame();

	MovePointer(D3DVECTOR *pDest, Object *pLink);
};

#endif