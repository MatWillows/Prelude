//version 0.72
// fixed combat event calling switching game out of combat mode
// integrated starting event
//version 0.73
// reintegrated spellcasting
//to be done
// fix saving and loading games

// Don't use MFC for this project
#define WIN32_LEAN_AND_MEAN 

//basic definitions
#include "defs.h"

//copyprotection
#include "registration.h"

//main window
#include "mainwindow.h"
#include "ZSbutton.h"

#include <time.h>
#include <assert.h>

#include "zsengine.h"
#include "zsutilities.h"
#include "creatures.h"
#include "items.h"
#include "ZSdescribe.h"
#include "spells.h"
#include "spellbook.h"
#include "minimap.h"
#include "equipobject.h"
#include "world.h"
#include "script.h"
#include "scriptfuncs.h"
#include "zstext.h"
#include "party.h"
#include "startscreen.h"
#include "createpartywin.h"
#include "zsoptions.h"
#include "events.h"
#include "ZSSaveLoad.h"
#include "zscutscene.h"
#include "zsbutton.h"
#include "combatmanager.h"
#include "path.h"
#include "blood.h"
#include "zsparticle.h"
#include "zstalk.h"
#include "zshelpwin.h"
#include "zsrest.h"
#include "zsOptions.h"
#include "zsaskwin.h"
#include "Mapwin.h"
#include "journal.h"

#include <mmsystem.h>

//don't forget to change in mainwindow, too.
#ifndef NDEBUG
#define VERSION_NUMBER			"v1.5"
#else
#define VERSION_NUMBER			"v1.5 release"
#endif


#define IDC_VERSION_NUMBER		8765

#define MASTER_ITEM_FILE		"items.txt"
#define MASTER_CREATURE_FILE	"creatures.txt"

HWND hMainWindow;
ZSMainWindow *pMain = NULL;

ZSFont *PapyrusParchment;

void OnExit(void);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow)
{
	INIT_DEBUG();

	//uncomment for demo version
	///GetHardwareID();
	//etRegisteredKey();

	//comment out for demo version
	Register();

	FillDistanceTable();

	srand((unsigned int)time(NULL));
	
	if(atexit(OnExit))
	{
		DEBUG_INFO("failed to register exit normal function\n");
		exit(1);
	}

	LoadFuncs();

	D3DXInitialize();
	
	Engine = new ZSEngine;

	Engine->Init(hinstance);

	PapyrusParchment = new ZSFont(Engine->Graphics()->GetFontEngine(), "papyrus.ddf", TEXTCOLOR(250, 250, 250));

	Engine->Graphics()->SetFont(PapyrusParchment);
	
	Engine->Graphics()->CreateProjectionMatrix(8.5f, 8.5f, VIEW_DEPTH*4);

	Engine->Graphics()->SetRenderState( D3DRENDERSTATE_ALPHATESTENABLE, TRUE );  
	Engine->Graphics()->SetRenderState( D3DRENDERSTATE_ALPHAREF, 0x024 );  
	Engine->Graphics()->SetRenderState( D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MIPFILTER, D3DTFP_LINEAR);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
	
	
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_AMBIENT, RGB_MAKE(32,32,32)); 
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);    
	//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZBIAS,12);

	//fill the primary and back buffers to 0
	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	LPDIRECTDRAWSURFACE7 LogoSurface, ProgressSurface;
	
	RECT rLogoTo;
	RECT rProgressTo;

	rLogoTo.left = (Engine->Graphics()->GetWidth() - 640) / 2;
	rLogoTo.right = rLogoTo.left + 640;
	rLogoTo.top = 0;
	rLogoTo.bottom = 69;

	rProgressTo.left = 24;
	rProgressTo.right = 152;
	rProgressTo.top = 128;
	rProgressTo.bottom = 128 + 128;

	LogoSurface = Engine->Graphics()->CreateSurfaceFromFile("logobanner.bmp",640,69,0, -1);

	ProgressSurface = Engine->Graphics()->CreateSurfaceFromFile("loadtextures.bmp",128,128,0, -1);

