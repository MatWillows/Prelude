//********************************************************************* 
//********************************************************************* 
//**************                path.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision: 0.085                                                                                       * 
//*Revisor:                                               
//*Purpose:                        
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        			               
//*			
//********************************************************************* 
//*********************************************************************
//0.084	added seperate pathing types
//0.085 switched from dynamic memory to a static fund of nodes
//		switched to joint closed/open list

#include "path.h"
#include "world.h"
#include "zsutilities.h"
#include "zsengine.h"
#include "objects.h"
#include "combatmanager.h"
#include "creatures.h"
#include <assert.h>

#define PATH_MESH_NUMBER 2
#define MAX_HEIGHT_DIFFERENCE	0.75f

int DistanceTable[MAX_PATH_LENGTH][MAX_PATH_LENGTH];

#define COMBATPATHCONVERT(x,y) (y - ConvertY) * COMBAT_WIDTH + (x - ConvertX)

int UsedNodes;
NODE NodeList[8008]; //padded by eight for the surrounding nodes
int WorstNode;

NODE *ClosedList;
NODE *OpenList;

void InitNodes()
{
	UsedNodes = 0;
	WorstNode = 0;
	ClosedList = NULL;
	OpenList = NULL;
}

NODE *GetNewNode()
{
	UsedNodes++;
	NodeList[UsedNodes].parent = NULL;
	NodeList[UsedNodes].pNext = NULL;
	NodeList[UsedNodes].pPrev = NULL;
	return &NodeList[UsedNodes]; 
}

NODE *InOpen(int x,int y)
{
	NODE *pNode;
	pNode = OpenList;

	while(pNode)
	{
		if(pNode->x == x && pNode->y == y)
		{
			return pNode;
		}
		else
		{
			pNode = pNode->pNext;
		}
	}
	return NULL;
}

NODE *InClosed(int x, int y)
{
	NODE *pNode;
	pNode = ClosedList;
	while(pNode)
	{
		if(pNode->x == x && pNode->y == y)
		{
			return pNode;
		}
		else
		{
			pNode = pNode->pNext;
		}
	}
	return FALSE;
}

void RemoveFromList(NODE *pNode)
{
	if(!pNode->pPrev)
	{
		if(OpenList == pNode)
		{
			OpenList = pNode->pNext;
		}
		else
		if(ClosedList == pNode)
		{
			ClosedList = pNode->pNext;
		}
	}
	else
	{
		pNode->pPrev->pNext = pNode->pNext;
	}

	if(pNode->pNext)
	{
		pNode->pNext->pPrev = pNode->pPrev;
	}

	pNode->pNext = NULL;
	pNode->pPrev = NULL;
}

void AddToOpen(NODE  *pAdd)
{
	if(OpenList)
	{
		NODE *pNode;
		pNode = OpenList;
		if(pNode->value > pAdd->value)
		{
			pAdd->pNext = OpenList;
			OpenList = pAdd;
			pAdd->pPrev = NULL;
			pNode->pPrev = pAdd;
			return;
		}
		else
		while(pNode->pNext)
		{
			if(pNode->pNext->value > pAdd->value)
			{
				pAdd->pPrev = pNode;
				pAdd->pNext = pNode->pNext;
				pNode->pNext = pAdd;
				pAdd->pNext->pPrev = pAdd;
				return;
			}
			pNode = pNode->pNext;
		}
		pNode->pNext = pAdd;
		pAdd->pPrev = pNode;
		pAdd->pNext = NULL;
		return;
	}
	else
	{
		OpenList = pAdd;
		pAdd->pPrev = NULL;
		pAdd->pNext = NULL;
	}
	return;
}

void AddToClose(NODE *pNode)
{
	if(!InClosed(pNode->x,pNode->y))
	{
		pNode->pPrev = NULL;
		pNode->pNext = ClosedList;
		if(ClosedList)
		{
			ClosedList->pPrev = pNode;
		}
		ClosedList = pNode;
	}
}


