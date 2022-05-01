//********************************************************************* 
//*                                                                                                                                    ** 
//**************				path.h							  ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:                                                                                            * 
//*Revisor:                                             
//*Purpose:                  
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*																						 
//********************************************************************* 
//*********************************************************************
#ifndef PATH_H
#define PATH_H

#include "defs.h"
#include "things.h"

//preprocessor defs ***********************************************

#define MAX_PATH_LENGTH 256
#define MAX_PATH_DEPTH_SEARCH 8000





//*******************************CLASS******************************** 
//**************                                 ********************* 
//**					                                  ** 
//******************************************************************** 
//*Purpose:                                                      
//******************************************************************** 
//*Invariants:                                       
//*                                                                  
//********************************************************************
struct NODE
{
	int x;  //coordinates
	int y;
	int gone;  //distance travelled so far
	int left;  //distance left (straight line estimate)
	int value; //total of gone + left
	
	DIRECTION_T fromdir; //the direction we travelled to get here
	
	NODE *parent;

	NODE *pNext;
	NODE *pPrev;
	
	NODE() { x = 0; y = 0; gone = 0; left = 0; value = 0; fromdir = DIR_NONE; parent = NULL; }
};

class Object;

class Path
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 
	Object *pTraveller;
	Object *pTarget;
	int TravellerSize;
	int TargetSize;
	int Length;
	int StartX;
	int StartY;
	int EndX;
	int EndY;
	D3DVECTOR StartVector;
	D3DVECTOR EndVector;
	int curnode;
	int PathLength;
	DIRECTION_T dirtravel[MAX_PATH_LENGTH];
	int PathX[MAX_PATH_LENGTH];
	int PathY[MAX_PATH_LENGTH];
	int MPUsed[MAX_PATH_LENGTH];
	int RangeNeeded;
	int ActionPoints;
	int MovePoints;
	int MaxDepth;

	BOOL FindPath(int x1, int y1, int x2, int y2,  BOOL (*TravelFunc)(int,int,int,int, float, Object *),float fRangeNeeded = 0.0f, Object *pTrav = NULL);
		
//************************************************************************************** 

public:

// Display Functions -------------------------------
	void Show();
	void Draw(Thing *pActive);
	
// Accessors ----------------------------------------
	void GetStart(int *pX, int *pY);
	void GetEnd(int *pX, int *pY);
	void GetCurXY(int *pX, int *pY);
	void GetNodeXY(int Node, int *pX, int *pY);
	int GetLength();
	DIRECTION_T GetNodeExit(int Node) { return dirtravel[Node]; }
	int GetCurNode() { return curnode; }
	BOOL IsOnPath(int x, int y);

	int GetTargetSize() { return TargetSize; }
	int GetTravellerSize() { return TravellerSize; }
	Object *GetTraveller() { return pTraveller; }
	Object *GetTarget() { return pTarget; }
	int GetDepth() { return MaxDepth; }
	
// Mutators -----------------------------------------
	//find a path, ignoring moveable objects
	BOOL FindPath(int x1, int y1, int x2, int y2, float fRange = 0.0f, Object *pTrav = NULL);
	BOOL FindLargePath(int x1, int y1, int x2, int y2, float fRange = 0.0f,Object *pTrav = NULL); //for creatures of size 2
	
	//find a path around moveable objects
	BOOL FindPathObjects(int x1, int y1, int x2, int y2, float fRange = 0.0f, Object *pTrav = NULL);
	BOOL FindLargePathObjects(int x1, int y1, int x2, int y2, float fRange = 0.0f, Object *pTrav = NULL); //for creatures of size 2
	
	//for Combat
	BOOL FindCombatPath(int x1, int y1, int x2, int y2, float fRange = 0.0f, Object *pTrav = NULL);
	BOOL FindLargeCombatPath(int x1, int y1, int x2, int y2, float fRange = 0.0f, Object *pTrav = NULL);

	//find a path around the current node
	BOOL PathAround();
	BOOL LargePathAround();

	//void FindPathTarget(int x1, int y1, int x2, int y2);

	int Traverse();
	void SetTargetSize(int NewSize) { TargetSize = NewSize; }
	void SetTravellerSize(int NewSize) { TravellerSize = NewSize; }
	void SetTarget(Object *pOb) { pTarget = pOb; }
	void SetTraveller(Object *pOb) {pTraveller = pOb; }

	void SetDepth(int NewDepth) { MaxDepth = NewDepth; }

	void SetEnd(int NewEndX, int NewEndY);
	
// Output ---------------------------------------------

  
// Constructors ---------------------------------------
	Path()
	{
		curnode = 0;
		PathLength = 0;
		TravellerSize = 0;
		pTraveller = NULL;
		TargetSize = 0;
		pTarget = 0;
		MaxDepth =  MAX_PATH_DEPTH_SEARCH;

	}
	
 
// Destructor -----------------------------------------


// Debug ----------------------------------------------


// Operators ------------------------------------------


};

extern int DistanceTable[MAX_PATH_LENGTH][MAX_PATH_LENGTH];

void FillDistanceTable();

extern inline int Distance(int x1,int y1, int x2, int y2);
extern void InitNodes();

#endif
