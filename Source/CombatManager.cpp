#include "combatmanager.h"
#include "objects.h"
#include "things.h"
#include "creatures.h"
#include "party.h"
#include "world.h"
#include "defs.h"
#include "zsportrait.h"
#include "zstext.h"
#include "assert.h"
#include "events.h"
#include "deathwin.h"
#include "modifiers.h"
#include "path.h"
#include "zshelpwin.h"
#include "zsmessage.h"
#include "regions.h"
#include "MainWindow.h"

#define COMBATCONVERT(x,y) (y - rCombat.top) * COMBAT_WIDTH + (x - rCombat.left)

void Combat::FillCombatArea()
{
	NumMoveVerts = 0;

	ZeroMemory(CombatArea,sizeof(unsigned short) * COMBAT_WIDTH * COMBAT_HEIGHT);
	//set up the combat rect
	rCombat.left = PreludeParty.GetLeader()->GetPosition()->x - CREATURE_RANGE;
	rCombat.right = rCombat.left + COMBAT_WIDTH;
	rCombat.top = PreludeParty.GetLeader()->GetPosition()->y - CREATURE_RANGE;
	rCombat.bottom = rCombat.top + COMBAT_HEIGHT;

	if(rCombat.left < 0)
	{
		rCombat.left = 0;
		rCombat.right = rCombat.left + COMBAT_WIDTH;
	}

	if(rCombat.top < 0)
	{
		rCombat.top = 0;
		rCombat.bottom = rCombat.top + COMBAT_HEIGHT;
	}

	if(rCombat.right >= Valley->GetWidth())
	{
		rCombat.right = Valley->GetWidth() - 1;
		if(rCombat.right - COMBAT_WIDTH >= 0)
		{
			rCombat.left = rCombat.right - COMBAT_WIDTH;
		}
		else
		{
			rCombat.left = 0;
		}
	}

	if(rCombat.bottom >= Valley->GetHeight())
	{
		rCombat.bottom = Valley->GetHeight() - 1;
		if(rCombat.bottom - COMBAT_HEIGHT >= 0)
		{
			rCombat.top = rCombat.bottom - COMBAT_HEIGHT;
		}
		else
		{
			rCombat.top = 0;
		}
	}
	
	int CWidth, CHeight;
	CWidth = rCombat.right - rCombat.left;
	CHeight = rCombat.bottom - rCombat.top;

	rCombatArea.left = 0;
	rCombatArea.top = 0;
	rCombatArea.bottom = CHeight;
	rCombatArea.right = CWidth;

	int xn;
	int yn;

	int StartX;
	int StartY;
	int EndX;
	int EndY;
	StartX = rCombat.left / CHUNK_TILE_WIDTH;
	StartY = rCombat.top / CHUNK_TILE_WIDTH;
	EndX = rCombat.right / CHUNK_TILE_WIDTH;
	EndY = rCombat.bottom / CHUNK_TILE_WIDTH;

	for(yn = StartY; yn <= EndY; yn++)
	for(xn = StartX; xn <= EndX; xn++)
	{
		Chunk *pChunk;
		pChunk = Valley->GetChunk(xn,yn);
		if(!pChunk)
		{
			Valley->LoadChunk(xn,yn);
			pChunk = Valley->GetChunk(xn,yn);
			if(pChunk)
			{
				Engine->Graphics()->GetD3D()->BeginScene();

				pChunk->CreateTexture(Valley->GetBaseTexture());
				
				Engine->Graphics()->GetD3D()->EndScene();
			}
		}

	}


	for(yn = rCombat.top + 1; yn < rCombat.bottom - 1; yn++)
	for(xn = rCombat.left + 1; xn < rCombat.right - 1; xn++)
	{
		if(Valley->GetBlocking(xn,yn))
		{
			CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
		}
		else
		{
			CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_EMPTY;
		}
	}


	for(yn = rCombat.top + 1; yn < rCombat.bottom - 1; yn++)
	for(xn = rCombat.left + 1; xn < rCombat.right - 1; xn++)
	{
		if(Valley->GetBlocking(xn,yn))
		{
			CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
		}
		else
		{
			CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_EMPTY;
		}
	}

	//blocked off the edges of combat
	xn = rCombat.left;
	for(yn = rCombat.top; yn < rCombat.bottom; yn++)
	{
		CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
	}

	xn = rCombat.right - 1;
	for(yn = rCombat.top; yn < rCombat.bottom; yn++)
	{
		CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
	}

	yn = rCombat.top;
	for(xn = rCombat.left; xn < rCombat.right; xn++)
	{
		CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
	}

	yn = rCombat.bottom -1;
	for(xn = rCombat.left; xn < rCombat.right; xn++)
	{
		CombatArea[COMBATCONVERT(xn,yn)] = COMBAT_LOCATION_BLOCKED;
	}
}

int Combat::GetMoveLeft(int xn, int yn)
{
	return (int)CreatureMove[COMBATCONVERT(xn,yn)];
}

void Combat::RecordMoveLeft(int xn, int yn, int mp)
{
	char *pi;
	pi = &CreatureMove[COMBATCONVERT(xn,yn)];
	if(mp > GetMoveLeft(xn,yn))
	{
		*pi = (char)mp;
	}
}


void inline Combat::CreautureMoveTileSurround(int xn, int yn, int AttackP, int MP)
{
	int tx = xn - 1;
	int ty = yn - 1;
	int AP = GetMoveLeft(xn,yn);
	if(AP < 0) return;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - (MP + MP / 2));
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - MP);
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - (MP + MP / 2));
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx = xn - 1;
	ty++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - MP);
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx = xn + 1;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - MP);
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx = xn - 1;
	ty++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - (MP + MP / 2));
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - MP);
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
	tx++;
	if(!(CreatureArea[COMBATCONVERT(tx,ty)] & (COMBAT_LOCATION_BLOCKED | COMBAT_LOCATION_OCCUPIED)))
	{
		CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_WALKABLE;
		RecordMoveLeft(tx,ty,AP - (MP + MP / 2));
		if(AP > AttackP)
			CreatureArea[COMBATCONVERT(tx,ty)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	}
}

float GetDistance(int x1, int y1, int x2, int y2)
{
	float a;
	float b;
	a = (float)abs(x1-x2);
	b = (float)abs(y1-y2);

	return sqrt(a*a + b*b);
}


