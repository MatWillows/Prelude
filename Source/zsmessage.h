#ifndef ZSMESSAGE_H
#define ZSMESSAGE_H

#include "ZSwindow.h"
#include "ZSbutton.h"
#include "ZStext.h"

#define ID_MESSAGE_OK		768
#define ID_MESSAGE_TEXT		769

class ZSMessage : public ZSWindow
{
protected:
	
public:
	int Command(int IDFrom, int Command, int Param);
	int GoModal();
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);

	ZSMessage(char *Text, char *ConfirmText);

};

void Message(char *Text, char *ConfirmText);

#endif
