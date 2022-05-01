#include "ZSsound.h"
#include "ZSEngine.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include "zsutilities.h"
#include "wavread.h"
#include "file_input.h"
#include "file_output.h"
#include "audio_output.h"
#include "decoder.h"
#include "mpeg_codec.h"
#include "xaudio.h"
#include "world.h"

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//globals for mp3 streaming
HSTREAM hStream;
HSTREAM hLastStream;
DWORD dwLatency;
DWORD dwStreamLength;
DWORD dwStartNextByte;
float fStreamTime;
float fStartNextTime;
float fLatency;
BOOL CreateNextStream = FALSE;
DWORD musicthreadID = 101;
HANDLE hmtxNextStream = 0;
HANDLE hMusicThread = 0;


void SoundEffect::SetVolume(long lVol)
{
	return;
}

void SoundEffect::Load(const char *Filename)
{
	strcpy(Name, Filename);

	hSample = BASS_SampleLoad(FALSE,Name,0,0,2,0);

	if(!hSample)
	{
		DWORD dwError;
		dwError = BASS_ErrorGetCode();
		switch(dwError)
		{
			case BASS_ERROR_INIT:
				SafeExit("BASS_Init has not been successfully called."); 
				break;
			case BASS_ERROR_FILEOPEN: 
				SafeExit("The file could not be opened."); 
				break;
			case BASS_ERROR_FORMAT: 
				SafeExit("The file's format is not recognised/supported."); 
				break;
			case BASS_ERROR_ILLPARAM: 
				SafeExit("max is invalid.");
				break;
			case BASS_ERROR_MEM: 
				SafeExit("There is insufficent memory. ");
				break;
			case BASS_ERROR_NO3D: 
				SafeExit("Couldn't initialize 3D support for the sample.");
				break;
			case BASS_ERROR_UNKNOWN:
				break;
		}

		SafeExit("Could not load Sound Sample");
	}
	
}

void SoundEffect::Play()
{
	BASS_SamplePlay(hSample);
}

void SoundEffect::Stop()
{
	BASS_SampleStop(hSample);
}

int ZSSoundSystem::PlayMusic(int n)
{
	if(!MusicOn) 
		return FALSE;
	
	StartMusic(Music[n].Name);

	return TRUE;
}

int ZSSoundSystem::PlayMusic(const char *SuiteName)
{
	
	StartMusic(SuiteName);

	return TRUE;
}

int ZSSoundSystem::PlayEffect(int n)
{
	if(!NumFX)
		return FALSE;
	assert(n < NumFX && n >= 0);
	

	if(FXOn)
		FX[n].Play();

	return TRUE;
}

int ZSSoundSystem::PlayEffect(const char *EffectName)
{
	if(!FXOn) return FALSE;
	
	if(!NumFX) return FALSE;

	for(int n = 0; n < NumFX; n++)
	{
		if(!strcmp(FX[n].GetName(),EffectName))
		{
			return PlayEffect(n);
		}
	}

	assert(n < NumFX);
	
	return FALSE;
}

int ZSSoundSystem::Init(HWND hWindow)
{
	hmusic = 0;
	DEBUG_INFO("Begin Init Sound\n");

	/* Initialize digital sound - default device, 44100hz, stereo, 16 bits, syncs used */
	if (!BASS_Init(-1, 44100, MAKELONG(BASS_DEVICE_LATENCY, 25), 0)) 
	{ 
		SafeExit("Could Not start BASS");	
	}

	BASS_INFO BI;

	BASS_Start();

	BI.size = sizeof(BASS_INFO);


	BASS_GetInfo(&BI);

	dwLatency = BI.latency;

	fLatency = (float)dwLatency / 1000.0f;

	fLatency *= 2.0f;

	char Root[256];
	char New[256];

	GetCurrentDirectory(256,Root);

	strcpy(New, Root);

	strcat(New,"\\Sound\\");

	SetCurrentDirectory(New);

	//open the soundfx file
	
	int sIndex = 0;

	FILE *fp;

	fp = SafeFileOpen(SOUND_INI_FILENAME, "rt");

	SeekTo(fp,"Number:");

	NumFX = GetInt(fp);

	if(NumFX)
	{
		FX = new SoundEffect[NumFX];
	}
	else
	{
		DEBUG_INFO("No Sounds\n");
		SetCurrentDirectory(Root);
		return TRUE;
	}

	char *SoundFileName;

	for(int n = 0; n < NumFX; n++)
	{
		SeekTo(fp,"File:");
		SoundFileName = GetStringNoWhite(fp);
		FX[n].Load(SoundFileName);
	}

	SeekTo(fp,"NumSuites:");
	NumMusic = GetInt(fp);
	Music = new MusicSuite[NumMusic];

	for(n = 0; n < NumMusic; n++)
	{
		Music[n].Load(fp);
	}

	fclose(fp);

	hmtxNextStream = CreateMutex( 
    NULL,                       // no security attributes
    FALSE,                      // initially not owned
    "NextStreamMutex"); 

	SetCurrentDirectory(Root);

	return TRUE;
}

