#include "ZSButton.h"
#include "zsutilities.h"
#include "assert.h"
#include "zsengine.h"

RECT ZSButton::rButtonNormal[BUTTON_PIC];
RECT ZSButton::rButtonMouseOver[BUTTON_PIC];
RECT ZSButton::rButtonPressed[BUTTON_PIC];
LPDIRECTDRAWSURFACE7	 ZSButton::ButtonSurface = NULL;

#define BUTTON_FRAME_RATE	66

int ZSButton::Draw()
{
	HRESULT hr;
	if(Visible)
	{
		if(BackGroundSurface)
		{
			if(BaseHeight)
			{
				//are we the focus?
				if(GetFocus() == this)
				{
					Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,&Pressed,NULL,NULL);
				}
				else
				//is the mouse over us
				if(Engine->Input()->GetMouseRect()->left >= Bounds.left &&
					Engine->Input()->GetMouseRect()->left <= Bounds.right &&
					Engine->Input()->GetMouseRect()->top >= Bounds.top &&
					Engine->Input()->GetMouseRect()->top <= Bounds.bottom)
				{
					if(BackGroundSurface != ButtonSurface)
					{
						MouseOver.top = (2 + CurFrame) * BaseHeight;
						MouseOver.bottom = MouseOver.top + BaseHeight;
						if(timeGetTime() > NextUpdate)
						{
							CurFrame++;
							if(CurFrame >= NumFrames) CurFrame = 0;
							NextUpdate = timeGetTime() + BUTTON_FRAME_RATE;
						}
					}
					
					hr = Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,&MouseOver,NULL,NULL);
					
					if(FAILED(hr))
					{
						Engine->ReportError(hr);
					}

					SetCursor(Cursor);
				}
				//draw basic button
				else
				{
					Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,&Normal,NULL,NULL);
				}
			}
			else
			{
				Engine->Graphics()->GetBBuffer()->Blt(&Bounds, BackGroundSurface, NULL, DDBLT_KEYSRC ,NULL);
			}
		}
		if(Text)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(),Bounds.left, Bounds.top, Text, TextColor);
		}
	}	
	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE;
}

int ZSButton::LeftButtonDown(int x, int y)
{
//	DEBUG_INFO("ButtonDown\n");
	SetFocus(this);
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}
	pParent->Command(ID,COMMAND_BUTTON_PRESSED,0);
	return TRUE;
}

int ZSButton::LeftButtonUp(int x, int y)
{
//	DEBUG_INFO("ButtonUp\n");
	ReleaseFocus();
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}
	if(x >= Bounds.left && x <= Bounds.right &&
		y >= Bounds.top && y <= Bounds.bottom)
	{
		Engine->Sound()->PlayEffect(26);
		pParent->Command(ID, COMMAND_BUTTON_CLICKED,0);
	}
	else
	{
		pParent->Command(ID, COMMAND_BUTTON_RELEASED,0);
	}
	return TRUE;
}

int ZSButton::RightButtonDown(int x, int y)
{
	SetFocus(this);
	return TRUE;
}

int ZSButton::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	pParent->Command(ID, COMMAND_BUTTON_RIGHTCLICKED,0);
	return TRUE;
}

ZSButton::ZSButton(BUTTON_T BType, int NewID, int x, int y, int width, int height)
{
	if(!ButtonSurface)
	{
		Init();
	}
	Cursor = CURSOR_POINT;
	ID = NewID;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.bottom = y + height;
	Bounds.right = x + width;
	if(BType != BUTTON_NONE)
	{
		BackGroundSurface = ZSButton::ButtonSurface;
		ZSButton::ButtonSurface->AddRef();
		Normal = rButtonNormal[BType];
		MouseOver = rButtonMouseOver[BType];
		Pressed = rButtonPressed[BType];
		BaseHeight = height;
		TextColor = 7;
	}
	else
	{
		BaseHeight = 0;
	}

	Text = NULL;
	pSibling = NULL;
	pParent = NULL;
	pChild = NULL;
	NumFrames = 0;
	SubFrame = 0;
	CurFrame = 0;

}