inline int Distance(int x1,int y1, int x2, int y2)
{
	//use manHattandistance;
	//return 10*(abs(x1-x2) + abs(y1-y2));
/*
	int a = 10 * (x2-x1);
	int b = 10 * (y2-y1);
	return (int)sqrt(((a*a)+(b*b)));
*/
	int a = abs(x2-x1);
	int b = abs(y2-y1);
	return DistanceTable[a][b]; //(int)sqrt(((a*a)+(b*b)));
}

inline int LargeDistance(int x1,int y1, int x2, int y2)
{
	//use manHattandistance;
	//return 10*(abs(x1-x2) + abs(y1-y2));
/*
	int a = 10 * (x2-x1);
	int b = 10 * (y2-y1);
	return (int)sqrt(((a*a)+(b*b)));
*/
	int topleft, topright, bottomleft, bottomright;
	int a;
	int b;
	int c;
	int d;
	int distance;
	a = abs(x2-x1);
	b = abs(y2-y1);
	c = abs(x2-(x1 + 1));
	d = abs(y2-(y1 + 1));

	topleft = DistanceTable[a][b]; //(int)sqrt(((a*a)+(b*b)));
	topright = DistanceTable[c][b]; //(int)sqrt(((a*a)+(b*b)));
	bottomleft = DistanceTable[a][d]; //(int)sqrt(((a*a)+(b*b)));
	bottomright = DistanceTable[c][d]; //(int)sqrt(((a*a)+(b*b)));
	
	distance = topleft;
	if(distance > topright)
		distance = topright;

	if(distance > bottomleft)
		distance = bottomleft;
	
	if(distance > bottomright)
		distance = bottomright;

	return distance;
}


//calculate the distance if possible between two adjacent points
BOOL CanTravel(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller);
BOOL CanTravelObject(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller);
BOOL CanTravelLarge(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller);
BOOL CanTravelLargeObject(int FromX, int FromY, int ToX, int ToY, float FromHeight,Object *pTraveller);

FILE *pathdebug;


//in sorted order
void AddNode(int x, int y, int dx, int dy, DIRECTION_T FromDir, NODE *np)
{
	int newvalue;
	NODE *child;

	child = InOpen(x,y);
	if(!child)
	{
		child = InClosed(x,y);
	}
	
	int Travelled;
	if(np->x == x || np->y == y)
	{
		if(FromDir == np->fromdir)
		{
			Travelled = 8;
		}
		else
		{
			Travelled = 10;
		}
	}
	else
	{
		if(FromDir == np->fromdir)
		{
			Travelled = 12;
		}
		else
		{
			Travelled = 15;
		}

	}

	if(!child)
	{
		child = GetNewNode();
		child->x = x;
		child->y = y;
		child->gone = np->gone + Travelled;
		child->left = Distance(x,y,dx,dy);
		child->value = child->gone + child->left;
		child->parent = np;
		child->fromdir = FromDir;
		AddToOpen(child);
	}
	else
	{
		newvalue = np->gone + Travelled + Distance(x,y,dx,dy);

		if(child->value <= newvalue)
		{
			return;
		}
		else
		{
			child->x = x;
			child->y = y;
			child->gone = np->gone + Travelled;
			child->left = Distance(x,y,dx,dy);
			child->value = child->gone + child->left;
			child->parent = np;
			child->fromdir = FromDir;
			RemoveFromList(child);
			AddToOpen(child);
		}
	}

}

//in sorted order
void AddLargeNode(int x, int y, int dx, int dy, DIRECTION_T FromDir, NODE *np)
{
	int newvalue;
	NODE *child;

	child = InOpen(x,y);
	if(!child)
	{
		child = InClosed(x,y);
	}
	
	int Travelled;
	if(np->x == x || np->y == y)
	{
		if(FromDir == np->fromdir)
		{
			Travelled = 8;
		}
		else
		{
			Travelled = 10;
		}
	}
	else
	{
		if(FromDir == np->fromdir)
		{
			Travelled = 12;
		}
		else
		{
			Travelled = 15;
		}

	}

	if(!child)
	{
		child = GetNewNode();
		child->x = x;
		child->y = y;
		child->gone = np->gone + Travelled;
		child->left = LargeDistance(x,y,dx,dy);
		child->value = child->gone + child->left;
		child->parent = np;
		child->fromdir = FromDir;
		AddToOpen(child);
	}
	else
	{
		newvalue = np->gone + Travelled + Distance(x,y,dx,dy);

		if(child->value <= newvalue)
		{
			return;
		}
		else
		{
			child->x = x;
			child->y = y;
			child->gone = np->gone + Travelled;
			child->left = LargeDistance(x,y,dx,dy);
			child->value = child->gone + child->left;
			child->parent = np;
			child->fromdir = FromDir;
			RemoveFromList(child);
			AddToOpen(child);
		}
	}
}


