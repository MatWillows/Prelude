//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSInput.cpp           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Oct 10, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*		This file provides implementation of ZSWindow class methods
//*
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*		Need to switch from directx surface as background to zssurface	
//*		No copy contructor or assignment operators defined
//*********************************************************************
//*********************************************************************
//revision 3: found source of memory leak
//revision 4: switched to event notification instead of immediate processing

#include "zsinput.h"
#include "zswindow.h"
#include "zsutilities.h"
#include "zssound.h"
#include "zsengine.h"

#define MOUSE_CURSOR_WIDTH	32
#define MOUSE_CURSOR_HEIGHT 32

int ZSInputSystem::Init(HWND hWindow, HINSTANCE hInstance, BOOL Windowed)
{
	DIPROPDWORD DIProp;

	DIProp.diph.dwSize = sizeof(DIPROPDWORD);
	DIProp.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	HRESULT hr;
	hr = DirectInputCreateEx(hInstance,
								DIRECTINPUT_VERSION,
								IID_IDirectInput7,
								(void **)&DirectInput,
								NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct input obejct");
	}

	hr = DirectInput->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7,
        (void**)&KeyBoard, NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct Keyboard");
	}

	hr = DirectInput->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7,
        (void**)&Mouse, NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct Mouse");
	}

