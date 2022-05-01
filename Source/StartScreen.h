#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include "zswindow.h"

typedef enum
{
	START_RESULT_NONE,
	START_RESULT_NEW,
	START_RESULT_LOAD,
	START_RESULT_OPTIONS,
	START_RESULT_QUIT,
} START_RESULT_T;

class PreludeStartScreen : public ZSWindow
{
private:

public:

	int Draw();

	int Command(int IDFrom, int Command, int Param);

	PreludeStartScreen();
	~PreludeStartScreen();

};

#endif