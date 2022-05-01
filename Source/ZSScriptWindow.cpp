#include "zsscriptwindow.h"
#include "zseditwindow.h"
#include "zsengine.h"
#include "script.h"
#include "zsutilities.h"
#include "zstalk.h"
#include "world.h"
#include "creatures.h"
#include "party.h"

#define IDC_EXECUTED_START		10
#define IDC_CURRENT_COMMAND	9999
#define IDC_SCRIPT_TALK_WIN	123456

const char DefaultFileName[] = "people.txt";

void ZSScriptWin::Clear()
{
	int n = IDC_EXECUTED_START;

	ZSWindow *pWin;
	while(TRUE)
	{
		pWin = GetChild(n);
		if(!pWin)
		{
			break;
		}
		else
		{
			RemoveChild(pWin);
		}
		n++;
	}
	NumExecutedScripts = 0;

	FILE *fp;

	fp = SafeFileOpen("tempscript.txt","wt");

	fclose(fp);

//	fp = SafeFileOpen("scriptlog.txt","wt");

//	fclose(fp);


}

void ZSScriptWin::ExecuteGameCommand()
{

	State = WINDOW_STATE_DONE;
	if(this->GetFocus() == GetChild(IDC_CURRENT_COMMAND))
	{
		this->ReleaseFocus();
	}
	this->Hide();

	char CommandText[128];
	ScriptBlock  SB;

	strcpy(CommandText,GetChild(IDC_CURRENT_COMMAND)->GetText());
	GetChild(IDC_CURRENT_COMMAND)->SetText("");

	if(!strcmp(CommandText,"$clear") || !strcmp(CommandText, "$CLEAR"))
	{
		Clear();
	}

	if(!strcmp(CommandText,"$run") || !strcmp(CommandText, "$RUN"))
	{
		SB.Import("tempscript.txt");

		FILE *fp;

		fp = SafeFileOpen("tempscript.txt","a+t");

		fprintf(fp," =");
	
		ScriptArg *Temp;

		Temp = SB.Process();

		if(Temp)
			Temp->Print(fp);

		fprintf(fp, "\n\n");

		fclose(fp);
	}

	FILE *fp;

	//water
	if(CommandText[1] == 'w' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 't' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'r')
	{
		Valley->MakeWater(FALSE);
		return;
	}

	//reblockdungeons
	//dungeon
	if(CommandText[1] == 'd' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 'n' &&
		CommandText[4] == 'g' &&
		CommandText[5] == 'e' &&
		CommandText[6] == 'o' &&
		CommandText[7] == 'n' &&
		CommandText[8] == 'b' &&
		CommandText[9] == 'l' &&
		CommandText[10] == 'o' &&
		CommandText[11] == 'c')
	{
		Area *pOldValley;
		pOldValley = Valley;
		int n;
		for(n = 1; n < PreludeWorld->GetNumAreas(); n++)
		{
			Valley = PreludeWorld->GetArea(n);
			Valley->DungeonBlock();
		}
		
		SetCurrentDirectory(".\\Areas");
	
		for(n = 1; n < PreludeWorld->GetNumAreas(); n++)
		{
			PreludeWorld->GetArea(n)->Save();
		}

		SetCurrentDirectory(Engine->GetRootDirectory());

		Valley = pOldValley;

		return;
	}

	//dungeon
	if(CommandText[1] == 'b' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'c' &&
		CommandText[5] == 'k' &&
		CommandText[6] == 'd' &&
		CommandText[7] == 'u' &&
		CommandText[8] == 'n' &&
		CommandText[9] == 'g' &&
		CommandText[10] == 'e' &&
		CommandText[11] == 'o')
	{
		Valley->DungeonBlock();
		
		//Valley->Save();

		return;
		
	}


	//cleanareas
	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'n' &&
		CommandText[6] == 'a' &&
		CommandText[7] == 'r')
	{
		PreludeWorld->CleanAreas();
		return;
	}

	//placepeople
	if(CommandText[1] == 'p' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'a' &&
		CommandText[4] == 'c' &&
		CommandText[5] == 'e' &&
		CommandText[6] == 'p' &&
		CommandText[7] == 'e')
	{
		Creature::PlaceByLocator();
		return;
	}
	//sitstand
	if(CommandText[1] == 's' &&
		CommandText[2] == 'i' &&
		CommandText[3] == 't' &&
		CommandText[4] == 's' &&
		CommandText[5] == 't' &&
		CommandText[6] == 'a' &&
		CommandText[7] == 'n')
	{
		Creature::OutputSitStand();
		return;
	}


	//dungeon
	if(CommandText[1] == 'd' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 'n' &&
		CommandText[4] == 'g' &&
		CommandText[5] == 'e' &&
		CommandText[6] == 'o' &&
		CommandText[7] == 'n' &&
		CommandText[8] == ' ')
	{
		char *pName;
		char *pWidth;
		char *pHeight;
		int TileWidth;
		int TileHeight;

		pName = strchr(CommandText,' ');
		pName[0] = '\0';
		pName++;

		pWidth = strchr(pName,' ');
		pWidth[0] = '\0';
		pWidth++;

		pHeight = strchr(pWidth,' ');
		pHeight[0] = '\0';
		pHeight++;

		Area *pArea;

		TileWidth = atoi(pWidth);
		TileHeight = atoi(pHeight);

		pArea = PreludeWorld->AddArea(pName);
		
		pArea->BuildCavern(pName,TileWidth,TileHeight);

		Describe("Done building dungeon.");
	}

	//schedules
	if(CommandText[1] == 's' &&
		CommandText[2] == 'c' &&
		CommandText[3] == 'h' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'd' &&
		CommandText[6] == 'u' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 'e')
	{
		Creature::FixSchedules();
		Describe("creature schedules repaired.");
		return;
	}

	if(CommandText[1] == 'f' &&
		CommandText[2] == 'i' &&
		CommandText[3] == 'x' &&
		CommandText[4] == 'I' &&
		CommandText[5] == 'D' )
	{
		Creature::FixIDs();
		Describe("creature ID's repaired.");
		return;
	}

	//addwalls
	if(CommandText[1] == 'a' &&
		CommandText[2] == 'd' &&
		CommandText[3] == 'd' &&
		CommandText[4] == 'w' &&
		CommandText[5] == 'a' &&
		CommandText[6] == 'l' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 's')
	{
		char *pName;
		char *pWidth;
		char *pHeight;
		int TileWidth;
		int TileHeight;

		pName = strchr(CommandText,' ');
		pName[0] = '\0';
		pName++;

		pWidth = strchr(pName,' ');
		pWidth[0] = '\0';
		pWidth++;

		pHeight = strchr(pWidth,' ');
		pHeight[0] = '\0';
		pHeight++;

		TileWidth = atoi(pWidth);
		TileHeight = atoi(pHeight);

		Valley->AddCaveWalls(pName,TileWidth,TileHeight);

		Describe("Done building dungeon.");
	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'g')
	{
		PreludeWorld->SaveGame(&CommandText[4]);
		return;
	}

	if(CommandText[1] == 'l' &&
		CommandText[2] == 'g')
	{
		PreludeWorld->LoadGame(&CommandText[4]);
		return;
	}

	if(CommandText[1] == 'f' &&
		CommandText[2] == 'i' &&
		CommandText[3] == 'x' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'v')
	{
		Valley->SetEventHeights();
		return;
	}

	if(CommandText[1] == 'h' &&
		CommandText[2] == 'e' &&
		CommandText[3] == 'i' &&
		CommandText[4] == 'g' &&
		CommandText[5] == 'h' &&
		CommandText[6] == 't' &&
		CommandText[7] == 'a')
	{

		for(int n = 1; n < PreludeWorld->GetNumAreas(); n++)
		{
			PreludeWorld->GetArea(n)->FixHeightMap();
			SetCurrentDirectory(".\\Areas");
			PreludeWorld->GetArea(n)->Save();
			SetCurrentDirectory(Engine->GetRootDirectory());
		}
		return;
	}


	if(CommandText[1] == 'h' &&
		CommandText[2] == 'e' &&
		CommandText[3] == 'i' &&
		CommandText[4] == 'g' &&
		CommandText[5] == 'h' &&
		CommandText[6] == 't' &&
		CommandText[7] == 's')
	{
		Valley->FixHeightMap();
		return;
	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'v' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'd' &&
		CommandText[6] == 'y' &&
		CommandText[7] == 'n')
	{
		char blargname[64];
		SetCurrentDirectory(".\\Areas");
		sprintf(blargname,"%s.dyn",Valley->GetName());
		fp = SafeFileOpen(blargname,"wb");
		int n;
		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			Valley->RemoveFromUpdate((Object *)PreludeParty.GetMember(n));
		}

		Valley->SaveNonStatic(fp);
	

		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			Valley->AddToUpdate((Object *)PreludeParty.GetMember(n));
		}
			
			fclose(fp);
		SetCurrentDirectory(Engine->GetRootDirectory());
		return;
	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'v' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'a' &&
		CommandText[6] == 'l' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 'd' &&
		CommandText[9] == 'y' &&
		CommandText[10] == 'n')
	{
		SetCurrentDirectory(".\\Areas");
		int n;

		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			Valley->RemoveFromUpdate((Object *)PreludeParty.GetMember(n));
		}

		for(n = 0; n < PreludeWorld->GetNumAreas(); n++)
		{
			PreludeWorld->GetArea(n)->SaveNonStatic(NULL);
		}

		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			Valley->AddToUpdate((Object *)PreludeParty.GetMember(n));
		}
		SetCurrentDirectory(Engine->GetRootDirectory());
		return;
	}

	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'r' &&
		CommandText[6] == 'f' &&
		CommandText[7] == 'i' &&
		CommandText[8] == 'r' &&
		CommandText[9] == 'e')
	{
		int xn,yn;
		Object *pOb;
		for(yn = 0; yn < Valley->GetHeight() / UPDATE_SEGMENT_HEIGHT; yn++)
		for(xn = 0; xn < Valley->GetWidth() / UPDATE_SEGMENT_WIDTH; xn++)
		{
			pOb = Valley->GetUpdateSegment(xn,yn);
			while(pOb)
			{
				if(pOb->GetObjectType() == OBJECT_FIRE)
				{
					Describe("fire removed");
					Valley->RemoveFromUpdate(pOb);
					pOb = Valley->GetUpdateSegment(xn,yn);
				}
				else
				{
					pOb = pOb->GetNext();
				}
			}

		}
		
		
		return;
	}


	if(CommandText[1] == 's' &&
		CommandText[2] == 'o' &&
		CommandText[3] == 'r' &&
		CommandText[4] == 't' &&
		CommandText[5] == 'p' &&
		CommandText[6] == 'e' &&
		CommandText[7] == 'o')
	{
		Creature::Sort();
		return;
	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'v' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'p' &&
		CommandText[6] == 'e' &&
		CommandText[7] == 'o')
	{
		//Creature::FixSchedules();
		Describe("Schedules confirmed.");
		Creature::Sort();
		Describe("Creature's sorted.");
		fp = SafeFileOpen("creatures.bin","wb");
		SaveBinCreatures(fp);
		fclose(fp);
		Describe("Saved.");
		return;
	}

	if(CommandText[1] == 'r' &&
		CommandText[2] == 'e' &&
		CommandText[3] == 'i' &&
		CommandText[4] == 'm' &&
		CommandText[5] == 'p' &&
		CommandText[6] == 'o' &&
		CommandText[7] == 'r')
	{
		fp = SafeFileOpen("creatures.txt","rt");
		ReImportCreatures(fp);
		fclose(fp);
		return;
	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'v' &&
		CommandText[4] == 'e' &&
		CommandText[5] == 'r' &&
		CommandText[6] == 'e' &&
		CommandText[7] == 'g')
	{
		PreludeWorld->SaveRegions("blarg.txt");
		Valley->SaveRegions();
		return;
	}

	if(CommandText[1] == 'l' &&
		CommandText[2] == 'o' &&
		CommandText[3] == 'a' &&
		CommandText[4] == 'd' &&
		CommandText[5] == 'w' &&
		CommandText[6] == 'o' &&
		CommandText[7] == 'r')
	{
		fp = SafeFileOpen("dynamic.bin","rb");
		Valley->LoadNonStatic(fp);
		fclose(fp);
		return;
	}

	if(CommandText[1] == 'l' &&
		CommandText[2] == 'o' &&
		CommandText[3] == 'a' &&
		CommandText[4] == 'd')
	{
		FILE *fp;
		
		fp = SafeFileOpen(&CommandText[6],"rt");

		if(!fp)
		{
			Describe("Couldn't load");
			Describe(&CommandText[6]);
		}
		else
		{
			fclose(fp);
			SB.Import(&CommandText[6]);
			
			fp = SafeFileOpen("tempscript.txt","wt");

			SB.Export(fp);

			fclose(fp);
		}	
	}

	if(CommandText[1] == 'c' &&
		CommandText[2] == 'o' &&
		CommandText[3] == 'm' &&
		CommandText[4] == 'b' &&
		CommandText[5] == 'i' &&
		CommandText[6] == 'n' &&
		CommandText[7] == 'e')
	{
		FILE *fp;
		
		fp = SafeFileOpen(&CommandText[9],"rb");

		if(!fp)
		{
			Describe("Couldn't combine ");
			Describe(&CommandText[6]);
		}
		else
		{
			Valley->Combine(fp);
			fclose(fp);
			return;
		}	
	}

	if(CommandText[1] == 'g' &&
		CommandText[2] == 'e' &&
		CommandText[3] == 't' &&
		CommandText[4] == 's' &&
		CommandText[5] == 't' &&
		CommandText[6] == 'u' &&
		CommandText[7] == 'f' &&
		CommandText[8] == 'f')

	{
		FILE *fp;
		
		fp = SafeFileOpen(&CommandText[10],"rb");

		if(!fp)
		{
			Describe("Couldn't combine ");
			Describe(&CommandText[6]);
		}
		else
		{
			Valley->ImportStuff(fp);
			fclose(fp);
		}	
	}


	if(CommandText[1] == 's' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'v' &&
		CommandText[4] == 'e')
	{
		SB.Import("tempscript.txt");	

		FILE *fp;
		fp = SafeFileOpen(&CommandText[6],"wb");

		SB.Save(fp);

		fclose(fp);
	}

	if(CommandText[1] == 'b' &&
		CommandText[2] == 'i' &&
		CommandText[3] == 'n')
	{
		FILE *fp;

		fp = SafeFileOpen(&CommandText[5], "rb");
		if(!fp)
		{
			Describe("Couldn't open:");
			Describe(&CommandText[5]);
		}
		else
		{
			SB.Load(fp);

			fclose(fp);

			fp = SafeFileOpen("tempscript.txt","wt");

			SB.Export(fp);

			fclose(fp);
		}
	}

	if(CommandText[1] == 't' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'l' &&
		CommandText[4] == 'k')
	{
		char *FileName;
		char *CharacterName;

		char CharacterID[32];

		CharacterName = strchr(CommandText,' ');
		CharacterName++;
		FileName = strchr(CharacterName, ' ');
		if(!FileName)
		{
			FileName = (char *)&DefaultFileName;
		}
		else
		{
			FileName[0] = '\0';
			FileName++;
			if(strlen(FileName) < 2)
			{
				FileName = (char *)&DefaultFileName;
			}
		}
		sprintf(CharacterID,"#%s#",CharacterName);

		FILE *fp;
		fp = SafeFileOpen(FileName,"rt");
		
		if(fp && SeekTo(fp,CharacterID))
		{
			DEBUG_INFO("Importing character: ");
			DEBUG_INFO(CharacterName);
			DEBUG_INFO("\n");

			fseek(fp, -(strlen(CharacterID) + 1), SEEK_CUR);
		
			SB.Import(fp);	
			fclose(fp);		
		
//			fp = SafeFileOpen("tempscript.txt","wt");
//			SB.Export(fp);
//			fclose(fp);

//			SB.Process();

			ZSWindow *pWin;
			pWin = new ZSTalkWin(IDC_SCRIPT_TALK_WIN, 125, 125, 550,350, &SB);

			Thing *pThing;
			pThing = Thing::Find((Thing *)Creature::GetFirst(),CharacterName);
			if(pThing)
				((ZSTalkWin *)pWin)->SetPortrait(pThing->GetData(INDEX_PORTRAIT).String);

			pWin->Show();
		
			ZSWindow::GetMain()->AddChild(pWin);
		
			pWin->SetFocus(pWin);

			pWin->GoModal();

			pWin->ReleaseFocus();
			pWin->Hide();
			
			ZSWindow::GetMain()->RemoveChild(pWin);
		}
		else
		{
			DEBUG_INFO("Failed to find character: ");
			DEBUG_INFO(CharacterName);
			DEBUG_INFO(" in file: ");
			DEBUG_INFO(FileName);
			DEBUG_INFO("\n");
			Describe("Failed to find character: ");
			Describe(CharacterName);
			Describe(" in file: ");
			Describe(FileName);
			Describe("\n");
			if(fp)
				fclose(fp);
		}
	}

	if(CommandText[1] == 'f' &&
		CommandText[2] == 's' &&
		CommandText[3] == 'a' &&
		CommandText[4] == 'v' &&
		CommandText[5] == 'e')
	{
		FILE *fp = NULL;
		
		if(strlen(CommandText) > 6)
		{
			fp = SafeFileOpen(&CommandText[7],"wb");
		}
		if(!fp)
		{ 
			fp = SafeFileOpen("flags.bin","wb");
		}
		if(fp)
		{
			PreludeFlags.Save(fp);
			fclose(fp);
		}
		else
		{
			Describe("Could not save flags.");
		}
		return;
	}

	if(CommandText[1] == 'f' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'd')
	{
		FILE *fp = NULL;
		
		if(strlen(CommandText) > 6)
		{
			fp = SafeFileOpen(&CommandText[7],"rb");
		}
		if(!fp)
		{ 
			fp = SafeFileOpen("flags.bin","rb");
		}
		if(fp)
		{
			PreludeFlags.Load(fp);
			fclose(fp);
		}
		else
		{
			Describe("Could not load flags.");
		}
	}

	if(CommandText[1] == 'f' &&
		CommandText[2] == 'p' &&
		CommandText[3] == 'r' &&
		CommandText[4] == 'i' &&
		CommandText[5] == 'n' &&
		CommandText[6] == 't')
	{
		FILE *fp = NULL;
		
		if(strlen(CommandText) > 7)
		{
			fp = SafeFileOpen(&CommandText[8],"wt");
		}
		if(!fp)
		{ 
			fp = SafeFileOpen("flags.bin","wt");
		}
		if(fp)
		{
			PreludeFlags.OutPutDebugInfo(fp);
			fclose(fp);
		}
		else
		{
			Describe("Could not print flags.");
		}
	}
	
	if(CommandText[1] == 'm' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'k' &&
		CommandText[4] == 'e')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->GenerateBase(&rArea);
	}

	if(CommandText[1] == 't' &&
		CommandText[2] == 'e' &&
		CommandText[3] == 'r' &&
		CommandText[4] == 'r')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->GenerateTerrain(&rArea);
	}


	if(CommandText[1] == 'b' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 'i' &&
		CommandText[4] == 'l' &&
		CommandText[5] == 'd')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		PreludeWorld->DefaultRegions(&rArea);
	}
		if(CommandText[1] == 'b' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'c' &&
		CommandText[5] == 'k' &&
		CommandText[6] == 's' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 'o')
	{
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		int iBase;
		float fSlope;


		iBase = atoi(pleft);
	
	    fSlope = (float)iBase / 100.0f; 

		int xn;
		int yn;
		int ChunkWidth;
		int ChunkHeight;
		ChunkWidth = Valley->GetWidth() / CHUNK_TILE_WIDTH;
		ChunkHeight = Valley->GetHeight() / CHUNK_TILE_HEIGHT;
		Chunk *pChunk;
		for(xn = 0; xn < ChunkWidth; xn ++)
		for(yn = 0; yn < ChunkHeight; yn ++)
		{
			pChunk = Valley->GetChunk(xn, yn);
			if(!pChunk) { Valley->LoadChunk(xn,yn); pChunk = Valley->GetChunk(xn,yn); }
			if(pChunk)
			{
				pChunk->BlockBySlope(fSlope);
			}

		}
		return;
	}



	if(CommandText[1] == 'b' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'c' &&
		CommandText[5] == 'k')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->ReBlock(&rArea);
	}

	if(CommandText[1] == 'c' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 't')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->Cut(&rArea);
		return;
	}

	if(CommandText[1] == 'g' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 't' &&
		CommandText[4] == 'h' &&
		CommandText[5] == 'e' &&
		CommandText[6] == 'r')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->RegionObjects(&rArea);
		return;

	}

	if(CommandText[1] == 'u' &&
		CommandText[2] == 'n' &&
		CommandText[3] == 'g' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 't' &&
		CommandText[6] == 'h')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->UnRegionObjects(&rArea);
		return;

	}

	if(CommandText[1] == 's' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 'b' &&
		CommandText[4] == 'd' &&
		CommandText[5] == 'y')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		char *pfname;
		pfname = strchr(pbottom,' ');
		pfname[0] = '\0';
		pfname++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->LoadSubDynamic(&rArea, pfname, OBJECT_NONE);
		return;
	}

	if(CommandText[1] == 'd' &&
		CommandText[2] == 'o' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'r' &&
		CommandText[5] == 's')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		char *pfname;
		pfname = strchr(pbottom,' ');
		pfname[0] = '\0';
		pfname++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->LoadSubDynamic(&rArea, pfname, OBJECT_PORTAL);
		Valley->LoadSubDynamic(&rArea, pfname, OBJECT_ENTRANCE);
		return;
	}

	if(CommandText[1] == 'e' &&
		CommandText[2] == 'v' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'n' &&
		CommandText[5] == 't')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		char *pfname;
		pfname = strchr(pbottom,' ');
		pfname[0] = '\0';
		pfname++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->LoadSubDynamic(&rArea, pfname, OBJECT_EVENT);
		return;
	}

	if(CommandText[1] == 'i' &&
		CommandText[2] == 't' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'm' &&
		CommandText[5] == 's')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		char *pfname;
		pfname = strchr(pbottom,' ');
		pfname[0] = '\0';
		pfname++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->LoadSubDynamic(&rArea, pfname, OBJECT_ITEM);
		return;
	}

	//rupdate
	if(CommandText[1] == 'r' &&
		CommandText[2] == 'u' &&
		CommandText[3] == 'p' &&
		CommandText[4] == 'd' &&
		CommandText[5] == 'a')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		rArea.left = atoi(pleft);
		rArea.top = atoi(ptop);

		Valley->ResetUpdate(rArea.left,rArea.top);
		return;
	}
	
	//cleandynamic
	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'n' &&
		CommandText[6] == 'd' &&
		CommandText[7] == 'y' &&
		CommandText[8] == 'n' &&
		CommandText[9] == 'a' &&
		CommandText[10] == 'm')
	{

		int n = 0;
		Area *pArea;

		while(PreludeWorld->GetArea(n))
		{
			pArea = PreludeWorld->GetArea(n);

			pArea->CleanDynamic();
					
			n++;
		}

		return;
	}

	//clearfire
	//clearallpeople
	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'r' &&
		CommandText[6] == 'a' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 'l' &&
		CommandText[9] == 'f' &&
		CommandText[10] == 'i')
	{

		int n = 0;
		Area *pArea;
		RECT rClear;

		while(PreludeWorld->GetArea(n))
		{
			pArea = PreludeWorld->GetArea(n);
			rClear.left = 0;
			rClear.top = 0;
			rClear.bottom = pArea->GetHeight();
			rClear.right = pArea->GetWidth();

			pArea->ClearDynamic(&rClear, OBJECT_FIRE);
					
			n++;
		}

		return;
	}


	//cleardynamic
	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'r' &&
		CommandText[6] == 'd' &&
		CommandText[7] == 'y' &&
		CommandText[8] == 'n')
	{
		RECT rArea;
		char *pleft;
		pleft = strchr(CommandText,' ');
		pleft[0] = '\0';
		pleft++;

		char *ptop;
		ptop = strchr(pleft,' ');
		ptop[0] = '\0';
		ptop++;

		char *pright;
		pright = strchr(ptop,' ');
		pright[0] = '\0';
		pright++;

		char *pbottom;
		pbottom = strchr(pright,' ');
		pbottom[0] = '\0';
		pbottom++;

		rArea.left = atoi(pleft);
		rArea.right = atoi(pright);
		rArea.bottom = atoi(pbottom);
		rArea.top = atoi(ptop);

		Valley->ClearDynamic(&rArea, OBJECT_NONE);
		return;
	}

	//clearallpeople
	if(CommandText[1] == 'c' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'e' &&
		CommandText[4] == 'a' &&
		CommandText[5] == 'r' &&
		CommandText[6] == 'a' &&
		CommandText[7] == 'l' &&
		CommandText[8] == 'l' &&
		CommandText[9] == 'p'
		)
	{

		int n = 0;
		Area *pArea;
		RECT rClear;

		while(PreludeWorld->GetArea(n))
		{
			pArea = PreludeWorld->GetArea(n);
			rClear.left = 0;
			rClear.top = 0;
			rClear.bottom = pArea->GetHeight();
			rClear.right = pArea->GetWidth();

			pArea->ClearDynamic(&rClear, OBJECT_CREATURE);
					
			n++;
		}

		Creature *pCreature;
		pCreature = Creature::GetFirst();
		while(pCreature)
		{
			pCreature->SetAreaIn(-1);
			pCreature = (Creature *)pCreature->GetNext();
		}

		return;
	}


