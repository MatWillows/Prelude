#ifndef ZSCONFIRM_H
#define ZSCONFIRM_H

#include "zswindow.h"

class ZSConfirmWin : public ZSWindow
{
private:

public:

	int Command(int IDFrom, int Command, int Param);
	int MoveMouse(long *x, long *y, long *z);

	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSConfirmWin(char *Message, char *ResponseA, char *ResponseB);

};

BOOL Confirm(ZSWindow *pParent, char *Message,  char *ResponseA,char *ResponseB);

#endif