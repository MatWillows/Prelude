#include "portals.h"
#include "world.h"
#include "zsutilities.h"
#include "regions.h"
#include "party.h"

BOOL Portal::AdvanceFrame()
{
	if((!pRegionOne || !pRegionTwo) && (RegionOneNum || RegionTwoNum))
	{
		if(PreludeWorld->GetRegion(RegionOneNum) && PreludeWorld->GetRegion(RegionTwoNum))
		{
		
			pRegionOne = PreludeWorld->GetRegion(RegionOneNum)->GetSubRegion(RegionOneSubNum);
			pRegionTwo = PreludeWorld->GetRegion(RegionTwoNum)->GetSubRegion(RegionTwoSubNum);
		}
		else
		{
			return TRUE;
		}
	}
	switch(State)
	{
		case PORTAL_OPENNING:
			if(!Frame)
			{	
				if(pRegionOne && pRegionTwo)
				{
					if(!pRegionOne->IsOccupied())
					{
							pRegionOne->SetOccupancy(REGION_SEEN);
					}
					if(!pRegionTwo->IsOccupied())
					{
							pRegionTwo->SetOccupancy(REGION_SEEN);
					}
				}
			}
			//calculate angle;
			SetAngle(ClosedAngle + ((float)Frame/(float)PORTAL_ANIMATION_FRAMES * (OpenAngle - ClosedAngle)));
			Frame++;
			if(Frame > PORTAL_ANIMATION_FRAMES)
			{
				Frame = 0;
				State = PORTAL_OPEN;
				
				/*if(ClosedAngle < (PI_DIV_2 - 0.1))
				{
					Valley->ClearBlocking((int)Position.x,(int)Position.y);
					Valley->ClearBlocking((int)Position.x,((int)Position.y) + 1);
				}
				*/
			}
			break;
		case PORTAL_CLOSING:
			SetAngle(OpenAngle + ((float)Frame/(float)PORTAL_ANIMATION_FRAMES * (ClosedAngle - OpenAngle)));
			Frame++;
			if(Frame > PORTAL_ANIMATION_FRAMES)
			{
				Frame = 0;
				State = PORTAL_CLOSED;
				
				if(pRegionOne && pRegionTwo)
				{
					if(pRegionOne->IsOccupied() != REGION_OCCUPIED && (pRegionOne->GetType() != REGION_EXTERIOR && !PreludeParty.Inside()))
					{
						pRegionOne->SetOccupancy(REGION_UNSEEN);
					}
					if(pRegionTwo->IsOccupied() != REGION_OCCUPIED && (pRegionTwo->GetType() != REGION_EXTERIOR && !PreludeParty.Inside()))
					{
						pRegionTwo->SetOccupancy(REGION_UNSEEN);
					}
				}
			}
			break;
		default:
			Frame = 0;
			break;
	}

	return TRUE;
}

int Portal::LookAt(Object *pLooker)
{
	if(State == PORTAL_LOCKED || (State == PORTAL_CLOSED && LockNum))
	{
		Describe("Locked.");
	}
	else
	{
		Describe("Unlocked.");
	}
	return TRUE;
}

int Portal::GetDefaultAction(Object *pactor)
{
	if(State == PORTAL_OPEN)
	{
		return ACTION_CLOSE;
	}
	else
	if(State == PORTAL_CLOSED)
	{
		return ACTION_OPEN;
	}

	return 0;
}

void Portal::Load(FILE *fp)
{
	pRegionOne = NULL;
	pRegionTwo = NULL;
	fread(&Position,	sizeof(Position),1,fp);
	fread(&Frame, sizeof(Frame),1,fp);
	fread(&Data, sizeof(Data),1,fp);
	fread(&Angle, sizeof(Angle),1,fp);
	fread(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fread(&Scale, sizeof(Scale),1,fp);
	fread(&MeshNum, sizeof(MeshNum),1,fp);
	fread(&TextureNum, sizeof(TextureNum),1,fp);
	fread(&State, sizeof(State),1,fp);
	fread(&ClosedAngle, sizeof(ClosedAngle),1,fp);
	fread(&OpenAngle, sizeof(OpenAngle),1,fp);
	fread(&OpenLocation, sizeof(OpenLocation),1,fp);
	fread(&ClosedLocation, sizeof(ClosedLocation),1,fp);
	fread(&RegionOneNum, sizeof(int),1,fp);
	fread(&RegionTwoNum, sizeof(int),1,fp);
	fread(&RegionOneSubNum, sizeof(int),1,fp);
	fread(&RegionTwoSubNum, sizeof(int),1,fp);
	
	fread(&KeyNum, sizeof(int),1,fp);
	fread(&EventNum, sizeof(int),1,fp);
	fread(&LockNum, sizeof(int),1,fp);


	
	pMesh = Engine->GetMesh(MeshNum);
	pTexture = Engine->GetTexture(TextureNum);

	if(PreludeWorld->GetRegion(RegionOneNum) && PreludeWorld->GetRegion(RegionTwoNum))
	{
		pRegionOne = PreludeWorld->GetRegion(RegionOneNum)->GetSubRegion(RegionOneSubNum);
		pRegionTwo = PreludeWorld->GetRegion(RegionTwoNum)->GetSubRegion(RegionTwoSubNum);
	}
}



void Portal::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	MeshNum = Engine->GetMeshNum(pMesh);
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&Position,	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	fwrite(&MeshNum, sizeof(MeshNum),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);
	fwrite(&State, sizeof(State),1,fp);
	fwrite(&ClosedAngle, sizeof(ClosedAngle),1,fp);
	fwrite(&OpenAngle, sizeof(OpenAngle),1,fp);
	fwrite(&OpenLocation, sizeof(OpenLocation),1,fp);
	fwrite(&ClosedLocation, sizeof(ClosedLocation),1,fp);
	fwrite(&RegionOneNum, sizeof(int),1,fp);
	fwrite(&RegionTwoNum, sizeof(int),1,fp);
	fwrite(&RegionOneSubNum, sizeof(int),1,fp);
	fwrite(&RegionTwoSubNum, sizeof(int),1,fp);
	
	fwrite(&KeyNum, sizeof(int),1,fp);
	fwrite(&EventNum, sizeof(int),1,fp);
	fwrite(&LockNum, sizeof(int),1,fp);


}

