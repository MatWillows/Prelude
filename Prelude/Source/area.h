//area is the generic class for an area.
//replaces the basic world class
//
#ifndef AREA_H
#define AREA_H

#include "chunks.h"
#include "zsengine.h"
#include "world.h"
#include "objects.h"

typedef struct
{
	char Name[32];
	int Width;
	int Height;
	int ChunkWidth;
	int ChunkHeight;
	int *ChunkOffsets;

} STATIC_FILE_HEADER_T;


class Region;
class Creature;
class Dungeon;
class Thing;

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**					           area                        
//******************************************************************** 
//*Purpose:  Manage the representation of sections of the game world and all objects within
//*			 it.                                                     
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                   
//********************************************************************

class Area
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 
	int TextureNum;
	ZSTexture *pBaseTexture;
	
	float HightLightTextureCoordinates[8];

	D3DDRAWPRIMITIVESTRIDEDDATA HighLightDataInfo;
	unsigned short HighLightDrawList[6];

	STATIC_FILE_HEADER_T Header;
		
	int Width;	//in tiles
	int Height;	//in tiles
	int ChunkWidth;
	int ChunkHeight;
	int UpdateWidth;
	int UpdateHeight;
	
	Chunk **BigMap;
	
	//lists of all the things by area.
	Object **UpdateSegments;

	FILE *StaticFile;

	unsigned short *Regions;

	int AreaID;

//************************************************************************************** 

public:

// Display Functions -------------------------------
	void Draw();
	void DrawTerrain();
	void DrawRegions();
	void ConvertChunks();

	//int Draw(HDC hdc);
	void DrawShadows();
	void ChangeCamera();
	void HighlightTile(int x, int y);
	void CombatHighlightTile(int x, int y);
	void CircleTile(int x, int y, COLOR_T Color);
	void DrawTiles();
	void DrawAltHighlight();