void ZSButton::Init()
{
	//open the gui file
	FILE *fp;
	fpos_t startbuttons;
	fp = SafeFileOpen("gui.ini","rt");
	
	SeekTo(fp,"[BUTTONS]");

	SeekTo(fp,"BACKGROUND");

	char *BFileName;

	BFileName = GetStringNoWhite(fp);
	
	int FileWidth;
	int FileHeight;

	FileWidth = GetInt(fp);
	FileHeight = GetInt(fp);

	ButtonSurface = Engine->Graphics()->CreateSurfaceFromFile(BFileName,FileWidth,FileHeight,0,Engine->Graphics()->GetMask());
	
	assert(ButtonSurface);

	delete[] BFileName;

	fgetpos(fp,&startbuttons);

	int x,y,w,h;
	
	fsetpos(fp,&startbuttons);

	SeekTo(fp,"BASE");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_NORMAL].left = x;
	rButtonNormal[BUTTON_NORMAL].right = x + w;
	rButtonNormal[BUTTON_NORMAL].top = y;
	rButtonNormal[BUTTON_NORMAL].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"PLUS");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_PLUS].left = x;
	rButtonNormal[BUTTON_PLUS].right = x + w;
	rButtonNormal[BUTTON_PLUS].top = y;
	rButtonNormal[BUTTON_PLUS].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"MINUS");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_MINUS].left = x;
	rButtonNormal[BUTTON_MINUS].right = x + w;
	rButtonNormal[BUTTON_MINUS].top = y;
	rButtonNormal[BUTTON_MINUS].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"CHECK");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_CHECK].left = x;
	rButtonNormal[BUTTON_CHECK].right = x + w;
	rButtonNormal[BUTTON_CHECK].top = y;
	rButtonNormal[BUTTON_CHECK].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"NORTH");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_NORTH].left = x;
	rButtonNormal[BUTTON_NORTH].right = x + w;
	rButtonNormal[BUTTON_NORTH].top = y;
	rButtonNormal[BUTTON_NORTH].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"SOUTH");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_SOUTH].left = x;
	rButtonNormal[BUTTON_SOUTH].right = x + w;
	rButtonNormal[BUTTON_SOUTH].top = y;
	rButtonNormal[BUTTON_SOUTH].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"EAST");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_EAST].left = x;
	rButtonNormal[BUTTON_EAST].right = x + w;
	rButtonNormal[BUTTON_EAST].top = y;
	rButtonNormal[BUTTON_EAST].bottom = y + h;

	fsetpos(fp,&startbuttons);

	SeekTo(fp,"WEST");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_WEST].left = x;
	rButtonNormal[BUTTON_WEST].right = x + w;
	rButtonNormal[BUTTON_WEST].top = y;
	rButtonNormal[BUTTON_WEST].bottom = y + h;
	
	fsetpos(fp,&startbuttons);

	SeekTo(fp,"THUMB");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_THUMB].left = x;
	rButtonNormal[BUTTON_THUMB].right = x + w;
	rButtonNormal[BUTTON_THUMB].top = y;
	rButtonNormal[BUTTON_THUMB].bottom = y + h;

	SeekTo(fp,"BULLET");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_BULLET].left = x;
	rButtonNormal[BUTTON_BULLET].right = x + w;
	rButtonNormal[BUTTON_BULLET].top = y;
	rButtonNormal[BUTTON_BULLET].bottom = y + h;




	SeekTo(fp,"ROTATERIGHT");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].left = x;
	rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].right = x + w;
	rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].top = y;
	rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].bottom = y + h;

	SeekTo(fp,"ROTATELEFT");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].left = x;
	rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].right = x + w;
	rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].top = y;
	rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].bottom = y + h;

	SeekTo(fp,"ROTATEOVER");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_CAMERA_ROTATE_OVER].left = x;
	rButtonNormal[BUTTON_CAMERA_ROTATE_OVER].right = x + w;
	rButtonNormal[BUTTON_CAMERA_ROTATE_OVER].top = y;
	rButtonNormal[BUTTON_CAMERA_ROTATE_OVER].bottom = y + h;

	SeekTo(fp,"ROTATEUNDER");
	x = GetInt(fp);
	y = GetInt(fp);
	w = GetInt(fp);
	h = GetInt(fp);

	rButtonNormal[BUTTON_CAMERA_ROTATE_UNDER].left = x;
	rButtonNormal[BUTTON_CAMERA_ROTATE_UNDER].right = x + w;
	rButtonNormal[BUTTON_CAMERA_ROTATE_UNDER].top = y;
	rButtonNormal[BUTTON_CAMERA_ROTATE_UNDER].bottom = y + h;


	int n;
	int width;
	int height;
	for(n = 0; n < BUTTON_PIC; n ++)
	{
		width = rButtonNormal[n].right - rButtonNormal[n].left;

		rButtonMouseOver[n].left = rButtonNormal[n].left + width + 1;
		rButtonMouseOver[n].right = rButtonNormal[n].right + width + 1;

		rButtonPressed[n].left = rButtonMouseOver[n].left + width + 1;
		rButtonPressed[n].right = rButtonMouseOver[n].right + width + 1;

		rButtonPressed[n].top = rButtonMouseOver[n].top = rButtonNormal[n].top;
		rButtonPressed[n].bottom = rButtonMouseOver[n].bottom = rButtonNormal[n].bottom;
	}

	height = rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].bottom - rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].top;

	rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].top = rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].top + height + 1;
	rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].bottom = rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].bottom + height + 1;

	rButtonPressed[BUTTON_CAMERA_ROTATE_RIGHT].top = rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].top + height + 1;
	rButtonPressed[BUTTON_CAMERA_ROTATE_RIGHT].bottom = rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].bottom + height + 1;

	rButtonPressed[BUTTON_CAMERA_ROTATE_RIGHT].left = rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].left = rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].left;
	rButtonPressed[BUTTON_CAMERA_ROTATE_RIGHT].right = rButtonMouseOver[BUTTON_CAMERA_ROTATE_RIGHT].right = rButtonNormal[BUTTON_CAMERA_ROTATE_RIGHT].right;

	height = rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].bottom - rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].top;

	rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].top = rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].top + height + 1;
	rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].bottom = rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].bottom + height + 1;

	rButtonPressed[BUTTON_CAMERA_ROTATE_LEFT].top = rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].top + height + 1;
	rButtonPressed[BUTTON_CAMERA_ROTATE_LEFT].bottom = rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].bottom + height + 1;

	rButtonPressed[BUTTON_CAMERA_ROTATE_LEFT].left = rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].left = rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].left;
	rButtonPressed[BUTTON_CAMERA_ROTATE_LEFT].right = rButtonMouseOver[BUTTON_CAMERA_ROTATE_LEFT].right = rButtonNormal[BUTTON_CAMERA_ROTATE_LEFT].right;



	rButtonNormal[BUTTON_NONE].left = 0;
	rButtonNormal[BUTTON_NONE].top = 0;
	rButtonNormal[BUTTON_NONE].right = 16;
	rButtonNormal[BUTTON_NONE].bottom = 16;

	rButtonPressed[BUTTON_NONE].left = 0;
	rButtonPressed[BUTTON_NONE].top = 0;
	rButtonPressed[BUTTON_NONE].right = 16;
	rButtonPressed[BUTTON_NONE].bottom = 16;

	rButtonMouseOver[BUTTON_NONE].left = 0;
	rButtonMouseOver[BUTTON_NONE].top = 0;
	rButtonMouseOver[BUTTON_NONE].right = 16;
	rButtonMouseOver[BUTTON_NONE].bottom = 16;
	
	fclose(fp);
}

