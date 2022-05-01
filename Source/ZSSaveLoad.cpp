#include "zssaveload.h"
#include "zsbutton.h"
#include "zsverticalscroll.h"
#include "world.h"
#include "zsconfirm.h"
#include "zsgettext.h"
#include "world.h"
#include "zstext.h"

#define MAX_GAMES_SHOWN 5

typedef enum
{
	IDC_GAME_DELETE,
	IDC_GAME_LOAD,
	IDC_GAME_SAVE,
	IDC_GAME_LIST,
	IDC_GAME_EXIT,
	IDC_GAME_SCROLL,
	IDC_GAME = 10,
	
} SAVELOAD_CONTROLS;

LPDIRECTDRAWSURFACE7 GameWin::GameWinSurface = NULL;
LPDIRECTDRAWSURFACE7 LoadWin::LoadWinSurface = NULL;
LPDIRECTDRAWSURFACE7 SaveWin::SaveWinSurface = NULL;



int GameWin::Command(int IDFrom, int Command, int Param)
{
	char FileName[64];
	char *SaveName;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
			case IDC_GAME_DELETE:
				//pass this on to the parent;
				GameListWin *pGList;
				pGList = (GameListWin *)this->GetParent();
				if(Confirm(this->GetParent(),"Delete Game?","yes","no"))	
				{
					pGList->DeleteGame(GameNumber);
				}
				break;
			case IDC_GAME_SAVE:
				if(!GameNumber)
				{
					GameNumber = 1;
					FILE *fp;
					do
					{
						sprintf(FileName,"save%i.gam",GameNumber);
						fp = fopen(FileName, "rb");
						if(fp) 
						{
							GameNumber++;
							fclose(fp);
						}
						else
						{
							break;
						}
					}while(TRUE);
				}
				sprintf(FileName,"save%i.gam",GameNumber);
				
				FILE *fp;
				fp = fopen(FileName,"rb");
				if(fp)
				{
					fclose(fp);
					if(Confirm(this->GetParent(),"Replace Game?","yes","no"))	
					{
						SaveName = GetModalText("Save Name",GetText(),32);
						if(SaveName)
						{
							PreludeWorld->SaveGame(FileName, SaveName);
							pParent->GetParent()->SetReturnCode(1);
							pParent->GetParent()->SetState(WINDOW_STATE_DONE);
							SetText(SaveName);
							delete[] SaveName;
						}
					}
				}
				else
				{
					SaveName = GetModalText("Save Name",GetText(),32);
					if(SaveName)
					{
						SetText(SaveName);
						PreludeWorld->SaveGame(FileName, SaveName);
						pParent->GetParent()->SetState(WINDOW_STATE_DONE);
						pParent->GetParent()->SetReturnCode(1);
						delete[] SaveName;
					}
				}
				break;
			case IDC_GAME_LOAD:
				if(Confirm(this->GetParent(),"Load Game?","yes","no"))
				{
					sprintf(FileName,"save%i.gam",GameNumber);
					pParent->GetParent()->SetReturnCode(1);
					pParent->GetParent()->SetState(WINDOW_STATE_DONE);
					PreludeWorld->LoadGame(FileName);
				}
				break;
		}
	}
	return TRUE;
}
	
void GameWin::SetGameNumber(int n)
{
	GameNumber = n;
	FILE *fp;
	char FileName[64];
	sprintf(FileName,"save%i.gam",n);
	ZSText *pText;
	fp = fopen(FileName,"rb");
	if(fp)
	{
		fread(FileName,sizeof(char),64,fp);
		SetText(FileName);
		
		int Hour;
		int TotalTime;
		//save date and time
		fread(&Hour, sizeof(int),1,fp);
		fread(&TotalTime, sizeof(int),1,fp);

		pText = (ZSText *)this->GetChild(-1);
		
		sprintf(FileName,"Day: %i  %i:%i", TotalTime / DAY_MINUTE_LENGTH, Hour, TotalTime % HOUR_LENGTH);

		pText->SetText(FileName);

		fclose(fp);
	}
	else
	{
		if(((GameListWin *)pParent)->IsSave())
		{

			SetText("New Game");
			pText = (ZSText *)this->GetChild(-1);
			
			sprintf(FileName,"Day: %i  Now", PreludeWorld->GetTotalTime() / DAY_MINUTE_LENGTH);

			pText->SetText(FileName);
		}
		else
		{
			Hide();
		}
	}
}


