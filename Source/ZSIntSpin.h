#ifndef ZSINTSPIN_H
#define ZSINTSPIN_H

#include "zswindow.h"

class ZSIntSpin : public ZSWindow
{
	private:
		int Value;
		int Max;
		int Min;

	public:

		int Command(int IDFrom, int Command, int Param);
		
		int GetMax() { return Max; }
		int GetMin() { return Min; }

		void SetMax(int NewMax) { Max = NewMax; }
		void SetMin(int NewMin) { Max = NewMin; }
		void SetMinMax(int NewMin, int NewMax) { Max = NewMax; Min = NewMin; }

		void SetValue(int NewValue);
		int GetValue() { return Value; }

		ZSIntSpin(int NewID, int x, int y, int width, int Height);

		void SetPlusMinus();
};

#endif