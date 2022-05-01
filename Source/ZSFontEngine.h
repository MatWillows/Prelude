//---------------------------------------------------------------------------
//
//	ZSFontEngine based in part on
//
//      The DirectDraw Bitmap Font Engine from "Fast Bitmap Fonts
//      for DirectDraw", which can be found at www.mr-gamemaker.com.
//
//		mat williams
//
//---------------------------------------------------------------------------

#ifndef DDFontEngineH
#define DDFontEngineH

#define NUM_FONT_COLORS 9

typedef enum
{
	TEXT_WHITE,
	TEXT_LIGHT_GREY,
	TEXT_DARK_GREY,
	TEXT_LIGHT_GREY_PARCHMENT,
	TEXT_DARK_GREY_PARCHMENT,
	TEXT_GREEN_PARCHMENT,
	TEXT_RED_PARCHMENT,
	TEXT_LIGHT_PAINTED_WOOD,
	TEXT_DARK_PAINTED_WOOD
} TEXT_COLOR_T;

#include <ddraw.h>
//---------------------------------------------------------------------------

// TEXTCOLOR macro
//      Used to pass textcolor to ZSFont constructor and ZSFont::SetTextColor
#define TEXTCOLOR(r, g, b) ((DWORD) (((BYTE) (b) | \
    ((WORD) (g) << 8)) | \
    (((DWORD) (BYTE) (r)) << 16)))


class ZSFontEngine;

class ZSFont
{
public:
    ZSFont(ZSFontEngine *ddfe, LPCSTR fontfile,
        DWORD textcolor=TEXTCOLOR(255, 255, 255));

    ~ZSFont();

    ZSFontEngine *Ddfe;
    LPDIRECTDRAWSURFACE7 lpFontSurf[NUM_FONT_COLORS];
	 char *FontFile;
    LPBITMAPINFO lpbi;
    LPVOID lpBits;
    TEXTMETRIC TextMetrics;
    ABC ABCWidths[256];
    LOGFONT LogFont;
    int SurfWidth, SurfHeight;
    int CellWidth, CellHeight;
    RECT SrcRects[256];     // Pre-calculated SrcRects for Blt
    int BPlusC[256];
    DWORD TextColor;

    bool LoadFont();
    void SetTextColor(DWORD textcolor);
};
//---------------------------------------------------------------------------

class ZSFontEngine
{
public:
    ZSFontEngine(LPDIRECTDRAW7 lpdd);
    ~ZSFontEngine();

    LPDIRECTDRAW7 lpDD;
    ZSFont *Ddf;
	
	void SelectDisplayFont(ZSFont *ddf);
    HRESULT DrawText(LPDIRECTDRAWSURFACE7 lpDDS, int x, int y, char *Text, int Color = 0);
	 HRESULT DrawText(LPDIRECTDRAWSURFACE7 lpDDS, RECT *rArea, char *Text, int Color = 0);
	 int BreakText(const char *Text, char ***NewLines, int LineWidth);
	 int CountLines(const char *Text, int LineWidth);
	 int SizeTextRect(const char *Text, RECT *rArea);	
	 int GetTextHeight();
	 int GetTextWidth(const char *Text);
	 int GetTextWidth(const char *Text, int start, int end);
	 int GetLetterWidth(int ACode);

};
//---------------------------------------------------------------------------

#endif
