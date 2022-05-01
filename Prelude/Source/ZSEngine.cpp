//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSEngine.cpp           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Sept 25, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*		This file provides implementation of ZSEngine class methods
//*
//*********************************************************************
//*Outstanding issues:                     
//*		need better mesh/texture management including adding removing
//*		meshes and textures after game start up                                            
//*********************************************************************
//*********************************************************************
//4. added re-import routines

#include "ZSEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
//#include <extensions.h>
//#include <commctrl.h>
#include <assert.h>
#include "ZSutilities.h"

#define TEXTURE_FILE "textures.txt"
#define MESH_FILE	"meshes.txt"


//global engine
ZSEngine *Engine = NULL;

//constructor
ZSEngine::ZSEngine() 
{
	NumMesh = 0;
	
	memset(MeshList,0,sizeof(ZSModelEx *) * MAX_MESH);

	NumTextures = 0;
	TextureList = NULL;

}


//destructor
BOOL ZSEngine::Init(HINSTANCE hInstance) 
{
	DEBUG_INFO("Beginning Engine Init\n");

	GetCurrentDirectory(256,RootDirectory);
	DEBUG_INFO("Working director: ")
	DEBUG_INFO(RootDirectory);
	DEBUG_INFO("\n");

	HWND WinHandle;

	WinHandle = ZSGraphics.Init(hInstance);

	ZSInput.Init(WinHandle, hInstance, ZSGraphics.IsWindowed());

	FILE *fp;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"SOUND");

	ZSSound.Init(WinHandle);

	if(GetInt(fp))
	{
		ZSSound.SetFX(TRUE);
	}
	else
	{
		ZSSound.SetFX(FALSE);
	}

	SeekTo(fp,"MUSIC");
	if(GetInt(fp))
	{
		ZSSound.SetMusic(TRUE);
	}
	else
	{
		ZSSound.SetMusic(FALSE);
	}
	fclose(fp);

	DEBUG_INFO("Engine Init Successful\n");
	return TRUE;
}


ZSEngine::~ZSEngine() 
{
	DEBUG_INFO("Shutting down engine\n");

	if(NumMesh)
	{
		for(int n = 0; n < NumMesh; n++)
		{
			if(MeshList[n])
			{
				delete MeshList[n];
				MeshList[n] = NULL;
			}
		}
		NumMesh = 0;
		DEBUG_INFO("Meshes deleted\n");
	}

	if(TextureList)
	{
		delete[] TextureList;
		TextureList = NULL;
		NumTextures = 0;
		DEBUG_INFO("Textures delete\n");
	}

	ZSSound.ShutDown();

	ZSInput.ShutDown();

	ZSGraphics.ShutDown();
	DEBUG_INFO("Engine Shut Down Successful\n");
}

void ZSEngine::ReportError(char * text) 
{
	DEBUG_INFO("Engine Problems: ");
	DEBUG_INFO(text);
	DEBUG_INFO("\n");
}


void ZSEngine::ReportError(HRESULT ddrval)
{
	switch(ddrval)
	{
	case DD_OK:
		DEBUG_INFO("ok?\n");
		break;
	case DDERR_EXCEPTION:
		DEBUG_INFO("exception\n");
		break;
	case DDERR_GENERIC:
		DEBUG_INFO("generic\n");
		break;  
	case DDERR_INVALIDOBJECT  :
		DEBUG_INFO("invalid ob\n");
		break;
	case DDERR_INVALIDPARAMS  :
		DEBUG_INFO("invalid param\n");
		break;
	case DDERR_INVALIDRECT  :
		DEBUG_INFO("invalid rect\n");
		break;
	case DDERR_NOBLTHW  :
		DEBUG_INFO("noblthw\n");
		break;
	case DDERR_SURFACEBUSY  :
		DEBUG_INFO("surface busy\n");
		break;
	case DDERR_SURFACELOST  :
		DEBUG_INFO("surface lost\n");
		break;
	case DDERR_UNSUPPORTED:
		DEBUG_INFO("unsupported\n");
		break; 
	case DDERR_WASSTILLDRAWING :
		DEBUG_INFO("was still drawing\n");
		break;
	case DDERR_BLTFASTCANTCLIP :
		DEBUG_INFO("blt fast cant clip\n");
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		DEBUG_INFO("Conflictig textuer filter\n");
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		DEBUG_INFO("Conflictig textuer Palette.\n");
		break;
	case D3DERR_TOOMANYOPERATIONS:
		DEBUG_INFO("Too Many operations.\n");
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		DEBUG_INFO("un alpha arg.\n");
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		DEBUG_INFO("un alpha op.\n");
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		DEBUG_INFO("un texture filter.\n");
		break;
	case D3DERR_WRONGTEXTUREFORMAT:
		DEBUG_INFO("wrong texture format.\n");
		break;

	default:
		DEBUG_INFO("unspecified error code\n");
		break;
	}
}



