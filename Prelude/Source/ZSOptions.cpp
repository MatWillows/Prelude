#include "zsoptions.h"
#include "zsbutton.h"
#include "world.h"
#include "zsengine.h"
#include "zstext.h"
#include "zsutilities.h"
#include "zssaveload.h"
#include "zsintspin.h"
#include "world.h"
#include "zsconfirm.h"
#include "zsmessage.h"
#include "zshelpwin.h"

//options
LPDIRECTDRAWSURFACE7 ZSOptionWin::OptionsSurface = NULL;
LPDIRECTDRAWSURFACE7 ZSMainOptionsMenu::MainOptionsSurface = NULL;

typedef enum
{
	IDC_FX_VOLUME,
    IDC_FX_TOGGLE,
	IDC_MUSIC_VOLUME,
	IDC_MUSIC_TOGGLE,
	IDC_SCREEN_SIZE,
	IDC_BACKSIDE_DRAW,
	IDC_TEXTURE_FILTERING,
	IDC_FRAME_SKIP,
	IDC_TARGET_HIGHLIGHT,
	IDC_DIFFICULTY_LEVEL,
	IDC_QUIT_OPTIONS,
	IDC_DEFAULT_OPTIONS,
	IDC_SHADOW_TOGGLE,
	IDC_PARTICLE_SYSTEM,
	IDC_REDUCED_TEXTURES,
	IDC_SMOKE,
	IDC_AUTOSAVE_RATE,
	IDC_XPCONFIRM,
	IDC_FULLBODYSELECT,
	IDC_HIGHLIGHTINFO,
	IDC_VERBOSECOMBAT

} OPTIONS_CONTROLS;

typedef enum
{
	IDC_LOAD_GAME,
	IDC_SAVE_GAME,
	IDC_OPTIONS,
	IDC_QUIT_GAME,
	IDC_RETURN_TO_GAME,
} MAIN_OPTIONS_MENU;


void ZSOptionWin::LoadSettings()
{
	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");

	BOOL Test;

	ZSWindow *pWin;
	
/*	SeekTo(fp,"WIDTH");
	int Width;
	Width = GetInt(fp);

	pWin = GetChild(IDC_SCREEN_SIZE);

	switch(Width)
	{
	case 640:
		pWin->SetText("640x480");
		break;
	case 800:
		pWin->SetText("800x600");
		break;
	case 1024:
		pWin->SetText("1024x768");
		break;
	case 1280:
		pWin->SetText("1280x1024");
		break;
	default:
		break;
	}

	SeekTo(fp,"HEIGHT");
*/	
	SeekTo(fp,"SOUND");	
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_FX_TOGGLE);
	Test = Engine->Sound()->FxAreOn();
	if(Test)
	{
		pWin->SetText("SoundFX On");
	}
	else
	{
		pWin->SetText("SoundFX Off");
	}
	SeekTo(fp,"SOUNDVOLUME");
	Test = GetInt(fp);
		
	SeekTo(fp,"MUSIC");				
	Test = (BOOL)GetInt(fp);

	Test = Engine->Sound()->MusicIsOn();
	pWin = GetChild(IDC_MUSIC_TOGGLE);
	if(Test)
	{
		pWin->SetText("Music On");
	}
	else
	{
		pWin->SetText("Music Off");
	}

	SeekTo(fp,"MUSICVOLUME");
	Test = GetInt(fp);


	SeekTo(fp,"FRAMESKIP");			
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_FRAME_SKIP);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	/*
	SeekTo(fp,"FILTERING");			
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_TEXTURE_FILTERING);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"BACKSIDES");			
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_BACKSIDE_DRAW);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"SHADOWS");			
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_SHADOW_TOGGLE);
	Test = PreludeWorld->GetDrawShadows();
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"PARTICLES");			
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_PARTICLE_SYSTEM);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"TEXTUREREDUCTION");
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_REDUCED_TEXTURES);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"SMOKE");				
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_SMOKE);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}

	SeekTo(fp,"TARGETHIGHLIGHT");				
	Test = (BOOL)GetInt(fp);
	pWin = GetChild(IDC_TARGET_HIGHLIGHT);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}

*/
	SeekTo(fp,"DIFFICULTY");
	int DiffLevel = GetInt(fp);
	PreludeWorld->SetDifficulty(DiffLevel);
	pWin = GetChild(IDC_DIFFICULTY_LEVEL);
	if(DiffLevel == 0)
	{
		pWin->SetText("Easy");
	}
	else
	if(DiffLevel == 1)
	{
		pWin->SetText("Normal");
	}
	else
	if(DiffLevel == 2)
	{
		pWin->SetText("Hard");
	}

	SeekTo(fp,"XPCONFIRM");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetXPConfirm(Test);
	pWin = GetChild(IDC_XPCONFIRM);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"FULLBODYSELECT");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetFullSelect(Test);
	pWin = GetChild(IDC_FULLBODYSELECT);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"HIGHLIGHTINFO");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetHighlightInfo(Test);
	pWin = GetChild(IDC_HIGHLIGHTINFO);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	SeekTo(fp,"VERBOSECOMBAT");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetVerboseCombat(Test);
	pWin = GetChild(IDC_VERBOSECOMBAT);
	if(Test)
	{
		pWin->SetText("On");
	}
	else
	{
		pWin->SetText("Off");
	}
	
	fclose(fp);
}

