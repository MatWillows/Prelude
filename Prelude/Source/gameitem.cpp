#include "gameitem.h"
#include "zsutilities.h"
#include "creatures.h"
#include "area.h"
#include "regions.h"
#include "party.h"
#include "events.h"
#include "zssaychar.h"
#include <assert.h>

BOOL GameItem::VisualScaling = FALSE;

ZSModelEx *pIntersectMesh = NULL;

D3DVECTOR *GameItem::GetPosition()
{
	if(Location == LOCATION_PERSON ||
	   Location == LOCATION_EQUIPPED 
	   && pOwner)
	{
		this->Position = *pOwner->GetPosition();
	}
	
	return &this->Position;
}


void GameItem::AddToArea(Area *pArea)
{
	pRegionIn = pArea->GetRegion(this->GetPosition());

	pArea->AddToUpdate(this);
}

void GameItem::AddToWorld()
{
	AddToArea(Valley);
}

int GameItem::AltHighLight()
{
	if(!pItem)
		return TRUE;

	if(pItem->GetData("ALTSHOW").Value)
	{
		D3DVECTOR TextVector;
		TextVector = *GetPosition();
		Engine->Graphics()->DrawText(&TextVector, pItem->GetData(INDEX_NAME).String);
	}

	return TRUE;
}

int GameItem::GetCompressed()
{
	return pItem->GetData(INDEX_ID).Value + (Quantity * 1000);
}

void GameItem::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = this->GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	
	int CompressedValue;
	CompressedValue = GetCompressed();
	fwrite(&CompressedValue,sizeof(OBJECT_T),1,fp);
	
	fwrite(GetPosition(),	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	//save the mesh and texture
	MeshNum = Engine->GetMeshNum(pMesh);
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&MeshNum, sizeof(MeshNum),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);

	fwrite(&this->ModStat,sizeof(int),1,fp);
	fwrite(&this->ModAmount,sizeof(int),1,fp);
	fwrite(&this->ModEnd,sizeof(ULONG),1,fp);

	int OwnerID = 0;
	if(pOwner)
	{
		if(pOwner->GetObjectType() == OBJECT_CREATURE)
		{
			OwnerID = ((Creature *)pOwner)->GetData(INDEX_ID).Value;
		}
	}
	fwrite(&OwnerID, sizeof(int),1,fp);

	//save the actions on the current stack
	//save contents if any
	int NumItems = 0;
	//save their inventory/contents;
	Object *pOb;
	pOb = GetContents();
	while(pOb)
	{
		NumItems++;
		pOb = pOb->GetNext();
	}
	fwrite(&NumItems,sizeof(int),1,fp);
	pOb = GetContents();
	for(int n = 0; n < NumItems; n++)
	{
		((GameItem *)pOb)->Save(fp);
		pOb = pOb->GetNext();
	}


}

