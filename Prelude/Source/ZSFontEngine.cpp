//---------------------------------------------------------------------------
//
//  DDFontEngine -  Michael Fötsch.
//	
//	
//      The DirectDraw Bitmap Font Engine from "Fast Bitmap Fonts
//      for DirectDraw", which can be found at www.mr-gamemaker.com.
//      The tutorial and a TrueType-to-DDF converter are also available at
//      http://www.crosswinds.net/~foetsch
//
//		ZSFontEngine extensions Mat Williams
//
//			size rect un implemented
//			drawing into a rectangle unimplemented
//---------------------------------------------------------------------------

#include "ZSFontEngine.h"
#include "ZSEngine.h"
#include "defs.h"
#include <assert.h>

#define SAFERELEASE(x)      if (x) { x->Release(); x = NULL; }
#define SAFEDELETE(x)       if (x) { delete x; x = NULL; }
#define SAFEDELETEARRAY(x)  if (x) { delete[] x; x = NULL; }

#define FONT_FILE_WIDTH		720
#define FONT_FILE_HEIGHT	294

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//-------------------------- class ZSFont ----------------------------------
//---------------------------------------------------------------------------

// ZSFont constructor
//      -> ddfe: Initialized ZSFontEngine that will be used to render the font
//      -> fontfile: Name of DDF file that should be loaded
//      -> textcolor: Desired color of text. Use TEXTCOLOR macro.
ZSFont::ZSFont(ZSFontEngine *ddfe, LPCSTR fontfile, DWORD textcolor)
{
    Ddfe = ddfe;
	 for(int n = 0; n < NUM_FONT_COLORS; n++)
	 {
		 lpFontSurf[n] = NULL;
	 }
    FontFile = new char[strlen(fontfile)+1]; 
	 strcpy(FontFile, fontfile);
    lpbi = NULL;
    lpBits = NULL;
    TextColor = textcolor;

    LoadFont();                 // Load font file
}
//---------------------------------------------------------------------------

// ZSFont destructor
ZSFont::~ZSFont()
{
	 for(int n = 0; n < NUM_FONT_COLORS; n++)
	 {
       SAFERELEASE(lpFontSurf[n]);    // Release DirectDraw Surface
	 }
    SAFEDELETEARRAY(FontFile);  // Free buffer for file name
    SAFEDELETEARRAY(lpBits);    // Free buffer for bitmap bits
    SAFEDELETEARRAY(lpbi);      // Free buffer for BITMAPINFO
}
//---------------------------------------------------------------------------

