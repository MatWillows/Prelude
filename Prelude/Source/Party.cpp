#include "party.h"
#include "regions.h"
#include "world.h"
#include <assert.h>
#include "zsportrait.h"
#include "maplocator.h"
#include "movepointer.h"
#include "gameitem.h"
#include "zsmenubar.h"
#include "Zsmessage.h"
#include "path.h"

Party PreludeParty;

BOOL Party::IsMember(Creature *ToTest)
{
	for(int n = 0; n < NumMembers; n++)
	{
		if(pMembers[n] == ToTest)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Party::IsLeader(Creature *ToText)
{
	if(pLeader == ToText) 
		return TRUE;
	else
		return FALSE;
}

BOOL Party::AddMember(Creature *ToAdd)
{
	if(NumMembers == MAX_PARTY_MEMBERS)
	{
		return FALSE;
	}
	else
	{
		pMembers[NumMembers] = ToAdd;
		NumMembers++;
		((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->SetPortraits();
		return TRUE;
	}
}

int Party::GetMemberNum(Creature *ToGet)
{
	for(int n = 0; n < NumMembers; n++)
	{
		if(pMembers[n] == ToGet)
		{
			return n;
		}
	}
	return 0;
}

BOOL Party::RemoveMember(Creature *ToRemove)
{
	int n, sn;
	for(n = 0; n < NumMembers; n++)
	{
		if(pMembers[n] == ToRemove)
		{
			if(MoveMarks[n])
			{
				RemoveMarker(MoveMarks[n]);
			}

			pMembers[n] = NULL;
			NumMembers--;
			for(sn = n; sn < NumMembers; sn ++)
			{
				pMembers[sn] = pMembers[sn + 1];
				MoveMarks[sn] = MoveMarks[sn + 1];	
			}
			if(sn < MAX_PARTY_MEMBERS)
			{
				pMembers[sn] = NULL;
				MoveMarks[sn] = NULL;
			}
			if(pLeader == ToRemove)
			{
				pLeader = pMembers[0];
			}

			((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->SetPortraits();

			return TRUE;
		}
	}
	return FALSE;

}

void Party::RemoveMarker(MovePointer *pRemove)
{
	int n;
	for(n = 0; n < NumMembers; n++)
	{
		if(MoveMarks[n] == pRemove)
		{
			MovePointer *pMP;
			pMP = MoveMarks[n];
			MoveMarks[n] = NULL;
			PreludeWorld->RemoveMainObject(pMP);
			//delete pMP;
			return;
		}
	}
	return;
}


BOOL Party::SetLeader(Creature *pNewLeader)
{
	if(IsMember(pNewLeader))
	{
		if(pLeader)
		{
			pLeader->SetActive(FALSE);
		}
		
		pLeader = pNewLeader; 

		pLeader->SetActive(TRUE);
		
		return TRUE;
	}
	return FALSE;
}

void Party::LoadDefaultParty(const char *Name)
{
	FILE *fp;

	fp = SafeFileOpen("DefaultParties.txt","rt");

	assert(fp);

	SeekTo(fp, Name);

	SeekTo(fp,"NumMembers:");

	int NewNumMembers = GetInt(fp);

	char *MemberName;
	
	Thing *pMember;

	for(int n = 0; n < NewNumMembers; n++)
	{
		SeekTo(fp,"Name:");
		MemberName = GetStringNoWhite(fp);
		pMember = Thing::Find((Thing *)Creature::GetFirst(),MemberName);
		if(pMember)
		{
			AddMember((Creature *)pMember);
		}
		delete MemberName;
	
	}

	SetLeader(pMembers[0]);

	fclose(fp);

}
		
Party::Party()
{
	NumMembers = 0;
	pLeader = NULL;
	memset(Keys,0, sizeof(int) * MAX_NUM_KEYS);
	memset(pMembers,0,sizeof(Creature *) * MAX_PARTY_MEMBERS);
	memset(MoveMarks,0,sizeof(MovePointer *) * MAX_PARTY_MEMBERS);
	RegionChange = TRUE;
	AllInside = FALSE;
	pLocations = NULL;
}

Creature *Party::GetBest(int StatNum)
{
	Creature *pBest;

	pBest = pMembers[0];
	
	for(int n = 1; n < NumMembers; n ++)
	{
		if(pMembers[n]->GetData(StatNum).Value > pBest->GetData(StatNum).Value)
		{
			pBest = pMembers[n];
		}
	}

	return pBest;
}

int Party::GetAverage(int StatNum)
{
	int Total = 0;
	
	for(int n = 0; n < NumMembers; n ++)
	{
		Total += pMembers[n]->GetData(StatNum).Value;
	}
	
	if(this->GetNumMembers())
		return (Total / this->GetNumMembers());
	else
		return Total;
}


Creature *Party::GetWorst(int StatNum)
{
	Creature *pWorst;

	pWorst = pMembers[0];
	
	for(int n = 1; n < NumMembers; n ++)
	{
		if(pMembers[n]->GetData(StatNum).Value < pWorst->GetData(StatNum).Value)
		{
			pWorst = pMembers[n];
		}
	}

	return pWorst;
}

BOOL Party::Give(Thing *pToGive, int Quantity)
{
	int n;
	for(n = 0; n < NumMembers; n ++)
	{
		if(pMembers[n]->Give(pToGive, Quantity))
		{
			return TRUE;
		}
	}

	//drop item?
	return FALSE;
}

BOOL Party::Has(Thing *pToCheck)
{
	int n;
	int Total = 0;
	for(n = 0; n < NumMembers; n++)
	{
		Total += pMembers[n]->Has(pToCheck); 
	}
	return Total;
}

BOOL Party::Take(Thing *pToTake, int Quantity)
{
	int n;
	for(n = 0; n < NumMembers; n++)
	{
		if(pMembers[n]->Has(pToTake))
		{
			pMembers[n]->Take(pToTake, Quantity);
		
			return TRUE;
		}
	}
	return FALSE;
}

Party::Party(FILE *fp)
{

}

void Party::Save(FILE *fp)
{
	unsigned long CheckSum;
	char blarg[32];
	fwrite(RestActions,sizeof(int),MAX_PARTY_MEMBERS,fp);

	fwrite(&NumMembers,sizeof(int),1,fp);

	int CurArea;
	CurArea = this->GetLeader()->GetAreaIn();

	fwrite(&CurArea,sizeof(int),1,fp);

	CheckSum = ftell(fp);
	sprintf(blarg,"sp: %i ",CheckSum);
	DEBUG_INFO(blarg);

	BOOL bTrue = TRUE;
	BOOL bFalse = FALSE;

	for(int n = 0; n < NumMembers; n++)
	{
		pMembers[n]->SaveBin(fp);
		fwrite(&pMembers[n]->Created, sizeof(BOOL), 1, fp);
		
		//save skill info
		//last time a skill improved.  For Now, for party members only
		fwrite(pMembers[n]->SkillImproved,sizeof(int),MAX_NUM_SKILLS,fp);

		//spells casting stuff
		fwrite(pMembers[n]->KnownSpells,sizeof(BYTE),MAX_SPELLS,fp);
		fwrite(&pMembers[n]->ReadySpell,sizeof(BYTE),1,fp);

		for(int en = 0; en < MAX_EQUIPMENT; en++)
		{
			if(pMembers[n]->Equipment[en])
			{
				fwrite(&bTrue,sizeof(BOOL),1,fp);
				pMembers[n]->Equipment[en]->Save(fp);
			}
			else
			{
				fwrite(&bFalse,sizeof(BOOL),1,fp);
			}
		}

		CheckSum = ftell(fp);
		sprintf(blarg,"%i ",CheckSum);
		DEBUG_INFO(blarg);
		fwrite(pMembers[n]->GetLastSkillImproves(),sizeof(int), MAX_NUM_SKILLS, fp);
	}

	int LeaderNum;
	LeaderNum = GetMemberNum(pLeader);
	fwrite(&LeaderNum,sizeof(int),1,fp);

	fwrite(Keys,sizeof(int),MAX_NUM_KEYS,fp);

	//journal
	PartyJournal.Save(fp);
	CheckSum = ftell(fp);
	sprintf(blarg,"%i\n",CheckSum);
	DEBUG_INFO(blarg);

	BOOL Locations = FALSE;
	if(pLocations)
	{
		Locations = TRUE;
	}

	fwrite(&Locations,sizeof(BOOL),1,fp);

	if(pLocations)
		pLocations->Save(fp);

}

void Party::Load(FILE *fp)
{
	unsigned long CheckSum;
	char blarg[32];
	int n;

	if(NumMembers)
	{
		for(n = 0; n < NumMembers; n++)
		{
			PreludeWorld->GetArea(pMembers[n]->GetAreaIn())->RemoveFromUpdate(pMembers[n]);
			delete pMembers[n];
			pMembers[n] = NULL;
		}
	}

	fread(RestActions,sizeof(int),MAX_PARTY_MEMBERS,fp);

	fread(&NumMembers,sizeof(int),1,fp);

	int CurArea;
	fread(&CurArea,sizeof(int),1,fp);


	CheckSum = ftell(fp);
	sprintf(blarg,"sp: %i ",CheckSum);
	DEBUG_INFO(blarg)
	Creature *pCreature;
	pCreature = Creature::GetFirst();

	for(n = 0; n < NumMembers; n++)
	{
		pMembers[n] = new Creature;
		pMembers[n]->SetFieldNames(pCreature->GetFieldNames());
		pMembers[n]->SetNumFields(pCreature->GetNumFields());
		pMembers[n]->SetDataTypes(pCreature->GetDataTypes());
		
		pMembers[n]->LoadBin(fp);
		fread(&pMembers[n]->Created, sizeof(BOOL), 1, fp);
		
		pMembers[n]->SetAreaIn(CurArea);

		//save skill info
		//last time a skill improved.  For Now, for party members only
		fread(pMembers[n]->SkillImproved,sizeof(int),MAX_NUM_SKILLS,fp);

		//spells casting stuff
		fread(pMembers[n]->KnownSpells,sizeof(BYTE),MAX_SPELLS,fp);
		fread(&pMembers[n]->ReadySpell,sizeof(BYTE),1,fp);
	
		BOOL bEquipAt;
		for(int en = 0; en < MAX_EQUIPMENT; en++)
		{
			fread(&bEquipAt,sizeof(BOOL),1,fp);
			if(bEquipAt)
			{
				pMembers[n]->Equipment[en] = (GameItem *)LoadObject(fp);
				pMembers[n]->Equipment[en]->SetLocation(LOCATION_EQUIPPED, pMembers[n]);
			
			}
			else
			{
				pMembers[n]->Equipment[en] = NULL;
			}
		}

		fread(pMembers[n]->GetLastSkillImproves(),sizeof(int), MAX_NUM_SKILLS, fp);

		CheckSum = ftell(fp);
		sprintf(blarg,"%i ",CheckSum);
		DEBUG_INFO(blarg);
	}

	int LeaderNum;
	fread(&LeaderNum,sizeof(int),1,fp);
	
	pLeader = pMembers[LeaderNum];
	
	fread(Keys,sizeof(int),MAX_NUM_KEYS,fp);

	//journal
	PartyJournal.Load(fp);

	CheckSum = ftell(fp);
	sprintf(blarg,"%i\n",CheckSum);
	DEBUG_INFO(blarg);

	BOOL Locations = FALSE;
	
	fread(&Locations,sizeof(BOOL),1,fp);

	if(Locations)
	{
		pLocations = new MapLocator;
		pLocations->Load(fp);
	}
	
	RegionChange = TRUE;

	for(n = 0; n < MAX_PARTY_MEMBERS; n++)
	{
		if(MoveMarks[n])
		{
			MoveMarks[n] = NULL;
		}
	}

}

BOOL Party::HasKey(int Num)
{
	for(int n = 0; n < MAX_NUM_KEYS; n++)
	{
		if(Keys[n] == Num)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void Party::AddKey(int Num)
{

}

void Party::TakeKey(int Num)
{

}

void Party::Teleport(int xTo, int yTo)
{
	//calculate our current position
	int xShould;
	int yShould;
	D3DVECTOR *pVector;
	D3DVECTOR pTo;
	pTo.x = (float)xTo + 0.5f;
	pTo.y = (float)yTo + 0.5f;
	pTo.z = 0;
	D3DVECTOR LeaderAt;
	if(!ValidateKey(NULL))
	{
		if(PreludeWorld->GetCurAreaNum() == 0 && (pTo.x < 1000 || pTo.x > 1600 ||
			pTo.y < 1000 || pTo.y > 1900))
		{
			Message("You must stay near Kellen in the Trial Version","Ok");
			return;
		}
	}

	pVector = PreludeParty.GetLeader()->GetPosition();

	int LeaderFacing;

	LeaderFacing = FindFacing(pVector,&pTo);

	if(!Valley->GetChunk(xTo / CHUNK_TILE_WIDTH, yTo/CHUNK_TILE_HEIGHT))
	{
		Valley->LoadChunk(xTo / CHUNK_TILE_WIDTH, yTo/ CHUNK_TILE_HEIGHT);
	}

	int MyPosition = 0;

	int pn = 0;
	D3DVECTOR vLeader, vNewPos;
	

	vNewPos.x = (float)xTo + 0.5f;
	vNewPos.y = (float)yTo + 0.5f;
	
	vNewPos.z = Valley->GetTileHeight((int)vNewPos.x, (int)vNewPos.y);
	D3DVECTOR *pPosition;

	pPosition = GetMember(pn)->GetPosition();
	*pPosition = vNewPos;
	GetMember(pn)->SetPosition(&vNewPos);
	Valley->AddToUpdate(GetMember(pn));
	GetMember(pn)->ClearActions();

	for(pn = 1; pn < GetNumMembers(); pn ++)
	{
		switch(LeaderFacing)
		{
			case NORTH:
				xShould = xTo;
				yShould = yTo + pn;
				break;
			case SOUTH:
				xShould = xTo;
				yShould = yTo - pn;
				break;
			case EAST:
				xShould = xTo - pn;
				yShould = yTo;
				break;
			case WEST:
				xShould = xTo + pn;
				yShould = yTo;
				break;
			case NORTHEAST:
				xShould = xTo - pn;
				yShould = yTo + pn;
				break;
			case NORTHWEST:
				xShould = xTo + pn;
				yShould = yTo + pn;
				break;
			case SOUTHEAST:
				xShould = xTo - pn;
				yShould = yTo - pn;
				break;
			case SOUTHWEST:
				xShould = xTo + pn;
				yShould = yTo - pn;
				break;
			default:
				break;
		}

		//stop the party members from doing anything they may be doing
		GetMember(pn)->ClearActions();

		//move them to their new location
		//GetMember(n)->InsertAction(ACTION_MOVETO, (void *)xShould,(void *)yShould);
		//Valley->RemoveFromUpdate(GetMember(n));
		float fxTo;
		float fyTo;
		fxTo = (float)xShould + 0.5f;
		fyTo = (float)yShould + 0.5f;
	
		int CurX;
		int CurY;
		int StartX;
		int StartY;

		CurX = StartX = (int)fxTo;
		CurY = StartY = (int)fyTo;

		//demands a clear path to leader
		Path TempPath;
		
		vLeader = *(PreludeParty.GetMember(0)->GetPosition());
		
		int LeadX;
		LeadX = vLeader.x;
		int LeadY;
		LeadY = vLeader.y;

		BOOL FoundClear = FALSE;

		if(!Valley->IsClear(CurX, CurY) || !TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetMember(0)))
		{
			int OffsetX = 0;
			int OffsetY = 0;
			int Start;
			int End;
			int n;
			

			while(!FoundClear && OffsetX < 5)
			{
				OffsetX++;
				OffsetY++;
				Start = StartX - OffsetX;
				End = StartX + OffsetX;
				CurY = StartY - OffsetY;

				for(n = Start; n <= End; n++)
				{
					CurX = n;
					if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetMember(0)))
					{
						FoundClear = TRUE;
						break;
					}
				}
				if(FoundClear) break;

				CurY = StartY + OffsetY;
				for(n = Start; n <= End; n++)
				{
					CurX = n;
					if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetMember(0)))
					{
						FoundClear = TRUE;
						break;
					}
				}
				if(FoundClear) break;

				
				Start = StartY - (OffsetY - 1);
				End = StartY + (OffsetY + 1);
				CurX = StartX + OffsetX;

				for(n = Start; n <= End; n++)
				{
					CurY = n;
					if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetMember(0)))
					{
						FoundClear = TRUE;
						break;
					}
				}
				if(FoundClear) break;

				CurX = StartX - OffsetX;

				for(n = Start; n <= End; n++)
				{
					CurY = n;
					if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetMember(0)))
					{
						FoundClear = TRUE;
						break;
					}
				}
				if(FoundClear) break;
			}
		}

		if(FoundClear)
		{
			vNewPos.x = (float)CurX + 0.5f;
			vNewPos.y = (float)CurY + 0.5f;
		}
		else
		{
			vNewPos.x = fxTo;
			vNewPos.y = fyTo;
		}
	
		vNewPos.z = Valley->GetTileHeight((int)vNewPos.x, (int)vNewPos.y);
		D3DVECTOR *pPosition;

		pPosition = GetMember(pn)->GetPosition();
		*pPosition = vNewPos;
		GetMember(pn)->SetPosition(&vNewPos);
		Valley->AddToUpdate(GetMember(pn));
	}

	this->Occupy();
	PreludeWorld->LookAt((Thing *)GetLeader());
}