void GameItem::Load(FILE *fp)
{
	int CompressedValue;
	fread(&CompressedValue,sizeof(OBJECT_T),1,fp);
	SetItem(CompressedValue);

#ifndef NDEBUG
	if(!pItem)
	{
		DEBUG_INFO("Item not found in load\n");
		char blarg[64];
		sprintf(blarg,"ID #: %i\n",CompressedValue % 1000);
		SafeExit(blarg);
	}
#endif


	fread(GetPosition(),	sizeof(Position),1,fp);
	fread(&Frame, sizeof(Frame),1,fp);
	fread(&Data, sizeof(Data),1,fp);
	fread(&Angle, sizeof(Angle),1,fp);
	fread(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fread(&Scale, sizeof(Scale),1,fp);

	fread(&MeshNum, sizeof(MeshNum),1,fp);
	fread(&TextureNum, sizeof(TextureNum),1,fp);

	fread(&this->ModStat,sizeof(int),1,fp);
	fread(&this->ModAmount,sizeof(int),1,fp);
	fread(&this->ModEnd,sizeof(ULONG),1,fp);

	pMesh = Engine->GetMesh(MeshNum);
	pTexture = Engine->GetTexture(TextureNum);

	SetLocation(LOCATION_WORLD);
	int OwnerID = 0;
	fread(&OwnerID, sizeof(int),1,fp);
	if(OwnerID)
	{
		pOwner = (Object *)Thing::Find(Creature::GetFirst(),OwnerID);
	}

	//load contents if any
	int NumItems = 0;
	//save their inventory/contents;
	fread(&NumItems,sizeof(int),1,fp);
	for(int n = 0; n < NumItems; n++)
	{
		GameItem *pGI;
		pGI = new GameItem();
		OBJECT_T OT;
		fread(&OT,sizeof(OBJECT_T),1,fp);
		pGI->Load(fp);
		pGI->SetNext(pContents);
		pGI->SetOwner(this);
		pGI->SetLocation(LOCATION_CONTAINER);
		pContents = (Object *)pGI;
	}

}

void GameItem::SetItem(int CompressedValue)
{
	int ItemID = CompressedValue % 1000;
	Item *pItem;
	pItem = (Item *)Thing::Find(Item::GetFirst(),ItemID);

#ifndef NDEBUG
	if(!pItem)
	{
		char blarg[64];
		sprintf(blarg,"Bad GameItem #: %i\n", ItemID);
		DEBUG_INFO(blarg);
	}
#endif

	int NewQuantity = CompressedValue / 1000;
	SetItem(pItem,NewQuantity);
}


void GameItem::SetItem(Item *pNewItem, int NewQuantity)
{
	pItem = pNewItem;
	if(pNewItem)
	{
		SetMesh(pItem->GetMesh());
		SetFrame(pItem->GetFrame());
		SetTexture(pItem->GetTexture());
		SetVisScale(pItem->GetData("VISUALSCALE").fValue);
	}
	else
	{
		SetMesh(NULL);
		SetFrame(0);
		SetTexture(NULL);
		SetVisScale(1.0f);
	}
	SetQuantity(NewQuantity);
}

void GameItem::Draw()
{
	if(pRegionIn)
	{
		if(!pRegionIn->IsOccupied())
		{
			return;
		}
	}
	else
	{
		if(PreludeParty.Inside())
		{
			return;
		}
	}


	if(pMesh)
	{
		Engine->Graphics()->SetTexture(pTexture);
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, pItem->GetFrame());
	}
	return;
}

//split a game item in two
GameItem *GameItem::Split(int Amount)
{
	if(Amount >= Quantity)
	{
		return this;
	}
	else
	{
		Quantity -= Amount;
		GameItem *GI;
		GI = new GameItem;
		GI->SetItem(GetItem());
		GI->SetQuantity(Amount);
		return GI;
	}
}

GameItem::GameItem()
{
	pNext = NULL;
	pItem = NULL;
	Quantity = 0;
	pOwner = NULL;
	ModStat = 0;
	ModAmount = 0;
	ModEnd = 0;
	pRegionIn = NULL;
	VisScale = 1.0f;
}

GameItem::GameItem(int CompressedValue)
{
	SetItem(CompressedValue);
}

int GameItem::GetValidActions(Thing *pActor, ActionMenu *pAMenu)
{
	
	return TRUE;
}

int GameItem::GetDefaultAction(Object *pactor)
{
	//test pathing
	if(Location == LOCATION_WORLD)
	{
		if(!pItem->GetData(INDEX_MOVEABLITY).Value)
			return ACTION_PICKUP;
		else
		{
			if(pItem->GetData(INDEX_USE_OVERRIDE).Value)
				return ACTION_USE;
			else
			if(pItem->GetData("CONTAINERSIZE").Value)
				return ACTION_OPEN;
			else
				return ACTION_LOOKAT;
		}
	}
	else
	{
		return 0;
	}
}

