//********************************************************************* 
//*                                                                                                                                    ** 
//**************			world.h				        ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:																			  
//*Revisor:                    
//*Purpose:                   
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*																						  
//********************************************************************* 
//*********************************************************************
#ifndef WORLD_H
#define WORLD_H

#include "chunks.h"
#include "zsengine.h"
#include "area.h"

#define MAX_WORLD_WIDTH	  200
#define MAX_WORLD_HEIGHT  200
#define MAX_WORLD_REGIONS 1024

#define WORLD_CHUNK_WIDTH	200
#define WORLD_CHUNK_HEIGHT 200
#define WORLD_WIDTH	(WORLD_CHUNK_WIDTH * CHUNK_TILE_WIDTH)
#define WORLD_HEIGHT	(WORLD_CHUNK_HEIGHT * CHUNK_TILE_HEIGHT)
#define UPDATE_SEGMENT_WIDTH		64
#define UPDATE_SEGMENT_HEIGHT		64
#define WORLD_USEG_WIDTH (WORLD_WIDTH / UPDATE_SEGMENT_WIDTH)
#define WORLD_USEG_HEIGHT (WORLD_HEIGHT / UPDATE_SEGMENT_HEIGHT)

#define HOURS_PER_DAY	24
#define HOUR_LENGTH		60
#define DAY_LENGTH		24
#define DAY_MINUTE_LENGTH (HOURS_PER_DAY * HOUR_LENGTH)

typedef struct 
{
	char Name[32];
	int NumRegions;
	unsigned short Regions[WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT * NUM_CHUNK_REGIONS];
	int RegionOffsets[1024];
} REGION_FILE_HEADER_T;


class Region;
class Creature;
class Thing;
class Combat;

//preprocessor defs ***********************************************

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**					           world                        
//******************************************************************** 
//*Purpose:  Manage the representation of the game world and all objects within
//*			 it.                                                     
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                   
//********************************************************************

typedef enum
{
	TER_NONE = -1,
	TER_GRAVEL = 0,
	TER_PAVEDPINK,
	TER_PAVEDGREY,
	TER_COBBLESTONE,
	TER_BRICK,
	TER_GRASS_ONE,
	TER_GRASS_TWO,
	TER_GRASS_THREE,
	TER_GRASS_FOUR,
	TER_MOSS,
	TER_SAND,
	TER_DIRT,
	TER_STONE_ONE,
	TER_STONE_TWO,
	TER_STONE_THREE,
	TER_STONE_FOUR,
	TER_FARM,
	TER_MOSS_ONE,
	TER_MOSS_TWO,
	TER_ROCK_ONE,
	TER_ROCK_TWO,
	TER_ROCK_THREE,
	TER_ROCK_FOUR,
	TER_ROCK_FIVE,
	TER_STICK_ONE,
	TER_STICK_TWO,
	TER_FLOWERS_ONE,
	TER_FLOWERS_TWO
} TERRAIN_T;

const unsigned short TerrainIndex[TER_FLOWERS_TWO+1] = 
{
	0*64,
	1*64,
	2*64,
	3*64,
	4*64,
	5*64,
	6*64,
	7*64,
	8*64,
	9*64,
	10*64,
	11*64,
	12*64,
	12*64+16,
	12*64+32,
	12*64+48,
	13*64,
	13*64+16,
	13*64+32,
	13*64+48,
	14*64,
	14*64+16,
	14*64+32,
	14*64+48,
	15*64,
	15*64+16,
	15*64+32,
	15*64+48,
};

class World
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 
	int ScreenX;
	int ScreenY;

	D3DVECTOR vCamera;
	D3DVECTOR vLookAt;
	D3DVECTOR vWorldUp;
	D3DMATRIX matCamera;
	D3DVECTOR vCamOffset;
	
	float CameraAngle;

	D3DLIGHT7	Light[DAY_LENGTH];
	
	int Hour;

	unsigned long TotalTime;

	GAME_STATE_T GameState;

	int DrawRadius;

	BOOL RenderBackTerrain;

	RECT UpdateRect;

	int NumTowns;

	char TownNames[16][32];
	char TownMusic[16][32];
	
	RECT TownAreas[16];

	BOOL Roads[200][200];
	
	BOOL ShadowDraw;

	BOOL EdittingEnabled;

	int NumAreas;
	Area *Areas[64]; 
	//max 64 areas
	//shouldn't be a problem as areas can be of unlimitted size...