//************** static Members *********************************

//************** Constructors  ****************************************

//simple constructor
//inlined in path.h

//copy constructor


//end:  Constructors ***************************************************



//*************** Destructor *******************************************


//end:  Destructor *****************************************************



//************  Accessors  *********************************************
void Path::GetStart(int *pX, int *pY)
{
	*pX = StartX;
	*pY = StartY;
}

void Path::GetEnd(int *pX, int *pY)
{
	*pX = EndX;
	*pY = EndY;
}

int Path::GetLength()
{
	return PathLength;
}

void Path::GetCurXY(int *pX, int *pY)
{
	if(curnode <= PathLength)
	{
		*pX = PathX[curnode];
		*pY = PathY[curnode];
	}
}

void Path::GetNodeXY(int Node, int *pX, int *pY)
{
	if(Node <= PathLength)
	{
		*pX = PathX[Node];
		*pY = PathY[Node];
	}
}

BOOL Path::IsOnPath(int x, int y)
{

	for(int n = 0; n <= PathLength; n++)
	{
		if(PathX[n] == x && PathY[n] == y)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//end: Accessors *******************************************************



//************  Display Functions  *************************************

void Path::Show()
{
	DEBUG_INFO("Starting Show Path...\n");
	char blarg[64];
	char temp[16];
	sprintf(blarg,"Start: %i x %i\nEnd: %i x %i\nCurrent: %i\n",StartX, StartY,EndX,EndY,curnode);
	for(int n = 0; n <= PathLength; n++)
	{
		sprintf(temp,"%i: ",n);
		strcat(blarg,temp);
		switch(dirtravel[n])
		{
		case NORTH:
			strcat(blarg,"N");
			break;
		case SOUTH:
			strcat(blarg,"S");
			break;
		case EAST:
			strcat(blarg,"E");
			break;
		case WEST:
			strcat(blarg,"W");
			break;
		case NORTHEAST:
			strcat(blarg,"NE");
			break;
		case SOUTHWEST:
			strcat(blarg,"SW");
			break;
		case SOUTHEAST:
			strcat(blarg,"SE");
			break;
		case NORTHWEST:
			strcat(blarg,"NW");
			break;
		default:
			strcat(blarg, " ");
			break;
		}
		sprintf(temp," %i x %i",PathX[n],PathY[n]);
		strcat(blarg,temp);
		strcat(blarg,"\n");
	}
	//GOT_HERE(blarg, NULL);
	DEBUG_INFO(blarg);
}

void Path::Draw(Thing *pActive)
{
	if(PathLength == 666)
	{
		return;
	}
	ZSModelEx *pMesh;

	pMesh = Engine->GetMesh("sphere");

	float ZOff;

	ZOff = -(pMesh->GetHeight() / 2);

	int AllEnd = 0;
	int StrongEnd = 0;
	int NormalEnd = 0;
	int QuickEnd = 0;
	int MoveEnd = 0;

	//calculate the last point we can actually move to
	int MP;
	MP = pActive->GetData(INDEX_MOVEPOINTS).Value;
	int AP;
	AP = pActive->GetData(INDEX_ACTIONPOINTS).Value;

	MoveEnd = 0;
	
	if(AP < MP)
	{
		return;
	}

	int AttackP;
	AttackP = pActive->GetData(INDEX_ATTACKPOINTS).Value;

	MoveEnd = 1;
	AP -= MPUsed[MoveEnd];
	while(AP >= AttackP && MoveEnd <= PathLength)
	{
		MoveEnd++;
		NormalEnd = MoveEnd;
		AP -= MPUsed[MoveEnd];
	}
	while(AP > 0 && MoveEnd <= PathLength)
	{
		MoveEnd++;
		AP -= MPUsed[MoveEnd];
	}

//	MoveEnd = pActive->GetData(INDEX_ACTIONPOINTS).Value / pActive->GetData(INDEX_MOVEPOINTS).Value;

	//calculate the last point where a strong attack can be done

	//calculate the last point where a normal attack can be done

//	NormalEnd = MoveEnd - pActive->GetData(INDEX_ATTACKPOINTS).Value / pActive->GetData(INDEX_MOVEPOINTS).Value;

	//calculate the last point where a quick attack can be done
	
	if(MoveEnd > PathLength)
	{
		MoveEnd = PathLength;
	}
	
	if(NormalEnd > PathLength)
	{
		NormalEnd = PathLength;
	}

	Engine->Graphics()->ClearTexture();

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_GREEN));

	int n;
	for(n = 1; n < NormalEnd; n++)
	{
		pMesh->Draw(Engine->Graphics()->GetD3D(),(float)PathX[n] + 0.5f,(float)PathY[n] + 0.5f,(float)ZOff + Valley->GetTileHeight(PathX[n],PathY[n]),0,0);
	}

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));

	for(; n <= MoveEnd; n++)
	{
		pMesh->Draw(Engine->Graphics()->GetD3D(),(float)PathX[n] + 0.5f,(float)PathY[n] + 0.5f,(float)ZOff  + Valley->GetTileHeight(PathX[n],PathY[n]),0,0);
	}

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
	
	for(; n <= PathLength; n++)
	{
		pMesh->Draw(Engine->Graphics()->GetD3D(),(float)PathX[n] + 0.5f,(float)PathY[n] + 0.5f,(float)ZOff + Valley->GetTileHeight(PathX[n],PathY[n]),0,0);
	}

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

}


