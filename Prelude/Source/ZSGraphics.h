//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSGraphics.h           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Sept 20, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*	provide an os independent input manager
//*
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//	
//		ZSSurfaces need to be implemented
//		Key masking is iffy, can get from file, but not independant of.
//		Better wrapping of Direct3D (as in settexture method), most external calls 
//    currently just pass through to Direct3D
//
//*********************************************************************
//*********************************************************************
#ifndef ZSGRAPHICS_H
#define ZSGRAPHICS_H

#ifndef D3D_OVERLOADS
#define D3D_OVERLOADS
#endif

#include <ddraw.h>
#include <d3d.h>
#include <d3dx.h>

#include "zstexture.h"
#include "zsfontengine.h"
#include <stdio.h>

#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

#define NUM_CURSORS			12
#define MAX_CURSOR_FRAMES	5

#define MOUSE_CURSOR_WIDTH		32
#define MOUSE_CURSOR_HEIGHT	32

#define ROTATION_ANGLE	(PI_MUL_2/64)
#define ZOOM_FACTOR		0.5f

#define MAX_ZOOM	24.0f

#define MIN_ZOOM	10.0f

#define VIEW_WIDTH	10
#define VIEW_HEIGHT	10
#define VIEW_DEPTH	80

#define TERRAIN_TEXTURE_WIDTH		256
#define TERRAIN_TEXTURE_HEIGHT	256

#define COLOR_KEY_FROM_FILE	-1
#define COLOR_KEY_BLACK			0

typedef struct
{
	float tu;
	float tv;
} TEXTURE_COORD;

//some basic colors
typedef enum
{
	COLOR_NONE = 0,
	COLOR_DEFAULT = 0,
	COLOR_RED,
	COLOR_ORANGE,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_PURPLE,
	COLOR_WHITE,
	COLOR_BLACK
} COLOR_T;


//types of cursors
typedef enum
{
	CURSOR_NONE = -1,
	CURSOR_POINT,
	CURSOR_GRAB,
	CURSOR_RIVER,
	CURSOR_CAMERA,
	CURSOR_ATTACK,
	CURSOR_FLAME,
	CURSOR_THAUMATURGY,
	CURSOR_TALK,
	CURSOR_DEFEND,
	CURSOR_MOVE,
	CURSOR_WAIT
} CURSOR_T;

typedef enum
{
	FILTER_NONE,
	FILTER_TERRAIN,
	FILTER_BOTH,
} FILTER_T;

//****************************CLASS************************************
//**************                                  *********************
//**                         ZSGraphicsSystem                          *
//*********************************************************************
//*Purpose:    
//*		provide OS independent drawint functions
//*********************************************************************
//*Invariants:                                                                                                                    *
//*                                                                   *
//*********************************************************************
class ZSGraphicsSystem
{
private:

	//screen information	
	int ScreenHeight;
	int ScreenWidth;
	float ViewDim;
	int Windowed;
	int BitDepth;

	FILTER_T FilterState;
	
	//necessary window stuff
	HINSTANCE Application;
	HWND MainWindow;
	
	// DirectDraw Interface
	LPDIRECTDRAW7 DirectDraw;

	// Clipping
	LPDIRECTDRAWCLIPPER Clipper;
	LPDIRECTDRAWCLIPPER WindowClipper;
	int ClipMaxY;
	int ClipMinY;
	int ClipMaxX;
	int ClipMinX;

	// Surfaces
	LPDIRECTDRAWSURFACE7 Primary;
	LPDIRECTDRAWSURFACE7 BBuffer;
	LPDIRECTDRAWSURFACE7 ZBuf;

	// Direct3D Interface
	LPDIRECT3DDEVICE7 D3DDevice;
	LPDIRECT3D7 Direct3D;

	//pixel format 16
	int PixelFormat;

	//type of renderer
	int D3DHardware;

	//identity matrix
	D3DMATRIX Identity;

	//colorkey
	WORD KeyMask;

	//world project matrix
	D3DMATRIX Projection;

	//current texture being used for rendering
	ZSTexture *pCurTexture;

	//a but of colored materials
	D3DMATERIAL7 Materials[COLOR_BLACK + 1];

	//fontengine
	ZSFontEngine *pZSFont;