void ZSSoundSystem::ShutDown()
{
	StopMusic();

	ReleaseMutex(hmtxNextStream);


	if(FX)
	{
		DEBUG_INFO("Deleting FX\n");
		delete[] FX;
	}

	if(MusicSuite)
	{
		DEBUG_INFO("Deleting Music\n");
		delete[] Music;
	}

	BASS_Stop();

	BASS_Free();

	DEBUG_INFO("Sound Shut Down Done\n");
}

void MusicSuite::Load(FILE *fp)
{
	SeekTo(fp,"Suite:");
	char *Blarg;
	Blarg = GetStringNoWhite(fp);
	strcpy(Name,Blarg);
	delete[] Blarg;
	SeekTo(fp,"NumFiles:");
	NumFiles = GetInt(fp);
	SeekTo(fp,"PatternLength:");
	PatternLength = GetInt(fp);
	for(int n = 0; n < PatternLength; n++)
	{
		SeekTo(fp,"Segment:");
		PatternData[n][0] = GetInt(fp);
		for(int sn = 0; sn < PatternData[n][0]; sn++)
		{
			PatternData[n][sn+1] = GetInt(fp);
		}
	}
	CurSegment = 0;
}


void CALLBACK MyProc(HSYNC handle, DWORD channel, DWORD data, DWORD user)
{
	ResumeThread(hMusicThread);

	/*
	HRESULT dwWaitResult;
	dwWaitResult = WaitForSingleObject( 
        hmtxNextStream,   // handle to mutex
        1000L);   // five-second time-out interval
 
    switch (dwWaitResult) 
    {
        // The thread got mutex ownership.
        case WAIT_OBJECT_0: 
            __try { 

					CreateNextStream = TRUE;
         				// Write to the database.
				} 

            __finally { 
                // Release ownership of the mutex object.
                if (! ReleaseMutex(hmtxNextStream)) { 
                    // Deal with error.
                } 

            break; 
        } 

        // Cannot get mutex ownership due to time-out.
        case WAIT_TIMEOUT: 
            return; 

        // Got ownership of the abandoned mutex object.
        case WAIT_ABANDONED: 
            return; 
    }
	*/
}


