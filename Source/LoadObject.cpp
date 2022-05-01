#include "objects.h"
#include "water.h"
#include "things.h"
#include "portals.h"
#include "items.h"
#include "creatures.h"
#include "events.h"
#include "gameitem.h"
#include "fountain.h"
#include "zsparticle.h"
#include "zsfire.h"
#include "cavewall.h"
#include "entrance.h"
#include "modifiers.h"

Object *LoadObject(FILE *fp)
{
	OBJECT_T Type;

	fread(&Type,sizeof(OBJECT_T),1,fp);

	Object *ReturnOb;
	ReturnOb = NULL;

	switch(Type)
	{
		case OBJECT_STATIC:
			ReturnOb = new Object();
			ReturnOb->Load(fp);
			return ReturnOb;
		case OBJECT_THING:
			Thing *RetThing;
			RetThing = new Thing();
			RetThing->Load(fp);
			return (Object *)RetThing;
		case OBJECT_PORTAL:
			Portal *RetPortal;
			RetPortal = new Portal();
			RetPortal->Load(fp);
			return (Object *)RetPortal;
		case OBJECT_WATER:
			Water *RetWater;
			RetWater = new Water();
			RetWater->Load(fp);
			return (Object *)RetWater;
		case OBJECT_EVENT:
			Event *RetEvent;
			RetEvent = new Event();
			RetEvent->Load(fp);
			return (Object *)RetEvent;
		case OBJECT_ITEM:
			GameItem *RetGameItem;
			RetGameItem = new GameItem;
			RetGameItem->Load(fp);
			return (Object *)RetGameItem;
		case OBJECT_CREATURE:
			Creature *RetCreature;
			RetCreature = LoadCreature(fp);
			return (Object *)RetCreature;
		case OBJECT_SWITCH:

			return NULL;
		case OBJECT_FOUNTAIN:
			Fountain *pFountain;
			pFountain = new Fountain;
			pFountain->Load(fp);
			return (Object *)pFountain;

		case OBJECT_PARTICLESYSTEM:
			ParticleSystem *pSystem;
			pSystem = new ParticleSystem;
			pSystem->Load(fp);
			return (Object *)pSystem;

		case OBJECT_FIRE:
			Fire *pFire;
			pFire = new Fire();
			pFire->Load(fp);
			return (Object *)pFire;

		case OBJECT_BARRIER:
			CaveWall *pCaveWall;
			pCaveWall = new CaveWall();
			pCaveWall->Load(fp);
			return (Object *)pCaveWall;

		case OBJECT_ENTRANCE:
			Entrance *pEntrance;
			pEntrance = new Entrance;
			pEntrance->Load(fp);
			return (Object *)pEntrance;

		case OBJECT_MODIFIER:
			Modifier *pMod;
			pMod = new Modifier;
			pMod->Load(fp);
			return (Object *)pMod;
		default:

			break;
	
	}
	return NULL;
}