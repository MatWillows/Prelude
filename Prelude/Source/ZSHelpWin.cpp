#include "zshelpwin.h"

#include "zslistbox.h"
#include "zsbutton.h"
#include "zsengine.h"
#include "zsutilities.h"
#include "zsdescribe.h"

//window controls
typedef enum
{
	IDC_HELP_TOPICS,
	IDC_HELP_TITLE,
	IDC_HELP_HELPTEXT,
	IDC_HELP_QUIT,
} HelpChildren;

//help functions
LPDIRECTDRAWSURFACE7 ZSHelpWin::HelpSurface = NULL;


int GetRelated(const char *Topic, char **Related);

int ZSHelpWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		State = WINDOW_STATE_DONE;
	}

	if(Command == COMMAND_LIST_SELECTED)
	{
		char Topic[64];
		ZSList *pTopics;
		ZSWindow *pWin;
		pWin = GetChild(IDC_HELP_HELPTEXT);
		pTopics = (ZSList *)GetChild(IDC_HELP_TOPICS);
		pTopics->GetText(Param,Topic);
		if(!strcmp(Topic,"<previous topic>"))
		{
			if(pCurNode->pPrev)
			{
				pCurNode = pCurNode->pPrev;
				GoTopic(pCurNode->ThisTopic);		
			}
		}
		else
		{
			GoTopic(Topic);
		}
	}
	return TRUE;
}
void ZSHelpWin::GoTopic(const char *Topic)
{
	char Blarg[64];
	char *HelpBuffer = NULL;
	sprintf(Blarg,"%s",Topic);

	ZSWindow *pWin;
	pWin = GetChild(IDC_HELP_TITLE);
	pWin->SetText(Blarg);

	sprintf(Blarg,"[%s]", Topic);

	pWin = GetChild(IDC_HELP_HELPTEXT);
		
	FILE *fp;
	fp = SafeFileOpen("help.txt","rt");

	if(!SeekTo(fp,Blarg))
	{
		pWin->SetText("Topic Not Found.\n");
		fclose(fp);
	}
	else
	{
		if(!SeekTo(fp,"\""))
		{
			SafeExit("Help Topic has no or bad text\n");
		}

		HelpBuffer = GetString(fp,'\"');
		fclose(fp);

		pWin->SetText(HelpBuffer);

		//check to see if this is the first node
		if(!pCurNode)
		{
			//create it if so
			pNodeBase = pCurNode = new HelpNode;
		}
		else
		{
			//check if the topic we're going to is already at the current node
			if(strcmp(pCurNode->ThisTopic,Topic))
			{
				//if not add it to the list for traversal
				if(!pCurNode->pNext)
				{
					pCurNode->pNext = new HelpNode;
					pCurNode->pNext->pPrev = pCurNode;
				}
				pCurNode = pCurNode->pNext;
			}
			//do nothing we're at the proper node

		}

		strcpy(pCurNode->ThisTopic, Topic);

		ZSList *pList;
		pList = (ZSList *)GetChild(IDC_HELP_TOPICS);

		pList->Clear();
		if(pCurNode->pPrev)
			pList->AddItem("<previous topic>");
		pList->AddItem("Main");
		char **RelatedList;
		RelatedList = new char *[128];
		int NumRelated;
		NumRelated = GetRelated(Topic, RelatedList);

		DEBUG_INFO("Adding Related Topics\n");
		for(int n = 0; n < NumRelated; n++)
		{
			pList->AddItem(RelatedList[n]);
			delete[] RelatedList[n];
		}
		DEBUG_INFO("\n");
		delete[] RelatedList;
		if(HelpBuffer) delete[] HelpBuffer;
	}
}

	
ZSHelpWin::ZSHelpWin(int NewID, const char *Topic)
{
	Type = WINDOW_TALK;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Bounds.left = 100;
	Bounds.right = 700;
	Bounds.top = 100;
	Bounds.bottom = 500;

	Cursor = CURSOR_POINT;
	
	if(!HelpSurface)
	{
		BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("helpscreen.bmp",600,400,NULL,NULL);
		HelpSurface = BackGroundSurface;
	}

	BackGroundSurface = HelpSurface;
	BackGroundSurface->AddRef();

	ZSWindow *pWin;
	pWin = new ZSWindow(WINDOW_TEXT, IDC_HELP_HELPTEXT, 280,142,391,318);
	pWin->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	pWin->Show();
	pWin->SetMoveable(FALSE);
	AddChild(pWin);

	pWin = new ZSWindow(WINDOW_TEXT, IDC_HELP_TITLE, 280,108,391,24);
	pWin->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	pWin->Show();
	pWin->SetMoveable(FALSE);
	AddChild(pWin);

	ZSButton *pButton;

	pButton = new ZSButton("closebutton",IDC_HELP_QUIT, 374,465,56,32,56,34,1);
	pButton->Show();
	AddChild(pButton);

	FILE *fp;

	fp = SafeFileOpen("help.txt","rt");

	//create the topic list
	ZSList *pList;

	pList = new ZSList(IDC_HELP_TOPICS, 120, 136, 152, 325, TRUE);
	pList->SetBorderWidth(4);
	pList->SetBackGround(NULL);
	pWin->SetMoveable(FALSE);
	pList->Clear();
	pList->Show();
	AddChild(pList);
	char Blarg[64];
	if(Topic)
	{
		strcpy(Blarg,Topic);
	}
	else
	{
		sprintf(Blarg,"Main");
	}

	pNodeBase = NULL;
	pCurNode = NULL;

	GoTopic(Blarg);
	/*

	char *String;

	while(SeekTo(fp,"["))
	{
		String = GetString(fp,']');
		pList->AddItem(String);
		delete String;
	}
	*/


	fclose(fp);

//automatic test
#ifdef AUTOTEST
	State = WINDOW_STATE_DONE;
#endif

}