void Party::MoveParty(int xTo, int yTo)
{
	//calculate our current position
	int xShould;
	int yShould;
	D3DVECTOR *pVector;
	D3DVECTOR pTo;
	pTo.x = (float)xTo;
	pTo.y = (float)yTo;
	pTo.z = 0;
	D3DVECTOR LeaderAt;

	if(!ValidateKey(NULL))
	{
		if(PreludeWorld->GetCurAreaNum() == 0 && (pTo.x < 1000 || pTo.x > 1600 ||
			pTo.y < 1000 || pTo.y > 1900))
		{
			Message("You can not leave the area of Kellen in the Trial Version.","Ok");
			return;
		}
	}

	pVector = PreludeParty.GetLeader()->GetPosition();

	int LeaderFacing;

	LeaderFacing = FindFacing(pVector,&pTo);

	int MyPosition = 0;

	int n;
	for(n = 0; n < GetNumMembers(); n ++)
	{
		switch(LeaderFacing)
		{
			case NORTH:
				xShould = xTo;
				yShould = yTo + n;
				break;
			case SOUTH:
				xShould = xTo;
				yShould = yTo - n;
				break;
			case EAST:
				xShould = xTo - n;
				yShould = yTo;
				break;
			case WEST:
				xShould = xTo + n;
				yShould = yTo;
				break;
			case NORTHEAST:
				xShould = xTo - n;
				yShould = yTo + n;
				break;
			case NORTHWEST:
				xShould = xTo + n;
				yShould = yTo + n;
				break;
			case SOUTHEAST:
				xShould = xTo - n;
				yShould = yTo - n;
				break;
			case SOUTHWEST:
				xShould = xTo + n;
				yShould = yTo - n;
				break;
			default:
				break;
		}
		//stop the party members from doing anything they may be doing
		GetMember(n)->ClearActions();

		int Offset = 0;
		//move them to their new location
		while(!Valley->IsClear(xShould,yShould) && Offset < 10)
		{
			Offset++;
			xShould += Offset % 2;
			yShould += (Offset % 2) - 1;

		}
		if(Offset < 10)
		{
			GetMember(n)->InsertAction(ACTION_MOVETO, (void *)xShould,(void *)yShould);
			D3DVECTOR vMoveTo;
			vMoveTo.x = (float)xShould + 0.5f;
			vMoveTo.y = (float)yShould + 0.5f;
			vMoveTo.z = Valley->GetTileHeight(xShould,yShould);
			if(MoveMarks[n])
			{
				MoveMarks[n]->SetPosition(&vMoveTo);
				MoveMarks[n]->NotMoveCount = 0;
			}
			else
			{
				MoveMarks[n] = new MovePointer(&vMoveTo,(Object *)GetMember(n));
				PreludeWorld->AddMainObject(MoveMarks[n]);
			}
		}
	}
}

