//********************************************************************* 
//*                                                                                                                                    ** 
//**************				defs.h          ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/16                                                                                          * 
//*Revisor:     mat williams                                          * 
//*Purpose:     macros and typedefs used throughout Prelude           * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*														              * 
//********************************************************************* 
//*********************************************************************
#ifndef DEFS_H
#define DEFS_H

#define D3D_OVERLOADS

#include <stdio.h>
#include <d3d.h>

#define DEBUG_FILE_NAME		"debug.txt"

#define DELIMIT_CHARACTER				'@'

//macros for error stuff
#ifndef NDEBUG
	#define FATAL_ERROR(message)	{ MessageBox(NULL, message, "Fatal Error", MB_OK | MB_ICONSTOP); exit(1); }
	#define GOT_HERE(message)		{ MessageBox(NULL, message, "Got Here", MB_OK); }
	#define DEBUG_INFO(message)	{	FILE *fp; \
												fp = fopen(DEBUG_FILE_NAME,"a+t"); \
												fprintf(fp, message); \
												fclose(fp); }
	#define INIT_DEBUG() {	FILE *fp; \
									fp = fopen(DEBUG_FILE_NAME,"wt"); \
									if(!fp) \
									{ \
										FATAL_ERROR("Failed to create debug file."); \
									} \
									fclose(fp); }
#endif

#ifdef NDEBUG
	#define FATAL_ERROR(message)
	#define GOT_HERE(message)
	#define DEBUG_INFO(message)
	#define INIT_DEBUG()
#endif

//3d math stuff
#define DEGRAD_CONSTANT   0.01744f
#define RADDEG_CONSTANT	57.32484f
#define RadToDeg(x) (x*RADDEG_CONSTANT)
#define DegToRad(x) (x*DEGRAD_CONSTANT)


const float PI       =  3.14159265358979323846f; // Pi
const float PI_MUL_2 =  6.28318530717958623200f; // 2 * Pi
const float PI_DIV_2 =  1.57079632679489655800f; // Pi / 2
const float PI_DIV_4 =  0.78539816339744827900f; // Pi / 4
const float INV_PI   =  0.31830988618379069122f; // 1 / Pi
const float DEGTORAD =  0.01745329251994329547f; // Degrees to Radians
const float RADTODEG = 57.29577951308232286465f; // Radians to Degrees
const float TOLERANCE = 0.01f;
const float DIRECTIONANGLES[9] = {	0,
												0,
												PI_DIV_4,
												PI_DIV_2,
												PI_DIV_2 + PI_DIV_4,
												PI,
												PI + PI_DIV_4,
												PI + PI_DIV_2,
												PI_MUL_2 - PI_DIV_4,
											};
								
const D3DCOLOR COLOR_SELECT = D3DRGB(1.0f,1.0f,1.0f);
const D3DCOLOR COLOR_NORMAL = D3DRGB(0.5f,0.5f,1.0f);
const D3DCOLOR COLOR_LASSO = D3DRGB(0.5f,0.5f,0.0f);


//#define ReportError(message) { MessageBox(NULL, message, "Got Here", MB_OK); }
									
//the 8 directions
typedef enum 
{
	DIR_NONE = 0,
   NORTH,
   NORTHEAST,
   EAST,
   SOUTHEAST,
   SOUTH,
   SOUTHWEST,
   WEST,
   NORTHWEST,
	N = 1,
   NE,
   E,
   SE,
   S,
   SW,
   W,
   NW
} DIRECTION_T;

//direction masks for use in blocking data
#define	N_MASK		1 
#define  NE_MASK		2
#define  E_MASK		4
#define  SE_MASK		8
#define  S_MASK		16
#define  SW_MASK		32
#define  W_MASK		64
#define  NW_MASK		128
#define  EMPTY_MASK	255

//solidity information
typedef enum
{
   PASSABLE = 0,
   EMPTY = 0,      //passable or empty is same thing
   MISSILES_ONLY,  //you can throw and see through but can't walk through
   NOPASS,		   //nothing can pass
} SOLIDITY_T;