//	ShowWindow(hMainWindow, SW_SHOW);
	
	ShowCursor(FALSE);

	//create the progress window
	ZSText *pProgressText;
	pProgressText = new ZSText(-5, 27, 104, "Textures");
	pProgressText->SetTextColor(TEXT_WHITE);
	pProgressText->Show();
	pProgressText->Draw();
	Engine->Graphics()->GetBBuffer()->Blt(&rLogoTo, LogoSurface, NULL, NULL, NULL);
	Engine->Graphics()->GetBBuffer()->Blt(&rProgressTo, ProgressSurface, NULL, NULL, NULL);
	
	Engine->Graphics()->Flip();
	Engine->Graphics()->GetBBuffer()->Blt(NULL, Engine->Graphics()->GetPrimay(),NULL,NULL,NULL);

	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");
	assert(fp);
	
	SeekTo(fp,"TEXTUREREDUCTION");

	ZSTexture::SetReduction((BOOL)GetInt(fp));
	fclose(fp);
	
	Engine->LoadTextures();
	Engine->Graphics()->SetUpCircles();

	BOOL DrawWorld;
	
	fp = SafeFileOpen("gui.ini","rt");
	assert(fp);

	SeekTo(fp,"DRAWWORLD");
	DrawWorld = (BOOL)GetInt(fp);

	//pProgressText->SetText("Initiating World");
	//pProgressText->Draw();

	//Engine->Graphics()->Flip();
	//Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	//create base world
	int Temp;
	SeekTo(fp, "SOUND");
	Temp = GetInt(fp);
	if(Temp) 
		Engine->Sound()->SetFX(TRUE);
	else
		Engine->Sound()->SetFX(FALSE);
	
	SeekTo(fp, "SOUNDVOLUME");
	Temp = GetInt(fp);
	Engine->Sound()->SetFXVolume(Temp);
	
	
	SeekTo(fp, "MUSIC");
	Temp = GetInt(fp);
	if(Temp) 
		Engine->Sound()->SetMusic(TRUE);
	else
		Engine->Sound()->SetMusic(FALSE);
	
	SeekTo(fp, "MUSICVOLUME");
	Temp = GetInt(fp);
	Engine->Sound()->SetMusicVolume(Temp);
	
	if(Engine->Sound()->MusicIsOn())
	{
		Engine->Sound()->PlayMusic("god");
	}



	PreludeWorld = new World;

	SeekTo(fp, "AUTOSAVERATE");
	PreludeWorld->SetAutosaveRate(GetInt(fp));
	
	pMain = new ZSMainWindow;
	pMain->SetText("Main Window");
	pMain->SetDrawWorld(DrawWorld);

	float fBias = 0.0f;
	int fbtemp;
	
	SeekTo(fp, "MIPMAPLODBIAS	");
	fbtemp = GetInt(fp);
	
	fBias = (float)fbtemp / 100.0f;
	Engine->Graphics()->GetD3D()->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&fBias)) );
	
	fclose(fp);

//	if(DrawWorld)
//	{
	
//	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	pProgressText->Move(128 + 80,0);
	pProgressText->SetText("Meshes");
	pProgressText->Draw();
	
	ProgressSurface->Release();

	rProgressTo.left += 128 + 80;
	rProgressTo.right += 128 + 80;
	rProgressTo.bottom += 0;
	rProgressTo.top += 0;
	ProgressSurface = Engine->Graphics()->CreateSurfaceFromFile("loadmeshes.bmp",128,128,0, -1);
	Engine->Graphics()->GetBBuffer()->Blt(&rLogoTo, LogoSurface, NULL, NULL, NULL);
	Engine->Graphics()->GetBBuffer()->Blt(&rProgressTo, ProgressSurface, NULL, NULL, NULL);
	
	Engine->Graphics()->Flip();
	Engine->Graphics()->GetBBuffer()->Blt(NULL, Engine->Graphics()->GetPrimay(),NULL,NULL,NULL);

	fp = SafeFileOpen("mesh.bin","rb");
	if(fp)
	{
		fclose(fp);
		Engine->LoadMeshes("mesh.bin");
	}
	else
	{
		Engine->ImportMeshes();
	}