void ZSOptionWin::SaveSettings()
{
	FILE *fp;
	FILE *ftemp;
	fp = SafeFileOpen("gui.ini","rt");
	ftemp = SafeFileOpen("temp.txt","wt");
	fprintf(ftemp,"[MAIN]\n");

	ZSWindow *pWin;

	
	//pWin = GetChild(IDC_SCREEN_SIZE);
/*	if(!strcmp(pWin->GetText(),"640x480"))
	{
		fprintf(ftemp,"WIDTH  640\nHEIGHT  480\n");
	}
	else
	if(!strcmp(pWin->GetText(),"800x600"))
	{
*/
		fprintf(ftemp,"WIDTH  800\nHEIGHT  600\n");
/*	}
	else
	if(!strcmp(pWin->GetText(),"1024x768"))
	{
		fprintf(ftemp,"WIDTH  1024\nHEIGHT  768\n");
	}
	else
	if(!strcmp(pWin->GetText(),"1280x1024"))
	{
		fprintf(ftemp,"WIDTH  1280\nHEIGHT  1024\n");
	}
*/
	fprintf(ftemp,"WINDOWED");
	SeekTo(fp,"WINDOWED");
	if(GetInt(fp))
	{
		fprintf(ftemp,"    1\n");
	}
	else
	{
		fprintf(ftemp,"    0\n");
	}


	fprintf(ftemp,"DRAWWORLD");
	SeekTo(fp,"DRAWWORLD");
	if(GetInt(fp))
	{
		fprintf(ftemp,"    1\n");
	}
	else
	{
		fprintf(ftemp,"    0\n");
	}


	fprintf(ftemp,"SOUND");	
	pWin = GetChild(IDC_FX_TOGGLE);
	if(Engine->Sound()->FxAreOn())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"		0\n");
	}
	fprintf(ftemp,"SOUNDVOLUME		%i\n", Engine->Sound()->GetFXVolume());		
	fprintf(ftemp,"MUSIC");				

	if(Engine->Sound()->MusicIsOn())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	0\n");
	}

	fprintf(ftemp,"MUSICVOLUME		%i\n", Engine->Sound()->GetMusicVolume());	
	
	fprintf(ftemp,"AUTOSAVERATE		%i\n", PreludeWorld->GetAutosaveRate());
	
	fprintf(ftemp,"FRAMESKIP");			
	pWin = GetChild(IDC_FRAME_SKIP);
	if(!strcmp(pWin->GetText(),"On"))
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	0\n");
	}
	fprintf(ftemp,"FILTERING");			