//smooth
	if(CommandText[1] == 's' &&
		CommandText[2] == 'm' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'o')
	{
		Valley->SmoothBaseTerrain();
	}

//slope	
	if(CommandText[1] == 's' &&
		CommandText[2] == 'l' &&
		CommandText[3] == 'o' &&
		CommandText[4] == 'p')
	{
		Valley->FixOutDoorTerrain();
	}

//paintwalls	
	if(CommandText[1] == 'p' &&
		CommandText[2] == 'a' &&
		CommandText[3] == 'i' &&
		CommandText[4] == 'n' &&
		CommandText[5] == 't' &&
		CommandText[6] == 'w' &&
		CommandText[7] == 'a' &&
		CommandText[8] == 'l' &&
		CommandText[9] == 'l'
		)
	{
		char *ptname;
		ptname = strchr(CommandText,' ');
		ptname[0] = '\0';
		ptname++;
		RECT rArea;
		rArea.left = 0;
		rArea.right = Valley->GetWidth();
		rArea.top = 0;
		rArea.bottom = Valley->GetHeight();

		Valley->PaintWalls(&rArea, Engine->GetTexture(ptname));
		return;
	}

//lighting
	if(CommandText[1] == 'l' &&
		CommandText[2] == 'i' &&
		CommandText[3] == 'g' &&
		CommandText[4] == 'h' &&
		CommandText[5] == 't')
	{
		int iDiffuse;
		float fDiffuse;

		char *cDiffuse;
		cDiffuse = strchr(CommandText,' ');
		cDiffuse[0] = '\0';
		cDiffuse++;

		iDiffuse = atoi(cDiffuse);
		fDiffuse = (float)iDiffuse / 100.0f;
	
		PreludeWorld->CreateLights(fDiffuse);

		return;
	}



}