ZSButton::ZSButton(char *picname, int NewID, int x, int y, int width, int height, int basewidth, int baseheight, int NewNumFrames)
{
	char PicFileName[64];
	sprintf(PicFileName,"%s.bmp",picname);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(PicFileName,basewidth, baseheight * (NewNumFrames + 2) ,NULL, COLOR_KEY_FROM_FILE);
	
	Type = WINDOW_BUTTON;
	State = WINDOW_STATE_NORMAL;
	ID = NewID;
	Visible = FALSE;
	NumFrames = NewNumFrames;
	BaseHeight = baseheight;
	CurFrame = 0;
	SubFrame = 0;

	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	Normal.left = 0;
	Normal.top = 0;
	Pressed.left = 0;
	Pressed.right = Normal.right = basewidth;
	Pressed.top = Normal.bottom = baseheight;

	Pressed.bottom = Pressed.top + baseheight;

	MouseOver.left = 0;
	MouseOver.right = basewidth;
	MouseOver.top = Pressed.bottom;
	MouseOver.bottom = MouseOver.top + MouseOver.bottom;

	NextUpdate = timeGetTime();
}

int ZSButton::MoveMouse(long *x, long *y, long *z)
{
	pParent->MoveMouse(x,y,z);
	SetCursor(Cursor);
	return TRUE;
}

void ZSButton::ShutDown()
{
	if(ButtonSurface)
	{
		ButtonSurface->Release();
		ButtonSurface = NULL;
	}
}