//	pWin = GetChild(IDC_TEXTURE_FILTERING);
//	if(!strcmp(pWin->GetText(),"On"))
//	{
		fprintf(ftemp,"   1\n");
//	}
//	else
//	{
//		fprintf(ftemp,"	0\n");
//	}

	
	fprintf(ftemp,"BACKSIDES");			
//	pWin = GetChild(IDC_BACKSIDE_DRAW);
	if(PreludeWorld->RenderingBackTerrain())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	0\n");
	}

	fprintf(ftemp,"SHADOWS");			
//	pWin = GetChild(IDC_SHADOW_TOGGLE);
	if(PreludeWorld->GetDrawShadows())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	0\n");
	}
	
	fprintf(ftemp,"PARTICLES");			
//	pWin = GetChild(IDC_PARTICLE_SYSTEM);
//	if(!strcmp(pWin->GetText(),"On"))
//	{
		fprintf(ftemp,"   1\n");
//	}
//	else
//	{
//		fprintf(ftemp,"	0\n");
//	}
	
	fprintf(ftemp,"TEXTUREREDUCTION");
//	pWin = GetChild(IDC_REDUCED_TEXTURES);
	if(ZSTexture::GetReduction())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	0\n");
	}
	
	fprintf(ftemp,"SMOKE");				
//	pWin = GetChild(IDC_SMOKE);
//	if(!strcmp(pWin->GetText(),"On"))
//	{
		fprintf(ftemp,"   1\n");
//	}
//	else
//	{
//		fprintf(ftemp,"	0\n");
//	}

	fprintf(ftemp,"TARGETHIGHLIGHT");				
//	pWin = GetChild(IDC_TARGET_HIGHLIGHT);
//	if(!strcmp(pWin->GetText(),"On"))
//	{
		fprintf(ftemp,"   1\n");
//	}
//	else
//	{
//		fprintf(ftemp,"	0\n");
//	}


	fprintf(ftemp,"DIFFICULTY");
	int DifLevel;
	DifLevel = PreludeWorld->GetDifficulty();
	fprintf(ftemp,"   %i\n", DifLevel);

	fprintf(ftemp,"XPCONFIRM");
	if(PreludeWorld->XPConfirm())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	   0\n");
	}
	fprintf(ftemp,"FULLBODYSELECT");
	if(PreludeWorld->FullSelect())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	   0\n");
	}
	fprintf(ftemp,"HIGHLIGHTINFO");
	if(PreludeWorld->HighlightInfo())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	   0\n");
	}
	fprintf(ftemp,"VERBOSECOMBAT");
	if(PreludeWorld->VerboseCombat())
	{
		fprintf(ftemp,"   1\n");
	}
	else
	{
		fprintf(ftemp,"	   0\n");
	}

	SeekTo(fp,"VERBOSECOMBAT");
	GetInt(fp);

	char c = '\0';

	c = (char)fgetc(fp);

	while(!feof(fp))
	{
		fputc((int)c,ftemp);
		c = (char)fgetc(fp);
	}

	fclose(fp);
	fclose(ftemp);
	remove("gui.ini");
	rename("temp.txt","gui.ini");


}

