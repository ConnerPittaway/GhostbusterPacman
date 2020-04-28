#pragma once
#define MUNCHIECOUNT 240
#define GHOSTCOUNT 4
#define WALLCOUNT 768
#define ScreenWidth 29
#define ScreenHeight 29

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

//Structure Definition
struct Player
{
	float _cPacmanSpeed;
	int _pacmanCurrentFrameTime;
	int _pacmanDirection;
	int _pacmanFrame;
	Rect* _pacmanSourceRect;
	Texture2D* _pacmanTexture;
	Vector2* _pacmanPosition;
	int _FrameTime;
	float speedMultiplier;
	bool dead;
	bool ghostBuster;
	bool timerOn;
	Rect* Box;
	int _lives;
};

struct Enemy
{
	int _frameCount;
	Rect* _rect;
	Texture2D* _texture;
	Vector2* _position;
	int _cFrameTime;
	int _frame;
	int _currentFrameTime;
	bool _eaten;
};

struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;
	bool isScared;
	bool isBeingSucked;
	bool isDead;
	Rect* HitBox;
	int ghostType;
};

struct Wall
{
	Rect* _wallRect;
	Texture2D* _wallTexture;
	Vector2* _wallPosition;
	bool isWall;
};

struct Timer
{
	double storedTime;
};

struct Score
{
	string _scoreString;
	int _scoreInt;
	int _highScore;
};



// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:
	Score* _score;
	Timer* _time;
	Player* _pacman;
	Enemy** _munchies;
	Enemy** _protonPacks;
	//Enemy* _cherry;
	MovingEnemy* _ghosts[GHOSTCOUNT];
	Wall* _map[ScreenHeight][ScreenWidth];
	
	// Int
	int _munchieCount;
	int _protonCount;
	int _munchiesEaten;
	int inputDirection;


	// Position for String
	Vector2* _stringPosition;

	//Data for Game Over
	Texture2D* _gameOverBackground;
	Rect* _gameOverRectangle;
	Vector2* _gameOverStringPosition;
	bool _gameOver;

	//Data for Game Win
	Texture2D* _gameWinBackground;
	Rect* _gameWinRectangle;
	Vector2* _gameWinStringPosition;
	bool _gameWin;

	//Data for Menu
	Texture2D* _menuBackground;
	Rect* _menuRectangle;
	Vector2* _menuStringPosition;
	bool _paused;

	//Data for Start Menu
	Texture2D* _startBackground;
	Rect* _startRectangle;
	Vector2* _startStringPostion;
	bool _start;

	//Data for Score Board
	Vector2* _scoreStringPosition;

	//Data for Keys
	bool _pKeyDown;
	bool _rKeyDown;
	bool _spaceKeyDown;
	bool _gameStart; //boolean so game cannot be reverted to start menu
	bool _enterKeyDown;

	//Sound Effects
	SoundEffect* _pop;
	SoundEffect* _packStart;
	SoundEffect* _packStop;
	SoundEffect* _deathNoise;
	SoundEffect* _suck;
	SoundEffect* _theme;
	SoundEffect* _siren;
	SoundEffect* _capture;
	SoundEffect* _win;

	//Private Methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void CheckPaused(Input::KeyboardState* state);
	void CheckViewportCollision();
	void UpdatePacman(int elapsedTime);
	void UpdateMunchie(Enemy* munchie, int elapsedTime);
	void UpdateCherry(int elapsedTime);
	void RandomCherry(Input::KeyboardState* state);
	void CheckGhostCollision(MovingEnemy*);
	void CheckGhostWallCollision(MovingEnemy*, int elapsedTime);
	void CheckPacWallCollision(int elapsedTime);
	void GhostPath();
	void UpdateGhost(MovingEnemy*, int elapsedTime);
	void CheckMunchieCollision();
	void CheckPackCollision();
	void UpdateProton(Enemy* proton, int elapsedTime);
	void GhostBuster(MovingEnemy*,int elapsedTime);
	void CheckCollision();
	void ResetTimer();
	void CheckTimer(double time);
	void CheckPacGhost(MovingEnemy*);
	void CheckWin();
	void StoreScore();
	void ReadSetScore();
	void PacmanDeath(int elapsedTime);
	void ResetPositions();
	void CheckEnter(Input::KeyboardState* state);

public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[], int munchieCount, int protonAmount);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);


};