void Combat::FillCreatureArea()
{

	//get the base area in
	NumMoveVerts = 0;
	ZeroMemory(MoveDrawList,sizeof(unsigned short) * 4 * COMBAT_WIDTH * COMBAT_HEIGHT);
	ZeroMemory(MoveVerts,sizeof(unsigned short) * 4 * COMBAT_WIDTH * COMBAT_HEIGHT);
	ZeroMemory(CreatureArea,sizeof(unsigned short) * COMBAT_WIDTH * COMBAT_HEIGHT);
	memset(CreatureMove, -1, sizeof(char) * COMBAT_WIDTH * COMBAT_HEIGHT);
	memcpy(CreatureArea,CombatArea,sizeof(unsigned short) * COMBAT_WIDTH * COMBAT_HEIGHT);
	Creature *pActive;
	pActive = (Creature *)this->GetActiveCombatant();
	ZeroMemory(EnemyReferenceList, sizeof(BOOL) * MAX_COMBATANTS);

	if(!PreludeParty.IsMember(pActive) && !pActive->GetData(INDEX_AICODE).Value) return;

	TileZOffset = 0.015f;
	
	//fill the moveable area
	int AP;
	AP = pActive->GetData(INDEX_ACTIONPOINTS).Value;

	int MP;
	MP = pActive->GetData(INDEX_MOVEPOINTS).Value;

	int AttackP;
	AttackP;
	AttackP = pActive->GetData(INDEX_ATTACKPOINTS).Value;

	int MaxRange;
	MaxRange = AP / MP;
	MaxRange += 1;

	int xn;
	int yn;
	int StartX;
	int StartY;
	int EndX;
	int EndY;
	int ActiveX;
	int ActiveY;
	int Offset;
	
	ActiveX = (int)pActive->GetPosition()->x;
	ActiveY = (int)pActive->GetPosition()->y;

	CreatureArea[COMBATCONVERT(ActiveX,ActiveY)] |= COMBAT_LOCATION_WALKABLE;

	
	//the creatures;
	Object *pOb;
	Creature *pCreature;
	pOb = this->GetCombatants();

	int BattleSide;
	BattleSide = pActive->GetData(INDEX_BATTLESIDE).Value;

	float fARange;
	fARange = pActive->GetData(INDEX_RANGE).fValue;

	EnemiesInRange = 0;
	Path TempPath;


	int EnemyNum;
	float fEnemyRange;
	int iER;


	while(pOb)
	{
		pCreature = (Creature *) pOb;
		EnemyNum = pOb->GetData();
		int cx;
		int cy;

		cx = (int)pCreature->GetPosition()->x;
		cy = (int)pCreature->GetPosition()->y;
	
		CreatureArea[COMBATCONVERT(cx,cy)] |= COMBAT_LOCATION_OCCUPIED;
		if(pCreature->IsLarge())
		{
			CreatureArea[COMBATCONVERT(cx+1,cy)] |= COMBAT_LOCATION_OCCUPIED;
			CreatureArea[COMBATCONVERT(cx,cy+1)] |= COMBAT_LOCATION_OCCUPIED;
			CreatureArea[COMBATCONVERT(cx+1,cy+1)] |= COMBAT_LOCATION_OCCUPIED;
		}

		if(pCreature->GetData(INDEX_BATTLESIDE).Value != BattleSide)
		{
			
			EnemyReferenceList[EnemyNum] = TRUE;
			float fDist = GetDistance(pCreature,pActive);
			
			if(fDist < fARange)
			{
				if(CheckLineOfSight(ActiveX, ActiveY, cx, cy, pActive, pOb))
				{
					EnemiesInRangeList[EnemiesInRange] = pOb;
					EnemyDistance[EnemiesInRange] = fDist; 
					if(AP >= AttackP)
					{
						AddEnemyTile(cx,cy,AttackP);
						if(pCreature->IsLarge())
						{
							AddEnemyTile(cx,cy+1,AttackP);
							AddEnemyTile(cx+1,cy,AttackP);
							AddEnemyTile(cx+1,cy+1,AttackP);
						}
			
					}
				}
			}
/*
			fEnemyRange = pCreature->GetData(INDEX_RANGE).fValue;
			iER = (int)fEnemyRange;
			StartX = cx-iER;
			EndX = cx+iER;
			StartY = cy-iER;
			EndY = cy+iER;
			if(StartX < 0) StartX = 0;
			if(StartY < 0) StartY = 0;
			if(EndX >= rCombat.right) EndX = rCombat.right - 1;
			if(EndY >= rCombat.bottom) EndY = rCombat.bottom - 1;
			
			for(yn = StartY; yn <= EndY; yn++)
			for(xn = StartX; xn <= EndX; xn++)
			{
				fDist = GetDistance(xn,yn,cx,cy);

				if(fDist < fEnemyRange && CheckLineOfSight(ActiveX, ActiveY, xn, yn, pActive, pOb))
				{
					if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_THREATENED)
					{
						if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_VERY_THREATENED)
						{
							CreatureArea[COMBATCONVERT(xn,yn)] |= COMBAT_LOCATION_EXTREME_THREAT;
						}
						else
						{
							CreatureArea[COMBATCONVERT(xn,yn)] |= COMBAT_LOCATION_VERY_THREATENED;
						}
					}
					else
					{
						CreatureArea[COMBATCONVERT(xn,yn)] |= COMBAT_LOCATION_THREATENED;
					}

				}
			}
*/
			if(TempPath.FindCombatPath(ActiveX, ActiveY,cx,cy, fARange))
			{
				TempPath.GetEnd(&PathAttackX[EnemyNum],
								&PathAttackY[EnemyNum]);
				PathToAttackLength[EnemyNum] = TempPath.GetLength();
			}
			else
			{
				PathAttackX[EnemyNum] = -1;
				PathAttackY[EnemyNum] = -1;
				PathToAttackLength[EnemyNum] = 999;
			}
			
		}
					
		pOb = pOb->GetNextUpdate();
	}

	CreatureArea[COMBATCONVERT(ActiveX,ActiveY)] |= COMBAT_LOCATION_WALKABLE;
	RecordMoveLeft(ActiveX,ActiveY,AP);
	if(AP > AttackP)
		CreatureArea[COMBATCONVERT(ActiveX,ActiveY)] |= COMBAT_LOCATION_TIME_FOR_ATTACK;
	CreautureMoveTileSurround(ActiveX, ActiveY, AttackP, MP);

	//fill movement area
	for(Offset = 1; Offset <= MaxRange; Offset++)
	{
		AP -= MP;
		yn = ActiveY - Offset;
		if(yn > rCombat.top)
		for(xn = ActiveX - Offset; xn < ActiveX + Offset; xn++)
		{
			if(xn > rCombat.left && xn < rCombat.right - 1 && yn < rCombat.bottom -1)
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_WALKABLE)
			{
				CreautureMoveTileSurround(xn,yn,AttackP, MP);
				//AddMoveTile(xn,yn, AttackP);
			}
		}
		xn = ActiveX + Offset;
		if(xn < rCombat.right - 1)
		for(yn = ActiveY - Offset; yn < ActiveY + Offset; yn++)
		{
			if(xn > rCombat.left && yn > rCombat.top && yn < rCombat.bottom -1)
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_WALKABLE)
			{
				CreautureMoveTileSurround(xn,yn,AttackP, MP);
				//AddMoveTile(xn,yn, AttackP);
			}
		}
		yn = ActiveY + Offset;
		if(yn < rCombat.bottom - 1)
		for(xn = ActiveX + Offset; xn > ActiveX - Offset; xn--)
		{
			if(xn > rCombat.left && yn > rCombat.top && xn < rCombat.right - 1)
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_WALKABLE)
			{
				CreautureMoveTileSurround(xn,yn,AttackP, MP);
				//AddMoveTile(xn,yn, AttackP);
			}
		}
		xn = ActiveX - Offset;
		if(xn > rCombat.left)
		for(yn = ActiveY + Offset; yn > ActiveY - Offset; yn--)
		{
			if(yn > rCombat.top && xn < rCombat.right - 1 && yn < rCombat.bottom -1)
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_WALKABLE)
			{
				CreautureMoveTileSurround(xn,yn,AttackP, MP);
				//AddMoveTile(xn,yn, AttackP);
			}
		}
	}
	
	int APRemaining;

	//fill in the draw list
	StartY = ActiveY - MaxRange;
	StartX = ActiveX - MaxRange;
	if(StartY < rCombat.top) StartY = rCombat.top;
	if(StartX < rCombat.left) StartX = rCombat.left;
	
	EndX = ActiveX + MaxRange;
	EndY = ActiveY + MaxRange;
	if(EndY >= rCombat.bottom) EndY = rCombat.bottom - 1;
	if(EndX >= rCombat.right) EndX = rCombat.right - 1;

	for(yn = StartY; yn <= EndY; yn++)
	for(xn = StartX; xn <= EndX; xn++)
	{
		if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_WALKABLE)
		{
			APRemaining = GetMoveLeft(xn,yn);
			if(APRemaining >= 0)
			{
				AddMoveTile(xn, yn, AttackP);
			}
		}
	}


}

