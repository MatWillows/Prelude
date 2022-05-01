#ifndef COMBAT_MANAGER_H
#define COMBAT_MANAGER_H

#define COMBAT_RADIUS 20.0f
#define COMBAT_WIDTH	128	 
#define COMBAT_HEIGHT	128
#define CREATURE_RANGE 64
#define MAX_COMBATANTS 256

#include "defs.h"

class Object;
class Thing;
class Modifier;
class Path;
class ZSTexture;

#define COMBAT_LOCATION_WALKABLE		4
#define COMBAT_LOCAIOTN_ATTACKABLE		2
#define COMBAT_LOCATION_TIME_FOR_ATTACK	1
#define COMBAT_LOCATION_EMPTY			0
#define COMBAT_LOCATION_BLOCKED			8
#define COMBAT_LOCATION_OCCUPIED		16
#define COMBAT_LOCATION_THREATENED		32
#define COMBAT_LOCATION_VERY_THREATENED (32 | 64)
#define COMBAT_LOCATION_EXTREME_THREAT	(32 | 64 | 128)

class Combat
{
private:
	float TileZOffset;
	int CombatRound;
	Object *Combatants;
	Object *pActiveCombatant;
	Object **CombatReferenceList;
	int NumCombatants;
	Modifier *pModList;
	RECT rCombat;
	RECT rCombatArea;
	unsigned short CombatArea[COMBAT_WIDTH * COMBAT_HEIGHT];
	int CreatureX;
	int CreatureY;
	unsigned short CreatureArea[COMBAT_WIDTH * COMBAT_HEIGHT];
	char CreatureMove[COMBAT_WIDTH * COMBAT_HEIGHT];
	unsigned short *MoveDrawList;
	int NumMoveVerts;
	D3DVERTEX *MoveVerts;
	
	int NumEnemies;
	BOOL *EnemyReferenceList;
	int *PathToAttackLength;
	int *PathAttackX;
	int *PathAttackY;

	int EnemiesInRange;
	Object **EnemiesInRangeList;
	float *EnemyDistance;

	D3DMATERIAL7 matMove;
	float fAlpha;
	float fAlphaMod;

	ZSTexture *pTexture;

	BOOL Active;

	void ClearMods();
	
	void inline CreautureMoveTileSurround(int xn, int yn, int AttackP, int MP);

public:

	void GetCombatRect(RECT *pRect) { *pRect = rCombat; }

	void FillCombatArea();

	void FillCreatureArea();

	void DrawMoveArea();
	
	int GetMoveLeft(int xn, int yn);
	void RecordMoveLeft(int xn, int yn, int ap);

	void AddMoveTile(int xn, int yn, int AttackP);
	void AddFriendTile(int xn, int yn, int AttackP);
	void AddEnemyTile(int xn, int yn, int AttackP);

	void ClearMoveArea();
	
	void HighlightEnemies();

	void DrawThreatArea();
	
	Object *GetCombatants() { return Combatants; }

	void AddToCombat(Object *pToAdd);
	void RemoveFromCombat(Object *pToRemove);
	Object *GetActiveCombatant();

	Object *FindNearestOpponent(Object *pFrom);
	Object *FindNearestLiveOpponent(Object *pFrom);

	int Update();

	inline int GetRound()
	{
		return CombatRound;
	}

	int SetActiveCombatant();
	int StartRound();
	int Start();
	int End();
	int Kill();
	BOOL CheckForEnd();

	void Draw();
	void DrawAltHighlight();

	Combat();
	~Combat();

	void AddMod(Modifier *pMod);
	void RemoveMod(Modifier *pMod);

	void PreAttack();
	void PostAttack();
	void ChooseTarget();
	void ChooseAction();
	void CastSpell();

	BOOL InCombat() { return Active; }
	BOOL CheckLineOfSight(int x1, int y1, int x2, int y2, Object *pSource, Object *pTarget);

	friend class Area;
	friend class World;
	friend class Path;

};

#endif