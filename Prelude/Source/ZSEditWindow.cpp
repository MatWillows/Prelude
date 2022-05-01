#include "zseditwindow.h"
#include "zsengine.h"

char ZSEdit::ClipBoard[512];

ZSEdit::ZSEdit(WINDOW_T NewType, int NewID, int x, int y, int width, int height) 
{ 
	Type = NewType;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	Cursor = CURSOR_POINT;
	
	BackGroundSurface = NULL;

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	DrawCursor = FALSE;

	CharOffset = 0;
	
	//calculate limit


	return; 
}

void ZSEdit::AddChar(int ASCIICode)
{
	char *OldText;
	char *NewText;

	OldText = GetText();
	
	
	if(OldText)
	{
		int TWidth;
		TWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth("OldText");
		
		TWidth += Engine->Graphics()->GetFontEngine()->GetLetterWidth(ASCIICode);

		if(TWidth > ((Bounds.right - Bounds.left) - 8))
		{
			return;
		}

		NewText = new char[strlen(OldText) + 2];
	}
	else
	{
		NewText = new char[2];
		NewText[0] = ASCIICode;
		NewText[1] = '\0';
		Text = NewText;
		CharOffset++;
		return;

	}
	for(int n = 0; n < CharOffset; n++)
	{
		NewText[n] = OldText[n];
	}

	NewText[CharOffset] = (char)ASCIICode;

	for(n = CharOffset; n < strlen(OldText); n++)
	{
		NewText[n+1] = OldText[n];
	}

	NewText[strlen(OldText) + 1] = '\0';

	//get the length and confirm that it fits in the text window's box
	
	if(Engine->Graphics()->GetFontEngine()->GetTextWidth(NewText) > (Bounds.right - Bounds.left))
	{
		delete[] NewText;
		return;
	}

	delete[] Text;

	Text = NewText;

	if(pParent)
	{
		pParent->Command(ID, COMMAND_EDIT_CHANGED,ASCIICode);
	}

	CharOffset++;
	return;
}

void ZSEdit::BackSpace()
{
	char *OldText;
	char *NewText;

	OldText = GetText();

	if(OldText && CharOffset)
	{
		if(strlen(OldText))
		{
			NewText = new char[strlen(OldText)];
		}
		else
		{
			delete[] OldText;

			CharOffset = 0;
			return;
		}
	}
	else
	{
		return;
	}

	for(int n = 0; n < CharOffset -1; n++)
	{
		NewText[n] = OldText[n];
	}

	for(n = CharOffset; n < strlen(OldText); n++)
	{
		NewText[n-1] = OldText[n];
	}
	if(strlen(OldText))
	{
		NewText[strlen(OldText) - 1] = '\0';

		delete[] Text;

		Text = NewText;

		if(pParent)
		{
			pParent->Command(ID, COMMAND_EDIT_CHANGED,0);
		}
		CharOffset--;
	}
	else
	{
		delete[] Text;
		delete[] NewText;

		Text = NULL;
	
		CharOffset = 0;
	}
	return;

}

void ZSEdit::Delete()
{
	char *OldText;
	char *NewText;

	OldText = GetText();

	if(OldText && CharOffset)
	{
		if(strlen(OldText))
		{
			if(CharOffset >= strlen(OldText))
			{
				return;
			}

			NewText = new char[strlen(OldText)];
		}
		else
		{
			delete[] OldText;

			CharOffset = 0;
			return;
		}
	}
	else
	{
		return;
	}

	for(int n = 0; n < CharOffset; n++)
	{
		NewText[n] = OldText[n];
	}

	for(n = CharOffset+1; n < strlen(OldText); n++)
	{
		NewText[n-1] = OldText[n];
	}

	if(strlen(OldText))
	{
		NewText[strlen(OldText) - 1] = '\0';

		delete[] Text;

		Text = NewText;

		if(pParent)
		{
			pParent->Command(ID, COMMAND_EDIT_CHANGED,0);
		}
	}
	else
	{
		delete[] Text;
		delete[] NewText;

		Text = NULL;
		
		CharOffset = 0;
	}

	return;

}

int ZSEdit::MoveMouse(long *x, long *y, long *z)
{


return TRUE;
}

int ZSEdit::LeftButtonDown(int x, int y)
{
	if(pInputFocus != this)
	{
		SetFocus(this);
	}
	return TRUE;
}