void Combat::DrawMoveArea()
{
	//clear matrix;

	Engine->Graphics()->GetD3D()->SetMaterial(&matMove);
	Engine->Graphics()->SetTexture(pTexture);

//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR, D3DRGBA(fAlpha,fAlpha,fAlpha,fAlpha));
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD);


	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, MoveVerts, NumMoveVerts * 4, MoveDrawList, NumMoveVerts * 6, 0);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	matMove.specular.a = matMove.ambient.a = matMove.diffuse.a = fAlpha;

	fAlpha += fAlphaMod;

	if(fAlpha > 0.35f)
	{
		fAlphaMod = -0.015f;
	}
	else
	if(fAlpha < 0.1f)
	{
		fAlphaMod = 0.015f;
	}	

}

void Combat::AddFriendTile(int xn, int yn, int AttackP)
{

}

void Combat::AddEnemyTile(int xn, int yn, int AttackP)
{
	unsigned short DrawBase = NumMoveVerts * 6;
	unsigned short VertBase = NumMoveVerts * 4;
	MoveDrawList[DrawBase] = VertBase;
	MoveDrawList[DrawBase + 1] = VertBase + 1;
	MoveDrawList[DrawBase + 2] = VertBase + 2;
	MoveDrawList[DrawBase + 3] = VertBase + 1;
	MoveDrawList[DrawBase + 4] = VertBase + 3;
	MoveDrawList[DrawBase + 5] = VertBase + 2;

	MoveVerts[VertBase + 2].x = MoveVerts[VertBase].x = (float)xn;
	MoveVerts[VertBase + 3].x = MoveVerts[VertBase + 1].x = (float)(xn + 1);

	MoveVerts[VertBase + 1].y = MoveVerts[VertBase].y = (float)yn;
	MoveVerts[VertBase + 2].y = MoveVerts[VertBase + 3].y = (float)(yn + 1);

		float TileZ;
	TileZ = Valley->GetTileHeight(xn,yn) + TileZOffset;

	float tne, tn, tnw, tw, tsw, ts, tse, te;

	tne = Valley->GetTileHeight(xn + 1, yn - 1) + TileZOffset;
	tn =  Valley->GetTileHeight(xn    , yn - 1) + TileZOffset;
	tnw = Valley->GetTileHeight(xn - 1, yn - 1) + TileZOffset;
	tw =  Valley->GetTileHeight(xn - 1, yn    ) + TileZOffset;
	tsw = Valley->GetTileHeight(xn - 1, yn + 1) + TileZOffset;
	ts =  Valley->GetTileHeight(xn    , yn + 1) + TileZOffset;
	tse = Valley->GetTileHeight(xn + 1, yn + 1) + TileZOffset;
	te =  Valley->GetTileHeight(xn + 1, yn    ) + TileZOffset;

	float fNW;
	float fNE;
	float fSE;
	float fSW;
	
	fNW = tnw;
	if(tn > fNW) fNW = tn;
	if(tw > fNW) fNW = tw;
	if(TileZ > fNW) fNW = TileZ;

	fNE = tne;
	if(tn > fNE) fNE = tn;
	if(te > fNE) fNE = te;
	if(TileZ > fNE) fNE = TileZ;

	fSW = tsw;
	if(ts > fSW) fSW = ts;
	if(tw > fSW) fSW = tw;
	if(TileZ > fSW) fSW = TileZ;

	fSE = tse;
	if(ts > fSE) fSE = ts;
	if(te > fSE) fSE = te;
	if(TileZ > fSE) fSE = TileZ;

	MoveVerts[VertBase].z = fNW; 
	MoveVerts[VertBase + 1].z = fNE;
	MoveVerts[VertBase + 2].z = fSW;
	MoveVerts[VertBase + 3].z = fSE;
	MoveVerts[VertBase].tu = 0.6f;
	MoveVerts[VertBase].tv = 0.6f;
	MoveVerts[VertBase+1].tu = 0.9f;
	MoveVerts[VertBase+1].tv = 0.6f;
	MoveVerts[VertBase+2].tu = 0.6f;
	MoveVerts[VertBase+2].tv = 0.9f;
	MoveVerts[VertBase+3].tu = 0.9f;
	MoveVerts[VertBase+3].tv = 0.9f;

	NumMoveVerts++;
}