//end: Display functions ***********************************************



//************ Mutators ************************************************
BOOL Path::FindPath(int x1, int y1, int x2, int y2, BOOL (*TravelFunc)(int,int,int,int,float, Object *), float fRangeNeeded, Object *pTrav)
{
	//confirm that all parameters lie within the actual world
	
	pTraveller = pTrav;
	RangeNeeded = (int)(fRangeNeeded * 10.0f);

	if(abs(x1-x2) > 128 || abs(y1-y2) > 128)
	{
		PathLength = 666;
		return FALSE;
	}

	InitNodes();
	NODE *np = NULL;
	NODE *child = NULL;
	int dx = x2;
	int dy = y2;
	np = GetNewNode();  //start
	np->x = x1;
	np->y = y1;
	np->gone = 0;
	np->left = Distance(x1,y1,dx,dy);
	np->value = np->gone+np->left;
	np->fromdir = DIR_NONE;
	AddToOpen(np);
	np = NULL;
	int checked = 0;
	int pathoffset;

	while(OpenList && checked < MaxDepth)
	{
		np = OpenList;
		RemoveFromList(np);
		AddToClose(np);

		if((np->x == dx && np->y == dy) ||
			np->left <= RangeNeeded)
		{
			//back down path and construct it.
			NODE *pnode = np;
			pathoffset = 0;
			while(pnode->parent)
			{
				pathoffset++;
				pnode = pnode->parent;
			}
			pnode = np;
			//pathoffset;
			PathLength = pathoffset;
			if(PathLength >= MAX_PATH_LENGTH)
			{
				PathLength = 666;
				return FALSE;
			}

			while(pathoffset > 0)
			{
				dirtravel[pathoffset] = pnode->fromdir;
				PathX[pathoffset] = pnode->x;
				PathY[pathoffset] = pnode->y;

				pnode = pnode->parent;
				pathoffset--;
			}
			PathX[pathoffset] = x1;
			PathY[pathoffset] = y1;
			dirtravel[pathoffset] = DIR_NONE;
		
			PathX[PathLength] = np->x;
			PathY[PathLength] = np->y;

			StartX = x1;
			StartY = y1;
			
			EndX = np->x;
			EndY = np->y;
			return TRUE;			
		}

		int ToX, ToY;
		int FromX, FromY;
		FromX = np->x;
		FromY = np->y;
		//North Neighbor
		ToX = np->x;
		ToY = np->y - 1;
		float MyHeight = 0.0f;
//		MyHeight = Valley->GetTileHeight(FromX,FromY);

		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller))// || (ToX == dx && ToY == dy))
		{
			AddNode(ToX,ToY,dx,dy,NORTH,np);
		}

		//South Neighbor
		ToX = np->x;
		ToY = np->y + 1;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller))// || (ToX == dx && ToY == dy))
		{
			AddNode(ToX,ToY,dx,dy,SOUTH,np);
		}

		//East Neighbor
		ToX = np->x + 1;
		ToY = np->y;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller))// || (ToX == dx && ToY == dy))
		{
			AddNode(ToX,ToY,dx,dy,EAST,np);
		}

		//West Neighbor
		ToX = np->x - 1;
		ToY = np->y;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller))// || (ToX == dx && ToY == dy))
		{
			AddNode(ToX,ToY,dx,dy,WEST,np);
		}

		//NorthEAST
		ToX = np->x + 1;
		ToY = np->y - 1;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller) //|| ((ToX == dx && ToY == dy) 
			&& !(Valley->GetBlocking(FromX,ToY) && Valley->GetBlocking(ToX,FromY)))//)
		{
			AddNode(ToX,ToY,dx,dy,NORTHEAST,np);
		}
		
		//Northwest
		ToX = np->x - 1;
		ToY = np->y - 1;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller) //|| ((ToX == dx && ToY == dy) 
			&& !(Valley->GetBlocking(FromX,ToY) && Valley->GetBlocking(ToX,FromY)))//)
		{
			AddNode(ToX,ToY,dx,dy,NORTHWEST,np);
		}

		//SouthEast
		ToX = np->x + 1;
		ToY = np->y + 1;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller) //|| ((ToX == dx && ToY == dy)
			&& !(Valley->GetBlocking(FromX,ToY) && Valley->GetBlocking(ToX,FromY)))//)
		{
			AddNode(ToX,ToY,dx,dy,SOUTHEAST,np);
		}
		
		//SouthWest
		ToX = np->x - 1;
		ToY = np->y + 1;
		if(TravelFunc(FromX,FromY,ToX,ToY,MyHeight, pTraveller) //|| ((ToX == dx && ToY == dy) 
			&& !(Valley->GetBlocking(FromX,ToY) && Valley->GetBlocking(ToX,FromY)))//)
		{
			AddNode(ToX,ToY,dx,dy,SOUTHWEST,np);
		}

		checked += 8;
	}

	PathLength = 666;
	return FALSE;
}