void ZSScriptWin::AddExecutedScript()
{
	int n;
	ZSWindow *pWin;
	RECT ChildBounds;
	for(n = IDC_EXECUTED_START; n < IDC_EXECUTED_START + NumExecutedScripts; n++)
	{
		pWin = GetChild(n);
		if(pWin)
		{
			pWin->GetBounds(&ChildBounds);
			ChildBounds.top -= Engine->Graphics()->GetFontEngine()->GetTextHeight();
			ChildBounds.bottom -= Engine->Graphics()->GetFontEngine()->GetTextHeight();
			
			if(ChildBounds.top < 0)
			{
				RemoveChild(pWin);
			}
			else
			{
				pWin->SetBounds(&ChildBounds);
			}

		}
	}

	char *CurrentLine = GetChild(IDC_CURRENT_COMMAND)->GetText();

	if(CurrentLine[0] == '$')
	{
		ExecuteGameCommand();
	}
	else
	{
		FILE *fp;
		fp = SafeFileOpen("tempscript.txt","a+t");

		fprintf(fp,"%s\n",GetChild(IDC_CURRENT_COMMAND)->GetText());
	
		fclose(fp);
	}


	pWin = new ZSWindow(WINDOW_TEXT, IDC_EXECUTED_START + NumExecutedScripts, 
								Bounds.left, Bounds.top - Engine->Graphics()->GetFontEngine()->GetTextHeight(),
								Engine->Graphics()->GetFontEngine()->GetTextWidth(GetChild(IDC_CURRENT_COMMAND)->GetText()),
								Engine->Graphics()->GetFontEngine()->GetTextHeight());
	pWin->SetText(GetChild(IDC_CURRENT_COMMAND)->GetText());
	pWin->Show();

	AddChild(pWin);
	
	GetChild(IDC_CURRENT_COMMAND)->SetText("");
	((ZSEdit *)GetChild(IDC_CURRENT_COMMAND))->SetOffset(0);

	NumExecutedScripts++;
}