int ZSEdit::LeftButtonUp(int x, int y)
{
	if(x <= Bounds.right && x >= Bounds.left && y >= Bounds.top && y <= Bounds.bottom)
	{
		if(Text)
		{
			CharOffset = strlen(Text);
		}
		else
		{
			CharOffset = 0;
		}
	}
	else
	{
		if(pInputFocus == this)
		{
			ReleaseFocus();
		}
	}
	
	return TRUE;
}

int ZSEdit::RightButtonDown(int x, int y)
{
	if(pInputFocus != this)
	{
		SetFocus(this);
	}
	return TRUE;
}

int ZSEdit::RightButtonUp(int x, int y)
{
	if(pInputFocus == this)
	{
		ReleaseFocus();
	}
	return TRUE;
}

int ZSEdit::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{

	#define Pressed(x)		(CurrentKeys[x] & 0x80)
	#define LastPressed(x)	(LastKeys[x] & 0x80)
	
//this is going to be big

	if(Pressed(DIK_ESCAPE))
	{
		ReleaseFocus();
		return TRUE;
	}
	if((Pressed(DIK_RETURN) && !LastPressed(DIK_RETURN)) || (Pressed(DIK_NUMPADENTER) && !LastPressed(DIK_NUMPADENTER)))
	{
		ReleaseFocus();
		if(pParent)
		{
			pParent->Command(ID, COMMAND_EDIT_DONE, 0);
		}
		return TRUE;
	}

	if(Pressed(DIK_BACKSPACE) && !LastPressed(DIK_BACKSPACE))
	{
		BackSpace();
		return TRUE;
	}

	if(Pressed(DIK_DELETE) && !LastPressed(DIK_DELETE))
	{
		Delete();
		return TRUE;
	}

	if(Pressed(DIK_SPACE) && !LastPressed(DIK_SPACE))
	{
		AddChar(' ');
		return TRUE;
	}

	if(Pressed(DIK_LCONTROL) || Pressed(DIK_RCONTROL))
	{
		if(Pressed(DIK_C) && !LastPressed(DIK_C))
		{
			strcpy(ClipBoard,Text);	
			return TRUE;
		}
		if(Pressed(DIK_V) && !LastPressed(DIK_V))
		{
			SetText(ClipBoard);
			SetOffset(strlen(Text));
			return TRUE;
		}
	}


	//process all alpha numeric characters
	//is the shift key down
	if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
	{
		//a shift key down
		if(Pressed(DIK_A) && !LastPressed(DIK_A))
		{
			AddChar('A');
			return TRUE;
		}
		if(Pressed(DIK_B) && !LastPressed(DIK_B))
		{
			AddChar('B');
			return TRUE;
		}
		if(Pressed(DIK_C) && !LastPressed(DIK_C))
		{
			AddChar('C');
			return TRUE;
		}
		if(Pressed(DIK_D) && !LastPressed(DIK_D))
		{
			AddChar('D');
			return TRUE;
		}
		if(Pressed(DIK_E) && !LastPressed(DIK_E))
		{
			AddChar('E');
			return TRUE;
		}
		if(Pressed(DIK_F) && !LastPressed(DIK_F))
		{
			AddChar('F');
			return TRUE;
		}
		if(Pressed(DIK_G) && !LastPressed(DIK_G))
		{
			AddChar('G');
			return TRUE;
		}
		if(Pressed(DIK_H) && !LastPressed(DIK_H))
		{
			AddChar('H');
			return TRUE;
		}
		if(Pressed(DIK_I) && !LastPressed(DIK_I))
		{
			AddChar('I');
			return TRUE;
		}
		if(Pressed(DIK_J) && !LastPressed(DIK_J))
		{
			AddChar('J');
			return TRUE;
		}
		if(Pressed(DIK_K) && !LastPressed(DIK_K))
		{
			AddChar('K');
			return TRUE;
		}
		if(Pressed(DIK_L) && !LastPressed(DIK_L))
		{
			AddChar('L');
			return TRUE;
		}
		if(Pressed(DIK_M) && !LastPressed(DIK_M))
		{
			AddChar('M');
			return TRUE;
		}
		if(Pressed(DIK_N) && !LastPressed(DIK_N))
		{
			AddChar('N');
			return TRUE;
		}
		if(Pressed(DIK_O) && !LastPressed(DIK_O))
		{
			AddChar('O');
			return TRUE;
		}
		if(Pressed(DIK_P) && !LastPressed(DIK_P))
		{
			AddChar('P');
			return TRUE;
		}
		if(Pressed(DIK_Q) && !LastPressed(DIK_Q))
		{
			AddChar('Q');
			return TRUE;
		}
		if(Pressed(DIK_R) && !LastPressed(DIK_R))
		{
			AddChar('R');
			return TRUE;
		}
		if(Pressed(DIK_S) && !LastPressed(DIK_S))
		{
			AddChar('S');
			return TRUE;
		}
		if(Pressed(DIK_T) && !LastPressed(DIK_T))
		{
			AddChar('T');
			return TRUE;
		}
		if(Pressed(DIK_U) && !LastPressed(DIK_U))
		{
			AddChar('U');
			return TRUE;
		}
		if(Pressed(DIK_V) && !LastPressed(DIK_V))
		{
			AddChar('V');
			return TRUE;
		}
		if(Pressed(DIK_W) && !LastPressed(DIK_W))
		{
			AddChar('W');
			return TRUE;
		}
		if(Pressed(DIK_X) && !LastPressed(DIK_X))
		{
			AddChar('X');
			return TRUE;
		}
		if(Pressed(DIK_Y) && !LastPressed(DIK_Y))
		{
			AddChar('Y');
			return TRUE;
		}
		if(Pressed(DIK_Z) && !LastPressed(DIK_Z))
		{
			AddChar('Z');
			return TRUE;
		}
		//numbers
		if(Pressed(DIK_1) && !LastPressed(DIK_1))
		{
			AddChar('!');
			return TRUE;
		}
		if(Pressed(DIK_2) && !LastPressed(DIK_2))
		{
			AddChar('@');
			return TRUE;
		}
		if(Pressed(DIK_3) && !LastPressed(DIK_3))
		{
			AddChar('#');
			return TRUE;
		}
		if(Pressed(DIK_4) && !LastPressed(DIK_4))
		{
			AddChar('$');
			return TRUE;
		}
		if(Pressed(DIK_5) && !LastPressed(DIK_5))
		{
			AddChar('%');
			return TRUE;
		}
		if(Pressed(DIK_6) && !LastPressed(DIK_6))
		{
			AddChar('^');
			return TRUE;
		}
		if(Pressed(DIK_7) && !LastPressed(DIK_7))
		{
			AddChar('&');
			return TRUE;
		}
		if(Pressed(DIK_8) && !LastPressed(DIK_8))
		{
			AddChar('*');
			return TRUE;
		}
		if(Pressed(DIK_9) && !LastPressed(DIK_9))
		{
			AddChar('(');
			return TRUE;
		}
		if(Pressed(DIK_0) && !LastPressed(DIK_0))
		{
			AddChar(')');
			return TRUE;
		}
		if(Pressed(DIK_MINUS) && !LastPressed(DIK_MINUS))
		{
			AddChar('_');
			return TRUE;
		}
		if(Pressed(DIK_EQUALS) && !LastPressed(DIK_EQUALS))
		{
			AddChar('+');
			return TRUE;
		}
		//non alpha characters
		if(Pressed(DIK_LBRACKET) && !LastPressed(DIK_LBRACKET))
		{
			AddChar('{');
			return TRUE;
		}
		if(Pressed(DIK_RBRACKET) && !LastPressed(DIK_RBRACKET))
		{
			AddChar('}');
			return TRUE;
		}
		if(Pressed(DIK_SEMICOLON) && !LastPressed(DIK_SEMICOLON))
		{
			AddChar(':');
			return TRUE;
		}
		if(Pressed(DIK_APOSTROPHE) && !LastPressed(DIK_APOSTROPHE))
		{
			AddChar('"');
			return TRUE;
		}
		if(Pressed(DIK_GRAVE) && !LastPressed(DIK_GRAVE))
		{
			AddChar('~');
			return TRUE;
		}
		if(Pressed(DIK_BACKSLASH) && !LastPressed(DIK_BACKSLASH))
		{
			AddChar('|');
			return TRUE;
		}
		if(Pressed(DIK_COMMA) && !LastPressed(DIK_COMMA))
		{
			AddChar('<');
			return TRUE;
		}
		if(Pressed(DIK_PERIOD) && !LastPressed(DIK_PERIOD))
		{
			AddChar('>');
			return TRUE;
		}
		if(Pressed(DIK_SLASH) && !LastPressed(DIK_SLASH))
		{
			AddChar('?');
			return TRUE;
		}
	}
	else //shift keys not down
	{
		if(Pressed(DIK_A) && !LastPressed(DIK_A))
		{
			AddChar('a');
			return TRUE;
		}
		if(Pressed(DIK_B) && !LastPressed(DIK_B))
		{
			AddChar('b');
			return TRUE;
		}
		if(Pressed(DIK_C) && !LastPressed(DIK_C))
		{
			AddChar('c');
			return TRUE;
		}
		if(Pressed(DIK_D) && !LastPressed(DIK_D))
		{
			AddChar('d');
			return TRUE;
		}
		if(Pressed(DIK_E) && !LastPressed(DIK_E))
		{
			AddChar('e');
			return TRUE;
		}
		if(Pressed(DIK_F) && !LastPressed(DIK_F))
		{
			AddChar('f');
			return TRUE;
		}
		if(Pressed(DIK_G) && !LastPressed(DIK_G))
		{
			AddChar('g');
			return TRUE;
		}
		if(Pressed(DIK_H) && !LastPressed(DIK_H))
		{
			AddChar('h');
			return TRUE;
		}
		if(Pressed(DIK_I) && !LastPressed(DIK_I))
		{
			AddChar('i');
			return TRUE;
		}
		if(Pressed(DIK_J) && !LastPressed(DIK_J))
		{
			AddChar('j');
			return TRUE;
		}
		if(Pressed(DIK_K) && !LastPressed(DIK_K))
		{
			AddChar('k');
			return TRUE;
		}
		if(Pressed(DIK_L) && !LastPressed(DIK_L))
		{
			AddChar('l');
			return TRUE;
		}
		if(Pressed(DIK_M) && !LastPressed(DIK_M))
		{
			AddChar('m');
			return TRUE;
		}
		if(Pressed(DIK_N) && !LastPressed(DIK_N))
		{
			AddChar('n');
			return TRUE;
		}
		if(Pressed(DIK_O) && !LastPressed(DIK_O))
		{
			AddChar('o');
			return TRUE;
		}
		if(Pressed(DIK_P) && !LastPressed(DIK_P))
		{
			AddChar('p');
			return TRUE;
		}
		if(Pressed(DIK_Q) && !LastPressed(DIK_Q))
		{
			AddChar('q');
			return TRUE;
		}
		if(Pressed(DIK_R) && !LastPressed(DIK_R))
		{
			AddChar('r');
			return TRUE;
		}
		if(Pressed(DIK_S) && !LastPressed(DIK_S))
		{
			AddChar('s');
			return TRUE;
		}
		if(Pressed(DIK_T) && !LastPressed(DIK_T))
		{
			AddChar('t');
			return TRUE;
		}
		if(Pressed(DIK_U) && !LastPressed(DIK_U))
		{
			AddChar('u');
			return TRUE;
		}
		if(Pressed(DIK_V) && !LastPressed(DIK_V))
		{
			AddChar('v');
			return TRUE;
		}
		if(Pressed(DIK_W) && !LastPressed(DIK_W))
		{
			AddChar('w');
			return TRUE;
		}
		if(Pressed(DIK_X) && !LastPressed(DIK_X))
		{
			AddChar('x');
			return TRUE;
		}
		if(Pressed(DIK_Y) && !LastPressed(DIK_Y))
		{
			AddChar('y');
			return TRUE;
		}
		if(Pressed(DIK_Z) && !LastPressed(DIK_Z))
		{
			AddChar('z');
			return TRUE;
		}

		//numbers
		if(Pressed(DIK_1) && !LastPressed(DIK_1))
		{
			AddChar('1');
			return TRUE;
		}
		if(Pressed(DIK_2) && !LastPressed(DIK_2))
		{
			AddChar('2');
			return TRUE;
		}
		if(Pressed(DIK_3) && !LastPressed(DIK_3))
		{
			AddChar('3');
			return TRUE;
		}
		if(Pressed(DIK_4) && !LastPressed(DIK_4))
		{
			AddChar('4');
			return TRUE;
		}
		if(Pressed(DIK_5) && !LastPressed(DIK_5))
		{
			AddChar('5');
			return TRUE;
		}
		if(Pressed(DIK_6) && !LastPressed(DIK_6))
		{
			AddChar('6');
			return TRUE;
		}
		if(Pressed(DIK_7) && !LastPressed(DIK_7))
		{
			AddChar('7');
			return TRUE;
		}
		if(Pressed(DIK_8) && !LastPressed(DIK_8))
		{
			AddChar('8');
			return TRUE;
		}
		if(Pressed(DIK_9) && !LastPressed(DIK_9))
		{
			AddChar('9');
			return TRUE;
		}
		if(Pressed(DIK_0) && !LastPressed(DIK_0))
		{
			AddChar('0');
			return TRUE;
		}
		if(Pressed(DIK_MINUS) && !LastPressed(DIK_MINUS))
		{
			AddChar('-');
			return TRUE;
		}
		if(Pressed(DIK_EQUALS) && !LastPressed(DIK_EQUALS))
		{
			AddChar('=');
			return TRUE;
		}	
		//non alpha characters
		if(Pressed(DIK_LBRACKET) && !LastPressed(DIK_LBRACKET))
		{
			AddChar('[');
			return TRUE;
		}
		if(Pressed(DIK_RBRACKET) && !LastPressed(DIK_RBRACKET))
		{
			AddChar(']');
			return TRUE;
		}
		if(Pressed(DIK_SEMICOLON) && !LastPressed(DIK_SEMICOLON))
		{
			AddChar(';');
			return TRUE;
		}
		if(Pressed(DIK_APOSTROPHE) && !LastPressed(DIK_APOSTROPHE))
		{
			AddChar('\'');
			return TRUE;
		}
		if(Pressed(DIK_GRAVE) && !LastPressed(DIK_GRAVE))
		{
			AddChar('`');
			return TRUE;
		}
		if(Pressed(DIK_BACKSLASH) && !LastPressed(DIK_BACKSLASH))
		{
			AddChar('\\');
			return TRUE;
		}
		if(Pressed(DIK_COMMA) && !LastPressed(DIK_COMMA))
		{
			AddChar(',');
			return TRUE;
		}
		if(Pressed(DIK_PERIOD) && !LastPressed(DIK_PERIOD))
		{
			AddChar('.');
			return TRUE;
		}
		if(Pressed(DIK_SLASH) && !LastPressed(DIK_SLASH))
		{
			AddChar('/');
			return TRUE;
		}
	}

	if(Pressed(DIK_LEFTARROW) && !LastPressed(DIK_LEFTARROW))
	{
		if(CharOffset)
		{
			CharOffset--;
			return TRUE;
		}
	}

	if(Pressed(DIK_RIGHTARROW) && !LastPressed(DIK_RIGHTARROW))
	{
		if(Text && CharOffset < strlen(Text))
		{
			CharOffset++;
			return TRUE;
		}
	}


	return TRUE;
}