GameWin::GameWin(int NewID, int x, int y, int width, int height)
{
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	ID = NewID;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	int ButtonWidth;
	int ButtonHeight;
	ButtonWidth = (width - 16)/ 3;
	ButtonHeight = (height - 16)/ 2;
	
	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_GAME_SAVE, x + 8, y + 8 + ButtonHeight, ButtonWidth, ButtonHeight);
	pButton->SetText("Save");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_GAME_LOAD, x + 8 + ButtonWidth, y + 8 + ButtonHeight, ButtonWidth, ButtonHeight);
	pButton->SetText("Load");
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_GAME_DELETE, x + 8 + ButtonWidth + ButtonWidth, y + 8 + ButtonHeight, ButtonWidth, ButtonHeight);
	pButton->SetText("Delete");
	pButton->Show();
	AddChild(pButton);

	ZSText *pText;
	pText = new ZSText(-1, Bounds.right - 128, Bounds.top + 8, "Day: WWW  WW:WW");
	pText->Show();
	AddChild(pText);

	SetText("New");

	if(!GameWinSurface)
	{
		CreateWoodBorderedBackground(8);
		GameWinSurface = BackGroundSurface;
	}

	BackGroundSurface = GameWinSurface;
	BackGroundSurface->AddRef();
}

void GameListWin::SetLoad()
{
	ZSWindow *pWin;
	pWin = GetChild();
	while(pWin)
	{
		if(pWin->GetType() != WINDOW_SCROLLVERTICAL)
		{
			pWin->GetChild(IDC_GAME_LOAD)->Show();
			pWin->GetChild(IDC_GAME_SAVE)->Hide();
		}
		pWin = pWin->GetSibling();
	}	
	Save = FALSE;
}

void GameListWin::SetSave()
{
	ZSWindow *pWin;
	pWin = GetChild();
	while(pWin)
	{
		if(pWin->GetType() != WINDOW_SCROLLVERTICAL)
		{
			pWin->GetChild(IDC_GAME_SAVE)->Show();
			pWin->GetChild(IDC_GAME_LOAD)->Hide();
		}
		pWin = pWin->GetSibling();
	}	
	Save = TRUE;

}

void GameListWin::SortGames()
{
	//count the number of games
	int n = 1;
	FILE *fp = NULL;
	char FileName[64];

	do
	{
		if(fp) fclose(fp);
	
		sprintf(FileName,"save%i.gam",n);
		
		fp = fopen(FileName,"rb");
		
		if(fp) n++;

	}while(fp);

	NumGames = n;

	GameWin *pGWin;

	int Limit;
	Limit = NumGames - TopGame;

	if(Limit >= MAX_GAMES_SHOWN)
	{
		Limit = MAX_GAMES_SHOWN;
	}
	int Start = 0;
	if(!Save) 
	{
		Start = 1;
	}

	for(n = Start; n < Limit + Start; n++)
	{
		pGWin = (GameWin *)GetChild(IDC_GAME + n - Start);
		pGWin->Show();
		pGWin->SetGameNumber(TopGame + n);

	}

	for(; n < MAX_GAMES_SHOWN; n++)
	{
		pGWin = (GameWin *)GetChild(IDC_GAME + n);
		if(pGWin)
		{
			pGWin->Hide();
			pGWin->SetGameNumber(0);
		}
	}

	ZSVScroll *pScroll;
	pScroll = (ZSVScroll *)GetChild(IDC_GAME_SCROLL);

	pScroll->SetUpper((NumGames - MAX_GAMES_SHOWN) - 1);
	pScroll->SetLower(0);
	pScroll->SetPos(TopGame);
}

void GameListWin::DeleteGame(int num)
{
	FILE *fp = NULL;
	int n;
	char FileName[64];
	char NewFileName[64];
	n = num;
	sprintf(FileName,"save%i.gam",n);
	remove(FileName);
	do
	{
		sprintf(FileName,"save%i.gam",n+1);
		sprintf(NewFileName,"save%i.gam",n);
		fp = fopen(FileName,"rb");
		if(!fp) break;
		fclose(fp);
		rename(FileName,NewFileName);
		n++;
	}while(TRUE);

	ZSVScroll *pScroll;
	pScroll = (ZSVScroll *)GetChild(IDC_GAME_SCROLL);

	pScroll->SetPos(0);
	TopGame = 0;

	SortGames();

}

void GameListWin::AddGame(int num)
{

}