//**********************************************************************************
//		Setting the mouse and keyboard to nonexclusive mode eats the hell out of system
//		resources, but is fairly necessary for debugging
//		all tests show that setting to exclusive mode eliminates resource gobbling
//		Resources are returned on safe exit
//
//************************************************************************************

	//set the devices' cooperation levels
	if(Windowed)
	{
		hr = Mouse->SetCooperativeLevel(hWindow, 
			  DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}
	else
	{
		hr = Mouse->SetCooperativeLevel(hWindow, 
			  DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}
	if(hr != DI_OK)
	{
		SafeExit("unable to set Mouse co-op");
	}

	if(Windowed)
	{
		hr = KeyBoard->SetCooperativeLevel(hWindow, 
        DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}
	else
	{
		hr = KeyBoard->SetCooperativeLevel(hWindow, 
			  DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard co-op");
	}

	hr = Mouse->SetDataFormat(&c_dfDIMouse);

	if(hr != DI_OK)
	{
		SafeExit("unable to set mouse format");
	}

	//set the starting cursor position 
	POINT mp;

	//get the windows cursor position
	GetCursorPos(&mp);

	rMouseScreen.left = mp.x;
	rMouseScreen.top = mp.y;

	//set resolution to 2
	//grainularity should be finer than this....
	MouseResolution = 2;

	hr = KeyBoard->SetDataFormat(&c_dfDIKeyboard);

	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard format");
	}

	//set event handling
	InputHandles[0] =	CreateEvent( NULL, FALSE, FALSE, NULL );
	hr = Mouse->SetEventNotification(InputHandles[0]);
	if(hr != DI_OK)
	{
		SafeExit("unable to set Mouse to Event Handling");
	}

	//acquire the mouse
	hr = Mouse->Acquire();
	if(hr != DI_OK)
	{
		SafeExit("unable to set acquire mouse");
	}

	//set event handling
	InputHandles[1] =	CreateEvent( NULL, FALSE, FALSE, NULL );
	hr = KeyBoard->SetEventNotification(InputHandles[1]);
	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard to event Handling");
	}

	//acquire the keyboard
	hr = KeyBoard->Acquire();
	if(hr != DI_OK)
	{
		SafeExit("unable to acquire keyboard");
	}


	return TRUE;
}

int ZSInputSystem::ShutDown()
{
	if(Mouse) {
		Mouse->SetEventNotification(NULL);
		CloseHandle(InputHandles[0]);
		Mouse->Unacquire();
		Mouse->Release();
		Mouse = NULL;
	}

	if(KeyBoard) {
		KeyBoard->SetEventNotification(NULL);
		CloseHandle(InputHandles[1]);
		KeyBoard->Unacquire();
		KeyBoard->Release();
		KeyBoard = NULL;
	}

	if(DirectInput) {
		DirectInput->Release();
		DirectInput = NULL;
	}
	return FALSE;
}


void ZSInputSystem::Update(ZSWindow *pWin)
{
	DIMOUSESTATE ms;
	HRESULT hr;
	DWORD dwResult;

//	Engine->Sound()->Update();
	
	//dwResult = MsgWaitForMultipleObjects(2, InputHandles, FALSE, 0, QS_ALLINPUT); 
	dwResult = WaitForSingleObject(InputHandles[0], 0); 
	
	if(dwResult == WAIT_OBJECT_0)
	{
  // Event 1 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
		//get the mouse state
		hr = Mouse->GetDeviceState(sizeof(DIMOUSESTATE),&ms);
		if(hr != DI_OK)
		{
			//if the mouse was lost attempt to re-acquire it
			if(hr == DIERR_INPUTLOST)
				Mouse->Acquire();
		}
		else
		{
			//update the mouse position from the new mouse state
			rMouseScreen.left += (MouseState.lX * MouseResolution);
			rMouseScreen.top += (MouseState.lY * MouseResolution);
			MouseState.lZ *= MouseResolution;

			//confirm the position from the input focus
			pWin->MoveMouse(&rMouseScreen.left,&rMouseScreen.top, &MouseState.lZ);
			rMouseScreen.right = rMouseScreen.left + MOUSE_CURSOR_WIDTH;
			rMouseScreen.bottom = rMouseScreen.top + MOUSE_CURSOR_HEIGHT;

			//check button 1
			if(MouseState.rgbButtons[0] &0x80)
			{
				if(ms.rgbButtons[0] & 0x80)
				{
				}
				else
				{
					//see if the left button up has already been handled
					if(!LeftButtonUpHandled)
					{
						LeftButtonUpHandled = TRUE;
						LeftButtonDownHandled = FALSE;

						//if not send a message to the input focus
						pWin->LeftButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[0] & 0x80)
				{
					//see if the left button down has been handled
					if(!LeftButtonDownHandled)
					{
						LeftButtonDownHandled = TRUE;
						LeftButtonUpHandled = FALSE;
						
						//if not send message
						pWin->LeftButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}

			if(MouseState.rgbButtons[1] & 0x80)
			{
				if(ms.rgbButtons[1] & 0x80)
				{
				}
				else
				{
					//see if right button up has been handled
					if(!RightButtonUpHandled)
					{
						RightButtonUpHandled = TRUE;
						RightButtonDownHandled = FALSE;

						//if not send message
						pWin->RightButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[1] & 0x80)
				{
					//see if right button down has been handled
					if(!RightButtonDownHandled)
					{
						RightButtonDownHandled = TRUE;
						RightButtonUpHandled = FALSE;

						//if not send message
						pWin->RightButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}
			MouseState = ms;

		}

		//done with mouse
	}

	//dwResult = MsgWaitForMultipleObjects(2, InputHandles, FALSE, 0, QS_ALLINPUT); 
	dwResult = WaitForSingleObject(InputHandles[1], 0); 
	
	if(dwResult == WAIT_OBJECT_0)
	{
		// Event 2 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
			//check keyboard
			hr = KeyBoard->GetDeviceState(256, NewKeys);
			if(hr != DI_OK)
			{
				//reacquire if necessary
				if(hr == DIERR_INPUTLOST)
					KeyBoard->Acquire();
				
			}
			else
			{
				//send the new and last keystates to the imput focus
				pWin->HandleKeys(NewKeys, KeyState);
				memcpy(KeyState,NewKeys,256);
				if(NewKeys[DIK_ESCAPE] & 0x80 &&
					(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80) &&
					(NewKeys[DIK_LSHIFT] & 0x80 || NewKeys[DIK_RSHIFT] & 0x80))
					{
						pWin->ReleaseFocus();
						DEBUG_INFO("forcing a focus release from input module\n");
					}
				else
				if(NewKeys[DIK_ESCAPE] & 0x80 &&
					(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80))
					{
					  SafeExit("hard exit from input module");
					}
				
		
			}

	}

/*   switch (dwResult) { 
    case WAIT_OBJECT_0: 
        // Event 1 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
		//get the mouse state
		hr = Mouse->GetDeviceState(sizeof(DIMOUSESTATE),&ms);
		if(hr != DI_OK)
		{
			//if the mouse was lost attempt to re-acquire it
			if(hr == DIERR_INPUTLOST)
				Mouse->Acquire();
		}
		else
		{
			//update the mouse position from the new mouse state
			rMouseScreen.left += (MouseState.lX * MouseResolution);
			rMouseScreen.top += (MouseState.lY * MouseResolution);

			//confirm the position from the input focus
			pWin->MoveMouse(&rMouseScreen.left,&rMouseScreen.top);
			rMouseScreen.right = rMouseScreen.left + MOUSE_CURSOR_WIDTH;
			rMouseScreen.bottom = rMouseScreen.top + MOUSE_CURSOR_HEIGHT;

			//check button 1
			if(MouseState.rgbButtons[0] &0x80)
			{
				if(ms.rgbButtons[0] & 0x80)
				{
				}
				else
				{
					//see if the left button up has already been handled
					if(!LeftButtonUpHandled)
					{
						LeftButtonUpHandled = TRUE;
						LeftButtonDownHandled = FALSE;

						//if not send a message to the input focus
						pWin->LeftButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[0] & 0x80)
				{
					//see if the left button down has been handled
					if(!LeftButtonDownHandled)
					{
						LeftButtonDownHandled = TRUE;
						LeftButtonUpHandled = FALSE;
						
						//if not send message
						pWin->LeftButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}

			if(MouseState.rgbButtons[1] & 0x80)
			{
				if(ms.rgbButtons[1] & 0x80)
				{
				}
				else
				{
					//see if right button up has been handled
					if(!RightButtonUpHandled)
					{
						RightButtonUpHandled = TRUE;
						RightButtonDownHandled = FALSE;

						//if not send message
						pWin->RightButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[1] & 0x80)
				{
					//see if right button down has been handled
					if(!RightButtonDownHandled)
					{
						RightButtonDownHandled = TRUE;
						RightButtonUpHandled = FALSE;

						//if not send message
						pWin->RightButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}
			MouseState = ms;

		}

		//done with mouse
	    break; 
 
    case WAIT_OBJECT_0 + 1: 
        // Event 2 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
			//check keyboard
			hr = KeyBoard->GetDeviceState(256, NewKeys);
			if(hr != DI_OK)
			{
				//reacquire if necessary
				if(hr == DIERR_INPUTLOST)
					KeyBoard->Acquire();
				
			}
			else
			{
				//send the new and last keystates to the imput focus
				pWin->HandleKeys(NewKeys, KeyState);
				memcpy(KeyState,NewKeys,256);
			}
	    break; 
		case WAIT_OBJECT_0 + 2:
        while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
        {
            if (msg.message == WM_QUIT) 
            {
                // stop loop if it's a quit message
					exit(1);
				} 
            else 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        break;
	}
*/
  
}

void ZSInputSystem::ClearStates()
{
	memset(&MouseState,0,sizeof(MouseState));
	memset(KeyState,0,256);
	RightButtonDownHandled = FALSE;
	RightButtonUpHandled = FALSE;
	LeftButtonUpHandled = FALSE;
	LeftButtonDownHandled = FALSE;
}


ZSInputSystem::ZSInputSystem()
{
	memset(&MouseState,0,sizeof(MouseState));
	memset(KeyState,0,256);
	RightButtonDownHandled = FALSE;
	RightButtonUpHandled = FALSE;
	LeftButtonUpHandled = FALSE;
	LeftButtonDownHandled = FALSE;
	Mouse = NULL;
	DirectInput = NULL;
	KeyBoard = NULL;

}

ZSInputSystem::~ZSInputSystem()
{
	//confirm shutdown
	
	//if mouse or keyboard or input exists we have not shut down
	if(Mouse || KeyBoard || DirectInput)
	{
		ShutDown();
	}

}