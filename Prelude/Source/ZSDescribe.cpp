#include "ZSDescribe.h"
#include "zsverticalscroll.h"
#include "zsbutton.h"
#include "zsengine.h"
#include "translucentwindow.h"
#include "world.h"

#define IDC_LIST_SCROLL 1024

DescribeItem::DescribeItem(char *NewText, int NewID)
{
	Text = new char[strlen(NewText) + 1];
	strcpy(Text,NewText);
	TextColor = TEXT_DARK_GREY_PARCHMENT;
	
	pNext = NULL;
	pPrev = NULL;

}


DescribeItem::~DescribeItem()
{
	delete[] Text;
	if(pPrev)
	{
		pPrev->pNext = pNext;
	}
	if(pNext)
	{
		pNext->pPrev = pPrev;
	}

}

void ZSDescribe::SetTextColor(int NewColor)
{
	DescribeItem *pDI;

	pDI = pCurTop;

	if(pDI)
	{
		while(pDI->pPrev)
		{
			pDI = pDI->pPrev;
		}
	}

	while(pDI)
	{
		pDI->TextColor = NewColor;
		pDI = pDI->pNext;
	}
}



ZSDescribe::~ZSDescribe()
{
	Clear();
	//PreludeWorld->RemoveMainObject(pTrans);
	delete pTrans;
}


void ZSDescribe::AddItem(char *Text)
{
	//first find the bottom of the list of items
	DescribeItem *pDI, *pNewLI;

	pDI = pCurTop;

	if(pDI)
	{
		while(pDI->pNext)
		{
			pDI = pDI->pNext;
		}
	}

	int NumLines;
	char **NewLines;

	NumLines = Engine->Graphics()->GetFontEngine()->BreakText(Text, &NewLines, (Bounds.right - Bounds.left) - (Border * 3 + 4));

	for(int n = 0; n < NumLines; n++)
	{

		pNewLI = new DescribeItem(NewLines[n], NumItems);
	
		pNewLI->pPrev = pDI;
		if(pDI)
		{
			pDI->pNext = pNewLI;
		}

		pDI = pNewLI;

		NumSelectors++;
		delete[] NewLines[n];
	}
	
	delete[] NewLines;

	NumItems++;

	ZSVScroll	*pScroll;

	pScroll = (ZSVScroll *)GetChild(IDC_LIST_SCROLL);

	pScroll->SetUpper(NumSelectors);

	if(NumSelectors - NumVisible < 1)
	{
		pScroll->Hide();
	}
	else
	{
		pScroll->Show();
	}

	if(NumSelectors > MAX_DESCRIBE_ITEMS)
	{
		while(pDI->pPrev)
		{
			pDI = pDI->pPrev;
		}

		while(NumSelectors > MAX_DESCRIBE_ITEMS)
		{
			pDI = pDI->pNext;
			if(pCurTop = pDI->pPrev)
				pCurTop = pDI->pNext;
			delete pDI->pPrev;
			pDI->pPrev = NULL;
			NumSelectors--;
		}
	}

	int Offset;

	if(pCurTop)
	{
		pDI = pCurTop;
		while(pDI->pNext)
		{
			pDI = pDI->pNext;
		}

		Offset = NumSelectors;

		for(n = 1; n < NumVisible; n++)
		{
			if(pDI->pPrev)
			{
				pDI = pDI->pPrev;
				Offset--;
			}
		}
	
		pCurTop = pDI;

		pScroll->SetPos(Offset);
	}
	else
	{
		if(pDI)
		{
			Offset = NumSelectors;

			for(n = 1; n < NumVisible; n++)
			{
				if(pDI->pPrev)
				{
					pDI = pDI->pPrev;
					Offset--;
				}
			}
		
			pCurTop = pDI;

			pScroll->SetPos(Offset);
		
			pCurTop = pDI;
		}
	}
}