void Combat::AddMoveTile(int xn, int yn, int AttackP)
{
	unsigned short DrawBase = NumMoveVerts * 6;
	unsigned short VertBase = NumMoveVerts * 4;
	MoveDrawList[DrawBase] = VertBase;
	MoveDrawList[DrawBase + 1] = VertBase + 1;
	MoveDrawList[DrawBase + 2] = VertBase + 2;
	MoveDrawList[DrawBase + 3] = VertBase + 1;
	MoveDrawList[DrawBase + 4] = VertBase + 3;
	MoveDrawList[DrawBase + 5] = VertBase + 2;

	
	MoveVerts[VertBase + 2].x = MoveVerts[VertBase].x = (float)xn;
	MoveVerts[VertBase + 3].x = MoveVerts[VertBase + 1].x = (float)(xn + 1);

	MoveVerts[VertBase + 1].y = MoveVerts[VertBase].y = (float)yn;
	MoveVerts[VertBase + 2].y = MoveVerts[VertBase + 3].y = (float)(yn + 1);

	float TileZ;
	TileZ = Valley->GetTileHeight(xn,yn) + TileZOffset;

	float tne, tn, tnw, tw, tsw, ts, tse, te;

	tne = Valley->GetTileHeight(xn + 1, yn - 1) + TileZOffset;
	tn =  Valley->GetTileHeight(xn    , yn - 1) + TileZOffset;
	tnw = Valley->GetTileHeight(xn - 1, yn - 1) + TileZOffset;
	tw =  Valley->GetTileHeight(xn - 1, yn    ) + TileZOffset;
	tsw = Valley->GetTileHeight(xn - 1, yn + 1) + TileZOffset;
	ts =  Valley->GetTileHeight(xn    , yn + 1) + TileZOffset;
	tse = Valley->GetTileHeight(xn + 1, yn + 1) + TileZOffset;
	te =  Valley->GetTileHeight(xn + 1, yn    ) + TileZOffset;

	float fNW;
	float fNE;
	float fSE;
	float fSW;
	
	fNW = tnw;
	if(tn > fNW) fNW = tn;
	if(tw > fNW) fNW = tw;
	if(TileZ > fNW) fNW = TileZ;

	fNE = tne;
	if(tn > fNE) fNE = tn;
	if(te > fNE) fNE = te;
	if(TileZ > fNE) fNE = TileZ;

	fSW = tsw;
	if(ts > fSW) fSW = ts;
	if(tw > fSW) fSW = tw;
	if(TileZ > fSW) fSW = TileZ;

	fSE = tse;
	if(ts > fSE) fSE = ts;
	if(te > fSE) fSE = te;
	if(TileZ > fSE) fSE = TileZ;

	MoveVerts[VertBase].z = fNW; 
	MoveVerts[VertBase + 1].z = fNE;
	MoveVerts[VertBase + 2].z = fSW;
	MoveVerts[VertBase + 3].z = fSE;

	int APRemaining;
	APRemaining = GetMoveLeft(xn,yn);
	if(APRemaining >= AttackP)
	{
		MoveVerts[VertBase].tu = 0.1f;
		MoveVerts[VertBase].tv = 0.1f;
		MoveVerts[VertBase+1].tu = 0.4f;
		MoveVerts[VertBase+1].tv = 0.1f;
		MoveVerts[VertBase+2].tu = 0.1f;
		MoveVerts[VertBase+2].tv = 0.4f;
		MoveVerts[VertBase+3].tu = 0.4f;
		MoveVerts[VertBase+3].tv = 0.4f;
	}
	else
	{
		MoveVerts[VertBase].tu = 0.6f;
		MoveVerts[VertBase].tv = 0.1f;
		MoveVerts[VertBase+1].tu = 0.9f;
		MoveVerts[VertBase+1].tv = 0.1f;
		MoveVerts[VertBase+2].tu = 0.6f;
		MoveVerts[VertBase+2].tv = 0.4f;
		MoveVerts[VertBase+3].tu = 0.9f;
		MoveVerts[VertBase+3].tv = 0.4f;
	}

		

	NumMoveVerts++;

}

void Combat::ClearMoveArea()
{

}

void Combat::HighlightEnemies()
{

}

BOOL Combat::CheckLineOfSight(int x1, int y1, int x2, int y2, Object *pSource, Object *pTarget)
{
	//check for straight line patterns
	if(pSource && pSource->GetObjectType() == OBJECT_CREATURE
		&&
		pTarget && pTarget->GetObjectType() == OBJECT_CREATURE)
	{
		if(((Creature *)pSource)->IsLarge() && ((Creature *)pTarget)->IsLarge())
		{
			if(CheckLineOfSight(x1,y1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2+1,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2,y2+1,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2+1,y2+1,NULL,NULL)) return TRUE;
			
			if(CheckLineOfSight(x1+1,y1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1,x2+1,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1,x2,y2+1,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1,x2+1,y2+1,NULL,NULL)) return TRUE;
			
			if(CheckLineOfSight(x1,y1+1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1+1,x2+1,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1+1,x2,y2+1,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1+1,x2+1,y2+1,NULL,NULL)) return TRUE;

			if(CheckLineOfSight(x1+1,y1+1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1+1,x2+1,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1+1,x2,y2+1,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1+1,x2+1,y2+1,NULL,NULL)) return TRUE;

			return FALSE;
		}
		else
		if(((Creature *)pSource)->IsLarge())
		{
			if(CheckLineOfSight(x1,y1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1+1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1+1,y1+1,x2,y2,NULL,NULL)) return TRUE;

			return FALSE;
		}
		else
		if(((Creature *)pTarget)->IsLarge())
		{
			if(CheckLineOfSight(x1,y1,x2,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2+1,y2,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2,y2+1,NULL,NULL)) return TRUE;
			if(CheckLineOfSight(x1,y1,x2+1,y2+1,NULL,NULL)) return TRUE;
	
			return FALSE;
		}
	}


	int xn,yn, cn;
	int XOffset;
	int YOffset;
	int Count;
	xn = x1;
	yn = y1;
	
	
	D3DVECTOR vLineStart;
	D3DVECTOR vLineEnd;
	vLineStart.x = x1 + 0.5f;
	vLineStart.y = y1 + 0.5f;
	vLineStart.z = Valley->GetTileHeight(x1,y1) + 1.5f;

	vLineEnd.x = x2 + 0.5f;
	vLineEnd.y = y2 + 0.5f;
	vLineEnd.z = Valley->GetTileHeight(x2,y2) + 1.5f;

	return Valley->CheckLOS(&vLineStart, &vLineEnd);
	/*
	Region *pRegion;

	pRegion = Valley->GetRegion(x1,y1);

	if(pRegion)
	{
		if(pRegion->LineIntersect(&vLineStart,&vLineEnd))
		{	
			return FALSE;
		}
	}
	
	pRegion = Valley->GetRegion(x2,y2);

	if(pRegion)
	{
		if(pRegion->LineIntersect(&vLineStart,&vLineEnd))
		{	
			return FALSE;
		}
	}
	
	pRegion = Valley->GetRegion(x1,y2);

	if(pRegion)
	{
		if(pRegion->LineIntersect(&vLineStart,&vLineEnd))
		{	
			return FALSE;
		}
	}

	pRegion = Valley->GetRegion(x2,y1);

	if(pRegion)
	{
		if(pRegion->LineIntersect(&vLineStart,&vLineEnd))
		{	
			return FALSE;
		}
	}

	pRegion = Valley->GetRegion((x1 + x2) / 2, (y1 + y2) / 2);

	if(pRegion)
	{
		if(pRegion->LineIntersect(&vLineStart,&vLineEnd))
		{	
			return FALSE;
		}
	}
	
	if(x2 == x1)
	{
		if(y1 == y2) return TRUE;

		xn = x1;
		if(y1 < y2)
		{
			YOffset = 1;
		}
		else
		{
			YOffset = -1;
		}

		for(yn = y1+1; yn != y2; yn += YOffset)
		{
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_BLOCKED)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	else
	if(y1 == y2)
	{
		yn = y1;
		if(x1 < x2)
		{
			XOffset = 1;
		}
		else
		{
			XOffset = -1;
		}

		for(xn = x1+1; xn != x2; xn += XOffset)
		{
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_BLOCKED)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	else
	if(abs(y2-y1) > abs(x2 - x1))
	{
		Count = abs(y2 - y1) / abs(x2 - x1);
		if(x1 > x2)
		{
			XOffset = -1;
		}
		else
		{
			XOffset = 1;
		}
		if(y1 > y2)
		{
			YOffset = -1;
		}
		else
		{
			YOffset = 1;
		}

		cn = 0;
		for(yn = y1; yn != y2; yn += YOffset)
		{
			if(cn == Count)
			{
				xn += XOffset;
				cn = 0;
			}
			else
			{
				cn++;
			}
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_BLOCKED)
			{
				return FALSE;
			}
		}
	}
	else
	{
		Count = abs(x2 - x1) / abs(y2 - y1);
		if(x1 > x2)
		{
			XOffset = -1;
		}
		else
		{
			XOffset = 1;
		}
		if(y1 > y2)
		{
			YOffset = -1;
		}
		else
		{
			YOffset = 1;
		}

		cn = 0;
		for(xn = x1; xn != x2; xn += XOffset)
		{
			if(cn == Count)
			{
				yn += YOffset;
				cn = 0;
			}
			else
			{
				cn++;
			}
			if(CreatureArea[COMBATCONVERT(xn,yn)] & COMBAT_LOCATION_BLOCKED)
			{
				return FALSE;
			}
		}
	}
*/		
	return TRUE;
}