//	}
//	else
//	{
//		Engine->LoadMeshes("mesh.bin");
//	}
//	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	DEBUG_INFO("about to load items\n");

	pProgressText->Move(128 + 80, 0);
	pProgressText->SetText("Items");
	pProgressText->Draw();
	rProgressTo.left += 128 + 80;
	rProgressTo.right += 128 + 80;
	rProgressTo.bottom += 0;
	rProgressTo.top += 0;
	ProgressSurface->Release();
	ProgressSurface = Engine->Graphics()->CreateSurfaceFromFile("loaditems.bmp",128,128,0, -1);
	Engine->Graphics()->GetBBuffer()->Blt(&rLogoTo, LogoSurface, NULL, NULL, NULL);
	Engine->Graphics()->GetBBuffer()->Blt(&rProgressTo, ProgressSurface, NULL, NULL, NULL);
	
	Engine->Graphics()->Flip();
	Engine->Graphics()->GetBBuffer()->Blt(NULL, Engine->Graphics()->GetPrimay(),NULL,NULL,NULL);

	fp = fopen("items.bin","rb");
	if(!fp)
	{
		fp = SafeFileOpen(MASTER_ITEM_FILE,"rt");
		assert(fp);
		fseek(fp,0,0);
		LoadItems(fp);
		fclose(fp);
		fp = SafeFileOpen("items.bin","wb");
		SaveBinItems(fp);
	}
	else
	{
		LoadBinItems(fp);
	}
	fclose(fp);

	DEBUG_INFO("Loading Spells\n");

	PreludeSpells.Init();
	PreludeSpells.Load("newspells.bin");


	pProgressText->Move(128 + 80, 0);
	pProgressText->SetText("Creatures");
	pProgressText->Draw();
	rProgressTo.left += 128 + 80;
	rProgressTo.right += 128 + 80;
	rProgressTo.bottom += 0;
	rProgressTo.top += 0;
	ProgressSurface->Release();
	ProgressSurface = Engine->Graphics()->CreateSurfaceFromFile("loadmonsters.bmp",128,128,0, -1);
	Engine->Graphics()->GetBBuffer()->Blt(&rLogoTo, LogoSurface, NULL, NULL, NULL);
	Engine->Graphics()->GetBBuffer()->Blt(&rProgressTo, ProgressSurface, NULL, NULL, NULL);

	Engine->Graphics()->Flip();
	Engine->Graphics()->GetBBuffer()->Blt(NULL, Engine->Graphics()->GetPrimay(),NULL,NULL,NULL);

	DEBUG_INFO("About to load Creatures\n");

	fp = SafeFileOpen("creatures.bin","rb");
	if(!fp)
	{
		fp = SafeFileOpen(MASTER_CREATURE_FILE,"rt");
		assert(fp);
		//start at beginning of file
		fseek(fp,0,0);
		LoadCreatures(fp);
		fclose(fp);
		fp = SafeFileOpen("creatures.bin","wb");
		SaveBinCreatures(fp);
	}
	else
	{
		LoadBinCreatures(fp);
	}
	
	ProgressSurface->Release();
	LogoSurface->Release();

	fclose(fp);

//EquipAllCreatures;
	Creature *pCreature;
	pCreature = Creature::GetFirst();
	while(pCreature)
	{
		pCreature->ReEquip();
		pCreature = (Creature *)pCreature->GetNext();
	}

//	PreludeParty.LoadDefaultParty("Default");