ZSModelEx *ZSEngine::GetMesh(const char *pMeshName)
{
	for(int n = 0; n < NumMesh; n++)
	{
		if(MeshList[n] && !strcmp(pMeshName,MeshList[n]->GetName()))
		{
			return MeshList[n];
		}
	}
//	DEBUG_INFO("Failed to find mesh named: ");
//	DEBUG_INFO(pMeshName);
//	DEBUG_INFO("\n\n");

	return NULL;
}


int ZSEngine::GetMeshNum(const char *pMeshName)
{
	for(int n = 0; n < NumMesh; n++)
	{
		if(!strcmp(pMeshName, MeshList[n]->GetName()))
		{
			return n;
		}
	}

	return 0;
}

int ZSEngine::GetMeshNum(ZSModelEx *pMesh)
{
	for(int n = 0; n < NumMesh; n++)
	{
		if(MeshList[n] == pMesh)
		{
			return n;
		}
	}

	return 0;
}


void ZSEngine::LoadMeshes()
{
	DEBUG_INFO("Loading Meshes\n");
	int n;

	if(!NumTextures)
	{
		DEBUG_INFO("Attempting to load meshes before loading textures!\n");
	}

	if(NumMesh)
	{
		DEBUG_INFO("Destroying current mesh list\n");
		for(n = 0; n < MAX_MESH; n++)
		{
			if(MeshList[n])
			{
				delete MeshList[n];
				MeshList[n] = NULL;
			}
		}
	}

	char meshdir[256];


	//get the maindirectory
	strcpy(meshdir, RootDirectory);
	strcat(meshdir, "\\meshes");

	//switch to the texture directory
	if(!SetCurrentDirectory(meshdir))
	{
		SafeExit("Mesh directory does not exist!\n");
	}

	FILE *fp;
	
	fp = SafeFileOpen(MESH_FILE,"rt");
	if(!fp)
	{
		SafeExit("Mesh file does not exist!\n");
	}
	
	//if it does exist load in the Meshes as specified in the file

	//read in the number of Meshes
	fseek(fp,0,0);
	NumMesh = GetInt(fp);
	
	char blarg[32];
	char BinFileName[64];
	char ModFileName[64];
	int tn; 
	int zpercent;
	int ypercent;
	int xpercent;
	
	for(n = 0; n < NumMesh; n++)
	{
		MeshList[n] = new ZSModelEx;
		//get the meshes file name (.mod)
		GetString(fp,MeshList[n]->filename);

		//create the binary file name
		strcpy(BinFileName,MeshList[n]->filename);
		strcat(BinFileName,".bin");
		FILE *binfp;

		//check for a binary version of the mesh
		binfp = SafeFileOpen(BinFileName,"rb");

		if(binfp)
		{
			MeshList[n]->LoadEx(binfp);
			fclose(binfp);
			binfp = NULL;
			GetString(fp,blarg);
			GetInt(fp); //x scale
			GetInt(fp);	//y scale
			GetInt(fp);	//z scale
			GetInt(fp);	//min
			GetInt(fp);	//max
		}
		else
		{
			sprintf(ModFileName,"%s.mod",MeshList[n]->filename);
			//if no binary version exists, import the .mod file
			if(!MeshList[n]->Import(ModFileName))
			{
				DEBUG_INFO(ModFileName);
				GetString(fp,blarg);
				GetInt(fp); //x scale
				GetInt(fp);	//y scale
				GetInt(fp);	//z scale
				GetInt(fp);	//min
				GetInt(fp);	//max
			}
			else
			{
				//assigne the mesh a default texture
				GetString(fp,blarg);
				for(tn = 0; tn < NumTextures; tn++)
				{
					if(!strcmp(TextureList[tn].GetName(),blarg))
					{
						//MeshList[n]->pTexture = &TextureList[tn];
						break;
					}
				}
				//get the scaling and random range assignment values
				xpercent = GetInt(fp);
				ypercent = GetInt(fp);
				zpercent = GetInt(fp);
				MeshList[n]->xscale = (float)xpercent/100.0f;
				MeshList[n]->yscale = (float)ypercent/100.0f;
				MeshList[n]->zscale = (float)zpercent/100.0f;
				MeshList[n]->rangemin = GetInt(fp);
				MeshList[n]->rangemax = GetInt(fp);
				MeshList[n]->Scale(MeshList[n]->xscale,MeshList[n]->yscale,MeshList[n]->zscale);
				MeshList[n]->GetBounds();
				MeshList[n]->Move(0.0f,0.0f,-MeshList[n]->bottombound);
				MeshList[n]->Center();
				
				//save a binary version so we don't have to go through the import time again
				MeshList[n]->SaveEx(BinFileName);
			}
		}

	}
	
	fclose(fp);

	//return to the main directory
	SetCurrentDirectory(RootDirectory);
	return;
}


