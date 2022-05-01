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
	
	DWORD format = D3DX_TEXTURE_NOMIPMAP; 
	D3DX_SURFACEFORMAT sf = D3DX_SF_A1R5G5B5 ;
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
	D3DXCheckTextureRequirements(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf);

 	hr = D3DXCreateTextureFromFile(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface,&NumMaps,fname, D3DX_FT_POINT);

// 	hr = D3DXCreateTextureFromFile(D3DDevice,NULL,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&Surface, &NumMaps, fname, D3DX_FT_POINT);

	if(hr != S_OK)
	{
		DEBUG_INFO(fname);
		return;
	}
	
	BOOL NeedColorKey = FALSE;
	if(sf != D3DX_SF_A1R5G5B5)
	{
		DEBUG_INFO("unable to get 16bit w/alpha");
		NeedColorKey = TRUE;
	}
	

	Width = ww;
	Height = hh;

	WORD* SurfPtr;  
	WORD AlphaMask;
	WORD RGBMask;
	if(!*Mask)
	{
		ZeroMemory( &Desc, sizeof( Desc ));  
		Desc.dwSize = sizeof( Desc );  
 
		Surface->Lock( NULL, &Desc, DDLOCK_WAIT, NULL );  
 
		AlphaMask = ( WORD )Desc.ddpfPixelFormat.dwRGBAlphaBitMask;  
		RGBMask = ( WORD )(Desc.ddpfPixelFormat.dwRBitMask | Desc.ddpfPixelFormat.dwGBitMask | Desc.ddpfPixelFormat.dwBBitMask );  
 
		SurfPtr = ( WORD* ) (( BYTE* )Desc.lpSurface);

		*Mask = *SurfPtr;
		*Mask &= RGBMask;
 		Surface->Unlock( NULL );
	}
	else
	{
		ZeroMemory( &Desc, sizeof( Desc ));  
		Desc.dwSize = sizeof( Desc );  
		Surface->Lock( NULL, &Desc, DDLOCK_WAIT, NULL );  
 
		AlphaMask = ( WORD )Desc.ddpfPixelFormat.dwRGBAlphaBitMask;  
		RGBMask = ( WORD )(Desc.ddpfPixelFormat.dwRBitMask | Desc.ddpfPixelFormat.dwGBitMask | Desc.ddpfPixelFormat.dwBBitMask );  
 
		Surface->Unlock( NULL );
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
		
	fp = fopen(OverLayName,"rb");
	if(fp)
	{
		fclose(fp);
		LPDIRECTDRAWSURFACE7 AlphaSurface;
		hr = D3DXCreateTextureFromFile(D3DDevice,&format,(LPDWORD)&ww,(LPDWORD)&hh,&sf,NULL,&AlphaSurface, &NumMaps, OverLayName, D3DX_FT_LINEAR);
		DDSURFACEDESC2 Desc2;
		Desc2.dwSize = sizeof( Desc2 );  

		assert(hr == S_OK);
		AlphaSurface->Lock(NULL, &Desc2, DDLOCK_WAIT, NULL);
		WORD *AlphaPtr;

		
		Surface->Lock(NULL,&Desc, DDLOCK_WAIT, NULL);
		if(!NeedColorKey)
		{
			for( DWORD y = 0; y < Desc.dwHeight; y++ )  
			{  
				SurfPtr = ( WORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  
				AlphaPtr =( WORD* )(( BYTE* )Desc2.lpSurface + y * Desc2.lPitch );  
				
				for( DWORD x = 0; x < Desc.dwWidth; x++ )  
				{  
					if((*AlphaPtr & RGBMask))
					{
						*SurfPtr |= AlphaMask; //&= RGBMask;//AlphaMask;
					}
					else  
					{
						*SurfPtr &= RGBMask; //|= AlphaMask;  
					}
					SurfPtr++;  
					AlphaPtr++;
				}  
			}
		}
		else
		{
			for( DWORD y = 0; y < Desc.dwHeight; y++ )  
			{  
				SurfPtr = ( WORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  
				AlphaPtr =( WORD* )(( BYTE* )Desc2.lpSurface + y * Desc2.lPitch );  
				
				for( DWORD x = 0; x < Desc.dwWidth; x++ )  
				{  
					if((*AlphaPtr & RGBMask))
					{
						*SurfPtr |= AlphaMask; //&= RGBMask;//AlphaMask;
					}
					else  
					{
						*SurfPtr &= RGBMask; //|= AlphaMask;  
					}
					SurfPtr++;  
					AlphaPtr++;
				}  
			}
		}

		Surface->Unlock(NULL);
		AlphaSurface->Unlock(NULL);
		AlphaSurface->Release();
	}
	else
	{
		Surface->Lock(NULL,&Desc, DDLOCK_WAIT, NULL);
		for( DWORD y = 0; y < Desc.dwHeight; y++ )  
		{  
			SurfPtr = ( WORD* )(( BYTE* )Desc.lpSurface + y * Desc.lPitch );  

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