long WINAPI MusicMan(LPVOID param)
{
	char FileName[256];
	

	DWORD dwSleepTime;
	DWORD dwStreamLength; 
	float fStreamTime;    

	MusicSuite *pMusic;
	pMusic = (MusicSuite *)param;
	pMusic->CurSegment = 0;

	int rn;
	int n;
	int sx;
	int sy;
	D3DVECTOR vScreen;
	char *musicname;

	n = 1;
	pMusic->CurSegment =  n - 1;

	if(pMusic->CurSegment >= pMusic->PatternLength)
	{
		pMusic->CurSegment = 0;
	}

	pMusic->CurSegment = 0;
	sprintf(FileName,"%s\\Sound\\%s%i.ogg",Engine->GetRootDirectory(),pMusic->Name,n);

	hStream = BASS_StreamCreateFile(FALSE, FileName, 0, 0, BASS_STREAM_AUTOFREE);

//	dwStreamLength = BASS_StreamGetLength(hStream);
//	fStreamTime = BASS_ChannelBytes2Seconds(hStream,dwStreamLength);

//	fStartNextTime = fStreamTime - fLatency;
//	dwStartNextByte = BASS_ChannelSeconds2Bytes(hStream, fStartNextTime);



/*	if(!BASS_ChannelSetSync(hStream, BASS_SYNC_POS, dwStartNextByte, MyProc, 0))
	{
		BASS_Stop();
		BASS_Free();
		DEBUG_INFO("could not start sound stream\n");
		exit(1);
	}	
*/
//	BASS_StreamPreBuf(hStream);

	dwStreamLength = BASS_StreamGetLength(hStream);
	fStreamTime = BASS_ChannelBytes2Seconds(hStream,dwStreamLength);

	dwSleepTime = (DWORD)((fStreamTime - fLatency) * 1000.0f);
	if(dwSleepTime < 0.0f) dwSleepTime = 0.0f;

	BASS_StreamPlay(hStream, FALSE,  NULL);

	rn = rand() % pMusic->PatternData[pMusic->CurSegment][0];
	rn++;
	n = pMusic->PatternData[pMusic->CurSegment][rn];
	pMusic->CurSegment = n - 1;

	if(pMusic->CurSegment >= pMusic->PatternLength)
	{
		pMusic->CurSegment = 0;
	}

	sprintf(FileName,"%s\\Sound\\%s%i.ogg",Engine->GetRootDirectory(),pMusic->Name,n);
	hStream = BASS_StreamCreateFile(FALSE, FileName,0,0,   BASS_STREAM_AUTOFREE);

	dwStreamLength = BASS_StreamGetLength(hStream);
	fStreamTime = BASS_ChannelBytes2Seconds(hStream,dwStreamLength);

//	fStartNextTime = fStreamTime - fLatency;
//	dwStartNextByte = BASS_ChannelSeconds2Bytes(hStream, fStartNextTime);
//
/*	if(!BASS_ChannelSetSync(hStream, BASS_SYNC_POS, dwStartNextByte, MyProc, 0))
	{
		BASS_Stop();
		BASS_Free();
		exit(1);
		DEBUG_INFO("could not start sound stream\n");
	}
*/
//	BASS_StreamPreBuf(hStream);

	Sleep(dwSleepTime);

//	SuspendThread(hMusicThread);

//	BOOL InternalNextStream = FALSE;
		
	// Main message loop:
	while (TRUE)
	{
	/*
		while(!InternalNextStream)
		{
			HRESULT dwWaitResult;
			dwWaitResult = WaitForSingleObject( 
				hmtxNextStream,   // handle to mutex
				1000L);   // one-second time-out interval
 
			switch (dwWaitResult) 
			{
				// The thread got mutex ownership.
				case WAIT_OBJECT_0: 
					__try { 
						
							if(CreateNextStream)
							{
								InternalNextStream = TRUE;
								CreateNextStream = FALSE;
							}
  							// Write to the database.
						} 

					__finally { 
						// Release ownership of the mutex object.
						if (! ReleaseMutex(hmtxNextStream)) { 
							// Deal with error.
						} 

					break; 
				} 

				// Cannot get mutex ownership due to time-out.
				case WAIT_TIMEOUT: 
					return FALSE; 

				// Got ownership of the abandoned mutex object.
				case WAIT_ABANDONED: 
					return FALSE; 
			}
		}

		InternalNextStream = FALSE;
	*/
		if(!BASS_StreamPlay(hStream, FALSE,  NULL))
		{
			DEBUG_INFO("Failed to start music stream.\n");
			return FALSE;
		}
		dwStreamLength = BASS_StreamGetLength(hStream);
		fStreamTime = BASS_ChannelBytes2Seconds(hStream,dwStreamLength);

		dwSleepTime = (DWORD)((fStreamTime - fLatency) * 1000.0f);
		if(dwSleepTime < 0.0f) dwSleepTime = 0.0f;

		if(PreludeWorld && PreludeWorld->GetGameState() == GAME_STATE_NORMAL)
		{
			vScreen = PreludeWorld->GetCenterScreen();
			sx = (int)vScreen.x;
			sy = (int)vScreen.y;
			musicname = PreludeWorld->GetMusic(sx,sy);
			MusicSuite *pNewMusic = NULL;
			if(musicname)
			{
				if(strcmp(pMusic->Name,musicname))
				{
					pNewMusic = Engine->Sound()->GetSuite(musicname);
				}				
			}
			else
			{
				if(strcmp(pMusic->Name,"god"))
				{
					pNewMusic = Engine->Sound()->GetSuite("god");
				}						
			}
			if(pNewMusic && pNewMusic != pMusic)
			{
				pMusic = pNewMusic;
				pMusic->CurSegment = 0;
				n = 1;
			}
			else
			{
				rn = rand() % pMusic->PatternData[pMusic->CurSegment][0];
				rn++;
				n = pMusic->PatternData[pMusic->CurSegment][rn];
				pMusic->CurSegment = n - 1;

				if(pMusic->CurSegment >= pMusic->PatternLength)
				{
					pMusic->CurSegment = 0;
				}
			}
		}
		else
		{
			rn = rand() % pMusic->PatternData[pMusic->CurSegment][0];
			rn++;
			n = pMusic->PatternData[pMusic->CurSegment][rn];
			pMusic->CurSegment = n - 1;

			if(pMusic->CurSegment >= pMusic->PatternLength)
			{
				pMusic->CurSegment = 0;
			}
		}

		sprintf(FileName,"%s\\Sound\\%s%i.ogg",Engine->GetRootDirectory(),pMusic->Name,n);
		
		hLastStream = hStream;

		hStream = BASS_StreamCreateFile(FALSE,FileName,0,0,  BASS_STREAM_AUTOFREE);

		if(!hStream)
		{
			DEBUG_INFO("Failed to open stream file: ");
			DEBUG_INFO(FileName);
			DEBUG_INFO("\n");
			return FALSE;
		}
	//	dwStreamLength = BASS_StreamGetLength(hStream);
	//	fStreamTime = BASS_ChannelBytes2Seconds(hStream,dwStreamLength);

	//	fStartNextTime = fStreamTime - fLatency;
	//	dwStartNextByte = BASS_ChannelSeconds2Bytes(hStream, fStartNextTime);

	/*	if(!BASS_ChannelSetSync(hStream, BASS_SYNC_POS, dwStartNextByte, MyProc, 0))
		{
			DEBUG_INFO("could not sync sound stream\n");
			return FALSE;
		}
	*/	
	//	BASS_StreamPreBuf(hStream);
		
		Sleep(dwSleepTime);

		//SuspendThread(hMusicThread);

	}

	return TRUE;
}

