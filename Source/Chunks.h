//********************************************************************* 
//*                                                                                                                                    ** 
//**************		chunks.h	        ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:       7/30                                             * 
//*Revisor:        Mat Williams                                     * 
//*Purpose:        interface for world chunks           * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*																						 * 
//********************************************************************* 
//*********************************************************************
#ifndef CHUNKS_H
#define CHUNKS_H

#include "objects.h"
#include "texturemanager.h"

#define TILE_WIDTH	32
#define TILE_HEIGHT	32

#define CHUNK_WIDTH	8
#define CHUNK_TILE_WIDTH 16
#define CHUNK_HEIGHT 8
#define CHUNK_TILE_HEIGHT 16

#define CHUNK_TEXTURE_WIDTH	(CHUNK_WIDTH * TILE_WIDTH)
#define CHUNK_TEXTURE_HEIGHT	(CHUNK_HEIGHT * TILE_HEIGHT)

#define NUM_OVERLAYS 8
#define NUM_CHUNK_REGIONS 5

#define TILE_HEIGHT_FACTOR 0.25f

//an enum to help reference the strided vertex array
typedef enum
{
	tx1 = 0,
	ty1,
	tz1,
	ntx1,
	nty1,
	ntz1,
	tx2,
	ty2,
	tz2,
	ntx2,
	nty2,
	ntz2,
	tx3,
	ty3,
	tz3,
	ntx3,
	nty3,
	ntz3,
	tx4,
	ty4,
	tz4,
	ntx4,
	nty4,
	ntz4,
} TILE_INDEX_HELPERS;

//preprocessor defs ***********************************************

//*******************************CLASS******************************** 
//**************        Chunk            ********************* 
//**					                                  ** 
//******************************************************************** 
//*Purpose: Manage basic static world geometry  
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************

#define NUM_CHUNK_VERTS	(CHUNK_WIDTH * CHUNK_HEIGHT * 4)
#define CHUNK_DRAW_LENGTH (CHUNK_WIDTH * CHUNK_HEIGHT * 6)

//class Region;

class Chunk
{
public:
	static D3DTLVERTEX vTerrain[16*4*4*4];
	
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 
	static float TextureCoordinates[2 * NUM_CHUNK_VERTS];

	static TextureGroup ChunkTextures;

	D3DDRAWPRIMITIVESTRIDEDDATA stridedDataInfo;

	//interleaved position and normal data
	float Verts[NUM_CHUNK_VERTS * 6];

	int	X;
	int	Y;
	ZSTexture *pTerrainTexture;
	Object *pObjectList;
	Object *pObjectList2;
	Object *pObjectListEnd;
	int NumObjects;
	
	unsigned short Terrain[CHUNK_WIDTH][CHUNK_HEIGHT];
	unsigned short Overlays[CHUNK_WIDTH][CHUNK_HEIGHT][NUM_OVERLAYS];

	//D3DVERTEX Verts[CHUNK_WIDTH * CHUNK_HEIGHT * 4];

	unsigned short DrawList[CHUNK_DRAW_LENGTH];

//	BYTE Blocking[CHUNK_WIDTH * CHUNK_HEIGHT];

	unsigned short Blocking[CHUNK_TILE_HEIGHT]; //16 bits across

	float TileHeights[CHUNK_TILE_HEIGHT * CHUNK_TILE_WIDTH];

	//unsigned short Regions[NUM_CHUNK_REGIONS];

	//VertexList
	//DrawLists
		//trianglelist
		//texture number
		//run length
	//Regions
	
//************************************************************************************** 

public:

// Display Functions -------------------------------
	void Draw();
	void DrawObjects();
	void DrawTiles();
	void DrawBacksides();

// Accessors ----------------------------------------
	int GetX() { return X; }
	int GetY() { return Y; }
	float *GetTile(int x, int y);
	unsigned short *GetTileDraw(int x, int y);
	int GetSize();
	int GetTerrain(int x, int y) { return Terrain[x][y]; };

	float GetHeight(int x, int y);

	ZSTexture *GetTexture() { return pTerrainTexture; }

	void ReleaseTexture();

	BOOL CheckLOS(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);

//	Region *GetRegion(D3DVECTOR *vAt);

// Mutators -----------------------------------------
	void Load(FILE *fp);
	int AddObject(Object *pAddObject);
	int RemoveObject(Object *pToRemove);
	
	inline void SetXY(int x, int y)
	{
		X = x;
		Y = y;
	}

	int CreateTexture(ZSTexture *pBaseTexture = NULL);

	void ResetTile(int x, int y);

	static void InitTerrain();

	void Build();

	void Smooth();

	void AddOverlay(int x, int y, int num);
	void SetTerrain(int x, int y, int num) { Terrain[x][y] = (unsigned short)num; }
	void ClearTile(int x, int y);

	BOOL GetBlocking(int x, int y);
	
	void SetBlocking(int x, int y);
	void RemoveBlocking(int x, int y);


	void ResetBlocking();

	BOOL IsFlipped(int x, int y) { return Blocking[x + y * CHUNK_WIDTH] & DRAW_FLIPPED; }
	void RemoveTile(int x, int y);

	Object *GetObjects() { return pObjectList; }
	Object *GetLastObject() { return pObjectListEnd; }

	void CreateHeightMap();
	void SetObjectHeights();

	void SetTileHeight(int x, int y, float NewHeight);

	void SortObjects();

// Output ---------------------------------------------
	void Save(FILE *fp);
	void SaveBrief(FILE *fp);
  
// Constructors ---------------------------------------
	Chunk();
 
// Destructor -----------------------------------------
	~Chunk();

// Debug ----------------------------------------------
	void OutPutDebugInfo(const char *filename);

// Operators ------------------------------------------

	friend class World;
	friend class Area;

// Editting

	BOOL IsRoadChunk();
	void FixTerrainHeights();
	void Smooth(BYTE *Terrains, int Pitch);
	void BlockBySlope(float fSlope);
	void DungeonBlock();

};

#endif
