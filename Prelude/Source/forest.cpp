#include "forest.h"
#include <assert.h>

void Forest::Load(const char *ForestName)
{
	FILE *fp;
	fp = SafeFileOpen("forests.txt","rt");

	char *NewName;
	int n;

	while(1)
	{
		if(SeekTo(fp,"[Forest]"))
		{
			SeekTo(fp,"Name:");
			SeekTo(fp,"\"");
			NewName = GetString(fp,'\"');
			if(!strcmp(NewName,ForestName))
			{
				strcpy(Name,NewName);
				delete NewName;
				
				SeekTo(fp,"NumTrees:");
				NumTrees = GetInt(fp);
				for(n = 0; n < NumTrees; n ++)
				{
					NewName = GetStringNoWhite(fp);
					ConvertToLowerCase(NewName);
					Trees[n] = Engine->GetMesh(NewName);
					delete NewName;
				}

				SeekTo(fp,"NumShrubs:");
				NumShrubs = GetInt(fp);
				for(n = 0; n < NumShrubs; n ++)
				{
					NewName = GetStringNoWhite(fp);
					ConvertToLowerCase(NewName);
					Shrubs[n] = Engine->GetMesh(NewName);
					delete NewName;
				}

				fclose(fp);
				return;
			}
			delete NewName;
		}
		else
		{
			fclose(fp);
			SafeExit("Forest does not exist\n");
		}
	}
	fclose(fp);
	return;
}


ZSModelEx *Forest::GetTree(float Density, int x, int y)
{
	//seed the random number generator with x + y + x % y
	if(Density < 0.01 || !NumTrees)
	{
		return NULL;
	}

	float PercentResult;

	int Tree;

	PercentResult = (float)(rand() % 500)/100.0f;

	if(PercentResult < Density)
	{
		Tree = rand() % NumTrees;
		return Trees[Tree];
	}

	return NULL;
}

ZSModelEx *Forest::GetShrub(float Density, int x, int y)
{
	if(Density < 0.01f || !NumShrubs)
	{
		return NULL;
	}
	
	float PercentResult;

	int Tree;

	PercentResult = (float)(rand() % 300)/100.0f;

	if(PercentResult < Density)
	{
		Tree = rand() % NumShrubs;
		return Shrubs[Tree];
	}

	return NULL;
}

ZSModelEx *Forest::GetTree()
{
	if(NumTrees)
	{
		int Tree;
		Tree = rand() % NumTrees;
		return Trees[Tree];
	}
	else
		return NULL;

}

ZSModelEx *Forest::GetShrub()
{
	if(NumShrubs)
	{
		int Tree;
		Tree = rand() % NumShrubs;
		return Shrubs[Tree];
	}
	else
		return NULL;

}



Forest::Forest()
{
	NumTrees = 0;

	NumShrubs = 0;
}

Forest *Forest::LoadAll(int *NumForests)
{
	FILE *fp;

	fp = SafeFileOpen("forests.txt","rt");

	//count the number of forests;
	
	*NumForests = 0;

	while(SeekTo(fp,"[Forest]"))
	{
		*NumForests += 1;
	}

	fseek(fp,0,SEEK_SET);

	Forest *pFArray;

	pFArray = new Forest[*NumForests];

	char *NewName;
	int tn;
	int n;

	for(n = 0; n < *NumForests; n++)
	{
		SeekTo(fp,"Name:");
		SeekTo(fp,"\"");
		NewName = GetString(fp,'\"');
		strcpy(pFArray[n].Name,NewName);
		delete NewName;
			
		SeekTo(fp,"NumTrees:");
		pFArray[n].NumTrees = GetInt(fp);
		for(tn = 0; tn < pFArray[n].NumTrees; tn ++)
		{
			NewName = GetStringNoWhite(fp);
			ConvertToLowerCase(NewName);
			pFArray[n].Trees[tn] = Engine->GetMesh(NewName);
			delete NewName;
		}

		SeekTo(fp,"NumShrubs:");
		pFArray[n].NumShrubs = GetInt(fp);
		for(tn = 0; tn < pFArray[n].NumShrubs; tn ++)
		{
			NewName = GetStringNoWhite(fp);
			ConvertToLowerCase(NewName);
			pFArray[n].Shrubs[tn] = Engine->GetMesh(NewName);
			delete NewName;
		}
	}
	fclose(fp);
	return pFArray;
}