Object *Combat::GetActiveCombatant()
{
	return pActiveCombatant;
}

int Combat::SetActiveCombatant()
{
	ZSPortrait *pPortrait;
		
	if(pActiveCombatant)
	{
		((Creature *)pActiveCombatant)->SetActive(FALSE);
		pPortrait = ((Creature *)pActiveCombatant)->GetPortrait();
		if(pPortrait)
		{
			pPortrait->Dirty();
		}
	}	
	Object *pObject;
	Thing *pThing;
	Creature *pActive;

	pObject = Combatants;
	int AP;
	AP = 0;

	pActive = NULL;
	
	while(pObject)
	{
		if(pObject->GetObjectType() == OBJECT_CREATURE)
		{
			pThing = (Thing *)pObject;
			if(pThing->GetData(INDEX_ACTIONPOINTS).Value > AP && pThing->GetData(INDEX_HITPOINTS).Value > 0)
			{
				pActive = (Creature *)pThing;
				AP = pThing->GetData(INDEX_ACTIONPOINTS).Value;
			}
			((Creature *)pThing)->SetActive(FALSE);
			((Creature *)pThing)->ClearActions();
		}
		pObject = pObject->GetNextUpdate();
	}
	
	pActiveCombatant = (Thing *)pActive;

	if(!pActive)
	{
		if(!CheckForEnd())
		{
			StartRound();

			SetActiveCombatant();
		}
		
		return TRUE;
	}

	//interrupt current action
	
//	while(((Creature *)pActiveCombatant)->RemoveAction(ACTION_IDLE))
//	{
//
//	}
	
	((Creature *)pActiveCombatant)->SetActive(TRUE);

	if(PreludeParty.IsMember((Creature *)pActiveCombatant))
	{
		((Creature *)pActiveCombatant)->ClearActions();
		pPortrait = ((Creature *)pActiveCombatant)->GetPortrait();
		if(pPortrait)
		{
			pPortrait->Dirty();
		}
		PreludeWorld->LookAt((Thing *)pActiveCombatant);
	}
	else
	{
		Engine->Graphics()->SetCursor(CURSOR_WAIT);
	}

	if(pActive->GetData(INDEX_ACTIONPOINTS).Value < pActive->GetData(INDEX_ATTACKPOINTS).Value
		&&		
		pActive->GetData(INDEX_ACTIONPOINTS).Value < pActive->GetData(INDEX_MOVEPOINTS).Value)
	{
		((Creature *)pActiveCombatant)->InsertAction(ACTION_DEFEND, NULL, NULL);
	}

	FillCreatureArea();

	return TRUE;
}

int Combat::Update()
{
	//Object *pOb;
	//first update non-creature things
	///int Offset;
	
	//now update everyone on the combat list

	Creature *pThing;
	ACTION_RESULT_T Result;
	Result = ACTION_RESULT_NONE;
	pThing = (Creature *)Combatants;

	while(pThing && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pThing->SetLastResult(ACTION_RESULT_NONE);
		do
		{
			BOOL SubAction =FALSE;
			if(pThing->GetAction())
				SubAction = pThing->GetAction()->IsSub();

			pThing->Update();
			Result = pThing->GetLastResult();
			if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
			{
				if(PreludeWorld->GetGameState() == GAME_STATE_INIT)
				{
					PreludeWorld->SetGameState(GAME_STATE_NORMAL);
				}
				
			}
			else
			{
				if(((Result == ACTION_RESULT_FINISHED && !SubAction) ||
					Result == ACTION_RESULT_NOT_POSSIBLE ||
					Result == ACTION_RESULT_OUT_OF_AP) && 
					pThing == pActiveCombatant)
				{
					if(Result == ACTION_RESULT_OUT_OF_AP
						&& pThing->GetData(INDEX_ACTIONPOINTS).Value)
					{
						pThing->InsertAction(ACTION_DEFEND,NULL,NULL);
					}
					else
					{
						pThing->InsertAction(ACTION_IDLE,NULL,NULL);
					}
					SetActiveCombatant();
				}
			}
		}while(Result != ACTION_RESULT_ADVANCE_FRAME && /*
				 Result != ACTION_RESULT_WAIT_FOR_USER &&
				 Result != ACTION_RESULT_OUT_OF_AP &&
				 Result != ACTION_RESULT_FINISHED &&
				 Result != ACTION_RESULT_NOT_POSSIBLE && */
				 Result != ACTION_RESULT_REMOVE_FROM_GAME);
		
		if(Result == ACTION_RESULT_REMOVE_FROM_GAME)
		{
			Object *pToRemove;
			pToRemove = (Object *)pThing;
			pThing = (Creature *)pThing->GetNextUpdate();
			RemoveFromCombat(pToRemove);
			Valley->RemoveFromUpdate(pToRemove);
			if(pActiveCombatant == pThing)
			{
				SetActiveCombatant();
			}
			if(((Creature *)pToRemove)->GetLastPlacedTime() == -1)
			{
				delete pToRemove;
			}
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
			{
			//	SetActiveCombatant();
				this->CheckForEnd();
			}
			
		}
		else
		{
			pThing = (Creature *)pThing->GetNextUpdate();
		}
	}

	if(PreludeParty.HasChangedPosition())
	{
		PreludeParty.Occupy();
	}

	return TRUE;
}

int Combat::StartRound()
{
	CombatRound++;


	Object *pOb;
	Creature *pCreature;

	pOb = Combatants;

	while(pOb)
	{
		pCreature = (Creature *)pOb;
		if(pOb->GetObjectType() != OBJECT_CREATURE)
		{
			Object *pTemp;
			pTemp = pOb;
			pOb = pOb->GetNextUpdate();
			RemoveFromCombat(pTemp);
//			SafeExit("Non Creature in Combat");
		}
		else
		{
			pCreature->StartRound();
			pOb = pOb->GetNextUpdate();
		}
	}

	//check for ended modifires

	Modifier *pMod, *pTempMod;
	pMod = pModList;
	while(pMod)
	{
		if(this->GetRound() >= pMod->GetStart() + pMod->GetDuration())
		{
			this->RemoveMod(pMod);
			pTempMod = pMod;
			pMod = (Modifier *)pMod->GetNext();
			delete pTempMod;
		}
		else
		{
			if(pMod->GetProgression())
			{
				pMod->AdvanceFrame();
			  //pMod->SetAmount(pMod->GetAmount() + pMod->GetProgression());
			  //pMod->Apply();
			}
			pMod = (Modifier *)pMod->GetNext();
		}
	}

	if(!PreludeWorld->GetCombatHelp())
	{
		PreludeWorld->SetCombatHelp(TRUE);
		ShowHelp("Combat");
	}

	ZSWindow *pText;
	pText = new ZSText(9999,Engine->Graphics()->GetWidth()/2,Engine->Graphics()->GetHeight()/4,"Starting new Round",5);
	pText->Show();
	ZSWindow::GetMain()->AddTopChild(pText);
	return TRUE;
}