//	pProgressText->SetText("Creating Spells");
//	pProgressText->Draw();
//	Engine->Graphics()->Flip();
//	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	
	FILE *fpValley;
	SetCurrentDirectory(".\\Areas");
	fpValley = SafeFileOpen("valley.bin","rb");
	if(!fpValley)
	{
		PreludeWorld->BringUpToDate();
	}
	else
	{
		SetCurrentDirectory(Engine->GetRootDirectory());
		fclose(fpValley);
		PreludeWorld->Load("worldbase.bin");
	}
	


	SetCurrentDirectory(Engine->GetRootDirectory());


	Engine->Graphics()->GetD3D()->BeginScene();
	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"FILTERING");

	if(GetInt(fp))
	{
		Engine->Graphics()->SetFilterState(FILTER_BOTH);
	}
	else
	{
		Engine->Graphics()->SetFilterState(FILTER_NONE);
	}
	fclose(fp);

	
	Engine->Graphics()->GetD3D()->EndScene();

	fp = SafeFileOpen("gui.ini","rt");
	assert(fp);

	SeekTo(fp,"[EDITTING]");
	SeekTo(fp,"Enabled:");
	int EditEnable;
	EditEnable = GetInt(fp);

	if(EditEnable)
	{
		PreludeWorld->SetEdittingEnabled(TRUE);
	}
	else
	{
		PreludeWorld->SetEdittingEnabled(FALSE);
	}

	fclose(fp);

	BOOL Test;

	fp = SafeFileOpen("gui.ini","rt");
	assert(fp);

	SeekTo(fp,"DIFFICULTY");
	int DiffLevel = GetInt(fp);
	PreludeWorld->SetDifficulty(DiffLevel);
	
	SeekTo(fp,"XPCONFIRM");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetXPConfirm(Test);
	
	SeekTo(fp,"FULLBODYSELECT");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetFullSelect(Test);
	
	SeekTo(fp,"HIGHLIGHTINFO");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetHighlightInfo(Test);
	
	SeekTo(fp,"VERBOSECOMBAT");
	Test = (BOOL)GetInt(fp);
	PreludeWorld->SetVerboseCombat(Test);

	fclose(fp);

	fp = SafeFileOpen("gui.ini","rt");
	assert(fp);

	int Shadowed;
	SeekTo(fp,"SHADOWS");
	Shadowed = (BOOL)GetInt(fp);
	
	if(Shadowed)
		PreludeWorld->SetDrawShadows(TRUE);
	else
		PreludeWorld->SetDrawShadows(FALSE);

	
	fclose(fp);

	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);
	//pProgressText->SetText("Loading Events");
	//pProgressText->Draw();
	//Engine->Graphics()->Flip();
	//Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);

	DEBUG_INFO("Loading Events\n");

	fp = fopen("events.bin","rb");
	if(fp)
	{
		fclose(fp);
		PreludeEvents.LoadEvents("events.bin");
	}
	else
	{
		PreludeEvents.ImportEvents("events.txt");
		PreludeEvents.SaveEvents("events.bin");
	}

	delete pProgressText;

//*************************
	if(!DrawWorld)
		Valley->GenerateBase(NULL);
//*************************

//	ZSText *pText;
//
//	pText = new ZSText(IDC_VERSION_NUMBER, 0, 0, VERSION_NUMBER);
//	pMain->AddChild(pText);
//	pText->Show();

	pMain->Show();
	
