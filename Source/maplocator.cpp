#include "maplocator.h"

void MapLocator::Save(FILE *fp)
{
	fwrite(Tag, sizeof(char),32,fp);
	fwrite(&X, sizeof(int),1,fp);
	fwrite(&Y, sizeof(int),1,fp);
	fwrite(&MapNum, sizeof(int),1,fp);
	fwrite(&MapX, sizeof(int),1,fp);
	fwrite(&MapY, sizeof(int),1,fp);
	fwrite(&Teleport, sizeof(BOOL),1,fp);

	BOOL BNext;
	if(pNext)
	{
		BNext = TRUE;
	}
	else
	{
		BNext = FALSE;
	}

	fwrite(&BNext, sizeof(BOOL),1,fp);
	if(BNext)
	{
		pNext->Save(fp);
	}
}


void MapLocator::Load(FILE *fp)
{
	fread(Tag, sizeof(char),32,fp);
	fread(&X, sizeof(int),1,fp);
	fread(&Y, sizeof(int),1,fp);
	fread(&MapNum, sizeof(int),1,fp);
	fread(&MapX, sizeof(int),1,fp);
	fread(&MapY, sizeof(int),1,fp);
	fread(&Teleport, sizeof(BOOL),1,fp);
	
	BOOL BNext;
	fread(&BNext, sizeof(BOOL),1,fp);
	if(BNext)
	{
		pNext = new MapLocator();
		pNext->Load(fp);
	}
}