void ZSDescribe::AddItem(char *Text, int NewTextColor)
{
	//first find the bottom of the list of items
	DescribeItem *pDI, *pNewLI;

	pDI = pCurTop;

	if(pDI)
	{
		while(pDI->pNext)
		{
			pDI = pDI->pNext;
		}
	}

	int NumLines;
	char **NewLines;

	NumLines = Engine->Graphics()->GetFontEngine()->BreakText(Text, &NewLines, (Bounds.right - Bounds.left) - 30);

	for(int n = 0; n < NumLines; n++)
	{

		pNewLI = new DescribeItem(NewLines[n], NumItems);
		pNewLI->TextColor = NewTextColor;
	
		pNewLI->pPrev = pDI;
		if(pDI)
		{
			pDI->pNext = pNewLI;
		}

		pDI = pNewLI;

		NumSelectors++;
		delete[] NewLines[n];
	}
	
	delete[] NewLines;

	NumItems++;

	ZSVScroll	*pScroll;

	pScroll = (ZSVScroll *)GetChild(IDC_LIST_SCROLL);

	pScroll->SetUpper(NumSelectors);

	if(NumSelectors - NumVisible < 1)
	{
		pScroll->Hide();
	}
	else
	{
		pScroll->Show();
	}

	if(NumSelectors > MAX_DESCRIBE_ITEMS)
	{
		while(pDI->pPrev)
		{
			pDI = pDI->pPrev;
		}

		while(NumSelectors > MAX_DESCRIBE_ITEMS)
		{
			pDI = pDI->pNext;
			delete pDI->pPrev;
			pDI->pPrev = NULL;
			NumSelectors--;
		}
	}

	if(pCurTop)
	{
		pDI = pCurTop;
		while(pDI->pNext)
		{
			pDI = pDI->pNext;
		}

		int Offset = NumSelectors;

		for(n = 1; n < NumVisible; n++)
		{
			if(pDI->pPrev)
			{
				pDI = pDI->pPrev;
				Offset--;
			}
		}
	
		pCurTop = pDI;

		pScroll->SetPos(Offset);
	}
	else
	{
		if(pDI)
		{
			while(pDI->pNext)
			{
				pDI = pDI->pNext;
			}

			int Offset = NumSelectors;

			for(n = 1; n < NumVisible; n++)
			{
				if(pDI->pPrev)
				{
					pDI = pDI->pPrev;
					Offset--;
				}
			}
		
			pCurTop = pDI;

			pScroll->SetPos(Offset);

		}
	}
}

int ZSDescribe::Command(int IDFrom, int Command, int Param)
{
	DescribeItem *pDI;
	int n;

	if(Command == COMMAND_SCROLL)
	{
		pDI = pCurTop;

		while(pDI->pPrev)
		{
			pDI = pDI->pPrev;
		}

		n = 0;
		while(pDI && n < Param)
		{
			pDI = pDI->pNext;
			n++;
		}
		if(pDI)
			pCurTop = pDI;	
	}

	
	return TRUE;
}

void ZSDescribe::Clear()
{
	DescribeItem *pDI;

	pDI = pCurTop;

	if(!pDI)
	{
		return;
	}

	while(pDI->pPrev)
	{
		pDI = pDI->pPrev;
	}

	while(pDI)
	{
		pCurTop = pDI->pNext;
		delete pDI;
		pDI = pCurTop;
	}
	NumItems = 0;
	NumSelectors = 0;
}

ZSDescribe::ZSDescribe(int NewID, int x, int y, int Width, int Height, BOOL IsParchment)
{
	Type = WINDOW_LIST;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	Cursor = CURSOR_POINT;
	
	Border = 8;

	Parchment = IsParchment;

	if(!Parchment)
	{
	//	CreateEmptyBorderedBackground(8);
		Border = 6;
	}
	else
	{
		CreateParchmentBorderedBackground(8);
	}

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	NumItems = 0;
	pCurTop = NULL;
	NumSelectors = 0;
	NumVisible = ((Bounds.bottom - Bounds.top) - Border * 2) / (Engine->Graphics()->GetFontEngine()->GetTextHeight() - 1);

	ZSWindow *pWin;
	pWin = new ZSVScroll(IDC_LIST_SCROLL, Bounds.right - (Border*2), Bounds.top, (Border*2), Bounds.bottom - Bounds.top);
	pWin->Hide();
	AddChild(pWin);

	((ZSVScroll *)pWin)->SetPage(NumVisible);

/*	if(!IsParchment)
	{
		pTrans = new TransWin(this, D3DRGBA(0.0f, 0.0f, 0.0f, 0.25f), 0);

		Valley->AddToDrawList((Object *)pTrans);
	}
*/	return; 
}

int ZSDescribe::Draw()
{
	//first standard draw
	if(Visible)
	{
		//draw our background if any
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}

	
		//now draw text
		DescribeItem *pDI;
		pDI = pCurTop;
		int x,y, FHeight;
		x = Bounds.left + Border + 3;
		y = Bounds.top + Border;
			
		FHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();

		while(pDI && y + FHeight <= (Bounds.bottom - Border))
		{
			if(Parchment)
				Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pDI->Text, pDI->TextColor);
			else
				Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pDI->Text, TEXT_LIGHT_PAINTED_WOOD);
			pDI = pDI->pNext;
			y += FHeight;
		}
		
		if(pChild)
		{
			pChild->Draw();
		}

	}
	
	if(pSibling)
	{
		pSibling->Draw();
	}

	return TRUE;
}

void ZSDescribe::ResetToTop()
{
	if(!pCurTop)
		return;
	while(pCurTop->pPrev)
	{
		pCurTop = pCurTop->pPrev;
	}
	
	ZSVScroll	*pScroll;

	pScroll = (ZSVScroll *)GetChild(IDC_LIST_SCROLL);

	pScroll->SetPos(0);
}
	