int GetRelated(const char *Topic, char **Related)
{
	char *retstring = NULL;
	char UpCase[64];
	char Label[64];
	int NumItems = 0;

	sprintf(Label,"[%s]",Topic);
	BOOL Found = FALSE;

	FILE *fp;
	fp = SafeFileOpen("help.txt","rt");
	if(SeekTo(fp,Label))
	{
		SeekTo(fp,"\"");
		Found = TRUE;
		DEBUG_INFO("Found Topic for related: ");
		DEBUG_INFO(Topic);
		DEBUG_INFO("\n");
	}
	else
	{
		fseek(fp,0,0);
		strcpy(UpCase, Topic);
		ConvertToCapitals(UpCase);
		sprintf(Label,"[%s]",UpCase);
		if(SeekTo(fp,Label))
		{
			SeekTo(fp,"\"");
			Found = TRUE;
		}
		DEBUG_INFO("Found Topic for related: ");
		DEBUG_INFO(UpCase);
		DEBUG_INFO("\n");

	}
	
	if(Found)
	{
		fpos_t Start;
		fpos_t End;
		fpos_t Cur;
		fgetpos(fp,&Start);
		
		SeekTo(fp,"[");
		fgetpos(fp,&End);

		fsetpos(fp,&Start);

		while(SeekTo(fp,"Related:"))
		{
			fgetpos(fp,&Cur);
			if(Cur >= End)
			{
				break;
			}
			SeekTo(fp,"\"");
			Related[NumItems] = GetString(fp,'\"');
			DEBUG_INFO(Related[NumItems]);
			NumItems++;
		}
	}
	fclose(fp);
	return NumItems;
}

void ShowHelp(const char *HelpTopic)
{
	ZSHelpWin *pHelp;
	pHelp = new ZSHelpWin(-1,HelpTopic);
	pHelp->Show();
	ZSWindow::GetMain()->AddTopChild(pHelp);
	pHelp->SetFocus(pHelp);
	pHelp->GoModal();
	pHelp->ReleaseFocus();
	
	HelpNode *pCur;
	HelpNode *pDel;

	pCur = pHelp->GetBaseNode();

	while(pCur)
	{
		pDel = pCur;
		pCur = pCur->pNext;
		delete pDel;
	}
	
	ZSWindow::GetMain()->RemoveChild(pHelp);


}