int ZSScriptWin::Command(int IDFrom, int Command, int Param)
{
	if(!Visible) return TRUE;
	if(Command == COMMAND_EDIT_DONE && State == WINDOW_STATE_NORMAL)
	{
		if(IDFrom == IDC_CURRENT_COMMAND)
		{
			if(strlen(GetChild(IDC_CURRENT_COMMAND)->GetText()) < 1)
			{
				State = WINDOW_STATE_DONE;
			}
			else
			{
				//execute script command
				AddExecutedScript();
				SetFocus(GetChild(IDC_CURRENT_COMMAND));
			}
		}
	}
	return TRUE;
}
	
ZSScriptWin::ZSScriptWin(int NewID)
{
	ID = NewID;
	Visible = FALSE;
	NumExecutedScripts = 0;
	Bounds.left = 2;
	Bounds.right = Engine->Graphics()->GetWidth() - 2;
	Bounds.bottom = Engine->Graphics()->GetHeight() - 2;
	Bounds.top = Bounds.bottom - 2 - Engine->Graphics()->GetFontEngine()->GetTextHeight();

	ZSEdit *pCurCommand;

	pCurCommand = new ZSEdit(WINDOW_EDIT, IDC_CURRENT_COMMAND, Bounds.left + 2, Bounds.top + 2, (Bounds.right - Bounds.left) - 4, (Bounds.bottom - Bounds.top) - 4);

	AddChild(pCurCommand);
	pCurCommand->SetText("");

	pCurCommand->Show();

	Clear();

	BackGroundSurface = Engine->Graphics()->CreateSurface(16,16,NULL,10);

	Engine->Graphics()->FillSurface(BackGroundSurface,0,NULL);


}

int ZSScriptWin::Draw()
{

	//only draw ourself if we're visible
	if(Visible)
	{
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}
		
		Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&Bounds,RGB(255,255,255));
		//draw our children
		if(pChild)
		{
			pChild->Draw();
		}
	}

	//draw our sibling if we have one
	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE; 

}

void ZSScriptWin::Show()
{
	SetFocus(GetChild(IDC_CURRENT_COMMAND));
	Visible = TRUE;
	return;
}

void ZSScriptWin::Hide()
{
	Visible = FALSE;
	if(pInputFocus == GetChild(IDC_CURRENT_COMMAND))
	{
		ReleaseFocus();
	}
	return;
}

int ZSScriptWin::LeftButtonDown(int x, int y)
{
	return TRUE;
}

int ZSScriptWin::LeftButtonUp(int x, int y)
{
	return TRUE;
}

//GoModal
//
int ZSScriptWin::GoModal()
{
	State = WINDOW_STATE_NORMAL;
	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
	}
	return ReturnCode;
} //GoModal

int ZSScriptWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{


	return TRUE;
}
	