#ifndef ZSFLOATSPIN_H
#define ZSFLOATSPIN_H

#include "zswindow.h"

class ZSFloatSpin : public ZSWindow
{
	private:
		float Value;

	public:

		int Command(int IDFrom, int Command, int Param);
		
		void SetValue(float NewValue);
		float GetValue() { return Value; }

		ZSFloatSpin(int NewID, int x, int y, int width, int Height);
};

#endif