int ZSOptionWin::Command(int IDFrom, int Command, int Param)
{
	ZSWindow *pWin;
	BOOL Test;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
/*		case IDC_SHADOW_TOGGLE:
			Test = PreludeWorld->GetDrawShadows();
			pWin = GetChild(IDC_SHADOW_TOGGLE);

			if(Test)
			{
				pWin->SetText("Off");
				PreludeWorld->SetDrawShadows(FALSE);
			}
			else
			{
				pWin->SetText("On");
				PreludeWorld->SetDrawShadows(TRUE);
			}

			break;
*/
		case IDC_DIFFICULTY_LEVEL:
			int DifLevel;
			pWin = GetChild(IDC_DIFFICULTY_LEVEL);

			DifLevel = PreludeWorld->GetDifficulty();
			DifLevel ++;
			if(DifLevel > 2) DifLevel = 0;
			PreludeWorld->SetDifficulty(DifLevel);
			switch(DifLevel)
			{
				default:
				case 0:
					pWin->SetText("Easy");
					break;
				case 1:
					pWin->SetText("Norm");
					break;
				case 2:
					pWin->SetText("Hard");
					break;

			}

			break;
		case IDC_XPCONFIRM:
			Test = PreludeWorld->XPConfirm();
			pWin = GetChild(IDC_XPCONFIRM);

			if(Test)
			{
				pWin->SetText("Off");
				PreludeWorld->SetXPConfirm(FALSE);
			}
			else
			{
				pWin->SetText("On");
				PreludeWorld->SetXPConfirm(TRUE);
			}
			break;
		case IDC_FULLBODYSELECT:
			Test = PreludeWorld->FullSelect();
			pWin = GetChild(IDC_FULLBODYSELECT);

			if(Test)
			{
				pWin->SetText("Off");
				PreludeWorld->SetFullSelect(FALSE);
			}
			else
			{
				pWin->SetText("On");
				PreludeWorld->SetFullSelect(TRUE);
			}
			break;
		case IDC_HIGHLIGHTINFO:
			Test = PreludeWorld->HighlightInfo();
			pWin = GetChild(IDC_HIGHLIGHTINFO);

			if(Test)
			{
				pWin->SetText("Off");
				PreludeWorld->SetHighlightInfo(FALSE);
			}
			else
			{
				pWin->SetText("On");
				PreludeWorld->SetHighlightInfo(TRUE);
			}
			break;
		case IDC_VERBOSECOMBAT:
			Test = PreludeWorld->VerboseCombat();
			pWin = GetChild(IDC_VERBOSECOMBAT);

			if(Test)
			{
				pWin->SetText("Off");
				PreludeWorld->SetVerboseCombat(FALSE);
			}
			else
			{
				pWin->SetText("On");
				PreludeWorld->SetVerboseCombat(TRUE);
			}
			break;


		case IDC_FX_TOGGLE:
			Test = Engine->Sound()->FxAreOn();
			if(Test)
			{
				Engine->Sound()->SetFX(FALSE);
				pWin = GetChild(IDC_FX_TOGGLE);
				pWin->SetText("SoundFX Off");
			}
			else
			{
				Engine->Sound()->SetFX(TRUE);
				pWin = GetChild(IDC_FX_TOGGLE);
				pWin->SetText("SoundFX On");
			}
			break;
		case IDC_MUSIC_TOGGLE:
			Test = Engine->Sound()->MusicIsOn();
			if(Test)
			{
				Engine->Sound()->SetMusic(FALSE);
				pWin = GetChild(IDC_MUSIC_TOGGLE);
				pWin->SetText("Music Off");
			}
			else
			{
				Engine->Sound()->SetMusic(TRUE);
				pWin = GetChild(IDC_MUSIC_TOGGLE);
				pWin->SetText("Music On");
			}
			break;
/*
		case IDC_TEXTURE_FILTERING:
		case IDC_PARTICLE_SYSTEM:
		case IDC_REDUCED_TEXTURES:
			Message("Changes will not occur until next time the game is started.","OK");
			pWin = GetChild(IDFrom);
			if(!strcmp(pWin->GetText(),"On"))
			{
				pWin->SetText("Off");
			}
			else
			{
				pWin->SetText("On");
			}
			break;

		case IDC_SCREEN_SIZE:
			pWin = GetChild(IDC_SCREEN_SIZE);
			if(!strcmp(pWin->GetText(),"640x480"))
			{
				pWin->SetText("800x600");
			}
			else
			if(!strcmp(pWin->GetText(),"800x600"))
			{
				pWin->SetText("1024x768");
			}
			else
			if(!strcmp(pWin->GetText(),"1024x768"))
			{
				pWin->SetText("1280x1024");
			}
			else
			if(!strcmp(pWin->GetText(),"1280x1024"))
			{
				pWin->SetText("640x480");
			}
			break;
		case IDC_DIFFICULTY_LEVEL:
			break;
*/		case IDC_QUIT_OPTIONS:
			State = WINDOW_STATE_DONE;
			Hide();
			break;
		case IDC_DEFAULT_OPTIONS:
			if(Confirm(this,"Restore Default Settings?","Yes","No"))
			{
			//	pWin = GetChild(IDC_SHADOW_TOGGLE);
			//	pWin->SetText("On");
			//	PreludeWorld->SetDrawShadows(TRUE);
				Engine->Sound()->SetFX(TRUE);
				pWin = GetChild(IDC_FX_TOGGLE);
				pWin->SetText("SoundFX On");
				Engine->Sound()->SetMusic(TRUE);
				pWin = GetChild(IDC_MUSIC_TOGGLE);
				pWin->SetText("Music On");
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)this->GetChild(IDC_MUSIC_VOLUME);
				pISpin->SetValue(5);
				Engine->Sound()->SetMusicVolume(pISpin->GetValue());
				pISpin = (ZSIntSpin *)this->GetChild(IDC_FX_VOLUME);
				pISpin->SetValue(5);
				Engine->Sound()->SetFXVolume(pISpin->GetValue());
				pISpin = (ZSIntSpin *)this->GetChild(IDC_AUTOSAVE_RATE);
				pISpin->SetValue(15);
				PreludeWorld->SetAutosaveRate(15);
			}
			break;
		default:
			pWin = GetChild(IDFrom);
			if(!strcmp(pWin->GetText(),"On"))
			{
				pWin->SetText("Off");
			}
			else
			{
				pWin->SetText("On");
			}
			break;
		}
	}
	if(Command == COMMAND_EDIT_CHANGED)
	{
		ZSIntSpin *pISpin;
		pISpin = (ZSIntSpin *)this->GetChild(IDFrom);
		
		switch (IDFrom)
		{
		case IDC_MUSIC_VOLUME:
			Engine->Sound()->SetMusicVolume(pISpin->GetValue());
			break;
		case IDC_FX_VOLUME:
			Engine->Sound()->SetFXVolume(pISpin->GetValue());
			break;
		case IDC_AUTOSAVE_RATE:
			PreludeWorld->SetAutosaveRate(pISpin->GetValue());
			break;
		}
	}
	return TRUE;
}

