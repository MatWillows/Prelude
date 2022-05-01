//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSWindow.h           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Oct 3, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*		This file provides a declaration of a base window class.
//*		Class defines default behaviour of all windows
//*
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*		Need to switch from directx surface as background to zssurface	
//*		No copy contructor or assignment operators defined
//*********************************************************************
//*********************************************************************
//revision 4. Added old focal stack in place of previous focus
//revision 5. Added virtual methods gain and lose focus
//revision 6. added flexible border


#ifndef ZSWINDOW_H
#define ZSWINDOW_H

#include "defs.h"				
#include "zsgraphics.h"

#define MAX_LIST_ITEMS	32
#define DELETE_ME			-666
#define MAX_FOCUS_STACK_DEPTH	64


//an enumerated type defining posssible states a window could be in
typedef enum
{
	WINDOW_STATE_NONE,		//window should never be in window stat none
	WINDOW_STATE_NORMAL,		//normal operations
	WINDOW_STATE_MOVING,		//moving
	WINDOW_STATE_DONE,		//done, used primarily for exitting modal windows
} WINDOW_STATE_T;


//types of windows and controls which may exist
typedef enum
{
	WINDOW_NONE,
	WINDOW_MESSAGEBOX,
	WINDOW_BUTTON,
	WINDOW_ICON,
	WINDOW_TEXT,
	WINDOW_LIST,
	WINDOW_SCROLLVERTICAL,
	WINDOW_SCROLLHORIZONTAL,
	WINDOW_SLIDER,
	WINDOW_EDIT,
	WINDOW_ITEMSLOT,
	WINDOW_CONTAINER,
	WINDOW_INVENTORY,
	WINDOW_BARTER,
	WINDOW_CHARACTERINFO,
	WINDOW_EXITBUTTON,
	WINDOW_DESCRIBE,
	WINDOW_COMBATINFO,
	WINDOW_TALK,
	WINDOW_SAY,
	WINDOW_VALLEYMAP,
	WINDOW_LOCALMAP,
	WINDOW_JOURNAL,
	WINDOW_OPTIONS,
	WINDOW_PORTRAIT,
	WINDOW_DIALOG,
	WINDOW_MAIN,
	WINDOW_EQUIP,
	WINDOW_PICKPOCKET
} WINDOW_T;


//commands that may be sent from a child window to a parent
typedef enum
{
	COMMAND_NONE = 0,
	COMMAND_BUTTON_PRESSED,
	COMMAND_BUTTON_RELEASED,
	COMMAND_BUTTON_CLICKED,
	COMMAND_BUTTON_RIGHTCLICKED,
	COMMAND_SLIDER_MOVED,
	COMMAND_LIST_SELECTIONCHANGED,
	COMMAND_LIST_SELECTED,
	COMMAND_LIST_RIGHTCLICKED,
	COMMAND_EDIT_CHANGED,
	COMMAND_EDIT_DONE,
	COMMAND_CHILD_GAINFOCUS,
	COMMAND_CHILD_LOSEFOCUS,
	COMMAND_SCROLL,
	COMMAND_ITEMSLOT_CHANGING,
	COMMAND_ITEMSLOT_CHANGED,
	COMMAND_SPECIAL,
} ZSWindow_COMMAND_T;

class Object;

//****************************CLASS************************************
//**************                                  *********************
//**                         ZSWindow                                 **
//*********************************************************************
//*Purpose:     Base window class.  Mirrors much Windows OS capabilities
//*			Without relying on a messaging system                     *
//*********************************************************************
//*Invariants:                                                                                                                    *
//*                                                                   *
//*********************************************************************

class ZSWindow
{
	protected:
	//***************MEMBERS****************************************************
	//**
		static ZSWindow *pMainWindow;	//The main application window, usually of type WINDOW_MAIN

		static ZSWindow *pInputFocus;	//The window currently receiving keyboard and mouse input

		static int OldFocusStackTop;	
		static ZSWindow *OldFocusStack[MAX_FOCUS_STACK_DEPTH]; //Old focus stack allows setting and releasing of focus
		static DWORD NextMouseUpdate;
		
		int ID;	//an ID, not necessarily unique, no siblings should have the same ID

		WINDOW_T Type;	//the kind of window
	
		BOOL Visible;	//is the window visible or not for drawing and detection purposes
	
		int TextColor;

		BOOL Moveable;	//can the window be dragged
	
		char *Text;	//text for the window

		ZSWindow *pParent;	//the Windows parent.  All windows except the main window must have a parent
		ZSWindow *pChild;		//a child window
		ZSWindow *pSibling;	//sibling windows.  All siblings have the same parent

		RECT Bounds;	//the windows current location
	
		RECT OldBounds;	//the last valid location of the window.  Needed to rebound to when a user attempts to drag 
								//a window to an invalid location
		
		int XMoveOffset;	//used to mark the point at which a user has grabbed a window for dragging
		int YMoveOffset;
		