int GameItem::LookAt(Object *looker)
{
	char LookAtString[512];
	char WeaponType[64];
	//check to see if there is a lookat over-ride
	if(GetData("LOOKATOVERRIDE").Value)
	{
		PreludeEvents.RunEvent(GetData("LOOKATOVERRIDE").Value);
		return TRUE;
	}
	//otherwise
	//check what type of item this is
	switch(GetData(INDEX_TYPE).Value)
	{
	case ITEM_TYPE_WEAPON:
		//weapon
		switch(this->GetData("SUBTYPE").Value)
		{
			case WEAPON_TYPE_MISSILE:
				sprintf(WeaponType,"Missile");
				break;
			case WEAPON_TYPE_THROWN:
				sprintf(WeaponType,"Thrown");
				break;
			case WEAPON_TYPE_SWORD:
				sprintf(WeaponType,"Sword");
				break;
			case WEAPON_TYPE_DAGGER:
				sprintf(WeaponType,"Dagger");
				break;
			case WEAPON_TYPE_AXE:
				sprintf(WeaponType,"Axe");
				break;
			case WEAPON_TYPE_BLUNT:
				sprintf(WeaponType,"Blunt");
				break;
			case WEAPON_TYPE_POLEARM:
				sprintf(WeaponType,"Polearm");
				break;
			case WEAPON_TYPE_UNARMED:
			default:
				sprintf(WeaponType,"Unarmed");
				break;
			
		}
		//display the item's name, damage, speed, and strength requirements 
		sprintf(LookAtString,"%s\n  %s\n  Speed: %i\n  Damage: %i - %i\n  Range: %i\n  Hands: %i\n  Required Str: %i\n  Required Dex: %i\n  Base Value: %i",
			GetData("NAME").String,
			WeaponType,
			GetData("SPEED").Value,
			GetData("MINDAMAGE").Value, 
			GetData("MAXDAMAGE").Value,
			(int)GetData("RANGE").fValue,
			GetData("NUMBERHANDS").Value,
			GetData("MINSTR").Value,
			GetData("MINDEX").Value,
			GetData("VALUE").Value);
		break;	
	case ITEM_TYPE_ARMOR:
		//armor
		//display the items' name, absorbtion rating, and strength requirements
		sprintf(LookAtString,"%s\n  Speed: -%i\n  Absorbtion: %i - %i\n  Required Str: %i\n  Required Dex: %i\n  Base Value: %i",
			GetData("NAME").String,
			GetData("SPEED").Value,
			GetData("ARMORMIN").Value, 
			GetData("ARMORMAX").Value,
			GetData("MINSTR").Value,
			GetData("MINDEX").Value,
			GetData("VALUE").Value);
		break;
	case ITEM_TYPE_AMMO:
		//ammo
		//display the item's name, damage, speed, and strength requirements 
		sprintf(LookAtString,"%s\n  Damage: %i - %i\n  Base Value: %i",
			GetData("NAME").String,
			GetData("MINDAMAGE").Value, 
			GetData("MAXDAMAGE").Value,
			GetData("VALUE").Value);
		break;	
	default:
		//for all other types of items, just display the item's name
		sprintf(LookAtString,"%s\n  Base Value: %i",
			GetData("NAME").String, 
			GetData("VALUE").Value);
		break;
	}
	//Describe(LookAtString);
	//check for stat mods
	int Index = this->GetItem()->GetIndex("MOD1");
	char TempMod[64];
	for(int n = 0; n < 5; n++)
	{
		if(this->GetData(Index).Value)
		{
			sprintf(TempMod,"\n  %s: %i",this->GetData(Index - 1).String, this->GetData(Index).Value);
			//check for an open character window
			strcat(LookAtString,TempMod);
		}

		Index += 2;
	}

	if(!strcmp(this->GetData("DESCRIPTION").String,"N"))
	{

	}
	else
	{
		strcat(LookAtString,"\n");
		strcat(LookAtString,this->GetData("DESCRIPTION").String);
	}

	ZSWindow *pWin;

	pWin = new ZSSayChar(-1, -1, -1, 0, 0, LookAtString, NULL);

	ZSWindow::GetMain()->AddTopChild(pWin);

	pWin->Show();

	pWin->SetFocus(pWin);

	pWin->GoModal();
	
	pWin->ReleaseFocus();

	ZSWindow::GetMain()->RemoveChild(pWin);

	assert(strlen(LookAtString) < 512);

	return TRUE;
}

BOOL GameItem::AdvanceFrame()
{
	return TRUE;
}

