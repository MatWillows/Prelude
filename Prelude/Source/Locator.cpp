#include "locator.h"

void Locator::Save(FILE *fp)
{
	fwrite(&rBounds,sizeof(RECT),1,fp);
	fwrite(&Start,sizeof(BYTE),1,fp);
	fwrite(&End,sizeof(BYTE),1,fp);
	fwrite(&Angle,sizeof(BYTE),1,fp);
	fwrite(&State,sizeof(BYTE),1,fp);
	fwrite(&AreaNum,sizeof(BYTE),1,fp);
}

void Locator::Load(FILE *fp)
{
	fread(&rBounds,sizeof(RECT),1,fp);
	fread(&Start,sizeof(BYTE),1,fp);
	fread(&End,sizeof(BYTE),1,fp);
	//switch these at next update
	//Angle = 0;
	//State = 0;
	fread(&Angle,sizeof(BYTE),1,fp);
	fread(&State,sizeof(BYTE),1,fp);
	//AreaNum = 0;
	fread(&AreaNum,sizeof(BYTE),1,fp);
}

Locator::Locator()
{
	Start = 0;
	End = 23;
	rBounds.top = 0;
	rBounds.bottom = 0;
	rBounds.right = 0;
	rBounds.left = 0;
	pNext = this;
	Angle = 0;
	State = 0;
	AreaNum = 0;
}