//region management stuff
	FILE *RegionFile;
	
	int NumRegions;
	int RegionOffsets[1024]; //max 1024 regions;
	Region *Regions[1024]; //max 1024 regions

//universal objects not in update list
	Object *pDrawList;

	Object *pMainObjects;

//combatmanager
	Combat *pCombat;

	BYTE CurAreaNum;

//for updates
	Object *pOffScreenCreature;
	int CleanX;
	int CleanY;

//for help tips
	BOOL CreationHelp;
	BOOL CharacterHelp;
	BOOL CombatHelp;
	BOOL BarterHelp;
	BOOL MapHelp;
	BOOL MainHelp;
	BOOL RestHelp;

	int AutosaveRate;
	unsigned long LastAutosave;

//for additional settings and info
	int Difficulty;
	BOOL XPConfirmation;
	BOOL VerboseCombatInfo;
	BOOL FullBodySelection;
	BOOL HighlightInformation;

//************************************************************************************** 

public:

// Display Functions -------------------------------
	
// Accessors ----------------------------------------

	int GetDifficulty() { return Difficulty; }
	BOOL XPConfirm() { return XPConfirmation; }
	BOOL VerboseCombat() { return VerboseCombatInfo; }
	BOOL FullSelect() { return FullBodySelection; }
	BOOL HighlightInfo() { return HighlightInformation; }

	void SetDifficulty(int newdif) { Difficulty = newdif; }
	void SetXPConfirm(BOOL newsetting) {  XPConfirmation =  newsetting; }
	void SetVerboseCombat(BOOL newsetting) {  VerboseCombatInfo =  newsetting; }
	void SetFullSelect(BOOL newsetting) {  FullBodySelection =  newsetting; }
	void SetHighlightInfo(BOOL newsetting) {  HighlightInformation =  newsetting; }


	int GetAutosaveRate();
	unsigned long GetLastAutosave();
	
	BOOL IsEdittingEnabled() { return EdittingEnabled; }

	BOOL GetCreationHelp() { return CreationHelp; }
	BOOL GetCharacterHelp() { return CharacterHelp; }
	BOOL GetCombatHelp() { return CombatHelp; }
	BOOL GetBarterHelp() { return BarterHelp; }
	BOOL GetMapHelp() { return MapHelp; }
	BOOL GetMainHelp() { return MainHelp; }
	BOOL GetRestHelp() { return RestHelp; }

	int PseudoRand(int modifier);

	int GetHour();
	int GetDay();
	int GetMinute();
	unsigned long GetTotalTime();
	
	void AdvanceTime(int Minutes);
	void IncrementTime();
	
	void SetHour(int NewHour);

	D3DLIGHT7 *GetLight(int Time);

	int SetLight(D3DLIGHT7 *NewLight);

	int CreateLights(float DiffuseFactor);
	
	D3DVECTOR GetCamera();
	D3DVECTOR GetCenterScreen();

	inline GAME_STATE_T GetGameState()
	{
		return GameState;
	}
	
	void ConvertMouseTile(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ);
	void ConvertMouse(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ);
	void ConvertToWorld(int MouseX, int MouseY, D3DVECTOR *vScreen);
	void ConvertToWorld(D3DVECTOR *vScreen);

	int GetScreenX() { return ScreenX; }
	int GetScreenY() { return ScreenY; }

	D3DVECTOR GetCameraOffset() { return vCamOffset; }
	float GetCameraAngle() { return CameraAngle; }
	void SetCameraAngle(float NewAngle);
	BOOL RenderingBackTerrain() { return RenderBackTerrain; }

	BOOL GetDrawShadows() { return ShadowDraw; }

	float GetScaling();

	Region * GetRegion(int Num) 
	{ 
		if(Regions[Num]) 
			return Regions[Num]; 
		else LoadRegion(Num); 
			return Regions[Num]; 
	}

	Object *GetActive();

	Object *GetDrawList() 
	{ 
		return pDrawList; 
	}

	Object *GetMainObjects()
	{
		return pMainObjects;
	}

	void AddToDrawList(Object *pToAdd);
	void RemoveFromDrawList(Object *pToRemove);

	int AddMainObject(Object *pToAdd);
	int RemoveMainObject(Object *pToRemove);

	Combat *GetCombat() { return pCombat; };

	int GetNumAreas() { return NumAreas; };

	BOOL InCombat();

