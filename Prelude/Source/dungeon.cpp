#include "dungeon.h"

BOOL Dungeon::Load(FILE *fp);

BOOL Dungeon::Save(FILE *fp);

BOOL Dungeon::SaveNonStatic(FILE *fp);
BOOL Dungeon::LoadNonStatic(FILE *fp);

Dungeon::Dungeon()
{
 Name[32] = '\0';
 Width = 0;
 Height = 0;
 pMap = NULL;
 pUpdateSegments = NULL;
 Regions = NULL;
}

Dungeon::~Dungeon()
{
	if(pMap)
	{
		delete[] pMap;
	}

	if(pUpdateSegments)
	{
		delete[] pUpdateSegments;
	}

	if(Regions)
	{
		delete[] Regions;
	}

}

void Dungeon::Draw()
{


}

void Dungeion::Update()
{

}

void Dungeiong::

