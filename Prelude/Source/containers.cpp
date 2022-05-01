#include "containers.h"

int Container::AltHighLight()
{

	return TRUE;
}

int Container::GetDefaultAction(Object *pActor)
{

	return TRUE;
}

int Container::GetValidActions(Thing *pActor, ActionMenu *pAMenu)
{
	return TRUE;
}

Container::Container()
{

}

Container::~Container()
{

}