void Combat::AddToCombat(Object *pToAdd)
{
	if(pToAdd->GetObjectType() != OBJECT_CREATURE)
	{
		SafeExit("Trying to add non-creature to combat");
	}
	pToAdd->SetNextUpdate(NULL);
	pToAdd->SetPrevUpdate(NULL);

	if(Combatants)
	{
		Combatants->SetPrevUpdate(pToAdd);
	}
	
	pToAdd->SetNextUpdate(Combatants);

	Combatants = pToAdd;
	
	pToAdd->SetPrevUpdate(NULL);
	
	CombatReferenceList[NumCombatants] = pToAdd;
	pToAdd->SetData(NumCombatants);
	NumCombatants++;
}

void Combat::RemoveFromCombat(Object *pToRemove)
{
	Object *pOb;
	pOb = Combatants;
	while(pOb)
	{
		if(pOb == pToRemove)
		{ 
			break;
		}
		else
		{
			pOb = pOb->GetNextUpdate();
		}
	}
	if(!pOb)
	{
		DEBUG_INFO("Remove combatant not in combat");
		return;
	}

	if(pActiveCombatant == pToRemove)
	{
		pActiveCombatant = pActiveCombatant->GetNextUpdate();
	}
	
	if(!pToRemove->GetPrevUpdate())
	{
		//start of list
		Combatants = pToRemove->GetNextUpdate();
	}
	else
	{
		pToRemove->GetPrevUpdate()->SetNextUpdate(pToRemove->GetNextUpdate());
	}
		
	if(pToRemove->GetNextUpdate())
	{
		pToRemove->GetNextUpdate()->SetPrevUpdate(pToRemove->GetPrevUpdate());
	}

	pToRemove->SetPrevUpdate(NULL);
	pToRemove->SetNextUpdate(NULL);

	
//check for ended modifires

	Modifier *pMod, *pTempMod;
	pMod = pModList;
	while(pMod)
	{
		if(pMod->GetTarget() == pToRemove)
		{
			this->RemoveMod(pMod);
			pTempMod = pMod;
			pMod = (Modifier *)pMod->GetNext();
			delete pTempMod;
		}
		else
		{
			pMod = (Modifier *)pMod->GetNext();
		}
	}

}
	

int Combat::End()
{
	Active = FALSE;
	Object *pOb;
	Creature *pCreature;

	//remove any combat modifiers
	Modifier *pMod;
	pMod = pModList;
	while(pMod)
	{
		pModList = (Modifier *)pMod->GetNext();
		pMod->Remove();
		delete pMod;
		pMod = pModList;
	}
	
	PreludeWorld->SetGameState(GAME_STATE_NORMAL);

	while(Combatants)
	{
		pOb = Combatants;
		pCreature = (Creature *)pOb;
		RemoveFromCombat(Combatants);
		
		if(pCreature->GetLastPlacedTime() == -1)
		{
			delete pCreature;
		}
		else
		if(pCreature->GetData(INDEX_HITPOINTS).Value <= 0)
		{
			if(pCreature->GetData(INDEX_BATTLESIDE).Value && !PreludeParty.IsMember(pCreature))
			{
				pCreature->InsertAction(ACTION_DIE,NULL,NULL,NULL);
			}
			else
			{
				//people on our side get to recove at the end of combat if they're not dead
				pCreature->SetData(INDEX_HITPOINTS,1);
				pCreature->SetData(INDEX_BATTLESTATUS, CREATURE_STATE_NONE);
			//if unconscious they should at this point stand up
				pCreature->ClearActions();
				pCreature->StartRound();
				pCreature->Update();
				Valley->AddToUpdate(pOb);
			}
		}
		else
		{
			pCreature->ClearActions();
			pCreature->StartRound();
			pCreature->InsertAction(ACTION_ANIMATE,NULL,(void *)EXIT_CROUCH,FALSE);
			Valley->AddToUpdate(pOb);
		}
	}
	
	
	if(ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		PreludeEvents.DoEndCombat();
		PreludeEvents.RunEvent(49);
		for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			if(PreludeParty.GetMember(n)->GetData(INDEX_MEDICAL).Value)
			{
				PreludeParty.GetMember(n)->ImproveSkill(INDEX_MEDICAL);
			}
		}

		Engine->Sound()->PlayMusic("god");
	}
	return TRUE;
}

int Combat::Kill()
{
	Active = FALSE;
	Object *pOb;
	Creature *pCreature;

	//remove any combat modifiers
	Modifier *pMod;
	pMod = pModList;
	while(pMod)
	{
		pModList = (Modifier *)pMod->GetNext();
		pMod->Remove();
		delete pMod;
		pMod = pModList;
	}
	
	while(Combatants)
	{
		pOb = Combatants;
		pCreature = (Creature *)pOb;
		RemoveFromCombat(Combatants);
	}
	
	return TRUE;

}
	

//StartCombat
//Start the Combat;
int Combat::Start()
{
	Active = TRUE;
	CombatRound = 0;
	pTexture = Engine->GetTexture("movearea");

	ZSMainWindow *pMain;
	pMain = (ZSMainWindow *)ZSWindow::GetMain();
	
	pMain->SetArcing(FALSE);
	pMain->SetZoom(FALSE);
	pMain->SetRotate(FALSE);
	pMain->SetScrolling(FALSE);


	//check everyone in 
	//call any events synced to start of combat
	Engine->Sound()->PlayMusic("combat");
	NumCombatants = 0;

	FillCombatArea();

	//set state to combat
	PreludeWorld->SetGameState(GAME_STATE_COMBAT);

	ZeroMemory(CombatReferenceList, MAX_COMBATANTS * sizeof(Object *));
	ZeroMemory(EnemyReferenceList,	MAX_COMBATANTS * sizeof(Object *));
	ZeroMemory(EnemiesInRangeList,	MAX_COMBATANTS * sizeof(Object *));

	CombatRound = 0;
	pActiveCombatant = NULL;
	Combatants = NULL;
	pModList = NULL;

	int xn; 
	int yn;
	int Offset;
	int cx, cy;
					
	Object *pOb;
	
	Object *pCreature;
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * Valley->UpdateWidth;
	BOOL PartyOutOfBounds = FALSE;

	for (int n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		pCreature = PreludeParty.GetMember(n);
		cx = (int)pCreature->GetPosition()->x;
		cy = (int)pCreature->GetPosition()->y;
		if(cx > rCombat.left && cy > rCombat.top && cx < rCombat.right - 1 && cy < rCombat.bottom -1)
		{
		
		}
		else
		{
			PartyOutOfBounds = TRUE;
		}
	}

	if(PartyOutOfBounds)
	{
		int tx;
		int ty;
		tx = (int)PreludeParty.GetLeader()->GetPosition()->x;
		ty = (int)PreludeParty.GetLeader()->GetPosition()->y;
	
		PreludeParty.Teleport(tx, ty);
	
		Message("The party gathers for combat,","Ok");
	}




	//update everything no in combat, adding to combat if necessary
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			Offset = xn + yn * Valley->UpdateWidth;
			pOb = Valley->UpdateSegments[Offset];

			while(pOb)
			{
				if(pOb->GetObjectType() == OBJECT_CREATURE)
				{
					pCreature = pOb;

					cx = (int)pCreature->GetPosition()->x;
					cy = (int)pCreature->GetPosition()->y;
					if(cx > rCombat.left && cy > rCombat.top && cx < rCombat.right - 1 && cy < rCombat.bottom -1)
					{
						pOb = pOb->GetNextUpdate();
						Valley->RemoveFromUpdate(pCreature);
						this->AddToCombat(pCreature);
						((Creature *)pCreature)->ClearActions();
						((Creature *)pCreature)->EnterCombat();
						((Creature *)pCreature)->StartRound();
					}
					else
					{
						pOb = pOb->GetNextUpdate();
					}
				}
				else
				{
					pOb = pOb->GetNextUpdate();
				}
			}
	
		}
	}
	
	

	PreludeEvents.DoStartCombat();
	StartRound();
	SetActiveCombatant();

	return TRUE;
}

