#ifndef SAVELOAD_H
#define SAVELOAD_H

#include "zswindow.h"

class GameWin : public ZSWindow
{
private:
	int GameNumber;

public:
	static LPDIRECTDRAWSURFACE7 GameWinSurface;
	int Command(int IDFrom, int Command, int Param);
	
	void SetGameNumber(int n);
	int GetGameNumber() { return GameNumber; }

	GameWin(int ID, int x, int y, int width, int height);

};

class GameListWin : public ZSWindow
{
private:
	int NumGames;
	int GamesShown;
	int TopGame;
	BOOL Save;

	void SortGames();
	void SortFiles();
public:
	
	BOOL IsSave() { return Save; }
	void SetLoad();
	void SetSave();
	
	void DeleteGame(int num);
	void AddGame(int num);

	int Command(int IDFrom, int Command, int Param);

	GameListWin(int ID, int x, int y, int width, int height, BOOL DoSave = TRUE);

};


class LoadWin : public ZSWindow
{
private:

public:
	static LPDIRECTDRAWSURFACE7 LoadWinSurface;
	int Command(int IDFrom, int Command, int Param);

	LoadWin(int ID, int x, int y, int width, int height);

};

class SaveWin : public ZSWindow
{
private:

public:
	static LPDIRECTDRAWSURFACE7 SaveWinSurface;
	int Command(int IDFrom, int Command, int Param);

	SaveWin(int ID, int x, int y, int width, int height);
};



#endif