//test blood
	DEBUG_INFO("Testing Blood....\n");

	Blood *pBlood;
	pBlood = new Blood(1.0f, NULL);

	if(!pBlood->GetSys()->TestDraw())
	{
		DEBUG_INFO("Blood Disabled\n");
		Blood::DisableBlood();
	}
	else
	{
		DEBUG_INFO("Blood Enabled\n");
	}

	delete pBlood;

	DEBUG_INFO("Initting terrain info\n");

	Chunk::InitTerrain();
	

	// enter main event loop

	PreludeStartScreen *pStart;
	CreatePartyWin	*pCreateParty;
	ZSOptionWin *pOptions;

	int Result = START_RESULT_NONE;
	int Created = FALSE;

	BOOL Init = TRUE;
	
	while(Result != START_RESULT_QUIT)
	{
		pMain->SetState(WINDOW_STATE_NORMAL);
		Created = FALSE;
		pStart = new PreludeStartScreen;
		pStart->Show();

		pMain->AddTopChild(pStart);

		pMain->SetDrawWorld(FALSE);

		pMain->SetFocus(pStart);
		Result = pStart->GoModal();
		pStart->ReleaseFocus();

		pMain->RemoveChild(pStart);
		if(Result == START_RESULT_OPTIONS)
		{
			if(!pMain->GetChild(OPTIONS_ID))
			{
				pOptions = new ZSOptionWin(OPTIONS_ID);
				pMain->AddTopChild(pOptions);
			}
			pOptions->Show();
			pOptions->SetFocus(pOptions);
			pOptions->GoModal();
			pOptions->ReleaseFocus();
			pMain->RemoveChild(pOptions);
			
		}

		int MemberNum;
				
		if(Result == START_RESULT_NEW)
		{
		//intro cut scene;
			if(!Init)
			{
				//clear out old info
				ClearStack();

				PreludeEvents.Clear();
				PreludeFlags.Clear();

				if(PreludeWorld->GetCombat())
					PreludeWorld->GetCombat()->Kill();
					
				DeleteCreatures();
			}	
			
			char IntroDir[256];
			//get the maindirectory
			strcpy(IntroDir, Engine->GetRootDirectory());
			strcat(IntroDir, "\\Intro");

			DEBUG_INFO("switching to Intro directory\n");

			//switch to the Texture directory
			SetCurrentDirectory(IntroDir);

			ZSCutScene *pCutScene;
			pCutScene = new ZSCutScene;
			pMain->AddChild(pCutScene);
			fp = SafeFileOpen("introscene.txt","rt");
			pCutScene->Load(fp);
			fclose(fp);

			pCutScene->Show();
			pCutScene->SetFocus(pCutScene);
			pCutScene->GoModal();
			
			pCutScene->ReleaseFocus();

			SetCurrentDirectory(Engine->GetRootDirectory());

			pMain->RemoveChild(pCutScene);

			if(!Init)
			{
				if(PreludeWorld)
				{
					if(PreludeWorld->GetCombat())
						PreludeWorld->GetCombat()->Kill();
					delete PreludeWorld;
					PreludeWorld = new World;
				}
				//clear out addlocations
				PreludeParty.ClearJournal();
				PreludeParty.ClearLocations();
								
				DEBUG_INFO("About to load Creatures\n");
				fp = SafeFileOpen("creatures.bin","rb");
				LoadBinCreatures(fp);
				fclose(fp);

			//EquipAllCreatures;
				Creature *pCreature;
				pCreature = Creature::GetFirst();
				while(pCreature)
				{
					pCreature->ReEquip();
					pCreature = (Creature *)pCreature->GetNext();
				}

				PreludeWorld->Load("worldbase.bin");

				DEBUG_INFO("Loading Events\n");

				PreludeEvents.LoadEvents("events.bin");
				Init = TRUE;
			}
			
			pCreateParty = new CreatePartyWin;
			ZSWindow *pChild;
			pChild = pMain->GetChild();

			while(pChild)
			{
				pChild->Hide();
				pChild = pChild->GetSibling();
			}
				
			pCreateParty->Show();
			pMain->AddTopChild(pCreateParty);

			if(Engine->Sound()->MusicIsOn())
			{
				Engine->Sound()->PlayMusic("god");
			}

			pCreateParty->SetFocus(pCreateParty);
			Created = pCreateParty->GoModal();
			pCreateParty->ReleaseFocus();

			pMain->RemoveChild(pCreateParty);
			pChild = pMain->GetChild(CREATE_CHARACTER_ID);
			if(pChild)
			{
				pMain->RemoveChild(pChild);
			}

			pChild = pMain->GetChild();
			
			while(pChild)
			{
				pChild->Show();
				pChild = pChild->GetSibling();
			}
			
			
			if(Created)
			{
				int n;
				for(n = 0; n < PreludeWorld->GetNumAreas(); n++)
				{
					PreludeWorld->GetArea(n)->ClearDynamic(NULL, OBJECT_CREATURE);
				}

				Creature::PlaceByLocator();
				
				for(MemberNum = 0; MemberNum < PreludeParty.GetNumMembers(); MemberNum++)
				{
					float xy;
					xy = 1200.5 + (float)MemberNum;
					PreludeParty.GetMember(MemberNum)->SetPosition(xy,xy,Valley->GetZ(xy,xy));
					PreludeParty.GetMember(MemberNum)->ReEquip();
				
				//	Valley->AddToUpdate(PreludeParty.GetMember(MemberNum));
					PreludeParty.GetMember(MemberNum)->AddToWorld();
				//	PreludeParty.GetMember(MemberNum)->InsertAction(ACTION_IDLE,NULL,NULL);
					PreludeParty.GetMember(MemberNum)->ClearActions();
					PreludeParty.GetMember(MemberNum)->InsertAction(ACTION_USER,NULL,NULL);
					PreludeParty.GetMember(MemberNum)->InsertAction(ACTION_IDLE,NULL,NULL);
					PreludeParty.GetMember(MemberNum)->SetCreated(TRUE);
				}
				PreludeWorld->LookAt(PreludeParty.GetLeader());
				pMain->SetDrawWorld(DrawWorld);
				pMain->Show();
				ClearDescribe();
				Describe(VERSION_NUMBER);
				Describe("Press 'F1' for help");
				PreludeWorld->SetGameState(GAME_STATE_NORMAL);
				PreludeEvents.RunEvent(0);

				pMain->GoModal();
				
				while(PreludeParty.GetMember(0))
				{
					PreludeParty.GetMember(0)->KillActions();
					PreludeParty.RemoveMember(PreludeParty.GetMember(0));
				}
				Init = FALSE;
			}
	
			Result = START_RESULT_NONE;
		}
		
		if(Result == START_RESULT_LOAD)
		{
			LoadWin *pLoadWin;
			pLoadWin = new LoadWin(-1,100,100,500,400);
			pLoadWin->Show();
			pMain->AddTopChild(pLoadWin);
			pMain->SetFocus(pLoadWin);
			
			int Loaded;
			Loaded = pLoadWin->GoModal();

			pLoadWin->ReleaseFocus();

			pMain->RemoveChild(pLoadWin);

			if(Loaded)
			{
				PreludeWorld->SetGameState(GAME_STATE_NORMAL);
				pMain->SetDrawWorld(DrawWorld);
				pMain->Show();
				ClearDescribe();

				Describe(VERSION_NUMBER);
				Describe("Press 'F1' for help");

				pMain->GoModal();
				
				while(PreludeParty.GetMember(0))
				{
					PreludeParty.GetMember(0)->KillActions();
					PreludeParty.RemoveMember(PreludeParty.GetMember(0));
				}
			
				Init = FALSE;
			}

			Result = START_RESULT_NONE;
		}

	}

	pMain->SetState(WINDOW_STATE_DONE);
		
	// end while
	ShowCursor(TRUE);

	// return to Windows like this
	return(0);
}

