#include "ZStext.h"
#include "ZSEngine.h"

int ZSText::Draw()
{
	if(Visible)
	{
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}

		RECT DrawTo;
		DrawTo.left = Bounds.left + Border;
		DrawTo.right = Bounds.right - Border;
		DrawTo.top = Bounds.top + Border;
		DrawTo.bottom = Bounds.bottom - Border;
		Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), &DrawTo, Text, TextColor);
	}
	if(pSibling)
	{
		pSibling->Draw();
	}
	if(Duration)
	{
		CurFrame++;
		if(CurFrame > Duration * 30)
		{
			pParent->RemoveChild(this);
		}
	}
	return TRUE;
}

ZSText::ZSText(int NewID, int x, int y, char *string)
{
	int Width;

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth(string);

	ID = NewID;
	Type = WINDOW_TEXT;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + Width + 4;
	Bounds.bottom = y +20;
	Text = NULL;
	pSibling = NULL;
	Duration = 0;
	CurFrame = 0;
	Moveable = FALSE;
	Visible = FALSE;
	SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	
	SetText(string);

}

ZSText::ZSText(int NewID, int x, int y, char *string, int NewDuration)
{
	int Width;

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth(string);

	ID = NewID;
	Type = WINDOW_TEXT;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + Width;
	Bounds.bottom = y + 20;
	Text = NULL;

	pSibling = NULL;
	Duration = NewDuration;
	CurFrame = 0;
	
	Moveable = FALSE;
	SetTextColor(TEXT_LIGHT_PAINTED_WOOD);

	SetText(string);

}

ZSText::ZSText(int NewID, int x, int y, int width, int height, char *string, int NewDuration, BOOL Parchment)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Type = WINDOW_TEXT;

	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;
	Text = NULL;
	pSibling = NULL;
	pChild = NULL;
	Moveable = FALSE;

	Duration = NewDuration;
	
	SetText(string);

	if(Parchment)
	{
		CreateParchmentBorderedBackground(4);
		SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	}
	else
	{
		SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	}

}