MusicSuite *ZSSoundSystem::GetSuite(int n)
{
	return &Music[n];
}

MusicSuite *ZSSoundSystem::GetSuite(const char *SuiteName)
{
	int n;
	for(n = 0; n < NumMusic; n++)
	{
		if(!strcmp(SuiteName,Music[n].Name))
		{
			return &Music[n];
		}
	}
	return NULL;

}



void ZSSoundSystem::StartMusic(const char *area)
{
	
	MusicSuite *CurMusic = NULL;

	int n;
	for(n = 0; n < NumMusic; n++)
	{
		if(!strcmp(area,Music[n].Name))
		{
			CurMusic = &Music[n];
			break;
		}
	}

	MusicPlaying = n;

	if(hmusic)
	{
		StopMusic();
	}

	//initialize the music thread and start it going.
	if(CurMusic)
	{
		if(!MusicOn) 
		
		return;

		hMusicThread = hmusic = CreateThread(NULL,
				0,
				(LPTHREAD_START_ROUTINE)MusicMan,
				(LPVOID)CurMusic,
				NULL, 
				&musicthreadID);
	//	SetThreadPriority(hmusic, THREAD_PRIORITY_BELOW_NORMAL);
	}
	else
	if(area)
	{
		if(!FXOn) 
			return;

		char filename[256];
		sprintf(filename,"%s\\Sound\\%s.mp3",Engine->GetRootDirectory(),area,n);
		if(hMp3Stream)
		{
			BASS_StreamFree(hMp3Stream);
		}
		hMp3Stream = BASS_StreamCreateFile(FALSE, filename, 0,0, BASS_STREAM_AUTOFREE);

		hMp3Channel = BASS_StreamPlay(hMp3Stream, FALSE, NULL);
	}
	else
	{
		DEBUG_INFO("Could not start music or MP3 track.\n");
	}
}

void ZSSoundSystem::PauseMusic()
{
	if(hmusic)
		SuspendThread(hmusic);
}


void ZSSoundSystem::UnPauseMusic()
{
	if(hmusic)
		ResumeThread(hmusic);
}

void ZSSoundSystem::StopMusic()
{
	if(hmusic)
	{
		BASS_StreamFree(hLastStream);
		TerminateThread(hmusic,0);
		hLastStream = 0;
		hMusicThread = hmusic = 0;
	}

	if(hMp3Stream)
	{
		BASS_StreamFree(hMp3Stream);
		hMp3Stream = 0;
	}
	
}


void ZSSoundSystem::SetMusic(BOOL OnOff)
{ 
	MusicOn = OnOff; 
	
	if(!MusicOn)
	{
		StopMusic();
	}


}

void ZSSoundSystem::SetFX(BOOL OnOff) 
{ 
	FXOn = OnOff; 

}

void ZSSoundSystem::SetFXVolume(int NewVolume)
{
	DWORD dwMain;
	DWORD dwSample;
	DWORD dwStream;


	BASS_GetGlobalVolumes(&dwMain,&dwSample,&dwStream);

	dwSample = NewVolume * 10;

	BASS_SetGlobalVolumes(dwMain,dwSample,dwStream);

	FXVolume = NewVolume;
}

void ZSSoundSystem::SetMusicVolume(int NewVolume)
{

	DWORD dwMain;
	DWORD dwSample;
	DWORD dwStream;

	BASS_GetGlobalVolumes(&dwMain,&dwSample,&dwStream);

	dwStream = NewVolume * 10;

	BASS_SetGlobalVolumes(dwMain,dwSample,dwStream);

	MusicVolume = NewVolume;
}

void ZSSoundSystem::SetMasterVolume(int NewVolume)
{
	DWORD dwMain;
	DWORD dwSample;
	DWORD dwStream;

	BASS_GetGlobalVolumes(&dwMain,&dwSample,&dwStream);

	dwMain = NewVolume * 10;

	BASS_SetGlobalVolumes(dwMain,dwSample,dwStream);

	MasterVolume = NewVolume;

}

void ZSSoundSystem::Update()
{
	BASS_Update();
}


