#ifndef JOURNAL_H
#define JOURNAL_H

#include "zswindow.h"

#define MAX_AREAS 32
#define MAX_QUESTS 128

class Journal
{
public:
	int NumEntries;
	static BOOL IsSetup;
	//time and Entry Number;
	
	unsigned long Entry[1048*2];
	unsigned int Area[1048];
	unsigned int Quest[1048];

	static int NumQuests;
	static char QuestNames[MAX_QUESTS][128];
	static int QuestAreas[MAX_QUESTS];
	static int NumAreas;
	static char AreaNames[MAX_AREAS][128];

	int GetAreaNum(char *AreaName);
	int GetQuestNum(char *QuestName);
	int GetQuestArea(int QuestNum);
	int GetQuestArea(char *QuestName) { return GetQuestArea(GetQuestNum(QuestName)); }

	int Current;

	void GetEntry(int num, char *Dest);
	char *GetEntry(int num);
	int GetEntryQuest(int num);
	int GetEntryArea(int num);

	char *GetSortedEntry(int num, int QuestNum, int AreaNum);
	
	BOOL AddEntry(int Num);
	void RemoveEntry(int Num);

	void Save(FILE *fp);
	void Load(FILE *fp);

	void Clear();

	//load quests and areas
	void Init();

	Journal();
	~Journal();

};

class JournalWin : public ZSWindow
{
private:
	Journal *pJournal;
	int NumLines;
	int ShowAreaNum;
	int ShowQuestNum;

	int JournalLeft;
	int JournalRight;

	void Sort();
	void SortQuests();
	void SetText();
	void PageLeft();
	void PageRight();

public:
	static LPDIRECTDRAWSURFACE7 JournalSurface;

	int Command(int IDFrom, int Command, int Param);

	JournalWin(int NewID, int x, int y, int width, int height);

};



#endif