void GameListWin::SortFiles()
{

	//count the number of games
	int n = 1;
	FILE *fp = NULL;
	char FileName[64];
	char ThisName[64];
	char NextName[64];

	do
	{
		if(fp) fclose(fp);
	
		sprintf(FileName,"save%i.gam",n);
		
		fp = fopen(FileName,"rb");
		
		if(fp) n++;

	}while(fp);

	int NumFiles = n;

	int ThisTime;
	int NextTime;

	FILE *nfp;

	int CurGame = 1;
	int NextGame = 2;

	do
	{	
		sprintf(FileName,"save%i.gam",CurGame);
		fp = fopen(FileName,"rb");
		
		sprintf(FileName,"save%i.gam",NextGame);
		nfp = fopen(FileName,"rb");
		
		if(fp && nfp)
		{
			fread(FileName,sizeof(char), 64, fp);
			fread(&ThisTime,sizeof(int), 1, fp);
			fread(&ThisTime,sizeof(int), 1, fp);

			fread(FileName,sizeof(char), 64, nfp);
			fread(&NextTime,sizeof(int), 1, nfp);
			fread(&NextTime,sizeof(int), 1, nfp);

			fclose(fp);
			fclose(nfp);
			
			//switch files if necessary
			if(ThisTime < NextTime)
			{
				sprintf(ThisName,"save%i.gam",CurGame);
				sprintf(NextName,"save%i.gam",NextGame);
				rename(ThisName,"temp.gam");
				rename(NextName,ThisName);
				rename("temp.gam",NextName);
				CurGame = 1;
				NextGame = 2;
			}
			else
			{
				CurGame++;
				NextGame++;
			}
		}
	} while(NextGame < NumFiles);
}

int GameListWin::Command(int IDFrom, int Command, int Param)
{
	//game list win can receive only three commands scroll, and delete
	switch(Command)
	{
		case COMMAND_SCROLL:
			//get the top game
			if(Param >= 0 && Param < NumGames)
			{
				TopGame = Param;
				SortGames();
			}
			break;
		break;
	}

	return TRUE;
}

GameListWin::GameListWin(int NewID, int x, int y, int width, int height, BOOL DoSave)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Save = DoSave;
	Moveable = FALSE;
		
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;
	
	//gamewindow height = 64
	int GameWindowHeight = 64;
	int GameWindowWidth = width - 48;

	GamesShown = height / GameWindowHeight;
	
	GameWin *pGameWin;

	for(int n = 0; n < GamesShown; n++)
	{
		pGameWin = new GameWin(IDC_GAME + n, x+32, y + n*GameWindowHeight, GameWindowWidth, GameWindowHeight);
		AddChild(pGameWin);
	}

	ZSVScroll *pScroll;
	pScroll = new ZSVScroll(IDC_GAME_SCROLL,x,y,32,height);
	pScroll->Show();
	AddChild(pScroll);
	pScroll->SetPage(1);
	
	TopGame = 0;

	SortFiles();

	SortGames();
}

int LoadWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_GAME_EXIT)
		{
			State = WINDOW_STATE_DONE;
		}
	}
	return TRUE;
}


LoadWin::LoadWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	ReturnCode = 0;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;
	
	if(!LoadWinSurface)
	{
		CreateWoodBorderedBackground(12);
		LoadWinSurface = BackGroundSurface;
	}

	BackGroundSurface = LoadWinSurface;
	BackGroundSurface->AddRef();

	//exit button
	GameListWin *pGL;
	pGL = new GameListWin(IDC_GAME_LIST, x + 12, y + 12, width - 24, height - 48, FALSE);
	pGL->Show();
	AddChild(pGL);
	pGL->SetLoad();

	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_GAME_EXIT, (x + width)-76, (y + height - 36), 64, 24);
	pButton->SetText("Exit");
	pButton->Show();
	AddChild(pButton);
}

int SaveWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_GAME_EXIT)
		{
			State = WINDOW_STATE_DONE;
		}
	}
	return TRUE;
}

SaveWin::SaveWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	ReturnCode = 0;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;
	
	if(!SaveWinSurface)
	{
		CreateWoodBorderedBackground(12);
		SaveWinSurface = BackGroundSurface;
	}

	BackGroundSurface = SaveWinSurface;
	BackGroundSurface->AddRef();
	
	//exit button
	GameListWin *pGL;
	pGL = new GameListWin(IDC_GAME_LIST, x + 12, y + 12, width - 24, height - 48, TRUE);
	pGL->Show();
	AddChild(pGL);
	pGL->SetSave();

	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_GAME_EXIT, (x + width)-76, (y + height - 36), 64, 24);
	pButton->SetText("Exit");
	pButton->Show();
	AddChild(pButton);

}