BOOL Combat::CheckForEnd()
{
	//first check to see if the party is dead
	int n;
	int NumUp = 0;
	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		if(PreludeParty.GetMember(n)->GetData(INDEX_HITPOINTS).Value > 0)
		{
			NumUp++;
		}
	}

	if(!NumUp)
	{
		End();
		//create death screen
		DeathWin *pDeathWin;
		pDeathWin = new DeathWin;
		ZSWindow::GetMain()->AddTopChild(pDeathWin);
		pDeathWin->Show();

		pDeathWin->GoModal();
		
		ZSWindow::GetMain()->RemoveChild(pDeathWin);
		
		//death screen go modal
		return TRUE;
	}
	
	Object *pOb;
	pOb = Combatants;
	NumUp = 0;
	while(pOb)
	{
		if(!PreludeParty.IsMember((Creature *)pOb) 
			&& ((Creature *)pOb)->GetData(INDEX_BATTLESIDE).Value 
			&& ((Creature *)pOb)->GetData(INDEX_HITPOINTS).Value > 0)
		{
			NumUp++;
		}
		pOb = pOb->GetNextUpdate();
	}

	if(!NumUp)
	{
		//victory!
		End();
		return TRUE;
	}

	//check distances
	Creature *pMember;
	Creature *pCreature;

	int LX, LY; //members coordinates;
	int CX, CY; //creatures coordinates;
	Path TempPath;

	int CreatureMove;
	int PlayerMove;
	int MoveTest;

	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		pMember = PreludeParty.GetMember(n);
		PlayerMove = 4 * pMember->GetData(INDEX_MAXACTIONPOINTS).Value /
					 pMember->GetData(INDEX_MOVEPOINTS).Value;

		pOb = Combatants;
		LX = pMember->GetPosition()->x;
		LY = pMember->GetPosition()->y;
		while(pOb)
		{

			if(!PreludeParty.IsMember((Creature *)pOb) 
				&& ((Creature *)pOb)->GetData(INDEX_BATTLESIDE).Value 
				&& ((Creature *)pOb)->GetData(INDEX_HITPOINTS).Value > 0)
			{
				CreatureMove = 4 * ((Creature *)pOb)->GetData(INDEX_MAXACTIONPOINTS).Value /
								((Creature *)pOb)->GetData(INDEX_MOVEPOINTS).Value;
				if(PlayerMove > CreatureMove)
				{
					MoveTest = CreatureMove;
				}
				else
				{
					MoveTest = PlayerMove;
				}

				if(MoveTest < 16)
				{
					MoveTest = 16;
				}

				if(MoveTest < (int)pMember->GetData(INDEX_RANGE).fValue)
				{
					MoveTest = (int)pMember->GetData(INDEX_RANGE).fValue;
				}

				CX = pOb->GetPosition()->x;
				CY = pOb->GetPosition()->y;
				TempPath.SetDepth(2000);
				TempPath.FindCombatPath(LX,LY, CX, CY, 5.0f,pMember);
				if(TempPath.GetLength() <= MoveTest)
				{
					return FALSE;
				}
			}
			pOb = pOb->GetNextUpdate();
		}
	}

	End();

	return TRUE;
}

Object *Combat::FindNearestOpponent(Object *pFrom)
{
//first check enemies in range
	float fDist;
	Object *pCurEnemy = NULL;
	fDist = 500.0f;
	for(int n = 0; n < EnemiesInRange; n++)
	{	
		if(EnemyDistance[n] < fDist)
		{
			pCurEnemy = EnemiesInRangeList[n];
			fDist = EnemyDistance[n];
		}
	}

	if(pCurEnemy) return pCurEnemy;

	//now check all enemies;
	int ShortestPath = 900;
	int CurPath;
	
	for(n = 0; n < NumCombatants; n++)
	{
		if(EnemyReferenceList[n])
		{
			CurPath = PathToAttackLength[n];
			if(CurPath < ShortestPath)
			{
				ShortestPath = CurPath;
				pCurEnemy = CombatReferenceList[n];
			}
		}
	}

/*
	int FromSide = ((Thing *)pFrom)->GetData(INDEX_BATTLESIDE).Value;

	Thing *pThing;

	Thing *pClosest = NULL;

	float ClosestDistance = 500.0f;

	float NewDistance;

	Object *pOb;
	pOb = Combatants;

	while(pOb)
	{
		pThing = (Thing *)pOb;

		if(pThing->GetData(INDEX_BATTLESIDE).Value != FromSide &&
			pThing->GetData(INDEX_HITPOINTS).Value > 0 )
		{
			NewDistance = GetDistance(pThing->GetPosition(),pFrom->GetPosition());

			if(NewDistance < ClosestDistance)
			{
				pClosest = pThing;
				ClosestDistance = NewDistance;
			}
		}
		pOb = pOb->GetNextUpdate();
	}
*/
	return pCurEnemy;
}

Object *Combat::FindNearestLiveOpponent(Object *pFrom)
{

	//first check enemies in range
	float fDist;
	Object *pCurEnemy = NULL;
	fDist = 500.0f;
	for(int n = 0; n < EnemiesInRange; n++)
	{	
		if(EnemyDistance[n] < fDist)
		{
			pCurEnemy = EnemiesInRangeList[n];
			fDist = EnemyDistance[n];
		}
	}

	if(pCurEnemy) return pCurEnemy;

	//now check all enemies;
	int ShortestPath = 900;
	int CurPath;
	
	for(n = 0; n < NumCombatants; n++)
	{
		if(EnemyReferenceList[n])
		{
			if(((Creature *)CombatReferenceList[n])->GetData(INDEX_HITPOINTS).Value > 0)
			{
				CurPath = PathToAttackLength[n];
				if(CurPath < ShortestPath)
				{
					ShortestPath = CurPath;
					pCurEnemy = CombatReferenceList[n];
				}
			}
		}
	}

	return pCurEnemy;

/*
	int FromSide = ((Thing *)pFrom)->GetData(INDEX_BATTLESIDE).Value;

	Thing *pThing;

	Thing *pClosest = NULL;

	float ClosestDistance = 500.0f;

	float NewDistance;

	Object *pOb;
	pOb = Combatants;

	while(pOb)
	{
		pThing = (Thing *)pOb;

		if(pThing->GetData(INDEX_BATTLESIDE).Value != FromSide &&
			pThing->GetData(INDEX_HITPOINTS).Value > 0 )
		{
			NewDistance = GetDistance(pThing->GetPosition(),pFrom->GetPosition());

			if(NewDistance < ClosestDistance)
			{
				pClosest = pThing;
				ClosestDistance = NewDistance;
			}
		}
		pOb = pOb->GetNextUpdate();
	}

	return pClosest;
*/
}