// ZSFont::LoadFont
//      Creates a DirectDraw Surface, loads the bitmap in the DDF File
//      into it and reads the text metrics and ABC widths into class structures
//
bool ZSFont::LoadFont()
{
    HANDLE f;
    HDC SurfDC=NULL;
    bool Success=false;

try
{
    // If DDF file is loaded for the first time (i.e. not as a response to
    // DDERR_SURFACELOST), load bitmap data and text metrics.
    // (Otherwise, the structures are already filled. Skip to StretchDIBits.)
    if (!lpFontSurf[0])
    {
        if ((f = CreateFile(
                FontFile,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL)) == INVALID_HANDLE_VALUE)
            throw (int)0;

        DWORD BytesRead;
        BITMAPFILEHEADER bmfh;
        ZeroMemory(&bmfh, sizeof(bmfh));
        ReadFile(f, &bmfh, sizeof(bmfh), &BytesRead, NULL);

     // Check whether it's a valid DDF file:
        if ((bmfh.bfType != 19778)  // 'BM'
          || (bmfh.bfReserved1 != 'DD') || (bmfh.bfReserved2 != 'FF'))
           throw (int)1;

        // Read BITMAPINFO and bitmap bits:
        lpbi = (LPBITMAPINFO)(new char[bmfh.bfOffBits - sizeof(bmfh)]);
        if (!lpbi) throw (int)2;    // ("new" should throw anyway but...)
        ReadFile(f, lpbi, bmfh.bfOffBits-sizeof(bmfh), &BytesRead, NULL);

        lpBits = (LPVOID)(new char[bmfh.bfSize-bmfh.bfOffBits]);
        ReadFile(f, lpBits, bmfh.bfSize-bmfh.bfOffBits, &BytesRead, NULL);

        SurfWidth = lpbi->bmiHeader.biWidth;
        SurfHeight = lpbi->bmiHeader.biHeight;

	     // Read text metrics:
        ReadFile(f, &TextMetrics, sizeof(TextMetrics), &BytesRead, NULL);
        ReadFile(f, (LPVOID)((DWORD)ABCWidths+32*sizeof(ABC)),
            224*sizeof(ABC), &BytesRead, NULL);
        if (!ReadFile(f, &LogFont, sizeof(LogFont), &BytesRead, NULL))
            throw (int)4;
        if (BytesRead < sizeof(LogFont)) throw (int)4;

        CellWidth = SurfWidth >> 4;
        CellHeight = SurfHeight / 14;

        // Pre-calculate SrcRects:
        ZeroMemory(SrcRects, 32*sizeof(RECT));
        //ZeroMemory(ABCWidths, 32*sizeof(ABC));
        ZeroMemory(BPlusC, 32*sizeof(int));
        
		int c;
		for(c = 0; c < 32; c++)
		{
			ABCWidths[c].abcA = 0;
			
			ABCWidths[c].abcB = 0;
		
			ABCWidths[c].abcC = 0;
			
			BPlusC[c] = 0;
            BPlusC[c] = ABCWidths[c].abcB + ABCWidths[c].abcC;
    	}

		for (c=32; c < 256; c++)
        {
            SrcRects[c].left = ((c-32) % 16) * CellWidth;
            SrcRects[c].top = ((c-32) >> 4) * CellHeight;
				//sert(ABCWidths[c].abcB);
            SrcRects[c].right = SrcRects[c].left + CellWidth - 1;//ABCWidths[c].abcB;
            SrcRects[c].bottom = SrcRects[c].top + CellHeight;
			BPlusC[c] = 0;
            BPlusC[c] = ABCWidths[c].abcB + ABCWidths[c].abcC;
			assert(BPlusC[c] < CellWidth);
			assert(BPlusC[c] >= 0);
        }

		 

    }
/*
    if (FAILED(lpFontSurf->GetDC(&SurfDC))) throw (int)6;
    StretchDIBits(SurfDC,
        0, 0, SurfWidth, SurfHeight,
        0, 0, SurfWidth, SurfHeight,
        lpBits,
        lpbi,
        DIB_RGB_COLORS,
        SRCCOPY);
    lpFontSurf->ReleaseDC(SurfDC);
	lpFontSurf->Release();
*/
	 DDSURFACEDESC2 TempDescription;

	lpFontSurf[0] = Engine->Graphics()->CreateSurfaceFromFile("fontwhite.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[1] = Engine->Graphics()->CreateSurfaceFromFile("fontlightgrey.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[2] = Engine->Graphics()->CreateSurfaceFromFile("fontdarkgrey.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[3] = Engine->Graphics()->CreateSurfaceFromFile("fontlightgreyparch.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[4] = Engine->Graphics()->CreateSurfaceFromFile("fontdarkgreyparch.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[5] = Engine->Graphics()->CreateSurfaceFromFile("fontgreenparch.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[6] = Engine->Graphics()->CreateSurfaceFromFile("fontredparch.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[7] = Engine->Graphics()->CreateSurfaceFromFile("fontpainted.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);
	lpFontSurf[8] = Engine->Graphics()->CreateSurfaceFromFile("fontpainteddark.bmp", FONT_FILE_WIDTH, FONT_FILE_HEIGHT, NULL,COLOR_KEY_FROM_FILE);

/*	WORD Mask;
	WORD* SurfPtr;  
	
   for(int n = 0; n < NUM_FONT_COLORS; n++)
   {
		ZeroMemory( &TempDescription, sizeof( TempDescription ));  
		TempDescription.dwSize = sizeof( TempDescription );  

		lpFontSurf[n]->Lock( NULL, &TempDescription, DDLOCK_WAIT, NULL );  

		SurfPtr = ( WORD* ) (( BYTE* )TempDescription.lpSurface);

		Mask = SurfPtr[SurfWidth + SurfWidth - 2];
		
		lpFontSurf[n]->Unlock( NULL );
		
		DDCOLORKEY ddck;
		ddck.dwColorSpaceHighValue = Mask;
		ddck.dwColorSpaceLowValue = Mask;

		lpFontSurf[n]->SetColorKey(DDCKEY_SRCBLT , &ddck);
   }
*/
   Success = true;
}
catch (...)
{
    SAFERELEASE(lpFontSurf[0]);
}
    CloseHandle(f);

    return Success;
}
//---------------------------------------------------------------------------

// ZSFont::SetTextColor
//      Changes the text color of the font
//      -> textcolor: Desired color of text. Use TEXTCOLOR macro.
void ZSFont::SetTextColor(DWORD textcolor)
{
    TextColor = textcolor;

    // Change foreground palette entry to requested text color:
    // (Assumes that entry 1 is foreground and 0 is background)
    DWORD *palentry = (DWORD*)&lpbi->bmiColors[1];
    *palentry = TextColor;

    HDC SurfDC;
    if (!FAILED(lpFontSurf[0]->GetDC(&SurfDC)))
    {    StretchDIBits(SurfDC,
            0, 0, SurfWidth, SurfHeight,
            0, 0, SurfWidth, SurfHeight,
            lpBits,
            lpbi,
            DIB_RGB_COLORS,
            SRCCOPY);
        lpFontSurf[0]->ReleaseDC(SurfDC);
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//-------------------------- class ZSFontEngine ----------------------------
//---------------------------------------------------------------------------

ZSFontEngine::ZSFontEngine(LPDIRECTDRAW7 lpdd)
{
    lpDD = lpdd;
    Ddf = 0;
}
//---------------------------------------------------------------------------

ZSFontEngine::~ZSFontEngine()
{
}
//---------------------------------------------------------------------------

void ZSFontEngine::SelectDisplayFont(ZSFont *ddf)
{
    Ddf = ddf;
}
//---------------------------------------------------------------------------

HRESULT ZSFontEngine::DrawText(LPDIRECTDRAWSURFACE7 lpDDS, int x, int y,
    char *Text, int color)
{
	 HRESULT hr;
    LPDIRECTDRAWSURFACE7 SrcSurf = Ddf->lpFontSurf[color];
/*
    if (SrcSurf->IsLost() == DDERR_SURFACELOST)
    {
        SrcSurf->Restore(); Ddf->LoadFont();
    }
*/

	 RECT rTo;
	 rTo.top = y;
	 rTo.bottom = y + Ddf->CellHeight;
			
	 int StringLength = strlen(Text);
    UCHAR ch;
    for (int i=0; i < StringLength; i++)
    {
        ch = Text[i];
			  x += Ddf->ABCWidths[ch].abcA;
			  rTo.left = x;
			  rTo.right = x + Ddf->CellWidth - 1;
			  hr = lpDDS->Blt(&rTo, SrcSurf, &Ddf->SrcRects[ch],
					DDBLT_KEYSRC,NULL);
			  if(hr != DD_OK)
			  {
				  char blarg[256];
				  sprintf(blarg,"failed to draw %s at %i,%i\n",Text,x,y);
				  DEBUG_INFO(blarg);
				  return DDERR_GENERIC;
			  }
			  x += Ddf->BPlusC[ch];
		
	 }

    return DD_OK;
}
//---------------------------------------------------------------------------

HRESULT ZSFontEngine::DrawText(LPDIRECTDRAWSURFACE7 lpDDS, RECT *rArea,
    char *Text, int color)
{
	 HRESULT hr;
    LPDIRECTDRAWSURFACE7 SrcSurf = Ddf->lpFontSurf[color];
/*
    if (SrcSurf->IsLost() == DDERR_SURFACELOST)
    {
        SrcSurf->Restore(); Ddf->LoadFont();
    }
*/
	int x;
	x = rArea->left;
	int y = 0;
	 RECT rTo;
	 rTo.top = rArea->top;
	 rTo.bottom = rArea->top + Ddf->CellHeight;
			
	 int StringLength = strlen(Text);
	 int WordLength;
    UCHAR ch;
	 int n;
    for (int i=0; i < StringLength; i++)
    {
      ch = Text[i];
		if(isspace(ch) && ch != '\\')
		{
			//count the next word
			n = i + 1;
			WordLength = 0;
			while(!isspace(Text[n]) && Text[n] != '\\' && Text[n] != '\0')
			{
				WordLength += Ddf->BPlusC[Text[n]];
				n++;
			}
			if(x + WordLength + 2 >= rArea->right)
			{
				x = rArea->left;
				rTo.top += Ddf->CellHeight;
				rTo.bottom += Ddf->CellHeight;
			}
		}
  		
		if(ch != '\\')
		{
		  if(x == rArea->left && isspace(ch))
		  {
			  
		  }
		  else
		  {
			  x += Ddf->ABCWidths[ch].abcA;
			  if( x > rArea->right)
			  {
				  x = rArea->left;
				  rTo.top += GetTextHeight();
				  rTo.bottom += GetTextHeight();
			  }
			  
			  rTo.left = x;
			  rTo.right = x + Ddf->CellWidth - 1;
			  hr = lpDDS->Blt(&rTo, SrcSurf, &Ddf->SrcRects[ch],
					DDBLT_KEYSRC,NULL);
			  if(hr != DD_OK)
			  {
				  char blarg[256];
				  sprintf(blarg,"failed to draw %s at %i,%i\n",Text,x,y);
				  DEBUG_INFO(blarg);
				  return DDERR_GENERIC;
				}
			  x += Ddf->BPlusC[ch];
		  }
		 }
		else
		{
			i++;
			ch = Text[i];
			if(ch == 'n')
			{
				x = rArea->left;
				rTo.top += GetTextHeight();
				rTo.bottom += GetTextHeight();
			}
		 }
    }

    return DD_OK;
}

int ZSFontEngine::CountLines(const char *Text, int LineWidth)
{
	int NumLines = 1;
	int StringLength = strlen(Text);
	int Width = 0;
	char ch;
	for(int n = 0; n < StringLength; n++)
	{
		ch = Text[n];
		if(ch == '\r' || ch == '\n' || (ch == '\\' && Text[n +1] == 'n'))
		{
			NumLines++;
			Width = 0;
		}
		else
		{
			Width += Ddf->ABCWidths[ch].abcA + Ddf->ABCWidths[ch].abcB + Ddf->ABCWidths[ch].abcC;
			if(Width > LineWidth)
			{
				NumLines++;
				Width = 0;
			}
		}
	}

	return NumLines;
}


//break a block of text into lines that are less than a given pixel width
int ZSFontEngine::BreakText(const char *Text, char ***NewLines, int LineWidth)
{
	int NumLines = 1;
	int EndPoints[20];
	//first count the number of lines we have, and mark the endpoint of each line
	int StringLength = strlen(Text);
	int Curwidth = 0;
	int n, sn;
	char ch;
	char **Lines;

	EndPoints[0] = 0;

	for (n = 0; n < StringLength; n++)
	{
		ch = Text[n];
		Curwidth += Ddf->ABCWidths[ch].abcA + Ddf->ABCWidths[ch].abcB + Ddf->ABCWidths[ch].abcC;
		
		if(ch == '\r' || ch == '\n' || (ch == '\\' && Text[n +1] == 'n'))
		{
			if(ch == '\\' && Text[n +1] == 'n')
			{
				EndPoints[NumLines] = n + 2;
				n++;
				NumLines++;
				Curwidth = 0;
			}
			else
			{
				//break only between words
				n++;
				EndPoints[NumLines] = n;
				NumLines++;
				Curwidth = 0;
			}
		}
		else
		if(Curwidth >= LineWidth)
		{
			//break only between words
			sn = n-1;
			while(!isspace(Text[sn]))
			{
				if(sn == 0 || (NumLines && sn <= EndPoints[NumLines-1]))
				{
					DEBUG_INFO("Couldn't break text\n");
					DEBUG_INFO(Text);
					Lines = new char *[1];
					Lines[0] = new char[strlen(Text)+1];
					strcpy(Lines[0],Text);
					*NewLines = Lines;
					return 1;
				}
				sn--;
			
			}
			n = sn+1;
			EndPoints[NumLines] = n;
			NumLines++;
			Curwidth = 0;
		}
		
	}
	EndPoints[NumLines] = StringLength+1;
	NumLines++;

	//allocate the line array
	Lines = new char *[NumLines-1];

	//add each line to the array
	for(n = 0; n < (NumLines-1); n++)
	{
		//allocate space for the new line
		Lines[n] = new char[EndPoints[n+1] - EndPoints[n] + 1];

		//copy the line from the text
		for(sn = EndPoints[n]; sn < (EndPoints[n+1]-1); sn++)
		{
			Lines[n][sn - EndPoints[n]] = Text[sn];
		}
		Lines[n][(EndPoints[n+1]-EndPoints[n])-1] = '\0'; 
		if(Lines[n][(EndPoints[n+1] - EndPoints[n]) - 2] == '\\')
		{
			Lines[n][(EndPoints[n+1] - EndPoints[n]) - 2] = ' ';
		}
	}

	*NewLines = Lines;
	//return the number of lines
	return (NumLines-1);
}


//size a rectangle to fin a block of text
int ZSFontEngine::SizeTextRect(const char *Text, RECT *rArea)
{
	int TextLength = 0;


	return TextLength;
}

int ZSFontEngine::GetTextHeight()
{
	return Ddf->CellHeight;
}

int ZSFontEngine::GetTextWidth(const char *Text)
{
	return GetTextWidth(Text,0,strlen(Text));
}

int ZSFontEngine::GetTextWidth(const char *Text, int start, int end)
{
	//confirm that a string exists
	if(!Text)
	{
		return 0;
	}

	int n;
	int Width = 0;	//the length of the text in pixels
	
	int TextLength = strlen(Text);
	
	//read from start to end or last character whichever is first
	if(TextLength > end)
	{
		TextLength = end;
	}

	//total the width of each character and add it to the length
	for(n = start; n < TextLength; n++)
	{
		Width += (Ddf->ABCWidths[Text[n]].abcA + Ddf->ABCWidths[Text[n]].abcB + Ddf->ABCWidths[Text[n]].abcC);
	}

	//return the length
	return Width;
}

int ZSFontEngine::GetLetterWidth(int ACode)
{
	return Ddf->ABCWidths[ACode].abcA + Ddf->ABCWidths[ACode].abcB + Ddf->ABCWidths[ACode].abcC;
}