int ZSEdit::Draw()
{
	//only draw ourself if we're visible
	if(Visible)
	{
		//draw our background if any
		if(BackGroundSurface)
		{
				Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}
		//Nor Children
		

		//draw our text in the upper left corner
		if(Text)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), Bounds.left + Border, Bounds.top + Border, Text, TextColor);
		}
		if(DrawCursor)
		{
			RECT CursorRect;
			CursorRect.left = Border + Bounds.left + Engine->Graphics()->GetFontEngine()->GetTextWidth(Text,0,CharOffset);
			CursorRect.right = CursorRect.left + 1;
			CursorRect.top =  Bounds.top + Border;
			CursorRect.bottom = Bounds.top + Engine->Graphics()->GetFontEngine()->GetTextHeight();
			Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&CursorRect,RGB(255,255,255));
		}
	}

	//draw our sibling if we have one
	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE; 

}

void ZSEdit::GainFocus()
{
	DEBUG_INFO("  Gaining focus: ");
	if(Text)
	{
		DEBUG_INFO(Text);
	}

	DrawCursor = TRUE;
	if(pParent)
	{
		pParent->Command(ID,COMMAND_CHILD_GAINFOCUS,NULL);
	}
	return;
}

void ZSEdit::LoseFocus()
{
	DEBUG_INFO("  Losinging focus: ");
	if(Text)
	{
		DEBUG_INFO(Text);
	}

	DrawCursor = FALSE;
	if(pParent)
	{
		pParent->Command(ID,COMMAND_CHILD_LOSEFOCUS,NULL);
	}
	return;
}