BOOL GameItem::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	if(!pIntersectMesh)
	{
		pIntersectMesh = Engine->GetMesh("selectbox");
	}
	if(this->pRegionIn != PreludeParty.GetLeader()->GetRegionIn())
	{
		return FALSE;
	}

	if(pItem->GetData(INDEX_MOVEABLITY).Value 
		&& !pItem->GetData(INDEX_USE_OVERRIDE).Value
		&& !pItem->GetData(INDEX_LOOKAT_OVERRIDE).Value
		&& !this->GetContents())
	{
		return FALSE;
	}

	int ItemX;
	int ItemY;
	int RayX;
	int RayY;
	RayX = (int)vRayEnd->x;
	RayY = (int)vRayEnd->y;
	ItemX = (int)GetPosition()->x;
	ItemY = (int)GetPosition()->y;

	if(this->GetPosition()->z > (Valley->GetTileHeight(ItemX, ItemY) + 0.15f))
	{
		if(!pMesh) return FALSE;
		//transform the ray into object coordinates;
		//first check if ray is close enough to matter
		//point to line distance
		float InverseHeight;
		float InverseWidth = 1.0f;
		float InverseDepth = 1.0f;

		InverseHeight = 1.0f/pMesh->GetHeight();

		//convert the ray to object coordinates
		//first translate then rotate
		D3DXMATRIX matRotate, matScale, matTransform;
		D3DXMatrixRotationZ( &matRotate, -(GetMyAngle() + PI));
		D3DXMatrixScaling(&matScale, InverseWidth, InverseDepth, InverseHeight);
		D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
		D3DXVECTOR4 vTransStart, vTransEnd;
		D3DVECTOR vMoveStart, vMoveEnd;
		vMoveStart = *vRayStart - *GetPosition();
		vMoveEnd = *vRayEnd - *GetPosition();
		D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
		D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
		return pIntersectMesh->Intersect(GetFrame(), (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
	}

	if(RayX == ItemX && RayY == ItemY)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

GameItem::operator =(GameItem &OtherThing)
{
	this->pItem = OtherThing.pItem;
	this->Quantity = OtherThing.Quantity;
	this->ModAmount = OtherThing.ModAmount;
	this->ModEnd = OtherThing.ModEnd;
	this->ModStat = OtherThing.ModStat;
	this->Location = OtherThing.Location;
	this->pRegionIn = OtherThing.pRegionIn;
	this->pOwner = OtherThing.pOwner;
}

DATA_FIELD_T GameItem::GetData(int fieldnum)
{
	if(ModStat && fieldnum == ModStat)
	{
		if(PreludeWorld->GetTotalTime() >= ModEnd)
		{
			DEBUG_INFO("Killing item Enchantment.\n");
			ModAmount = 0;
			ModStat = 0;
			ModEnd = 0;
		}
		else
		{
			DATA_FIELD_T RetVal;
			RetVal.Value = (pItem->GetData(fieldnum).Value + ModAmount);
			return RetVal;
		}
	}
	//reference the data fields by the number passed
	return pItem->GetData(fieldnum);
}

DATA_FIELD_T GameItem::GetData(char *fieldname)
{
	//compare each data field name to the field name passed
	//when a match is found return the value at that fieldname
	//if no match is found display an error message
	int n;
	n = pItem->GetIndex(fieldname);

	if(n != -1)
	{
		return this->GetData(n);
	}

	//display error
	
	DEBUG_INFO("GetData: Failed to find field: ");
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("bad GetData");

	return this->GetData(n);
}

GameItem::GameItem(GameItem *pGIFrom)
{
	this->pItem = pGIFrom->pItem;
	this->Quantity = pGIFrom->Quantity;
	this->ModAmount = pGIFrom->ModAmount;
	this->ModEnd = pGIFrom->ModEnd;
	this->ModStat = pGIFrom->ModStat;
	this->Location = pGIFrom->Location;
	this->pRegionIn = pGIFrom->pRegionIn;
	this->pOwner = pGIFrom->pOwner;
	this->pMesh = pGIFrom->pMesh;
	this->pTexture = pGIFrom->pTexture;
}

BOOL GameItem::TileBlock(int TileX, int TileY)
{
	//does this item block space
	int Blocking;
	Blocking = pItem->GetData(INDEX_SOLIDITY).Value;

	if(Blocking)
	{
		if((int)this->GetPosition()->x == TileX &&
			(int)this->GetPosition()->y == TileY)
		{
			return TRUE;
		}
	}

	return FALSE;
}