void ZSEngine::LoadMeshes(const char *FileName)
{
	DEBUG_INFO("Loading meshes from: ");
	DEBUG_INFO(FileName);
	DEBUG_INFO("\n\n");
	FILE *fp;

	fp = SafeFileOpen(FileName,"rb");

	fread(&NumMesh, sizeof(NumMesh),1,fp);

	int n;
	
	for(n = 0; n < NumMesh; n++)
	{
		MeshList[n] = new ZSModelEx;
		MeshList[n]->LoadEx(fp);
	}

	fclose(fp);

	DEBUG_INFO("Done Loading Meshes\n\n");
}

void ZSEngine::SaveMeshes(const char *FileName)
{
	DEBUG_INFO("Saving meshes to: ");
	DEBUG_INFO(FileName);
	DEBUG_INFO("\n");
	FILE *fp;

	fp = SafeFileOpen(FileName,"wb");

	if(!fp)
	{
		return;
	}

	fwrite(&NumMesh, sizeof(NumMesh),1,fp);

	int n;

	for(n = 0; n < NumMesh; n++)
	{
		MeshList[n]->SaveEx(fp);
	}

	fclose(fp);

	DEBUG_INFO("Done saving Meshes\n\n");
}

ZSTexture *ZSEngine::GetTexture(const char *pTextureName)
{
	char blarg[32];

	strcpy(blarg,pTextureName);
	strcat(blarg,".bmp");

	for(int n = 0; n < NumTextures; n++)
	{
		if(!strcmp(blarg,TextureList[n].GetName()))
		{
			return &TextureList[n];
		}
	}
//	DEBUG_INFO("Failed to find texture named: ");
//	DEBUG_INFO(pTextureName);
//	DEBUG_INFO("\n\n");

	return NULL;
}

int ZSEngine::GetTextureNum(const char *pTextureName)
{
	char blarg[32];

	strcpy(blarg,pTextureName);
	strcat(blarg,".bmp");

	for(int n = 0; n < NumTextures; n++)
	{
		if(!strcmp(blarg,TextureList[n].GetName()))
		{
			return n;
		}
	}

	return 0;
}

int ZSEngine::GetTextureNum(ZSTexture *pTexture)
{
	for(int n = 0; n < NumTextures; n++)
	{
		if(&TextureList[n] == pTexture)
		{
			return n;
		}
	}

	return 0;
}