		WINDOW_STATE_T State;	//state of the window, moving, normal, done, etc...

		CURSOR_T Cursor;	//the cursor to be used when the mouse is over the window
		
		LPDIRECTDRAWSURFACE7 BackGroundSurface;	//the background of the window

		DWORD ReturnCode;	//return value for modal operations 

		int Border; //width of window border if any

		static LPDIRECTDRAWSURFACE7 lpddsParchment;
		static LPDIRECTDRAWSURFACE7 lpddsWood;
		static LPDIRECTDRAWSURFACE7 lpddsVBorder[4];
		static LPDIRECTDRAWSURFACE7 lpddsHBorder[4];
		static LPDIRECTDRAWSURFACE7 lpddsCorner[4];

	public:

//********************METHODS***************************************************
//**

//Display
//--------------------------------------------------------------------------	
		virtual int DrawMouse(int x, int y);
		virtual int Draw();

//Accessors		
//------------------------------------------------------------------------------	
		static inline ZSWindow *GetMain() { return pMainWindow; }

		static inline ZSWindow *GetFocus() { return pInputFocus; }
		
		static inline void SetMainWindow(ZSWindow *pWin) { pMainWindow = pWin; }

		static void Init();
		static void Shutdown();

	
		inline ZSWindow *GetParent() {	return pParent;	}
		inline ZSWindow *GetChild()	 {	return pChild;		}
		inline ZSWindow *GetSibling(){	return pSibling;	}

		void GetBounds(RECT *r){ *r = Bounds; }

		virtual void SetFocus(ZSWindow *pToFocus);
		virtual void ReleaseFocus();
		
		ZSWindow *GetChild(int x, int y);
		ZSWindow *GetChild(int ID);
		LPDIRECTDRAWSURFACE7 GetBackGround() { return BackGroundSurface; }

		BOOL	IsVisible() { return Visible; }

		char *GetText() { return Text; };

		int GetBorderWidth() { return Border; };

		WINDOW_STATE_T	GetState() { return State; };
		ZSWindow *GetBottomChild(int x, int y);
		WINDOW_T GetType() { return Type; };
		int GetID() { return ID; }
		DWORD GetReturnCode() { return ReturnCode; }

//Mutators
//------------------------------------------------------------------------
		virtual void GainFocus();
		virtual void LoseFocus();

		void SetBounds(RECT *r){ Bounds = *r; }

		virtual void Show() { Visible = TRUE; }
		virtual void Hide() { Visible = FALSE; }

		
		int SetText(char *NewText);
		int SetText(int n);

		int AddChild(ZSWindow *ToAdd);
		int AddTopChild(ZSWindow *ToAdd);
		
		int RemoveChild(ZSWindow *ToRemove);

		int AddSibling(ZSWindow *ToAdd);
		int AddTopSibling(ZSWindow *ToAdd);

		void SetCursor(CURSOR_T CursorNum) { Cursor = CursorNum; };
		CURSOR_T GetCursor() { return Cursor; };

		virtual int MoveMouse(long *x, long *y, long *z);
		virtual int Move(RECT NewBounds);
		virtual int Move(int, int);
		virtual int ValidateMove(RECT *NewBounds);
		
		
		virtual int LeftButtonDown(int x, int y);
		virtual int LeftButtonUp(int x, int y);
		virtual int RightButtonDown(int x, int y);
		virtual int RightButtonUp(int x, int y);
		virtual int Command(int IDFrom, int Command, int Param);
		virtual int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
		
		virtual int GoModal();

		void SetBackGround(LPDIRECTDRAWSURFACE7 NewBack); 

		LPDIRECTDRAWSURFACE7 CreatePortrait(const char *PortraitName, BOOL GetFace = FALSE);
		void CreateEmptyBorderedBackground(int BorderWidth, int btype = 3);
		void CreateParchmentBorderedBackground(int BorderWidth, int btype = 3);
		void CreateWoodBorderedBackground(int BorderWidth, int btype = 3);
		void AddBorder(int BorderWidth, int btype = 3);

		void SetBorderWidth(int NewWidth) { Border = NewWidth; };

		void SetState(WINDOW_STATE_T NewState) { State = NewState; };

		void SetMoveable(BOOL NewMoveState) { Moveable = NewMoveState; };

		virtual void SetTextColor(int NewColor) { TextColor = NewColor; }

		void SetReturnCode(DWORD NewCode) { ReturnCode = NewCode; }

		virtual BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);


//Constrtuctors
//-------------------------------------------------------------------------------
		ZSWindow();
		ZSWindow(WINDOW_T NewType, int NewID, int x, int y, int width, int height);

//Destructor
//------------------------------------------------------------------------------
		~ZSWindow();

//debug
//-------------------------------------------------------------------------------
		virtual void OutputDebugInfo(FILE *fp);
		virtual void OutputDebugInfo(const char *FileName);
};

#endif