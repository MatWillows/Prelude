#ifndef ZSSOUND_H
#define ZSSOUND_H

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <wave.h>
#include "BASS.h"
#include <stdio.h>


#define SOUND_INI_FILENAME	"sound.ini"
#define MAX_SOUND_NAME		48

class SoundEffect
{
private:
	HSAMPLE		hSample;
	char Name[MAX_SOUND_NAME];
	

public:

	void Load(const char *Filename);
	void Play();
	void Stop();
	void SetVolume(long lVol);
	char *GetName() { return Name; }

	SoundEffect()
	{
		hSample = NULL;
		Name[0] = '\0';
	}

	~SoundEffect()
	{
		if(hSample)
		{
			BASS_SampleFree(hSample);
			hSample = NULL;
		}
	}

};

class MusicSuite
{
public:
	int NumFiles;
	int PatternData[64][16];
	int PatternLength;
	char Name[32];
	int CurSegment;
	void Load(FILE *fp);

};

class ZSSoundSystem
{
private:

	//DirectSound
	SoundEffect *FX;
	MusicSuite *Music;

	BOOL FXOn;
	BOOL MusicOn;

	int FXVolume;
	int MusicVolume;
	int MasterVolume;

	int NumFX;
	int NumMusic;

	int MusicPlaying;
	HANDLE hmusic;

	HSTREAM  hMp3Stream;
	HCHANNEL hMp3Channel;

public:

	ZSSoundSystem()
	{ 
		hmusic = NULL;
		FX = NULL;
		Music = NULL;
		FXOn = TRUE;
		MusicOn = FALSE;
		FXVolume = 10;
		MusicVolume = 10;
		MasterVolume = 10;
		NumFX = 0;
		NumMusic = 0;
		MusicPlaying = 0;
		hMp3Stream = 0;
		hMp3Channel = 0;

	}

	int PlayMusic(int n);
	int PlayMusic(const char *SuiteName);

	int PlayEffect(int n);
	int PlayEffect(const char *EffectName);

	int Init(HWND hWindow);
	void ShutDown();

	BOOL MusicIsOn() { return MusicOn; }
	BOOL FxAreOn() { return FXOn; }

	void SetMusic(BOOL OnOff);
	void SetFX(BOOL OnOff);

	int GetFXVolume() { return FXVolume; }
	int GetMusicVolume() { return MusicVolume; }
	int GetMasterVolume() { return MasterVolume; }
	void SetFXVolume(int NewVolume); 
	void SetMusicVolume(int NewVolume);
	void SetMasterVolume(int NewVolume);

	void StartMusic(const char *area);
	void StopMusic();
	void PauseMusic();
	void UnPauseMusic();

	MusicSuite *GetSuite(int n);
	MusicSuite *GetSuite(const char *SuiteName);

	int GetMusicPlaying() { return MusicPlaying; }

	void Update();

};


#endif