void ZSEngine::LoadTextures()
{
//	DEBUG_INFO("Loading Textures\n");

	if(NumTextures)
	{
		delete[] TextureList;
	}

	char Texturedir[256];
	int n;
	//get the maindirectory
	strcpy(Texturedir, RootDirectory);
	strcat(Texturedir, "\\Textures");

//	DEBUG_INFO("switching to texture directory\n");

	//switch to the Texture directory
	if(!SetCurrentDirectory(Texturedir))
	{
		SafeExit("Texture directory does not exist");
	}

	FILE *fp;
	//check to see if Texture.txt exists
	
	fp = SafeFileOpen(TEXTURE_FILE,"rt");
	if(!fp)
	{
		//Texture file does not exist
		SafeExit("Texture file does not exist!");
	}
	
	//if it does exist load in the Textures as specified in the file
	//then append any new Textures onto the end of the Texture list

	//read in the number of Textures
	fseek(fp,0,0);

	NumTextures = GetInt(fp);
	TextureList = new ZSTexture[NumTextures];


	for(n = 0; n < NumTextures; n++)
	{
		GetString(fp,TextureList[n].filename);
		TextureList[n].Width = GetInt(fp);
		TextureList[n].Height = GetInt(fp);
		TextureList[n].Load(TextureList[n].filename, ZSGraphics.GetD3D(), ZSGraphics.GetDirectDraw(),&ZSGraphics.KeyMask,TextureList[n].Width,TextureList[n].Height);
	}
	
	fclose(fp);

//	DEBUG_INFO("returning to main directory\n");

	SetCurrentDirectory(RootDirectory);

//	DEBUG_INFO("Done Loading Textures\n\n");

	return;
}

void ZSEngine::ImportMeshes()
{
//	DEBUG_INFO("Loading Meshes\n");
	int n;

	if(!NumTextures)
	{
//		DEBUG_INFO("Attempting to load meshes before loading textures!\n");
	}

	char meshdir[256];

	//get the maindirectory
	strcpy(meshdir, RootDirectory);
	strcat(meshdir, "\\meshes");

	//switch to the texture directory
	if(!SetCurrentDirectory(meshdir))
	{
		SafeExit("Mesh directory does not exist!\n");
	}

	FILE *fp;
	
	fp = SafeFileOpen(MESH_FILE,"rt");
	
	//if it does exist load in the Meshes as specified in the file

	//read in the number of Meshes
	fseek(fp,0,0);
	NumMesh = GetInt(fp);
	
	FILE *binfp;
	char blarg[32];
	char BinFileName[64];
	char ModFileName[64];
	int zpercent;
	int ypercent;
	int xpercent;
	
	for(n = 0; n < NumMesh; n++)
	{
		if(MeshList[n])
		{
			GetString(fp,MeshList[n]->filename);
			GetString(fp,blarg);
			GetInt(fp); //x scale
			GetInt(fp);	//y scale
			GetInt(fp);	//z scale
			GetInt(fp);	//min
			GetInt(fp);	//max
		}
		else
		{
			MeshList[n] = new ZSModelEx;
			GetString(fp,MeshList[n]->filename);
			sprintf(ModFileName,"%s.mod",MeshList[n]->filename);
			sprintf(BinFileName,"%s.bin",MeshList[n]->filename);
			binfp = SafeFileOpen(BinFileName,"rb");
			if(binfp)
			{
				fclose(binfp);
				MeshList[n]->LoadEx(BinFileName);
				GetString(fp,blarg); //texture
				GetInt(fp); //x scale
				GetInt(fp);	//y scale
				GetInt(fp);	//z scale
				GetInt(fp);	//min
				GetInt(fp);	//max
			}
			else
			//if no binaryversion exists, import the .mod file
			if(!MeshList[n]->Import(ModFileName))
			{
				MeshList[n]->LoadEx("bar.bin");
				GetString(fp,blarg); //texture
				GetInt(fp); //x scale
				GetInt(fp);	//y scale
				GetInt(fp);	//z scale
				GetInt(fp);	//min
				GetInt(fp);	//max
				//delete MeshList[n];
				//MeshList[n] = NULL;
			}
			else
			{
				//assigne the mesh a default texture
				GetString(fp,blarg);
				MeshList[n]->pTexture = GetTexture(blarg);
				
				//get the scaling and random range assignment values
				xpercent = GetInt(fp);
				ypercent = GetInt(fp);
				zpercent = GetInt(fp);
				MeshList[n]->xscale = (float)xpercent/100.0f;
				MeshList[n]->yscale = (float)ypercent/100.0f;
				MeshList[n]->zscale = (float)zpercent/100.0f;
				MeshList[n]->rangemin = GetInt(fp);
				MeshList[n]->rangemax = GetInt(fp);
				MeshList[n]->Scale(MeshList[n]->xscale,MeshList[n]->yscale,MeshList[n]->zscale);
				MeshList[n]->GetBounds();
				MeshList[n]->Move(0.0f, 0.0f, -MeshList[n]->bottombound);
				MeshList[n]->Center();
				MeshList[n]->SaveEx(BinFileName);
			}			
			//save a binary version so we don't have to go through the import time again
		}
	}
	
	fclose(fp);

	//return to the main directory
	SetCurrentDirectory(RootDirectory);
	return;
}

