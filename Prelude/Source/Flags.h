#ifndef FLAGS_H
#define FLAGS_H

/*
 * flags.h
 */

#include <stdio.h>

#define NUM_NFLAGS	512
#define NUM_BFLAGS	2048

#define FLAG_FILENAME	"flags.txt"

//definitions for scripto  -- completely abitrary, but large so as not to interfere with user defined objects
#define TARGETVALUE				9999
#define PARTYVALUE				9100
#define NPCVALUE					9200
#define LEADERVALUE				9300

//definition for script directroy
#define SCRIPTDIRECTORY		"\\script\\"

class Flag 
{
public:
	char Name[32];
	void *Value;
	Flag *pNext;

	void Save(FILE *fp);
	void Load(FILE *fp);

	void Print(FILE *fp);

	Flag() { Name[0] = '\0'; pNext = NULL; Value = NULL;}
	~Flag() { if(pNext) delete pNext; pNext = NULL; }
};

// DATA STRUCTURES
class Flags {
public:
	Flag Buckets[26];
		
	Flag *Get(const char *FlagName);
	int Kill(char *FlagName);
	
	void Save(FILE *fp);
	void Load(FILE *fp);
	
	void Import();

	void AddFlag(const char *FlagName);
	void OutPutDebugInfo(FILE *fp);
	void Clear();

	~Flags();
	
private:
	
};


#endif