// Accessors ----------------------------------------
	char *GetName() { return Header.Name; };
	int GetTexture() { return TextureNum; };
	Object *GetUpdateSegment(int SegX, int SegY);

	Object *GetEvent(float x, float y);

	void AdvanceTime(int Minutes);

	void SetHour(int NewHour);

	D3DLIGHT7 *GetLight(int Time);

	int SetLight(D3DLIGHT7 *NewLight);
	
	int GetWidth();
	int GetHeight();
	
	float GetTileHeight(int TileX, int TileY);
	float GetTileHeight(int TileX, int TileY, DIRECTION_T Side);
	
	float GetZ(float x, float y);

	void SetTileHeight(int TileX, int TileY, float NewHeight);
	void RaiseTileHeight(int TileX, int TileY, float NewHeight);
		
	int GetTileBlocking(int TileX, int TileY);
	unsigned short *GetOverlays(int x, int y);
	int GetTerrain(int x,int y);
	void SetTerrain(int x, int y, int NewVal);
	
	Chunk * GetChunk(int x, int y); 
	
	BOOL IsClear(int x, int y, Object *pTester = NULL);

	Object *FindThing(int x, int y);
	Object *FindNextThing(Object *pThing, int x, int y);
	Object *FindObject(int x, int y, OBJECT_T oType);
	Object *FindOtherObject(int x, int y, OBJECT_T oType, Object *pExclude);
	

	void CreateChunks();
	void ResetChunk(int x, int y)
	{
//		MapChunks[x][y].CreateTexture();
	}

	void ConvertMouse(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ);
	void ConvertToWorld(int MouseX, int MouseY, D3DVECTOR *vScreen);
	void ConvertToWorld(D3DVECTOR *vScreen);

	Region *GetRegion(D3DVECTOR *vAt);
	Region *GetRegion(int x, int y);
	Object *GetTarget(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	Object *GetStaticTarget(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	ZSTexture *GetBaseTexture() { return pBaseTexture; }
	int GetID() { return AreaID; }

	BOOL CheckLOS(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);
	BOOL CheckChunkLOS(int xn, int yn, D3DVECTOR *vLineSTart, D3DVECTOR *vLineEnd);

// Mutators -----------------------------------------
	void SetID(int NewID) { AreaID = NewID; }
	void SetTexture(int tnum) { TextureNum = tnum; };

	int Load();

	void LoadNonStatic(FILE *fp);
	
	void LoadRegions();
	void SaveRegions();

	int Update();

	int UpdateOffscreen();

	int SetCameraPosition(D3DVECTOR NewPosition);
	int SetCenterScreen(D3DVECTOR NewCenterScreen);
	int RotateCamera(float Amount);
	int MoveCamera(int Direction, float Amount);

	float GetScaling();
		
	int AddObject(Object *ToAdd);
	
	void Damage(float Radius, D3DVECTOR Center, int Min, int Max, DAMAGE_T Type, Thing *pSource);

	void DamageTile(int x, int y, int Min, int Max, DAMAGE_T Type, Thing *pSource);

	void LoadChunk(int x, int y);
	void CleanUpChunks();

	void ClearTile(int x, int y);

	BYTE GetBlocking(int x, int y);
	void SetBlocking(int x, int y);
	void ClearBlocking(int x, int y);

	void AddToUpdate(Object *pToAdd);
	void AddToUpdate(Object *pToAdd, int xUpdate, int yUpdate);
	
	void RemoveFromUpdate(Object *pToRemove);
	void RemoveFromUpdate(Object *pToRemove, int xUpdate, int yUpdate);
	
	void MakeWater(BOOL Vert);
	void UnOccupyRegions();

	char *GetTown(int x, int y);

	void AddRegion(Region *ToAdd);
	void RemoveRegion(Region *ToRemove);
	void SetBaseTexture(ZSTexture *pTexture) { pBaseTexture = pTexture; }
	void RemoveChunk(int x, int y);

	void SetRegionsUnchecked(int ChunkX, int ChunkY);
		
// Output ---------------------------------------------
	
	int Save();
	int SaveBrief();

	void SaveHeader(FILE *fp);
	void LoadHeader(FILE *fp);

	void SaveNonStatic(FILE *fp);
	
	void LoadStaticHeader(FILE *fp, STATIC_FILE_HEADER_T *pDest);

	void SaveStaticHeader(FILE *fp, STATIC_FILE_HEADER_T *pSource);

	void IllegalActivity(Object *pLawBreaker);
  
// Constructors ---------------------------------------
	Area();
	Area(const char *filename);
	 
// Destructor -----------------------------------------
	void Clear();
	~Area();

// Debug ----------------------------------------------
	int Draw2d();
	int Smooth();
	int AddOverlay(int x, int y, int num);

// Editting
	int GenerateBase(RECT *rArea);
	void GenerateTerrain(RECT *rArea);
	void SmoothBaseTerrain();
	void ReBlock(RECT *rArea);
	void RegionObjects(RECT *rArea);
	void UnRegionObjects(RECT *rArea);
	void Cut(RECT *rArea);
	void Combine(FILE *fpMain);
	void ImportStuff(FILE *fpImport);
	void BuildCavern(char *filename, int TileWidth, int TileHeight);
	void AddCaveWalls(char *Texture, int WallHeight, int Type);
	void FixOutDoorTerrain();
	void SetEventHeights();
	void LoadSubDynamic(RECT *rArea, const char *fname, OBJECT_T ToLoad);
	void ClearDynamic(RECT *rArea, OBJECT_T ToClear);
	void ResetUpdate(int x, int y);
	void FixHeightMap();
	void ClearWalls(RECT *pArea);
	void FlattenWalls(RECT *pArea);
	void PaintWalls(RECT *pArea, ZSTexture *pTexture);
	void CleanDynamic();
	void DungeonBlock();
		
// Operators ------------------------------------------
	friend class World;
	friend class Combat;

};


#endif