void ZSEngine::DeleteMesh(int Num)
{
//	DEBUG_INFO("Deleting Mesh\n");

	char meshdir[256];

	//get the maindirectory
	strcpy(meshdir, RootDirectory);
	strcat(meshdir, "\\meshes");

	//switch to the texture directory
	if(!SetCurrentDirectory(meshdir))
	{
		SafeExit("Mesh directory does not exist!\n");
	}

	//if it does exist load in the Meshes as specified in the file

	//read in the number of Meshes
	delete MeshList[Num];
	MeshList[Num] = new ZSModelEx;

	//import the .mod file
	if(!MeshList[Num]->Import("bar.mod"))
	{
		SafeExit("Failed to Delete Mesh\n");
	}

	//assign the mesh a default texture
	//get the scaling and random range assignment values
	MeshList[Num]->GetBounds();
	MeshList[Num]->Center();
	
	//return to the main directory
	SetCurrentDirectory(RootDirectory);

	return;
}

void ZSEngine::Import(int Num)
{
	
//	DEBUG_INFO("ReImporting Mesh\n");
	int n;

	char meshdir[256];

	//get the maindirectory
	strcpy(meshdir, RootDirectory);
	strcat(meshdir, "\\meshes");

	//switch to the texture directory
	if(!SetCurrentDirectory(meshdir))
	{
		SafeExit("Mesh directory does not exist!\n");
	}

	//if it does exist load in the Meshes as specified in the file

	//read in the number of Meshes
	char *DotP;
	int zpercent;
	int ypercent;
	int xpercent;

	delete MeshList[Num];
	MeshList[Num] = new ZSModelEx;

	//import the .mod file
	char FileName[64];
	FILE *mfp;
	mfp = SafeFileOpen("meshes.txt","rt");
	for(n = 0; n < Num; n++)
	{
		SeekTo(mfp,"\"");
		SeekTo(mfp,"\"");	
		SeekTo(mfp,"\"");
		SeekTo(mfp,"\"");
	}
	GetString(mfp,FileName);
	strcpy(MeshList[Num]->filename,FileName);
	char blarg[64];
	GetString(mfp,blarg);
	MeshList[Num]->pTexture = GetTexture(blarg);
	
	//get the scaling and random range assignment values
	xpercent = GetInt(mfp);
	ypercent = GetInt(mfp);
	zpercent = GetInt(mfp);
	MeshList[Num]->xscale = (float)xpercent/100.0f;
	MeshList[Num]->yscale = (float)ypercent/100.0f;
	MeshList[Num]->zscale = (float)zpercent/100.0f;
	MeshList[Num]->rangemin = GetInt(mfp);
	MeshList[Num]->rangemax = GetInt(mfp);

	
	
	fclose(mfp);

	DotP = strchr(FileName,'.');
	if(DotP)
	{
		strcpy(DotP,".mod");
	}
	else
	{
		strcat(FileName,".mod");
	}

	if(!MeshList[Num]->Import(FileName))
	{
		DEBUG_INFO("Failed to Import Mesh\n");
	//	DEBUG_INFO(FileName);
	//	DEBUG_INFO("\n");
		SafeExit("Mesh import failed");
	}

	//assign the mesh a default texture
	//get the scaling and random range assignment values
	MeshList[Num]->Scale(MeshList[Num]->xscale, MeshList[Num]->xscale, MeshList[Num]->xscale);
	MeshList[Num]->GetBounds();
	MeshList[Num]->Move(0.0f,0.0f,-MeshList[Num]->bottombound);
	MeshList[Num]->Center();
	
	//return to the main directory
	SetCurrentDirectory(RootDirectory);

	return;
}