	//mouse draw stuff
	RECT MouseCursor[NUM_CURSORS * MAX_CURSOR_FRAMES];
	int NumCursorFrames[NUM_CURSORS];
	int CursorFrame;
	int Cursor;
	int CursorSubFrame;
	int CursorFrameRate;
	int CursorOffsetX[NUM_CURSORS];
	int CursorOffsetY[NUM_CURSORS];
	D3DVERTEX MouseVerts[4];		//for highlighting the tile the mouse is over
	int CurMouseCursor;
	LPDIRECTDRAWSURFACE7 MouseSurface;
	int CircleFrame;
	ZSTexture *CircleTextures[3];
	
public:

	//startup 
	HWND Init(HINSTANCE hInstance);
	ZSGraphicsSystem();
	void SetUpCircles();
	
	// 2D DirectDraw Functions
	LPDIRECTDRAWSURFACE7 CreateSurface(int width, int height, int mem_flags, DWORD colour_key_value);
	LPDIRECTDRAWSURFACE7 CreateSurfaceFromFile(char *filename, int width, int height, int mem_flags, DWORD colour_key_value);
	BOOL LoadFileIntoSurface(LPDIRECTDRAWSURFACE7 lpddInto, char *filename, int width, int height);

	LPDIRECTDRAWCLIPPER AttachClipper(LPDIRECTDRAWSURFACE7 surface, int RectCount, LPRECT ClipList);
	
	void FillSurface(LPDIRECTDRAWSURFACE7 lpdds, DWORD colour, RECT * client);

	void DrawBox(LPDIRECTDRAWSURFACE7 Target, LPRECT rBox, DWORD Color);
	
	void Flip();

	void TextureBlt(LPDIRECTDRAWSURFACE7 Dest, LPRECT rArea, ZSTexture *source);

	void DrawCursor(RECT *rDrawAt);
	void AdvanceCursorFrame();
	void SetCursor(int CursorNum);
	void SetCursorFrame(int FrameNum);

	//3D Direct3DFunctions
	int CreateProjectionMatrix(float width, float height, float depth);
	int Zoom(float offset);
	float GetViewDim() { return ViewDim; }
	void SetViewDim(float NewDim);

	BOOL SetTexture(ZSTexture *ToSet);
	BOOL ClearTexture();

	HRESULT DrawText(int x, int y, char *Text);
	HRESULT DrawText(D3DVECTOR *pVector, char *Text);
	HRESULT DrawText(RECT *rArea, char *Text);
	void SetFont(ZSFont *pFont);

	FILTER_T GetFilterState() { return FilterState; };
	void SetFilterState(FILTER_T NewState) { FilterState = NewState; }

	int Circle(D3DVECTOR *vAt, COLOR_T Color, float Radius);
	void AdvanceCircleFrame() { CircleFrame++; if(CircleFrame == 3) CircleFrame = 0; }
	ZSTexture *GetCircleTexture() { return CircleTextures[CircleFrame]; }

//accessors
	int GetWidth() { return ScreenWidth; }
	int GetHeight() { return ScreenHeight; }
	HINSTANCE GetInstance() { return Application; }
	int IsWindowed() { return Windowed; }
	HWND GetWinHandle() { return MainWindow; }
	WORD GetMask() { return KeyMask; }
	D3DMATERIAL7	*GetMaterial(COLOR_T Color) { return &Materials[Color]; }
	D3DMATRIX		*GetIdentity() { return &Identity; }

	D3DVERTEX *GetMouseVerts() { return MouseVerts; }
	LPDIRECT3DDEVICE7		GetD3D()		 {	return D3DDevice; }
	LPDIRECTDRAW7			GetDirectDraw() { return DirectDraw; }
	LPDIRECTDRAWSURFACE7 GetPrimay()  {	return Primary;	}
	LPDIRECTDRAWSURFACE7 GetBBuffer() {	return BBuffer;	}
	ZSFontEngine *GetFontEngine() { return pZSFont; }

	void SetRenderState(D3DRENDERSTATETYPE dwType, DWORD dwState);
	//shutdown
	int ShutDown();

//debug
	void OutPutDebugInfo(FILE *fp);

	
	friend class ZSEngine;
};




#endif