#ifndef FOREST_H
#define FOREST_H

#include "zsutilities.h"
#include "zsengine.h"


class Forest
{
public:
	char Name[64];
	int NumTrees;
	ZSModelEx *Trees[32];

	int NumShrubs;
	ZSModelEx *Shrubs[32];
	
	void Load(const char *Name);

	ZSModelEx *GetTree(float Density, int x, int y);

	ZSModelEx *GetShrub(float Density, int x, int y);

	ZSModelEx *GetTree();

	ZSModelEx *GetShrub();

	
	Forest();

	static Forest *LoadAll(int *NumForests);

};


#endif