int Path::Traverse()
{
	if(curnode >= PathLength)
	{
		DEBUG_INFO("Attempting to walk beyond path");
		exit(1);
	}
	curnode++;
	return dirtravel[curnode];
}

BOOL Path::FindPath(int x1, int y1, int x2, int y2, float frange, Object *pTrav)
{
	return FindPath(x1,y1,x2,y2, CanTravel, frange, pTrav);
}//for creatures of size 2


BOOL Path::FindLargePath(int x1, int y1, int x2, int y2, float frange, Object *pTrav)
{
	return FindPath(x1,y1,x2,y2, CanTravelLarge,  frange,pTrav);
}//for creatures of size 2
	
//find a path around moveable objects
BOOL Path::FindPathObjects(int x1, int y1, int x2, int y2, float frange, Object *pTrav)
{
	return FindPath(x1,y1,x2,y2, CanTravelObject, frange,pTrav);
}

BOOL Path::FindLargePathObjects(int x1, int y1, int x2, int y2, float frange, Object *pTrav)
{
	return FindPath(x1,y1,x2,y2, CanTravelLargeObject, frange, pTrav);
}//for creatures of size 2
	
//find a path around the current node
BOOL Path::PathAround()
{

	return FALSE;
}

BOOL Path::LargePathAround()
{

	return FALSE;
}




//end: Mutators ********************************************************



//************ Outputs *************************************************

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************

//helpers
void FillDistanceTable()
{
	int xn;
	int yn;
	int a, b;
	for(yn = 0; yn < MAX_PATH_LENGTH; yn++)
	for(xn = 0; xn < MAX_PATH_LENGTH; xn++)
	{
		a = xn * 10;
		b = yn * 10;
		DistanceTable[xn][yn] = (int)sqrt((a*a) + (b*b));
	}
}






