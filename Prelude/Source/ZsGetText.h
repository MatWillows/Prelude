#ifndef ZSGETTEXT_H
#define ZSGETTEXT_H

#include "zswindow.h"


class ZSGetText : public ZSWindow
{
private:
	
public:

	int Command(int IDFrom, int Command, int Param);

	ZSGetText(char *Message, char *StartText);

};


char *GetModalText(char *Message, char *StartText = NULL, int Length = 32);


#endif