//types of things
typedef enum 
{
   NONE = 0,
   CREATURE, 
   SWITCH,
   ITEM,
   EVENT,
   SPELL
} THING_T; 

//a data container used for all things
typedef union
{
	int Value;
	float fValue;
	char *String;
	D3DVECTOR *pVector;
} DATA_FIELD_T;

//types of data in a container
typedef enum
{
	DATA_ERROR = -1,
	DATA_NONE = 0,
	DATA_INT,
	DATA_FLOAT,
	DATA_VECTOR,
	DATA_STRING
} DATA_T;

//types of damage
typedef enum
{
	DAMAGE_NORMAL = 0,
	DAMAGE_FIRE,
	DAMAGE_WATER,
	DAMAGE_AIR,
	DAMAGE_EARTH,
	DAMAGE_DARK,
	DAMAGE_MENTAL,
	DAMAGE_LIGHT,
	DAMAGE_POISON
} DAMAGE_T;
	
//types of attacks
typedef enum
{
	ATTACK_NORMAL = 0,
	ATTACK_RAPID,
	ATTACK_STRONG,
	ATTACK_AIMED,
	ATTACK_FEINT
} ATTACK_T;

//types of weapons
typedef enum
{
	WEAPON_TYPE_SWORD,
	WEAPON_TYPE_DAGGER,
	WEAPON_TYPE_AXE,
	WEAPON_TYPE_BLUNT,
	WEAPON_TYPE_POLEARM,
	WEAPON_TYPE_MISSILE,
	WEAPON_TYPE_UNARMED,
	WEAPON_TYPE_THROWN
} WEAPON_T;

//game states
typedef enum
{
	GAME_STATE_NORMAL,
	GAME_STATE_INIT,
	GAME_STATE_COMBAT,
	GAME_STATE_TEXT,
	GAME_STATE_EDIT,
	GAME_STATE_EXIT
} GAME_STATE_T;

//no blocking information
#define BLOCK_NONE		0

//everything is blocked by whatever's in this tile
#define BLOCK_ALL			1

//only direct movement is blocked
#define BLOCK_PARTIAL	2

//entire or direct movement is blocked
#define BLOCK_ANY			3

//miscellaneous blocking factors
#define DRAW_FLIPPED		4
#define BLOCK_CREATURE	8
#define BLOCK_WALL		16
#define BLOCK_ITEM		32
#define BLOCK_EVENT		64
#define BLOCK_PORTAL		128

//main game window controls
#define IDC_MAIN_OPTIONS			-606
#define IDC_MAIN_SAVE				-608
#define IDC_MAIN_LOAD				-610
#define IDC_MAIN_MAP				-612
#define IDC_MAIN_REST				-614
#define IDC_MAIN_CHARACTER			-616
#define IDC_MAIN_BARTER				-618
#define IDC_MAIN_ACTION				-620
#define IDC_MAIN_TALK				-622
#define IDC_MAIN_PICKPOCKET			-624
#define IDC_MAIN_WORLD_EDIT			-626
#define IDC_MAIN_MESH_EDIT			-628
#define IDC_MAIN_CREATE_PARTY		-630
#define IDC_MAIN_CREATE_CHARACTER	-632
#define IDC_MAIN_CONFIGURE			-634
#define IDC_MAIN_PEOPLE_EDIT		-636	
#define IDC_MAIN_SCRIPT_WINDOW		-638
#define IDC_MAIN_MENUBAR			-640
#define IDC_MAIN_DEATH				-642
#define IDC_MAIN_COMPASS			-644
#define IDC_MAIN_JOURNAL			-646
#define IDC_MAIN_START				-648
#define IDC_MAIN_REGION_EDIT		-650

//maximum number of spells in the game
//seems low, but each spell may have an unlimitted number of variants, most have 2-4
#define MAX_SPELLS	48

extern char *ExitErrorMessage;

#endif

