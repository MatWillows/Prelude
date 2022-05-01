#include "ZSTexture.h"
#include "defs.h"

#include <stdio.h>
#include <assert.h>

#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 6) + ((r & 31) << 11))

BOOL ZSTexture::Reduced = FALSE;

HRESULT CALLBACK EnumZSTextures( LPDDPIXELFORMAT DDPixelFormat, LPVOID pDDDesiredPixelFormat )  
{  
    if( DDPixelFormat->dwFlags & DDPF_ALPHAPIXELS && DDPixelFormat->dwRGBBitCount == 16 )  
    {  
        if( DDPixelFormat->dwRGBAlphaBitMask == 1 || DDPixelFormat->dwRGBAlphaBitMask == 0x8000 )  
        {  
            memcpy( pDDDesiredPixelFormat, DDPixelFormat, sizeof(DDPIXELFORMAT) );  
            return D3DENUMRET_CANCEL;  
        }  
    }  
    return D3DENUMRET_OK;  
}

void ZSTexture::Load(char * fname, LPDIRECT3DDEVICE7 D3DDevice, LPDIRECTDRAW7 DirectDraw, WORD *Mask, int w = 0, int h = 0) {
	
	DWORD format;
	
	//DWORD format = D3DX_TEXTURE_NOMIPMAP; 
	//D3DX_SURFACEFORMAT sf = D3DX_SF_A1R5G5B5 ;
	D3DX_SURFACEFORMAT sf = D3DX_SF_A8R8G8B8;
	DWORD NumMaps = 4;

	format = D3DX_DEFAULT;

	Width = D3DX_DEFAULT;
	Height = D3DX_DEFAULT;
	
	
	DDSURFACEDESC2 Desc;
	Desc.dwSize = sizeof( Desc );  

	//if a width and height were passed, set the ZSTexture width and height to be so
	if(w)
	{
		Width = w;
	}
	else
	{
		Width = D3DX_DEFAULT;
	}
	if(h)
	{
		Height = h;
	}
	else
	{
		Height = D3DX_DEFAULT;
	}

	DWORD hh,ww;
	HRESULT hr;
	hh = Height;
	ww = Width;
	if(Reduced)
	{
		hh /= 2;
		ww /= 2;
	}
	
	
	D3DXCheckTextureRequirements(D3DDevice, NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf);

	hr = D3DXCreateTextureFromFile(D3DDevice, NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface,&NumMaps,fname, D3DX_FT_LINEAR);

//	D3DXCheckTextureRequirements(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf);

//	hr = D3DXCreateTextureFromFile(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface,&NumMaps,fname, D3DX_FT_POINT);

// 	hr = D3DXCreateTextureFromFile(D3DDevice,NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface, &NumMaps, fname, D3DX_FT_POINT);

	if(hr != S_OK)
	{
		DEBUG_INFO(fname);
		return;
	}
	
	BOOL NeedColorKey = FALSE;
//	if(sf != D3DX_SF_A1R5G5B5)
//	{
//		DEBUG_INFO("unable to get 16bit w/alpha");
//		NeedColorKey = TRUE;
//	}
	

	Width = ww;
	Height = hh;

	DWORD* SurfPtr;  
	DWORD AlphaMask;
	DWORD RGBMask;
	DWORD BaseColor;
	DWORD AlphaColor;
	DWORD NewColor;
	DWORD AlphaLevel;


	if(!*Mask)
	{
	/*	ZeroMemory( &Desc, sizeof( Desc ));  
		Desc.dwSize = sizeof( Desc );  
 
		Surface->Lock( NULL, &Desc, DDLOCK_WAIT, NULL );  
 
		AlphaMask = ( WORD )Desc.ddpfPixelFormat.dwRGBAlphaBitMask;  
		RGBMask = ( WORD )(Desc.ddpfPixelFormat.dwRBitMask | Desc.ddpfPixelFormat.dwGBitMask | Desc.ddpfPixelFormat.dwBBitMask );  
 
		SurfPtr = ( DWORD* ) (( BYTE* )Desc.lpSurface);

		*Mask = *SurfPtr;
		*Mask &= RGBMask;
 		Surface->Unlock( NULL );
	*/
	}
	else
	{
		/*
		ZeroMemory( &Desc, sizeof( Desc ));  
		Desc.dwSize = sizeof( Desc );  
		Surface->Lock( NULL, &Desc, DDLOCK_WAIT, NULL );  
 
		AlphaMask = ( DWORD )Desc.ddpfPixelFormat.dwRGBAlphaBitMask;  
		RGBMask = ( DWORD )(Desc.ddpfPixelFormat.dwRBitMask | Desc.ddpfPixelFormat.dwGBitMask | Desc.ddpfPixelFormat.dwBBitMask );  
 
		Surface->Unlock( NULL );
		*/
	}

	AlphaMask = 0xFFFF;
	AlphaMask ^= RGBMask;
	DDCOLORKEY ddck;
	ddck.dwColorSpaceHighValue = *Mask;
	ddck.dwColorSpaceLowValue = *Mask;

	FILE *fp;
	char OverLayName[48];
	strcpy(OverLayName,fname);
	char *cp;
	cp = strchr(OverLayName,'.');
	strcpy(cp,".ovr");
	
	Surface->SetColorKey(DDCKEY_SRCBLT , &ddck);

	LPDIRECTDRAWSURFACE7 pAlphaLevel, pNextAlphaLevel, pSourceLevel, pNextSourceLevel; 
		
	fp = fopen(OverLayName,"rb");
	if(fp)
	{
		fclose(fp);
		
		DWORD *AlphaPtr;
		
		LPDIRECTDRAWSURFACE7 AlphaSurface;
		hr = D3DXCreateTextureFromFile(D3DDevice, NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&AlphaSurface, &NumMaps, OverLayName, D3DX_FT_LINEAR);
	
		DDSURFACEDESC2 Desc2;
		ZeroMemory( &Desc2, sizeof( Desc2 ));  
		Desc2.dwSize = sizeof( Desc2 );  
		ZeroMemory( &Desc, sizeof( Desc ));  
		Desc.dwSize = sizeof( Desc );  
		assert(hr == S_OK);
		
		DDSCAPS2 ddsCaps; 
		HRESULT ddres1,ddres2;
		ZeroMemory( &ddsCaps, sizeof( ddsCaps ));  
		
		pAlphaLevel = AlphaSurface; 
		pAlphaLevel->AddRef(); 

		pSourceLevel = Surface;
		pSourceLevel->AddRef();
	
		ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP; 
		ddres1 = ddres2 = DD_OK; 

		DWORD sr, sg, sb, sa;
		int NumMips = 0;

		while (ddres1 == DD_OK && ddres2 == DD_OK) 
		{ 
				NumMips++;
		pAlphaLevel->Lock(NULL, &Desc2, DDLOCK_WAIT, NULL);
		pSourceLevel->Lock(NULL, &Desc, DDLOCK_WAIT, NULL);
		
		if(!NeedColorKey)
		{
			for( DWORD y = 0; y < Desc.dwHeight; y++ )  
			{  
				SurfPtr = ( DWORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  
				AlphaPtr =( DWORD* )(( BYTE* )Desc2.lpSurface + y * Desc2.lPitch );  
				
				for( DWORD x = 0; x < Desc.dwWidth; x++ )  
				{  
					AlphaLevel = (RGBA_GETRED(*AlphaPtr) + RGBA_GETGREEN(*AlphaPtr) + RGBA_GETBLUE(*AlphaPtr)) / 3;
					sr = RGBA_GETRED(*SurfPtr);
					sg = RGBA_GETGREEN(*SurfPtr);
					sb = RGBA_GETBLUE(*SurfPtr);
					sa = RGBA_GETALPHA(*SurfPtr);
					
					NewColor = RGBA_MAKE(sr, sg, sb, AlphaLevel);
					*SurfPtr = NewColor;
				
					SurfPtr++;  
					AlphaPtr++;
				}  
			}
		}
		else
		{
			for( DWORD y = 0; y < Desc.dwHeight; y++ )  
			{  
				SurfPtr = ( DWORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  
				AlphaPtr =( DWORD* )(( BYTE* )Desc2.lpSurface + y * Desc2.lPitch );  
				
				for( DWORD x = 0; x < Desc.dwWidth; x++ )  
				{  
					BaseColor = *SurfPtr;
					AlphaColor = *AlphaPtr;

					AlphaLevel = (RGBA_GETRED(*AlphaPtr) + RGBA_GETGREEN(*AlphaPtr) + RGBA_GETBLUE(*AlphaPtr)) / 3;
					sr = RGBA_GETRED(*SurfPtr);
					sg = RGBA_GETGREEN(*SurfPtr);
					sb = RGBA_GETBLUE(*SurfPtr);
					sa = RGBA_GETALPHA(*SurfPtr);
					
					NewColor = RGBA_MAKE(sr, sg, sb, AlphaLevel);
					*SurfPtr = NewColor;
					
					SurfPtr++;  
					AlphaPtr++;
				}  
			}
		}

		pSourceLevel->Unlock(NULL);
		pAlphaLevel->Unlock(NULL);
		
		ddres1 = pAlphaLevel->GetAttachedSurface(&ddsCaps, &pNextAlphaLevel); 
		pAlphaLevel->Release();
		pAlphaLevel = pNextAlphaLevel; 
		
		ddres2 = pSourceLevel->GetAttachedSurface(&ddsCaps, &pNextSourceLevel); 
		pSourceLevel->Release();
		pSourceLevel = pNextSourceLevel; 
	
		} 

		if (((ddres1 != DD_OK) && (ddres1 != DDERR_NOTFOUND)) || ((ddres2 != DD_OK) && (ddres2 != DDERR_NOTFOUND))) 
		{
			char Error1[256];
			char Error2[256];
			D3DXGetErrorString(ddres1, 256, Error1);
			D3DXGetErrorString(ddres1, 256, Error2);
		}
		NumMips++;
	}
	else
	{
		/*
		Surface->Lock(NULL,&Desc, DDLOCK_WAIT, NULL);
		for( DWORD y = 0; y < Desc.dwHeight; y++ )  
		{  
			SurfPtr = ( DWORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  

			for( DWORD x = 0; x < Desc.dwWidth; x++ )  
			{  
				if(( *SurfPtr & RGBMask ) == (*Mask & RGBMask) )  
				{
					*SurfPtr = 0;//AlphaMask;//RGBMask;  
				}
				else  
				{
					*SurfPtr |= AlphaMask;  
				}
				SurfPtr++;  
			}  
		}  
		Surface->Unlock(NULL);
		*/
	}

	strcpy(filename,fname);

};

ZSTexture::~ZSTexture() 
{
	if(Surface)
	{
		Surface->Release();
		Surface = NULL;
	}
};

ZSTexture::ZSTexture()
{
	Width = 0;
	Height = 0;
	Surface = NULL;
	Used = FALSE;
	pPrev = NULL;
	pNext = NULL;

};

ZSTexture::ZSTexture(char * filename, LPDIRECT3DDEVICE7 D3DDevice, LPDIRECTDRAW7 DirectDraw, WORD *Mask, int w = 0, int h = 0) 
{	
	Used = FALSE;
	pPrev = NULL;
	pNext = NULL;
	Load(filename, D3DDevice,DirectDraw, Mask, w , h );
};

ZSTexture::ZSTexture(LPDIRECT3DDEVICE7 D3DDevice, int NewWidth, int NewHeight, WORD *Mask)
{
	Used = FALSE;
	pPrev = NULL;
	pNext = NULL;
	DWORD format = D3DX_TEXTURE_NOMIPMAP; 
	D3DX_SURFACEFORMAT sf = D3DX_SF_A1R5G5B5 ;
	DWORD NumMaps = 4;

	format = D3DX_DEFAULT;

	DWORD hh,ww;
	HRESULT hr;
	hh = NewHeight;
	ww = NewWidth;

	Width = NewWidth;
	Height = NewHeight;

	if(Reduced)
	{
		ww /= 2;
		hh /= 2;
		Width /= 2;
		Height /= 2;
	}

//	D3DXCheckTextureRequirements(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf);

	hr = D3DXCreateTexture(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface,&NumMaps);
	//hr = D3DXCreateTexture(D3DDevice,NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface,&NumMaps);
	assert(hr == S_OK);
//	assert(sf == D3DX_SF_A1R5G5B5);
//	assert(Width == ww);
//	assert(Height == hh);

	if(Mask)
	{
		DDCOLORKEY ddck;
		ddck.dwColorSpaceHighValue = *Mask;
		ddck.dwColorSpaceLowValue = *Mask;

		Surface->SetColorKey(DDCKEY_SRCBLT , &ddck);
	}

}

ZSTexture::ZSTexture(LPDIRECTDRAWSURFACE7 FromSurface, LPDIRECT3DDEVICE7 D3DDevice, int NewWidth, int NewHeight)
{
	Used = FALSE;
	pPrev = NULL;
	pNext = NULL;
	DWORD format = D3DX_TEXTURE_NOMIPMAP; 
	D3DX_SURFACEFORMAT sf = D3DX_SF_A1R5G5B5 ;
	DWORD NumMaps = 3;

	format = D3DX_DEFAULT;


	DWORD hh,ww;
	HRESULT hr;
	hh = NewHeight;
	ww = NewWidth;

	Width = NewWidth;
	Height = NewHeight;

	if(Reduced)
	{
		ww /= 2;
		hh /= 2;
		Width /= 2;
		Height /= 2;
	}

//	D3DXCheckTextureRequirements(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf);

 	hr = D3DXCreateTexture(D3DDevice, &format, (LPDWORD)&ww, (LPDWORD)&hh, &sf, NULL,&Surface,&NumMaps);
	
	hr = D3DXLoadTextureFromSurface(D3DDevice, Surface, D3DX_DEFAULT, FromSurface, NULL, NULL, D3DX_FT_POINT);

	assert(hr == S_OK);

}