void OnExit(void)
{
	if(ZSTalkWin::TalkSurface)
		ZSTalkWin::TalkSurface->Release();
	DEBUG_INFO("released talk\n");
	
	if(ZSHelpWin::HelpSurface)
		ZSHelpWin::HelpSurface->Release();
	DEBUG_INFO("released help\n");
	
	if(ZSOptionWin::OptionsSurface)
		ZSOptionWin::OptionsSurface->Release();
	DEBUG_INFO("released options\n");
	
	if(ZSMainOptionsMenu::MainOptionsSurface)
		ZSMainOptionsMenu::MainOptionsSurface->Release();
	DEBUG_INFO("released mainoptions\n");
	
	if(ZSRest::RestSurface)
		ZSRest::RestSurface->Release();
	DEBUG_INFO("released rest\n");
	
	if(ZSAskWin::AskSurface)
		ZSAskWin::AskSurface->Release();
	DEBUG_INFO("released ask\n");
	
	if(MapWin::MapBackground)
		MapWin::MapBackground->Release();
	DEBUG_INFO("released mpaback\n");
	
	if(MapWin::MapSurface)
		MapWin::MapSurface->Release();
	DEBUG_INFO("released map\n");
	
	if(GameWin::GameWinSurface)
		GameWin::GameWinSurface->Release();
	DEBUG_INFO("released game\n");
	
	if(LoadWin::LoadWinSurface)
		LoadWin::LoadWinSurface->Release();
	DEBUG_INFO("released load\n");
	
	if(SaveWin::SaveWinSurface)
		SaveWin::SaveWinSurface->Release();
	DEBUG_INFO("released save\n");

	if(JournalWin::JournalSurface)
		JournalWin::JournalSurface->Release();
	DEBUG_INFO("released journal\n");

	ZSButton::ShutDown();

	D3DXUninitialize();

	ClearStack();

	PreludeEvents.Clear();
	PreludeFlags.Clear();
	
	if(PreludeWorld)
	{
		if(PreludeWorld->GetCombat())
			PreludeWorld->GetCombat()->Kill();
		delete PreludeWorld;
		PreludeWorld = NULL;
	}

	DeleteCreatures();
	DeleteItems();

	if(Engine)
	{
		delete Engine;
		Engine = NULL;
	}

	if(PapyrusParchment)
	{
		delete PapyrusParchment;
		PapyrusParchment = NULL;
	}


	//delete windows last

	if(pMain)
	{
		delete pMain;
		pMain = NULL;
	}
	
	ZSWindow::Shutdown();

	Action::ReleaseAll();\
	DEBUG_INFO("Actions cleared\n");

	char blarg[64];
	sprintf(blarg, "End Script Blocks: %i\nargs: %i\n", G_NumBlocks, G_NumArgs);
	DEBUG_INFO(blarg);

	if(ExitErrorMessage)
	{
		MessageBox(NULL, ExitErrorMessage, "Fatal Error", MB_OK | MB_ICONSTOP);
		delete[] ExitErrorMessage;
	}
}