Portal::~Portal()
{

}

Portal::Portal()
{
	pNext = NULL;
	Angle = 0.0f;
	TextureNum = 0;
	pTexture = NULL;
	Frame = 0;
	Position = D3DVECTOR(0.0f,0.0f,0.0f);
	Scale = 1.0f;
	MeshNum = 0;
	pMesh = NULL;

	KeyNum = LockNum = EventNum = 0;


	State = PORTAL_CLOSED;
	pRegionOne = NULL;
	RegionOneNum = 0;
	RegionOneSubNum = 0;
	pRegionTwo = NULL;
	RegionTwoNum = 0;
	RegionTwoSubNum = 0;
	OpenAngle = 0.0f;
	ClosedAngle = PI_DIV_2;
}

void Portal::Draw()
{
	//get a pointer to the mesh indicated by the things mesh field
	//set the current texture to the texture specified in the texture field
	//call the mesh's  draw function with the proper position, rotation,
	//and scaling factors
	//done
	if(pRegionOne && pRegionTwo && (pRegionOne->IsOccupied() || pRegionTwo->IsOccupied()) && pMesh)
	{
		Engine->Graphics()->SetTexture(pMesh->GetTexture());
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, 0);
	}
	else
	if(!RegionOneNum && !RegionTwoNum)
	{
		Engine->Graphics()->SetTexture(pMesh->GetTexture());
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, 0);
	}
	return;
}

void Portal::SetRegionOne(Region *NewOne)
{
	pRegionOne = NewOne;
	if(!pRegionOne->GetParent())
	{
		RegionOneNum = pRegionOne->GetID();
		RegionOneSubNum = 0;
	}
	else
	{
		RegionOneNum = pRegionOne->GetParent()->GetID();
		RegionOneSubNum = pRegionOne->GetID();
	}
	
}

void Portal::SetRegionTwo(Region *NewTwo)
{
	pRegionTwo = NewTwo;
	if(!pRegionTwo->GetParent())
	{
		RegionTwoNum = pRegionTwo->GetID();
		RegionTwoSubNum = 0;
	}
	else
	{
		RegionTwoNum = pRegionTwo->GetParent()->GetID();
		RegionTwoSubNum = pRegionTwo->GetID();
	}
}

BOOL Portal::TileBlock(int TileX, int TileY)
{
	//if open
	if(State == PORTAL_OPEN)
	{
		return FALSE;
	}
	else
	if(ClosedAngle > (PI_DIV_2 - 0.1))
	{
		if(TileX == (int)(Position.x + 0.4f) && (TileY == (int)(Position.y + 0.4f)   || TileY == (int)(Position.y + 0.4f) + 1))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if(TileY == (int)(Position.y + 0.4f) && (TileX == (int)(Position.x + 0.4f)  || TileX == (int)(Position.x + 0.4f) + 1))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

BOOL Portal::TileIntersect(int TileX, int TileY)
{
	if(State == PORTAL_OPEN)
	{
		return FALSE;
	}
	else
	if(ClosedAngle > (PI_DIV_2 - 0.1))
	{
		if(TileX == (int)(Position.x + 0.4f) && (TileY == (int)Position.y || TileY == (int)Position.y  + 1))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if(TileY == (int)(Position.y + 0.4f) && (TileX == (int)Position.x  || TileX == (int)Position.x + 1))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

int Portal::AltHighLight()
{
	//display text in the appropriate location
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

	this->HighLight(COLOR_RED);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);

	return TRUE;

}