ZSOptionWin::ZSOptionWin(int NewID)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Type = WINDOW_OPTIONS;
	Moveable = FALSE;

	RECT rBase;
	RECT rCurrent;
	RECT rLoad;

	rBase.left = 0;
	rBase.right = 800;
	rBase.top = 0;
	rBase.bottom = 600;
	
	rCurrent.left = 0;
	rCurrent.top = 0;
	rCurrent.right = Engine->Graphics()->GetWidth();
	rCurrent.bottom = Engine->Graphics()->GetHeight();

	//create all the buttons
	ZSButton *pButton;
	ZSText	*pText;
	
	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[OPTIONS]");
	SeekTo(fp,"POSITION");

	LoadRect(&rLoad,fp);
//	ScaleRect(&rLoad, &rBase, &rCurrent);

	Bounds = rLoad;
	Bounds.left -= 8;
	Bounds.top -= 8;
	Bounds.bottom += 8;
	Bounds.right += 8;

	if(!OptionsSurface)
	{
		CreateWoodBorderedBackground(8,2);
		OptionsSurface = BackGroundSurface;
	}

	BackGroundSurface = OptionsSurface;
	BackGroundSurface->AddRef();

	SeekTo(fp,"FXTOGGLE");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_FX_TOGGLE, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"FXVOLUME");
	LoadRect(&rLoad,fp);
	ZSIntSpin *pISpin;
	pISpin = new ZSIntSpin(IDC_FX_VOLUME, XYWH(rLoad));
	pISpin->SetValue(Engine->Sound()->GetFXVolume());
	pISpin->SetMinMax(1,10);
	pISpin->Show();
	pISpin->SetPlusMinus();
	AddChild(pISpin);

	
	SeekTo(fp,"MUSICTOGGLE");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_MUSIC_TOGGLE, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	
	SeekTo(fp,"MUSICVOLUME");	
	LoadRect(&rLoad,fp);

	pISpin = new ZSIntSpin(IDC_MUSIC_VOLUME, XYWH(rLoad));
	pISpin->SetValue(Engine->Sound()->GetMusicVolume());
	pISpin->SetMinMax(1,10);
	pISpin->Show();
	AddChild(pISpin);
	pISpin->SetPlusMinus();

	SeekTo(fp,"AUTOSAVE");	
	LoadRect(&rLoad,fp);

	pISpin = new ZSIntSpin(IDC_AUTOSAVE_RATE, XYWH(rLoad));
	pISpin->SetValue(PreludeWorld->GetAutosaveRate());
	pISpin->SetMinMax(0,60);
	pISpin->Show();
	AddChild(pISpin);
	pISpin->SetPlusMinus();

	pText = new ZSText(IDC_AUTOSAVE_RATE + 50, rLoad.right + 2, rLoad.top, "Autosave Rate (minutes)");
	pText->Show();
	AddChild(pText);


	SeekTo(fp,"SCREENSIZE");	
	LoadRect(&rLoad,fp);

