#ifndef JOURNAL_H
#define JOURNAL_H

#include "zswindow.h"

class Journal
{
public:
	int NumEntries;
	//time and Entry Number;
	unsigned long Entry[1024*2];
	int Current;

	void GetEntry(int num, char *Dest);
	char *GetEntry(int num);
	BOOL AddEntry(int Num);
	void RemoveEntry(int Num);

	void Save(FILE *fp);
	void Load(FILE *fp);

	void Clear();
	Journal();
	~Journal();

};

class JournalWin : public ZSWindow
{
private:
	Journal *pJournal;
	int NumLines;

	void SetText();

public:
	static LPDIRECTDRAWSURFACE7 JournalSurface;

	int Command(int IDFrom, int Command, int Param);

	JournalWin(int NewID, int x, int y, int width, int height);

};



#endif