BOOL CanTravel(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller)
{

	if(Valley->GetBlocking(ToX, ToY))
	{
		return FALSE;
	}

	return !(Valley->GetBlocking(FromX,ToY) && Valley->GetBlocking(ToX,FromY));
}

BOOL CanTravelObject(int FromX, int FromY, int ToX, int ToY, float FromHeight,Object *pTraveller)
{
	if(!Valley->IsClear(ToX, ToY, pTraveller))
	{
		return FALSE;
	}
	return (Valley->IsClear(FromX,ToY, pTraveller) && Valley->IsClear(ToX,FromY,pTraveller));
}

BOOL CanTravelLarge(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller)
{
	//large objects are always anchored in the upperneft (northwest) corner
	if(Valley->GetBlocking(ToX, ToY) ||
	   Valley->GetBlocking(ToX+1, ToY) ||
	   Valley->GetBlocking(ToX, ToY+1) ||
	   Valley->GetBlocking(ToX+1, ToY+1))
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CanTravelLargeObject(int FromX, int FromY, int ToX, int ToY, float FromHeight, Object *pTraveller)
{
//large objects are always anchored in the upperneft (northwest) corner
	if(!Valley->IsClear(ToX, ToY, pTraveller) ||
	   !Valley->IsClear(ToX+1, ToY, pTraveller) ||
	   !Valley->IsClear(ToX, ToY+1, pTraveller) ||
	   !Valley->IsClear(ToX+1, ToY+1, pTraveller))
	{
		return FALSE;
	}
	return TRUE;
}

//for Combat
BOOL Path::FindCombatPath(int x1, int y1, int x2, int y2, float fRange, Object *pTrav)
{
	//confirm that all parameters lie within the actual world
	pTraveller = pTrav;
	RangeNeeded = (int)(fRange * 10.0f);

	RECT rCombat;
	PreludeWorld->GetCombat()->GetCombatRect(&rCombat);

	if(x1 < rCombat.left || x2 < rCombat.left ||
		y1 < rCombat.top || y2 < rCombat.top ||
		x1 >= rCombat.right || x2 >= rCombat.right ||
		y1 >= rCombat.bottom || y2 >= rCombat.bottom)
	{
		PathLength = 666;
		return FALSE;
	}

	if(pTrav)
	{
		Creature *pCreature;
		pCreature = (Creature *)pTrav;
		ActionPoints = pCreature->GetData(INDEX_ACTIONPOINTS).Value;
		MovePoints = pCreature->GetData(INDEX_MOVEPOINTS).Value;
	}
	else
	{
		ActionPoints = 128;
		MovePoints = 1;
	}

	unsigned short *CreatureArea;
	CreatureArea = PreludeWorld->GetCombat()->CreatureArea;
	Combat *pCombat = PreludeWorld->GetCombat();

	InitNodes();
	NODE *np = NULL;
	NODE *child = NULL;
	int dx = x2;
	int dy = y2;
	np = GetNewNode();  //start
	np->x = x1;
	np->y = y1;
	np->gone = 0;
	np->left = Distance(x1,y1,dx,dy);
	np->value = np->gone+np->left;
	np->fromdir = DIR_NONE;
	AddToOpen(np);
	np = NULL;
	int checked = 0;
	int pathoffset;

	int ConvertX;
	ConvertX = pCombat->rCombat.left;
	int ConvertY;
	ConvertY = pCombat->rCombat.top;


	while(OpenList && checked < MaxDepth)
	{
		np = OpenList;
		RemoveFromList(np);
		AddToClose(np);

		if(np->left <= RangeNeeded && pCombat->CheckLineOfSight(np->x,np->y,x2,y2, NULL,NULL))
		{
			//back down path and construct it.
			EndX = np->x;
			EndY = np->y;
			NODE *pnode = np;
			pathoffset = 0;
			while(pnode->parent)
			{
				pathoffset++;
				pnode = pnode->parent;
			}
			pnode = np;
			//pathoffset;
			PathLength = pathoffset;
			if(PathLength >= MAX_PATH_LENGTH)
			{
				PathLength = 666;
				return FALSE;
			}

			while(pathoffset > 0)
			{
				dirtravel[pathoffset] = pnode->fromdir;
				PathX[pathoffset] = pnode->x;
				PathY[pathoffset] = pnode->y;
				switch(dirtravel[pathoffset])
				{
				case N:
				case S:
				case E:
				case W:
					MPUsed[pathoffset] = MovePoints;
					break;
				default:
					MPUsed[pathoffset] = MovePoints + MovePoints / 2;
					break;
				}

				pnode = pnode->parent;
				pathoffset--;
			}
			PathX[pathoffset] = x1;
			PathY[pathoffset] = y1;
			dirtravel[pathoffset] = DIR_NONE;
		
			PathX[PathLength] = np->x;
			PathY[PathLength] = np->y;

			StartX = x1;
			StartY = y1;
			
			return TRUE;			
		}


		int ToX, ToY;
		int FromX, FromY;
		FromX = np->x;
		FromY = np->y;
		//North Neighbor
		ToX = np->x;
		ToY = np->y - 1;

		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,NORTH,np);
		}

		//South Neighbor
		ToX = np->x;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,SOUTH,np);
		}

		//East Neighbor
		ToX = np->x + 1;
		ToY = np->y;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,EAST,np);
		}

		//West Neighbor
		ToX = np->x - 1;
		ToY = np->y;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,WEST,np);
		}

		//NorthEAST
		ToX = np->x + 1;
		ToY = np->y - 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) 
			&& !(CreatureArea[COMBATPATHCONVERT(FromX,ToY)] & CreatureArea[COMBATPATHCONVERT(ToX,FromY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,NORTHEAST,np);
		}
		
		//Northwest
		ToX = np->x - 1;
		ToY = np->y - 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) 
			&& !(CreatureArea[COMBATPATHCONVERT(FromX,ToY)] & CreatureArea[COMBATPATHCONVERT(ToX,FromY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,NORTHWEST,np);
		}

		//SouthEast
		ToX = np->x + 1;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) 
			&& !(CreatureArea[COMBATPATHCONVERT(FromX,ToY)] & CreatureArea[COMBATPATHCONVERT(ToX,FromY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,SOUTHEAST,np);
		}
		
		//SouthWest
		ToX = np->x - 1;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) 
			&& !(CreatureArea[COMBATPATHCONVERT(FromX,ToY)] & CreatureArea[COMBATPATHCONVERT(ToX,FromY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddNode(ToX,ToY,dx,dy,SOUTHWEST,np);
		}

		checked += 8;
	}

	PathLength = 666;
	return FALSE;

}