Combat::~Combat()
{
	delete[] CombatReferenceList;
	//delete[] CombatArea;
	//delete[] CreatureMove;
	//delete[] CreatureArea;
	delete[] MoveDrawList;
	delete[] MoveVerts;
	
	delete[] EnemyReferenceList;
	delete[] PathToAttackLength;
	delete[] PathAttackX;
	delete[] PathAttackY;

	delete[] EnemiesInRangeList;
	delete[] EnemyDistance;

}
Combat::Combat()
{
	Active = FALSE;
	pTexture = NULL;
	CombatRound = 0;
	Combatants = NULL;
	pActiveCombatant = NULL;
	pModList = NULL;
	NumCombatants = 0;
	pModList = NULL;
	NumEnemies = 0;
	EnemiesInRange = 0;
	
	matMove.specular.r = 1.0f;
	matMove.specular.g = 1.0f;
	matMove.specular.b = 1.0f;
	matMove.specular.a = 0.5f;

	matMove.emissive.r = 0.0f;
	matMove.emissive.g = 0.0f;
	matMove.emissive.b = 0.0f;
	matMove.emissive.a = 0.0f;

	matMove.power = 1.0f;
	matMove.ambient.r = matMove.diffuse.r = 1.0f;
	matMove.ambient.g = matMove.diffuse.g = 1.0f;
	matMove.ambient.b = matMove.diffuse.b = 1.0f;
	matMove.ambient.a = matMove.diffuse.a = 0.5f;

	fAlpha = 0.32f;
	fAlphaMod = -0.020f;


	CombatReferenceList = new Object *[MAX_COMBATANTS];
	//CombatArea= new unsigned short[COMBAT_WIDTH * COMBAT_HEIGHT];
	//CreatureArea = new unsigned short[COMBAT_WIDTH * COMBAT_HEIGHT];
	//CreatureMove = new char[COMBAT_WIDTH * COMBAT_HEIGHT];
	MoveDrawList = new unsigned short[6 * COMBAT_WIDTH * COMBAT_HEIGHT];
	MoveVerts = new D3DVERTEX[4 * COMBAT_WIDTH * COMBAT_HEIGHT];
	

	NumMoveVerts = 0;

	for(int n = 0; n < 4 * COMBAT_WIDTH * COMBAT_HEIGHT; n+= 4)
	{
		MoveVerts[n].tu = 0.0f;
		MoveVerts[n].tu = 0.0f;
		MoveVerts[n + 1].tu = 1.0f;
		MoveVerts[n + 1].tv = 0.0f;
		MoveVerts[n + 2].tu = 0.0f;
		MoveVerts[n + 2].tv = 1.0f;
		MoveVerts[n + 3].tu = 1.0f;
		MoveVerts[n + 3].tv = 1.0f;

		MoveVerts[n].nx = 0.0f;
		MoveVerts[n].ny = 0.0f;
		MoveVerts[n].nz = 1.0f;
		
		MoveVerts[n+1].nx = 0.0f;
		MoveVerts[n+1].ny = 0.0f;
		MoveVerts[n+1].nz = 1.0f;

		
		MoveVerts[n+2].nx = 0.0f;
		MoveVerts[n+2].ny = 0.0f;
		MoveVerts[n+2].nz = 1.0f;

		MoveVerts[n+3].nx = 0.0f;
		MoveVerts[n+3].ny = 0.0f;
		MoveVerts[n+3].nz = 1.0f;

/*		MoveVerts[n].color = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n].specular = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 1].color = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 1].specular = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 2].color = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 2].specular = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 3].color = D3DRGB(1.0f,1.0f,1.0f);
		MoveVerts[n + 3].specular = D3DRGB(1.0f,1.0f,1.0f);
*/
	}
	
	
	EnemyReferenceList= new BOOL[MAX_COMBATANTS];
	PathToAttackLength = new int[MAX_COMBATANTS];
	PathAttackX= new int[MAX_COMBATANTS];
	PathAttackY = new int[MAX_COMBATANTS];

	EnemiesInRangeList= new Object *[256];
	EnemyDistance = new float[256];


}

void Combat::Draw()
{
	Object *pOb;

	pOb = Combatants;

	while(pOb)
	{
		pOb->Draw();
//		pOb->Shadow();
		pOb = pOb->GetNextUpdate();
	}

}

void Combat::DrawAltHighlight()
{
	Object *pOb;

	pOb = Combatants;

	while(pOb)
	{
		pOb->AltHighLight();;
//		pOb->Shadow();
		pOb = pOb->GetNextUpdate();
	}

}

void Combat::AddMod(Modifier *pMod)
{
	pMod->SetNext(pModList);
	pMod->SetStart(this->GetRound());
	pMod->SetCombat(TRUE);
	pModList = pMod;
}

void Combat::RemoveMod(Modifier *pMod)
{
	Modifier *pM, *pLM;
	pM = pModList;
	if(pM == pMod)
	{
		pMod->Remove();
		pModList = (Modifier *)pModList->GetNext();
		return;
	}
	while(pM)
	{
		pLM = pM;
		pM = (Modifier *)pM->GetNext();
		if(pM == pMod)
		{
			pLM->SetNext(pM->GetNext());
			pM->Remove();
			return;
		}
	}
	return;
}

void Combat::PreAttack()
{
	Creature *pActive;
	pActive = (Creature *)pActiveCombatant;
	int AP;
	AP = pActive->GetData(INDEX_ACTIONPOINTS).Value;
	int MP;
	MP = pActive->GetData(INDEX_MOVEPOINTS).Value;
	int AttackP;
	AttackP = pActive->GetData(INDEX_MOVEPOINTS).Value;

	int NumAttacks;
	NumAttacks = AP / AttackP;

	int AvailableToMove;
	AvailableToMove = AP % AttackP;

	if(AvailableToMove > MP)
	{
		//we can move and then attack or attack and then move
	}

	//check our intelligence
	//calculate who we have the best odds of attacking
	//coordinate attacks when possible, ie. have everyone attack same person
	int OddsOfHit;
	int BestOdds;
	




}

void Combat::PostAttack()
{

}

void Combat::ChooseTarget()
{

}

void Combat::ChooseAction()
{
	Creature *pActive;
	pActive = (Creature *)pActiveCombatant;
	int AP;
	AP = pActive->GetData(INDEX_ACTIONPOINTS).Value;
	int MP;
	MP = pActive->GetData(INDEX_MOVEPOINTS).Value;
	int AttackP;
	AttackP = pActive->GetData(INDEX_MOVEPOINTS).Value;

	if(AttackP >= AP)
	{
		PreAttack();
	}
	else
	{
		PostAttack();
	}
}

void Combat::CastSpell()
{

}