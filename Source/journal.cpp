#include "journal.h"
#include "zsutilities.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zsengine.h"
#include "party.h"
#include "world.h"

typedef enum
{
	IDC_JOURNAL_QUIT,
	IDC_JOURNAL_PAGEUP,
	IDC_JOURNAL_PAGEDOWN,
	IDC_JOURNAL_LEFT_TEXT,
	IDC_JOURNAL_RIGHT_TEXT,
} JOURNAL_CONTROLS;

LPDIRECTDRAWSURFACE7 JournalWin::JournalSurface = NULL;


void Journal::GetEntry(int num, char *Dest)
{

}

char *Journal::GetEntry(int num)
{
	int EntryNum;
	EntryNum = Entry[num*2];
	int Date;
	Date = Entry[num*2+1];
	char DayString[16];
	sprintf(DayString,"Day %i:   ",Date);
	char IDNum[8];
	sprintf(IDNum,"%i",EntryNum);
	FILE *fp;
	fp = SafeFileOpen("journal.txt","rt");

	SeekToSkip(fp,IDNum);
	SeekTo(fp,"[");
	char *JournalString;
	JournalString = GetString(fp,']');

	fclose(fp);
	char *RetString;
	RetString = new char[strlen(DayString) + strlen(JournalString) + 2];
	strcpy(RetString,DayString);
	strcat(RetString,JournalString);
	delete JournalString;
	
	return RetString;
}

BOOL Journal::AddEntry(int Num)
{
	if(PreludeParty.GetBest(INDEX_LITERACY_AND_LORE)->GetData(INDEX_LITERACY_AND_LORE).Value)
	{

		for(int n = 0; n < (NumEntries*2); n+=2)
		{
			if(Entry[n] == Num)
			{
				return FALSE;
			}
		}

		Entry[NumEntries*2] = Num;
		Entry[NumEntries*2+1] = PreludeWorld->GetDay();
		NumEntries++;
		Current = NumEntries - 1;
		if(Current % 2)
		{
			Current -= 1;
		}
	}
	return TRUE;
}

void Journal::RemoveEntry(int Num)
{
	int n, sn;
	for(n = 0; n < NumEntries; n++)
	{
		if(Entry[n*2] == Num)
		{
			NumEntries--;
			for(sn = n; sn < NumEntries; sn ++)
			{
				Entry[sn*2] = Entry[(sn+1)*2];
				Entry[sn*2+1] = Entry[(sn+1)*2+1];
			}
			Current = NumEntries - 1;
			if(Current % 2)
			{
				Current -= 1;
			}
			return;
		}
	}
}

void Journal::Save(FILE *fp)
{
	fwrite(&NumEntries,sizeof(int),1,fp);
	fwrite(&Current,sizeof(int),1,fp);
	fwrite(Entry,sizeof(unsigned long),1024*2,fp);
}

void Journal::Load(FILE *fp)
{
	fread(&NumEntries,sizeof(int),1,fp);
	fread(&Current,sizeof(int),1,fp);
	fread(Entry,sizeof(unsigned long),1024*2,fp);
}

Journal::Journal()
{
	Current = 0;
	NumEntries = 0;
	ZeroMemory(Entry, 1024*2*sizeof(unsigned long));
}

Journal::~Journal()
{

}

void JournalWin::SetText()
{
	if(pJournal->NumEntries)
	{
		ZSWindow *pWin;
		pWin = GetChild(IDC_JOURNAL_LEFT_TEXT);
		pWin->SetText(pJournal->GetEntry(pJournal->Current));

		if(pJournal->Current < pJournal->NumEntries -1)
		{
			pWin = GetChild(IDC_JOURNAL_RIGHT_TEXT);
			pWin->SetText(pJournal->GetEntry(pJournal->Current+1));
		}
		else
		{
			pWin = GetChild(IDC_JOURNAL_RIGHT_TEXT);
			pWin->SetText(" ");
		}
	}
}

int JournalWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_JOURNAL_QUIT)
		{
			State = WINDOW_STATE_DONE;
		}
		else
		if(IDFrom == IDC_JOURNAL_PAGEUP)
		{
			if(pJournal->Current - 2 >= 0)
			{
				pJournal->Current -= 2;
				SetText();
			}
		}
		else
		if(IDFrom == IDC_JOURNAL_PAGEDOWN)
		{
			if(pJournal->Current + 2 < pJournal->NumEntries)
			{
				pJournal->Current += 2;
				SetText();
			}
		}
	}
	return TRUE;
}

JournalWin::JournalWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Type = WINDOW_JOURNAL;
	Visible = FALSE;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	FILE *fp;
	RECT rBounds;
	char *FileName;
	int Width;
	int Height;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[JOURNAL]");

	SeekTo(fp,"BACKGROUND");
	
	FileName = GetStringNoWhite(fp);
	Width = GetInt(fp);
	Height = GetInt(fp);

	if(!JournalSurface)
	{
		BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,Width,Height,NULL,0);
		JournalSurface = BackGroundSurface;
	}
	BackGroundSurface = JournalSurface;
	BackGroundSurface->AddRef();
		
	delete FileName;

	ZSButton *pButton;

	SeekTo(fp,"QUIT");
	LoadRect(&rBounds,fp);

	pButton = new ZSButton(BUTTON_NONE, IDC_JOURNAL_QUIT, XYWH(rBounds));
	pButton->Show();
	pButton->SetText("Close");
	AddChild(pButton);

	SeekTo(fp,"PAGEUP");
	LoadRect(&rBounds,fp);

	pButton = new ZSButton("leftpagebutton", IDC_JOURNAL_PAGEUP, XYWH(rBounds),51,68,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"PAGEDOWN");
	LoadRect(&rBounds,fp);

	pButton = new ZSButton("rightpagebutton", IDC_JOURNAL_PAGEDOWN, XYWH(rBounds),51,68,1);
	pButton->Show();
	AddChild(pButton);

	ZSText *pText;

	SeekTo(fp,"LEFTPAGE");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_JOURNAL_LEFT_TEXT, XYWH(rBounds)," ",0);	
	pText->Show();
	pText->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	AddTopChild(pText);

	SeekTo(fp,"RIGHTPAGE");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_JOURNAL_RIGHT_TEXT, XYWH(rBounds)," ",0);	
	pText->Show();
	pText->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	AddTopChild(pText);

	fclose(fp);

	pJournal = PreludeParty.GetJournal();

	SetText();

}

void Journal::Clear()
{
	NumEntries = 0;
	Current = 0;
	ZeroMemory(Entry, 1024*2*sizeof(unsigned long));
}