//	pButton = new ZSButton(BUTTON_NORMAL, IDC_SCREEN_SIZE, XYWH(rLoad));

//resolution change disabled
//	pButton->Hide();
//	AddChild(pButton);

	SeekTo(fp,"BACKSIDEDRAW");
	LoadRect(&rLoad,fp);

	SeekTo(fp,"XPCONFIRM");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_XPCONFIRM, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pText = new ZSText(IDC_XPCONFIRM + 50, rLoad.right + 2, rLoad.top, "XP Confirmation");
	pText->Show();
	AddChild(pText);

//	pButton = new ZSButton(BUTTON_NORMAL, IDC_BACKSIDE_DRAW, XYWH(rLoad));
//	pButton->Show();
//	AddChild(pButton);

//	pText = new ZSText(IDC_BACKSIDE_DRAW + 50, rLoad.right + 2, rLoad.top, "Back Face Terrain Draw");
//	pText->Show();
//	AddChild(pText);
	
//	SeekTo(fp,"FILTERING");
//	LoadRect(&rLoad,fp);

//	pButton = new ZSButton(BUTTON_NORMAL, IDC_TEXTURE_FILTERING, XYWH(rLoad));
//	pButton->Show();
//	AddChild(pButton);

//	pText = new ZSText(IDC_BACKSIDE_DRAW + 50, rLoad.right + 2, rLoad.top, "Bilinear Texture Filtering");
//	pText->Show();
//	AddChild(pText);

	SeekTo(fp,"FRAMESKIP");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_FRAME_SKIP, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);

	pText = new ZSText(IDC_BACKSIDE_DRAW + 50, rLoad.right + 2, rLoad.top, "Animation Frame Skip");
	pText->Show();
	AddChild(pText);

	SeekTo(fp,"HIGHLIGHTINFO");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_HIGHLIGHTINFO, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pText = new ZSText(IDC_HIGHLIGHTINFO + 50, rLoad.right + 2, rLoad.top, "Addition Highlight Info");
	pText->Show();
	AddChild(pText);

	SeekTo(fp,"FULLBODYSELECT");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_FULLBODYSELECT, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pText = new ZSText(IDC_FULLBODYSELECT + 50, rLoad.right + 2, rLoad.top, "Full Body Selection");
	pText->Show();
	AddChild(pText);

	SeekTo(fp,"VERBOSECOMBAT");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_VERBOSECOMBAT, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pText = new ZSText(IDC_VERBOSECOMBAT + 50, rLoad.right + 2, rLoad.top, "Verbose Combat");
	pText->Show();
	AddChild(pText);

	SeekTo(fp,"EXIT");
	LoadRect(&rLoad,fp);		

	pButton = new ZSButton(BUTTON_NORMAL, IDC_QUIT_OPTIONS, XYWH(rLoad));
	pButton->Show();
	pButton->SetText("Exit");
	AddChild(pButton);

	SeekTo(fp,"DIFFICULTY");	
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_DIFFICULTY_LEVEL, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pText = new ZSText(IDC_DIFFICULTY_LEVEL + 50, rLoad.right + 2, rLoad.top, "Difficulty");
	pText->Show();
	AddChild(pText);
	
	SeekTo(fp,"DEFAULTS");
	LoadRect(&rLoad,fp);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_DEFAULT_OPTIONS, XYWH(rLoad));
	pButton->Show();
	AddChild(pButton);
	pButton->SetText("Defaults");
		
	fclose(fp);
}

