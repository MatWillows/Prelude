#ifndef DUNGEON_H
#define DUNGEON_H

#include "chunks.h"
#include "world.h"

class Region;
class Object;

class Dungeon : public world
{
private:
	char Name[32];
	
	int Width;
	int Height;

	Chunk **pMap;  
	
	Object **pUpdateObjects;
	
	unsigned short *Regions;
	
public:

	BOOL Load(FILE *fp);

	BOOL Save(FILE *fp);

	BOOL SaveNonStatic(FILE *fp);
	BOOL LoadNonStatic(FILE *fp);

	Dungeon();
	~Dungeon();

};

#endif