BOOL Path::FindLargeCombatPath(int x1, int y1, int x2, int y2, float fRange, Object *pTrav)
{
	//confirm that all parameters lie within the actual world
	pTraveller = pTrav;
	RangeNeeded = (int)(fRange * 10.0f);

	RECT rCombat;
	PreludeWorld->GetCombat()->GetCombatRect(&rCombat);

	if(x1 < rCombat.left || x2 < rCombat.left ||
		y1 < rCombat.top || y2 < rCombat.top ||
		x1 >= rCombat.right || x2 >= rCombat.right ||
		y1 >= rCombat.bottom || y2 >= rCombat.bottom)
	{
		PathLength = 666;
		return FALSE;
	}

	if(pTrav)
	{
		Creature *pCreature;
		pCreature = (Creature *)pTrav;
		ActionPoints = pCreature->GetData(INDEX_ACTIONPOINTS).Value;
		MovePoints = pCreature->GetData(INDEX_MOVEPOINTS).Value;
	}
	else
	{
		ActionPoints = 128;
		MovePoints = 1;
	}

	unsigned short *CreatureArea;
	CreatureArea = PreludeWorld->GetCombat()->CreatureArea;
	Combat *pCombat = PreludeWorld->GetCombat();

	InitNodes();
	NODE *np = NULL;
	NODE *child = NULL;
	int dx = x2;
	int dy = y2;
	np = GetNewNode();  //start
	np->x = x1;
	np->y = y1;
	np->gone = 0;
	np->left = LargeDistance(x1,y1,dx,dy);
	np->value = np->gone+np->left;
	np->fromdir = DIR_NONE;
	AddToOpen(np);
	np = NULL;
	int checked = 0;
	int pathoffset;

	int ConvertX;
	ConvertX = pCombat->rCombat.left;
	int ConvertY;
	ConvertY = pCombat->rCombat.top;


	while(OpenList && checked < MaxDepth)
	{
		np = OpenList;
		RemoveFromList(np);
		AddToClose(np);

		if(np->left <= RangeNeeded && pCombat->CheckLineOfSight(np->x,np->y,x2,y2, pTrav, NULL))
		{
			//back down path and construct it.
			EndX = np->x;
			EndY = np->y;
			NODE *pnode = np;
			pathoffset = 0;
			while(pnode->parent)
			{
				pathoffset++;
				pnode = pnode->parent;
			}
			pnode = np;
			//pathoffset;
			PathLength = pathoffset;
			if(PathLength >= MAX_PATH_LENGTH)
			{
				PathLength = 666;
				return FALSE;
			}

			while(pathoffset > 0)
			{
				dirtravel[pathoffset] = pnode->fromdir;
				PathX[pathoffset] = pnode->x;
				PathY[pathoffset] = pnode->y;
				switch(dirtravel[pathoffset])
				{
				case N:
				case S:
				case E:
				case W:
					MPUsed[pathoffset] = MovePoints;
					break;
				default:
					MPUsed[pathoffset] = MovePoints + MovePoints / 2;
					break;
				}

				pnode = pnode->parent;
				pathoffset--;
			}
			PathX[pathoffset] = x1;
			PathY[pathoffset] = y1;
			dirtravel[pathoffset] = DIR_NONE;
		
			PathX[PathLength] = np->x;
			PathY[PathLength] = np->y;

			StartX = x1;
			StartY = y1;
			
			EndX = np->x;
			EndY = np->y;
			
			
			return TRUE;			
		}


		int ToX, ToY;
		int FromX, FromY;
		FromX = np->x;
		FromY = np->y;
		//North Neighbor
		ToX = np->x;
		ToY = np->y - 1;

		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED))
			&&
		   !(CreatureArea[COMBATPATHCONVERT(ToX + 1,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,NORTH,np);
		}

		//South Neighbor
		ToX = np->x;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY + 1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX + 1,ToY + 1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,SOUTH,np);
		}

		//East Neighbor
		ToX = np->x + 1;
		ToY = np->y;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX + 1,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX + 1,ToY + 1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,EAST,np);
		}

		//West Neighbor
		ToX = np->x - 1;
		ToY = np->y;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX,ToY + 1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,WEST,np);
		}

		//NorthEAST
		ToX = np->x + 1;
		ToY = np->y - 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX+1,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX+1,ToY+1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,NORTHEAST,np);
		}
		
		//Northwest
		ToX = np->x - 1;
		ToY = np->y - 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX+1,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX,ToY+1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,NORTHWEST,np);
		}

		//SouthEast
		ToX = np->x + 1;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX+1,ToY+1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX+2,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX,ToY+2)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,SOUTHEAST,np);
		}
		
		//SouthWest
		ToX = np->x - 1;
		ToY = np->y + 1;
		if(!(CreatureArea[COMBATPATHCONVERT(ToX,ToY)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX,ToY+1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)) &&
			!(CreatureArea[COMBATPATHCONVERT(ToX + 1, ToY + 1)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
		{
			AddLargeNode(ToX,ToY,dx,dy,SOUTHWEST,np);
		}

		checked += 8;
	}

	PathLength = 666;
	return FALSE;
}

void Path::SetEnd(int NewEndX, int NewEndY)
{
	if(NewEndX == EndX && NewEndY == EndY)
		return;

	int n;

	n = 0;

	while(n < this->GetLength() && (PathX[n] != NewEndX || PathY[n] != NewEndY))
	{
		n++;
	}

	if(n >= this->GetLength())
	{
		DEBUG_INFO("Attempted to set path end beyond length.\n");
		return;
	}

	Length = PathLength = n + 1;

	EndX = NewEndX;
	EndY = NewEndY;

}