void ZSOptionWin::Show()
{
	Visible = TRUE;
	LoadSettings();
}

void ZSOptionWin::Hide()
{
	Visible = FALSE;
	SaveSettings();
}

int ZSMainOptionsMenu::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_LOAD_GAME:
			LoadWin *pLoadWin;
			pLoadWin = new LoadWin(-1,100,100,500,400);
			pLoadWin->Show();
			GetMain()->AddTopChild(pLoadWin);
			SetFocus(pLoadWin);
			Hide();
			pLoadWin->GoModal();
			ReleaseFocus();

			GetMain()->RemoveChild(pLoadWin);
			break;

		case IDC_SAVE_GAME:
			SaveWin *pSaveWin;
			pSaveWin = new SaveWin(-1,100,100,500,400);
			pSaveWin->Show();
			GetMain()->AddTopChild(pSaveWin);
			SetFocus(pSaveWin);
			Hide();
			pSaveWin->GoModal();
			ReleaseFocus();

			GetMain()->RemoveChild(pSaveWin);
			
			break;
		case IDC_OPTIONS:
			ZSOptionWin *pZSOptionWin;
			pZSOptionWin = new ZSOptionWin(-1);
			pZSOptionWin->Show();
			GetMain()->AddTopChild(pZSOptionWin);
			SetFocus(pZSOptionWin);
			Hide();
			pZSOptionWin->GoModal();
			ReleaseFocus();
			GetMain()->RemoveChild(pZSOptionWin);
			break;

		case IDC_QUIT_GAME:
			if(Confirm(ZSWindow::GetMain(),"Are you sure?","Quit","Return"))
				GetMain()->SetState(WINDOW_STATE_DONE);
			else
				return TRUE;
			break;
		case IDC_RETURN_TO_GAME:
			break;
		}
		Hide();
		State = WINDOW_STATE_DONE;
	}
	

	return TRUE;
}


ZSMainOptionsMenu::ZSMainOptionsMenu(int NewID)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = 342;
	Bounds.right = 458;
 	Bounds.top = 192;
	Bounds.bottom = Bounds.top + 24 * 5 + 16;

	if(!MainOptionsSurface)
	{
		CreateWoodBorderedBackground(8,2);
		MainOptionsSurface = BackGroundSurface;
	}

	BackGroundSurface = MainOptionsSurface;
	BackGroundSurface->AddRef();

	
	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_RETURN_TO_GAME, 350, 200, 100, 24);
	pButton->Show();
	pButton->SetText("Return");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOAD_GAME, 350, 224, 100, 24);
	pButton->Show();
	pButton->SetText("Load");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SAVE_GAME, 350, 248, 100, 24);
	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		pButton->Hide();
	else
		pButton->Show();
	pButton->SetText("Save");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_OPTIONS, 350, 272, 100, 24);
	pButton->Show();
	pButton->SetText("Configure");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_QUIT_GAME, 350, 296, 100, 24);
	pButton->Show();
	pButton->SetText("Quit");
	AddChild(pButton);



}

int ZSOptionWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		ShowHelp("Configure");
		return TRUE;
	}
	if(CurrentKeys[DIK_ESCAPE] & 0x80)
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;
}