void Party::Occupy()
{
	UnOccupy();
	Region *pRegion;
	D3DVECTOR *pvAt;
	int NumInside;
	NumInside = 0;
	
	for(int n = 0; n < NumMembers; n++)
	{
		pvAt = pMembers[n]->GetPosition();
		pRegion = Valley->GetRegion(pvAt);
		pMembers[n]->SetRegionIn(pRegion);
		pRegions[n] = pRegion;
		if(pRegion)
		{
			NumInside++;
			if(pRegion->IsOccupied() < REGION_OCCUPIED)
				pRegion->SetOccupancy(REGION_OCCUPIED);
		}
	}
	if(NumInside == NumMembers)
	{
		AllInside = TRUE;
	}
	else
	{
		AllInside = FALSE;
	}
	RegionChange = FALSE;
}

void Party::UnOccupy()
{
	//set allregions the area of the party to unseen..
	Valley->UnOccupyRegions();
}

BOOL Party::AddLocation(char *Tag, int x, int y, BOOL Tele, int MapNum, int MapX, int MapY)
{	
	MapLocator *pLocator;

	pLocator = GetLocations();
	
	if(!pLocator)
	{
		pLocations = new MapLocator(Tag, x, y, Tele);
		pLocations->SetMapNum(MapNum);
		pLocations->SetMapX(MapX);
		pLocations->SetMapY(MapY);

		return TRUE;
	}

	while(pLocator)
	{
		if(pLocator->GetMapNum() == MapNum && pLocator->GetX() == x && pLocator->GetY() == y)
		{
			return FALSE;
		}
		pLocator = pLocator->GetNext();
	}

	pLocator = new MapLocator(Tag, x, y, Tele);
	pLocator->SetNext(GetLocations());
	 
	pLocations = pLocator;
	pLocations->SetMapNum(MapNum);
	pLocations->SetMapX(MapX);
	pLocations->SetMapY(MapY);

	return TRUE;
}