//area controls

	void GotoArea(int AreaNum, int x, int y);
	int GetAreaNum(char *AreaName);
	int GetAreaNum(Area *pArea);

	Area *GetArea(int Num);
	Area *AddArea(char *AreaName);

	int GetCurAreaNum() { return CurAreaNum; }

	void GetNearestRoad(int CurX, int CurY, int *DropX, int *DropY);

// Mutators -----------------------------------------
	void SetEdittingEnabled(BOOL Enable) { EdittingEnabled = Enable; }
	void SetAutosaveRate(int NewRate) { AutosaveRate = NewRate; }

	void SetCreationHelp(BOOL NewHelp) { CreationHelp = NewHelp; }
	void SetCharacterHelp(BOOL NewHelp) { CharacterHelp = NewHelp; }
	void SetCombatHelp(BOOL NewHelp) { CombatHelp = NewHelp; }
	void SetBarterHelp(BOOL NewHelp) { BarterHelp = NewHelp; }
	void SetMapHelp(BOOL NewHelp) { MapHelp = NewHelp; }
	void SetMainHelp(BOOL NewHelp) { MainHelp = NewHelp; }
	void SetRestHelp(BOOL NewHelp) { RestHelp = NewHelp; }

	void LoadRegion(int Num);

	void SetDrawShadows(BOOL NewDraw) { ShadowDraw = NewDraw; }

	void LoadGame(const char *filename);
	
	int SetCameraPosition(D3DVECTOR NewPosition);
	int SetCenterScreen(D3DVECTOR NewCenterScreen);
	int RotateCamera(float Amount);
	int MoveCamera(int Direction, float Amount);

	void SetGameState(GAME_STATE_T NewGameState)
	{
		this->GameState = NewGameState;
	}

	int LookAt(Thing *pThing);

	void SetCameraOffset(D3DVECTOR NewOffset) { vCamOffset = NewOffset; }
	void UpdateCameraOffset(D3DVECTOR UpdateRay);
	void ToggleBackTerrain() { if(RenderBackTerrain) RenderBackTerrain = FALSE; else RenderBackTerrain = TRUE; }

	void ArcCamera(float ArcAmount);
	
	void GetUpdateRect(RECT *pRect) { *pRect = UpdateRect; }
	void UpdateOffScreenCreatures();
	void CleanOffScreenChunks();

	char *GetTown(int x, int y);

	char *GetMusic(int x, int y);


	int Update();

	int GetNextFreeRegion();
	void RemoveRegion(int ID);

	void LoadDynamic(FILE *fp);

	int Load(const char *filename);

	void ChangeCamera();

// Output ---------------------------------------------
	int Save(const char *filename);
	int Save(FILE *fp);
	void SaveGame(const char *filename, char *GameID = NULL);
	void AutoSave();
	void QuickSave();
	void SaveRegions(const char *filename);
	void SaveDynamic(FILE *fp);

// Constructors ---------------------------------------
	World();
	World(const char *filename);
	void LoadTowns();
	
	 
// Destructor -----------------------------------------
	~World();

// Debug ----------------------------------------------

// Editting
	void AddRegion(Region *ToAdd);
	void ClearRegions();
	void DefaultRegions(RECT *rArea);
	void AddRegion(RECT *rArea);

	void LoadRoads();

	void BringUpToDate();
	void CleanAreas();
	void CleanDynamic();

// Operators ------------------------------------------
	friend class Area;
	friend class Combat;
	
};

extern Area *Valley;
extern World *PreludeWorld;

//void _cdecl CleanWorld(void *pParam);



#endif