void Party::RemoveLocation(char *Tag)
{
	MapLocator *pLocator;

	pLocator = GetLocations();

	if(!strcmp(pLocator->GetTag(),Tag))
	{
		pLocations = pLocator->GetNext();
		pLocator->SetNext(NULL);
		delete pLocator;
	}

	while(pLocator->GetNext())
	{
		if(!strcmp(pLocator->GetTag(),Tag))
		{
			pLocator->SetNext(pLocator->GetNext()->GetNext());
			pLocator->GetNext()->SetNext(NULL);
			delete pLocator;
		}
	

		pLocator = pLocator->GetNext();
	}
	
	return;
}

void Party::RemoveLocation(int x, int y)
{
	MapLocator *pLocator;

	pLocator = GetLocations();

	if(pLocator->GetX() == x && pLocator->GetY() == y)
	{
		pLocations = pLocator->GetNext();
		pLocator->SetNext(NULL);
		delete pLocator;
	}

	while(pLocator->GetNext())
	{
		if(pLocator->GetNext()->GetX() == x && pLocator->GetNext()->GetY() == y)
		{
			pLocator->SetNext(pLocator->GetNext()->GetNext());
			pLocator->GetNext()->SetNext(NULL);
			delete pLocator;
		}
	

		pLocator = pLocator->GetNext();
	}
	
	return;
}

void Party::ClearLocations()
{
	
	delete pLocations;
	
	pLocations = NULL;

}

void Party::ClearJournal()
{
	PartyJournal.Clear();
}
