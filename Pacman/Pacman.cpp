#include "Pacman.h"

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <time.h>
#include <math.h>
#include <chrono>
#include <conio.h>
#include <cstdio>
#include <ctime>

// debug only
#include <iostream>
int i, x, y;


Pacman::Pacman(int argc, char* argv[], int munchieCount, int protonAmount) : Game(argc, argv){
	//Local Variables
	_paused = false;
	_start = true;
	_gameOver = false;
	_gameWin = false;

	_munchiesEaten = 0;
	_munchieCount = munchieCount;
	_protonCount = protonAmount;

	_munchies = new Enemy*[_munchieCount];
	_protonPacks = new Enemy * [_protonCount];

	//Munchie Initialisation
	for (i = 0; i < _munchieCount; i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->_currentFrameTime = 0;
		_munchies[i]->_frameCount = rand() % 1;
		_munchies[i]->_cFrameTime = rand() % 500 + 50;
	}

	for (i = 0; i < _protonCount; i++)
	{
		_protonPacks[i] = new Enemy();
		_protonPacks[i]->_currentFrameTime = 0;
		_protonPacks[i]->_frameCount = rand() % 1;
		_protonPacks[i]->_cFrameTime = rand() % 500 + 50;
	}

	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			_map[y][x] = new Wall();
		}
	}

	//Timer Intialisation
	_time = new Timer();
	_time->storedTime = 5;

	//Score Initialisation
	_score = new Score();
	_score->_scoreInt = 0;

	//Cherry Intialisation
	/*_cherry = new Enemy();
	_cherry->_currentFrameTime = 0;
	_cherry->_frameCount = 0;
	_cherry->_cFrameTime = 500;*/

	//Intialise member variables
	_pacman = new Player();
	_pacman->_pacmanCurrentFrameTime = 0;
	_pacman->_pacmanDirection = 0;
	_pacman->_pacmanFrame = 500;
	_pacman->_cPacmanSpeed = 0.15f;
	_pacman->_FrameTime = 1000;
	_pacman->speedMultiplier = 1.0f;
	_pacman->dead = false;
	_pacman->ghostBuster = false;
	_pacman->Box = new Rect;
	_pacman->_lives = 3;

	//Ghost Initialisation
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->direction = i;
		_ghosts[i]->speed = 0.2f;
		_ghosts[i]->isScared = false;
		_ghosts[i]->isDead = false;
		_ghosts[i]->isBeingSucked = false;
		_ghosts[i]->HitBox = new Rect;
	}

	//Initialise Audio
	_pop = new SoundEffect();
	_packStart = new SoundEffect();
	_packStop = new SoundEffect();
	_deathNoise = new SoundEffect();
	_suck = new SoundEffect();
	_theme = new SoundEffect();
	_siren = new SoundEffect();
	_capture = new SoundEffect();
	_win = new SoundEffect();

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 928, 1100, false, 25, 25, "Pacman", 60);
	Input::Initialise();
	

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();


}

Pacman::~Pacman()
{
	//Clean Pointers
	delete _pacman->_pacmanTexture;
	delete _pacman->_pacmanSourceRect;
	delete _pacman->_pacmanPosition;
	//Clean Pacman Structure Pointer
	delete _pacman;

	//Clean Ghosts
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		delete _ghosts[i]->texture;
		delete _ghosts[i]->sourceRect;
		delete _ghosts[i]->position;
		delete _ghosts[i]->HitBox;
	}
	delete[] _ghosts;

	//Clean Munchies
	for (int i = 0; i < _munchieCount; i++)
	{
		delete _munchies[i]->_rect;
		delete _munchies[i]->_texture;
		delete _munchies[i]->_position;
	}
	delete[] _munchies;

	//Clean map
	for (y = 0; y < 24; y++)
	{
		for (x = 0; x < 30; x++)
		{
			delete _map[y][x]->_wallRect;
			delete _map[y][x]->_wallTexture;
			delete _map[y][x]->_wallPosition;
		}
	}
	delete[] _map;

	for (i = 0; i < _protonCount; i++)
	{
		delete _protonPacks[i]->_rect;
		delete _protonPacks[i]->_texture;
		delete _protonPacks[i]->_position;
	}
	delete[] _protonPacks;

	delete _score;

	delete _time;

	//Clean Audio
	delete _pop;
	delete _packStart;
	delete _packStop;
	delete _deathNoise;
	delete _suck;
	delete _theme;
	delete _siren;
	delete _capture;
	delete _win;
}

void Pacman::LoadContent()
{
	ReadSetScore();
	//Load Audio
	_pop->Load("Sounds/pop.wav");
	_packStart->Load("Sounds/KJH_PackstartCombo.wav");
	_packStop->Load("Sounds/PackStop.wav");
	_deathNoise->Load("Sounds/death.wav");
	_suck->Load("Sounds/Suck.wav");
	_theme->Load("Sounds/theme.wav");
	_siren->Load("Sounds/Siren.wav");
	_siren->SetLooping(true);
	_capture->Load("Sounds/capture.wav");
	_win->Load("Sounds/win.wav");

	int munchieAmount = 0, protonPackAmount = 0;

	// Load Pacman
	_pacman->_pacmanTexture = new Texture2D();
	_pacman ->_pacmanTexture->Load("Textures/Pacman2.png", false);
	_pacman ->_pacmanPosition = new Vector2(704.0f, 448.0f);
	_pacman -> _pacmanSourceRect = new Rect(0.0f, 0.0f, 20, 20);
	
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures / Munchie.png", true);
	
	string temp;
	char mapInput[ScreenHeight][ScreenWidth];
	int r = 0, v = 0;

	char ch;
	fstream fin("Map4.txt", fstream::in);

	for (int y = 0; y < 29; y++) {
		for (int x = 0; x < 29; x++) {
			char ch = fin.get();
			if (ch != '\n') mapInput[y][x] = ch;
		}
	}

	//Load Map
	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			_map[y][x]->_wallTexture = new Texture2D();
			if (mapInput[y][x] == '+')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidCorner1.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '^')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidCorner2.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '<')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidCorner4.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '>')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidCorner3.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '/')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidWall1.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '|')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidWall1.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '-')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidWall.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == '_')
			{
				_map[y][x]->_wallTexture->Load("Textures/MidWall.png", true);
				_map[y][x]->isWall = true;
			}
			else if (mapInput[y][x] == 'X')
			{
				_map[y][x]->_wallTexture->Load("Textures/Invi.png", true);
				_map[y][x]->isWall = true;
			}
			else
			{
				_map[y][x]->_wallTexture->Load("Textures/Invi.png", true);
				_map[y][x]->isWall = false;
			}
			_map[y][x]->_wallPosition = new Vector2(x * 32.0f, y * 32.0f);
			_map[y][x]->_wallRect = new Rect(0.0f, 0.0f, 32, 32);
		}
	}

	//Load Munchie and Consumerables
	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			if (mapInput[y][x] == '.')
			{
				_munchies[munchieAmount]->_texture = new Texture2D();
				_munchies[munchieAmount]->_texture->Load("Textures/Munchie.png", true);
				_munchies[munchieAmount]->_position = new Vector2(x*32.5f, y*32.5f);
				_munchies[munchieAmount]->_rect = new Rect(0.0f, 0.0f, 12, 12);
				_munchies[munchieAmount]->_eaten = false;
				munchieAmount++;
			}
			else if (mapInput[y][x] == 'O')
			{
				_protonPacks[protonPackAmount]->_texture = new Texture2D();
				_protonPacks[protonPackAmount]->_texture->Load("Textures/protonPacks.png", true);
				_protonPacks[protonPackAmount]->_position = new Vector2(x * 32.5f, y * 32.5f);
				_protonPacks[protonPackAmount]->_rect = new Rect(0.0f, 0.0f, 18, 18);
				protonPackAmount++;
			}
		}
	}
	


	// Load Cherry
	//_cherry->_texture = new Texture2D();
	//_cherry->_texture->Load("Textures/Cherry.png", true);
	//_cherry->_position = new Vector2(200.0f, 200.0f);
	//_cherry->_rect = new Rect(0.0f, 0.0f, 12, 12);


	//Initialise ghost character
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->texture = new Texture2D();
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
		if (i == 0)
		{
			_ghosts[i]->texture->Load("Textures/RedGhost.png", false); //Red Ghost is 0
			_ghosts[i]->position = new Vector2(355.0f, 355.0f);
			_ghosts[i]->ghostType = 1;
		}
		else if (i == 1)
		{
			_ghosts[i]->texture->Load("Textures/GhostBlue2.png", false); //Blue Ghosts is 1
			_ghosts[i]->position = new Vector2(579.0f, 355.0f);
			_ghosts[i]->ghostType = 2;
		}
		else if (i == 2)
		{
			_ghosts[i]->texture->Load("Textures/PinkGhost.png", false); //Pink Ghost is 2
			_ghosts[i]->position = new Vector2(355.0f, 483.0f);
			_ghosts[i]->ghostType = 3;
		}
		else if (i == 3)
		{
			_ghosts[i]->texture->Load("Textures/OrangeGhost.png", false); //Orange Ghost is 3
			_ghosts[i]->position = new Vector2(579.0f, 483.0f);
			_ghosts[i]->ghostType = 4;
		}
		
	}

	//Set score string position
	_scoreStringPosition = new Vector2(25.0f, 950.0f);
	

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);

	// Set Menu Parameters
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/Transparency.png", false);
	_menuRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	//Set Game Over Parameters
	_gameOverBackground = new Texture2D();
	_gameOverBackground->Load("Textures/Transparency.png", false);
	_gameOverRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_gameOverStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	//Set Game Win Parameters
	_gameWinBackground = new Texture2D();
	_gameWinBackground->Load("Textures/Transparency.png", false);
	_gameWinRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_gameWinStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	//Set Start Menu Parameters
	_startBackground = new Texture2D();
	_startBackground->Load("Textures/Transparency.png", false);
	_startRectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_startStringPostion = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	//Set Key Parameters
	_pKeyDown = false;
	_rKeyDown = false;
	_gameStart = false;
	_enterKeyDown = false;
}

void Pacman::Update(int elapsedTime)
{
	//Gets state of keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	Input::MouseState* mouseState = Input::Mouse::GetState();
	CheckPaused(keyboardState);
	if (_pacman->_lives <= 0)
	{
		_gameOver = true;
	}

	if (_gameOver || _gameWin)
	{
		CheckEnter(keyboardState);
	}

	if (!_paused && !_start && !_gameOver && !_gameWin && _pacman->dead == false)
	{
		
		Input(elapsedTime, keyboardState, mouseState);
		UpdatePacman(elapsedTime);
		CheckMunchieCollision();
		CheckWin();

		for (i = 0; i < _protonCount; i++)
		{
			UpdateProton(_protonPacks[i], elapsedTime);
		}

		CheckPackCollision();
		for (i = 0; i < GHOSTCOUNT; i++)
		{
			if (_pacman->dead == false)
			{
				CheckPacGhost(_ghosts[i]);
			}
		}
		CheckPacWallCollision(elapsedTime);
		///*UpdateCherry(elapsedTime);*/
		//RandomCherry(keyboardState);

		for (i = 0; i < GHOSTCOUNT; i++)
		{
			UpdateGhost(_ghosts[i], elapsedTime);
		}
		
		if (_pacman->ghostBuster)
		{
			clock_t start = clock(); //Start timer for ghostbuster
			for (i = 0; i < GHOSTCOUNT; i++)
			{
				_pacman->_pacmanTexture->Load("Textures/PacmanBuster.png", true); //Load Ghostbuster Texture
				_ghosts[i]->isScared = true; //Ghosts can be eaten
				GhostBuster(_ghosts[i], elapsedTime); //Check ghost with ghostbuster function
			}
			clock_t end = clock();
			double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
			CheckTimer(elapsed_secs);
			if (_time->storedTime < 0)
			{
				Audio::Pause(_theme);
				Audio::Play(_packStop);
				_pacman->ghostBuster = false;
				_pacman->_pacmanTexture->Load("Textures/Pacman2.png", true);
				for (i = 0; i < GHOSTCOUNT; i++)
				{
					_ghosts[i]->isScared = false;
					_ghosts[i]->isBeingSucked = false;
				}
			}
		}
		else if (!_pacman->ghostBuster)
		{
			SoundEffectState state = _siren->GetState();
			if (state == SoundEffectState::STOPPED)
			{
				Audio::Play(_siren);
			}
		}
		
		for (i = 0; i < GHOSTCOUNT; i++)
		{
			if (_pacman->dead == false)
			{
				CheckGhostCollision(_ghosts[i]);
			}
			CheckGhostWallCollision(_ghosts[i], elapsedTime);
		}
		
		for (i = 0; i < _munchieCount; i++)
		{
			UpdateMunchie(_munchies[i], elapsedTime);
		}
		
		CheckViewportCollision();
	}
	else if (_pacman->dead == true)
	{
		_pacman->_pacmanTexture->Load("Textures/PacmanDead.png", true);
		PacmanDeath(elapsedTime);
		if (_pacman->_pacmanFrame >= 8)
		{
			_pacman->_pacmanFrame = 0;
			_pacman->_pacmanCurrentFrameTime = 0;
			_pacman->dead = false;
			_pacman->_pacmanTexture->Load("Textures/Pacman2.png", true);
			ResetPositions();
		}
	}
	
}
	
void Pacman::Draw(int elapsedTime)
{

	// Allows us to easily create a string
	std::stringstream stream;
	/*float pacmanPosX = _pacman->_pacmanPosition->X / 32;
	float pacmanPosY = (_pacman->_pacmanPosition->Y + _pacman->_pacmanSourceRect->Height) / 32;
	int pacX = pacmanPosX;
	int X = (floor((pacmanPosX * 2) + 0.5) / 2);
*/
	// stream << "Pacman X: " << _pacman->_pacmanPosition->X << " Y: " << _pacman->_pacmanPosition->Y << " Pacman Pos X " << pacmanPosX << " Pacman Pos Y " << pacmanPosY << " Ceil X " << ceil(pacmanPosX) << " ceil Y " << ceil(pacmanPosY) << " floor X " << floor(pacmanPosX) << " floor Y " << floor(pacmanPosY) << "Pacman lives" << _pacman->_lives;

	SpriteBatch::BeginDraw(); // Starts Drawing


	if (!_gameOver || !_gameWin)
	{
		SpriteBatch::Draw(_pacman->_pacmanTexture, _pacman->_pacmanPosition, _pacman->_pacmanSourceRect); // Draws Pacman
	}
	


	//Draw Map
	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			SpriteBatch::Draw(_map[y][x]->_wallTexture, _map[y][x]->_wallPosition, _map[y][x]->_wallRect);
		}
	}

	//Draw ghost
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		if (_ghosts[i]->isDead == false)
		{
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
		}
		else if (_ghosts[i]->isDead == true)
		{
			SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
		}
	}
	

	//Draw Munchies
	for (i = 0; i < _munchieCount; i++)
	{
		SpriteBatch::Draw(_munchies[i]->_texture, _munchies[i]->_position, _munchies[i]->_rect);
	} 

	//Draw Proton Packs
	for (i = 0; i < _protonCount; i++)
	{
		SpriteBatch::Draw(_protonPacks[i]->_texture, _protonPacks[i]->_position, _protonPacks[i]->_rect);
	}

	////Draw Cherry
	//SpriteBatch::Draw(_cherry->_texture, _cherry->_position, _cherry->_rect);

	// Draws String and Menu
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);

	if (_start)
	{
		std::stringstream startStream;
		startStream << "PRESS SPACE TO START";

		SpriteBatch::Draw(_startBackground, _startRectangle, nullptr);
		SpriteBatch::DrawString(startStream.str().c_str(), _startStringPostion, Color::Red);
	}

	if (_paused)
	{
		std::stringstream menuStream;
		menuStream << "PAUSED!";

		SpriteBatch::Draw(_menuBackground, _menuRectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}


	

	std::stringstream scoreStream;
	_score ->_scoreString = to_string(_score->_scoreInt);
	scoreStream << "Current Score: " << _score->_scoreString << "\t High Score: "  << _score->_highScore << "\t Lives:" << _pacman->_lives;
	SpriteBatch::DrawString(scoreStream.str().c_str(), _scoreStringPosition, Color::White);

	if (_gameOver)
	{
		std::stringstream gameOverStream;
		gameOverStream << "GAME OVER! PRESS ENTER KEY TO RESTART!";

		SpriteBatch::Draw(_gameOverBackground, _gameOverRectangle, nullptr);
		SpriteBatch::DrawString(gameOverStream.str().c_str(), _gameOverStringPosition, Color::Yellow);

		Audio::Stop(_siren);
		Audio::Stop(_theme);
		_pacman->ghostBuster = false;
		if (_enterKeyDown == true)
		{
			if (_score->_scoreInt > _score->_highScore)
			{
				_score->_highScore = _score->_scoreInt;
				StoreScore();
			}
			LoadContent();
			_pacman->_lives = 3;
			_score->_scoreInt = 0;
			_munchiesEaten = 0;
			for (i = 0; i < GHOSTCOUNT; i++)
			{
				_ghosts[i]->isScared = false;
				_ghosts[i]->isBeingSucked = false;
			}
			_gameOver = false;
			_pacman->ghostBuster = false;
			_pacman->dead = false;
			_start = true;
			Update(elapsedTime);
		}
	}
	else if (_gameWin)
	{
		std::stringstream gameWinStream;
		gameWinStream << "YOU WIN! PRESS ENTER TO CONTINUE";

		SpriteBatch::Draw(_gameWinBackground, _gameWinRectangle, nullptr);
		SpriteBatch::DrawString(gameWinStream.str().c_str(), _gameWinStringPosition, Color::Green);
		Audio::Stop(_siren);
		Audio::Stop(_theme);
		_pacman->ghostBuster = false;

		if (_enterKeyDown == true)
		{
			LoadContent();
			_pacman->_lives = _pacman->_lives;
			_munchiesEaten = 0;
			
			_pacman->ghostBuster = false;
			_gameWin = false;
			for (i = 0; i < GHOSTCOUNT; i++)
			{
				_ghosts[i]->isScared = false;
				_ghosts[i]->isBeingSucked = false;
			}
			_pacman->dead = false;
			_start = true;
			Update(elapsedTime);
		}
	}

	SpriteBatch::EndDraw(); // Ends Drawing
	
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) {

	float pacmanSpeed = _pacman->_cPacmanSpeed * elapsedTime * _pacman->speedMultiplier;

	float pacmanPosx = (_pacman->_pacmanPosition->X ) / 32;
	float pacmanPosy = _pacman->_pacmanPosition->Y / 32;

	float ceilpacmanPosy =( _pacman->_pacmanPosition->Y - 10)/ 32; //Half of pacRect
	float ceilpacmanPosx = (_pacman->_pacmanPosition->X - 10) / 32; //Half of pacRect

	float floorpacmanPosy = (_pacman->_pacmanPosition->Y + 10) / 32; //Half of pacRect
	float floorpacmanPosx = (_pacman->_pacmanPosition->X + 10) / 32; //Half of pacRect

	int ceilX = ceil(ceilpacmanPosx);
	int ceily = ceil(ceilpacmanPosy);
	int floorX = floor(pacmanPosx); 
	int floorY = floor(pacmanPosy); //It just works


	if (!_start && _gameStart && !_gameOver)
	{
		if (!_paused)
		{	
			//Speed Multiplier
			if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT))
			{
				//Apply Multiplier
				_pacman->speedMultiplier = 2.0f;
			}
			else
			{
				//Reset Multiplier
				_pacman->speedMultiplier = 1.0f;
			}

			// Checks if D key is pressed
			if (keyboardState->IsKeyDown(Input::Keys::D)) //&& (_map[pacY][pacX +1]->_wallPosition->X + _pacman->speedMultiplier > _pacman->_pacmanPosition->X))
			{
				inputDirection = 0;
				 //Moves Pacman across X axis
				

			}

			else if (keyboardState->IsKeyDown(Input::Keys::A)) //&& (_map[pacY][pacX - 1]->_isWall == false))
			{
				inputDirection = 2;
				 //Moves Pacman across X axis
				
			}

			else if (keyboardState->IsKeyDown(Input::Keys::W)) //&& (_map[pacmanPosY - 1][pacmanPosX]->_isWall == false))
			{
				inputDirection = 3;
				 //Moves Pacman across Y axis
				
			}


			else if (keyboardState->IsKeyDown(Input::Keys::S)) //.&& (_map[pacmanPosY + 1][pacmanPosX]->_isWall == false))
			{
				inputDirection = 1;
				 //Moves Pacman across Y axis
				
			}


			//Move Cherry
			/*if (mouseState->LeftButton == Input::ButtonState::PRESSED)
			{
				_cherry->_position->X = mouseState->X;
				_cherry->_position->Y = mouseState->Y;
			}*/

			//Input Check Right
			if (inputDirection == 0 && _map[floorY][ceilX+1]->isWall== false && _pacman->_pacmanDirection == 1)
			{
				_pacman->_pacmanDirection = 0; //Move Right
			}
			else if (inputDirection == 0 && _map[ceily][ceilX + 1]->isWall == false && _pacman->_pacmanDirection == 3)
			{
				_pacman->_pacmanDirection = 0; //Move Right
			}
			else if (inputDirection == 0 && _pacman->_pacmanDirection == 2)
			{
				_pacman->_pacmanDirection = 0; //Move Right
			}

			//Input Check Left
			if (inputDirection == 2 && _map[floorY][floorX-1]->isWall== false && _pacman->_pacmanDirection == 1)
			{
				_pacman->_pacmanDirection = 2; //Move Left
			}
			else if (inputDirection == 2 && _map[ceily][floorX - 1]->isWall == false && _pacman->_pacmanDirection == 3)
			{
				_pacman->_pacmanDirection = 2; //Move Left
			}
			else if (inputDirection == 2  && _pacman->_pacmanDirection == 0)
			{
				_pacman->_pacmanDirection = 2; //Move Left
			}

			//Input Check Up
			if (inputDirection == 3 && _map[floorY - 1][floorX]->isWall == false && _pacman->_pacmanDirection == 0)
			{
				_pacman->_pacmanDirection = 3; //Move Up
			}
			else if (inputDirection == 3 && _map[floorY - 1][ceilX]->isWall == false && _pacman->_pacmanDirection == 2)
			{
				_pacman->_pacmanDirection = 3; //Move Up
			}
			else if (inputDirection == 3  && _pacman->_pacmanDirection == 1)
			{
				_pacman->_pacmanDirection = 3; //Move Up
			}

			//Input Check Down
			if (inputDirection == 1 && _map[floorY + 1][floorX]->isWall == false && _pacman->_pacmanDirection == 0)
			{
				_pacman->_pacmanDirection = 1; //Move Down
			}
			else if (inputDirection == 1 && _map[floorY + 1][ceilX]->isWall == false && _pacman->_pacmanDirection == 2)
			{
				_pacman->_pacmanDirection = 1; //Move Down
			}
			else if (inputDirection == 1 && _pacman->_pacmanDirection == 3)
			{
				_pacman->_pacmanDirection = 1; //Move Down
			}

			//Direction Check
			if (_pacman->_pacmanDirection == 0)
			{
				_pacman->_pacmanPosition->X += pacmanSpeed; //Move Right
			}
			else if (_pacman->_pacmanDirection == 2)
			{
				_pacman->_pacmanPosition->X -= pacmanSpeed; //Move Left
			}
			else if (_pacman->_pacmanDirection == 3)
			{
				_pacman->_pacmanPosition->Y -= pacmanSpeed; //Move Up
			}
			else if (_pacman->_pacmanDirection == 1)
			{
				_pacman->_pacmanPosition->Y += pacmanSpeed; //Move Down
			}
		}
		
	}
}

//void Pacman::RandomCherry(Input::KeyboardState* keyboardState) {
//
//	if (keyboardState->IsKeyDown(Input::Keys::R) && !_rKeyDown)
//	{
//		_rKeyDown = true;
//		//Random Cherry Position
//		_cherry->_position = new Vector2(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight());
//	}
//
//	if (keyboardState->IsKeyUp(Input::Keys::R))
//	{
//		_rKeyDown = false;
//	}
//}

void Pacman::CheckEnter(Input::KeyboardState* keyboardState)
{
	if (keyboardState->IsKeyDown(Input::Keys::RETURN) && !_enterKeyDown)
	{
		//cout << "Enter key is pressed" << endl;
		_enterKeyDown = true;
	}

	if (keyboardState->IsKeyUp(Input::Keys::RETURN))
	{
		_enterKeyDown = false;
	}
}

void Pacman::CheckPaused(Input::KeyboardState* keyboardState) {

	if (keyboardState->IsKeyDown(Input::Keys::SPACE) && !_gameStart)
	{
		_start = !_start;
		_gameStart = true;
	}

	if (keyboardState->IsKeyDown(Input::Keys::P) && !_pKeyDown)
	{
		_pKeyDown = true;
		_paused = !_paused;
	}

	if (keyboardState->IsKeyUp(Input::Keys::P))
		_pKeyDown = false;
}

void Pacman::CheckViewportCollision() {

	//Checks if Pacman is trying to disappear
	if (_pacman->_pacmanPosition->X + _pacman->_pacmanSourceRect->Width > Graphics::GetViewportWidth()) //1024 is game width
	{
		//Pacman hit right wall - reset position
		_pacman->_pacmanPosition->X = 0;
	}

	if (_pacman->_pacmanPosition->X < 0) //1024 is game width
	{
		//Pacman hit left wall - reset position
		_pacman->_pacmanPosition->X = Graphics::GetViewportWidth() - _pacman->_pacmanSourceRect->Width;
	}

	if (_pacman->_pacmanPosition->Y + _pacman->_pacmanSourceRect->Height > Graphics::GetViewportHeight()) //768 is game height
	{
		//Pacman hit bottom wall - reset position
		_pacman->_pacmanPosition->Y = 0;
	}

	if (_pacman->_pacmanPosition->Y < 0) //768 is game height
	{
		//Pacman hit top wall - reset position
		_pacman->_pacmanPosition->Y = Graphics::GetViewportHeight() - _pacman->_pacmanSourceRect->Height;
	}

}

void Pacman::UpdatePacman(int elapsedTime) {
	//Pacman Animation
	_pacman->_pacmanCurrentFrameTime += elapsedTime;

	if (_pacman->_pacmanCurrentFrameTime > _pacman->_FrameTime)
	{
		_pacman->_pacmanFrame++;

		if (_pacman->_pacmanFrame >= 2)
			_pacman->_pacmanFrame = 0;

		_pacman->_pacmanCurrentFrameTime = 0;
	}
	_pacman->_pacmanSourceRect->X = _pacman->_pacmanSourceRect->Width * _pacman->_pacmanFrame;
	_pacman->_pacmanSourceRect->Y = _pacman->_pacmanSourceRect->Height * _pacman->_pacmanDirection;
}

void Pacman::UpdateMunchie(Enemy* munchie, int elapsedTime) {
	//Munchie Animation
	
		munchie->_currentFrameTime += elapsedTime;

		if (munchie->_currentFrameTime > munchie->_cFrameTime)
		{
			munchie->_frameCount++;


			if (munchie->_frameCount >= 2)
				munchie->_frameCount = 0;

			munchie->_currentFrameTime = 0;
		}
		munchie->_rect->X = munchie->_rect->Width * munchie->_frameCount;
	
}

void Pacman::UpdateProton(Enemy* proton, int elapsedTime) {
	//Proton Animation

	proton->_currentFrameTime += elapsedTime;

	if (proton->_currentFrameTime > proton->_cFrameTime)
	{
		proton->_frameCount++;


		if (proton->_frameCount >= 2)
			proton->_frameCount = 0;

		proton->_currentFrameTime = 0;
	}
	proton->_rect->X = proton->_rect->Width * proton->_frameCount;

}

//void Pacman::UpdateCherry(int elapsedTime) {
//	//Munchie Animation
//
//	_cherry->_currentFrameTime += elapsedTime;
//
//	if (_cherry->_currentFrameTime > _cherry->_cFrameTime)
//	{
//		_cherry->_frameCount++;
//
//		if (_cherry->_frameCount >= 2)
//			_cherry->_frameCount = 0;
//
//		_cherry->_currentFrameTime = 0;
//	}
//	_cherry->_rect->X = _cherry->_rect->Width * _cherry->_frameCount;
//}

//void Pacman::GhostPath()
//{
//	string temp;
//	char mapInput[ScreenHeight][ScreenWidth];
//	int r = 0, v = 0;
//
//	char ch;
//	fstream fin("Map4.txt", fstream::in);
//
//	for (int y = 0; y < 29; y++) {
//		for (int x = 0; x < 29; x++) {
//			char ch = fin.get();
//			if (ch != '\n') mapInput[y][x] = ch;
//		}
//	}
//
//	struct sNode
//	{
//		bool isWall = false;
//		bool visited = false;
//		int parentX;
//		int parentY;
//		float gCost;
//		float hCost;
//		float fCost;
//		int posX;
//		int posY;
//	};
//	sNode* nodes[ScreenHeight][ScreenWidth];
//
//	int destX = _pacman->_pacmanPosition->X / 32;
//	int destY = _pacman->_pacmanPosition->Y / 32;
//
//	for (y = 0; y < 29; y++)
//	{
//		for (x = 0; x < 32; x++)
//		{
//			nodes[y][x] = new sNode();
//			nodes[y][x]->posX = x * 32;
//			nodes[y][x]->posY = y * 32;
//			nodes[y][x]->visited = false;
//			if (mapInput[y][x] != ' ' && mapInput[y][x] != '.')
//			{
//				nodes[y][x]->isWall = true;
//			}
//			else
//				nodes[y][x]->isWall = false;
//		}
//	}
//
//
//	while (_ghosts[0]->position->X != destX && _ghosts[0]->position->Y != destY)
//	{
//
//	}
//}


void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->isBeingSucked == false)
	{
		if (ghost->direction == 0) //Moves Right
		{
			ghost->position->X += ghost->speed * elapsedTime;
		}
		else if (ghost->direction == 1) //Moves Left
		{
			ghost->position->X -= ghost->speed * elapsedTime;
		}
		else if (ghost->direction == 2) //Moves Up
		{
			ghost->position->Y -= ghost->speed * elapsedTime;
		}
		else if (ghost->direction == 3) //Moves Down
		{
			ghost->position->Y += ghost->speed * elapsedTime;
		}

		ghost->sourceRect->X = ghost->sourceRect->Width * ghost->direction;

		if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) //Hits Right Edge
		{
			ghost->direction = 1; //Change direction
		}
		else if (ghost->position->X <= 0) //Hits left edge
		{
			ghost->direction = 0; //Change direction
		}
	}
	else if (ghost->isBeingSucked == true)
	{
		ghost->position = ghost->position;
	}
}

void Pacman::CheckGhostWallCollision(MovingEnemy* ghost, int elapsedTime)
{
	char mapInput[ScreenHeight][ScreenWidth];
	char ch;
	fstream fin("Map4.txt", fstream::in);
	for (int y = 0; y < 29; y++) {
		for (int x = 0; x < 29; x++) {
			char ch = fin.get();
			if (ch != '\n') mapInput[y][x] = ch;
		}
	}

	ghost->HitBox->X = ghost->position->X;
	ghost->HitBox->Y = ghost->position->Y;
	ghost->HitBox->Width = ghost->sourceRect->Width;
	ghost->HitBox->Height = ghost->sourceRect->Height;

	float currentPacX = _pacman->_pacmanPosition->X;
	float currentPacy = _pacman->_pacmanPosition->Y;
	int currentPacw = _pacman->_pacmanSourceRect->Width;
	int currentPach = _pacman->_pacmanSourceRect->Height;
	Rect currentPacRect = Rect(currentPacX, currentPacy, currentPacw, currentPach);

	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			float currentWallX = _map[y][x]->_wallPosition->X;
			float currentWally = _map[y][x]->_wallPosition->Y;
			int currentWallw = _map[y][x]->_wallRect->Width;
			int currentWallh = _map[y][x]->_wallRect->Height;
			Rect currentWallRect = Rect(currentWallX, currentWally, (currentWallw), currentWallh);

			if (ghost->HitBox->Intersects(Rect(currentWallRect)) && (ghost->isBeingSucked == true) && (ghost->HitBox->Intersects(Rect(currentPacRect))))
			{
			//cout << "Hit Wall and Being Sucked" << endl; //Crash Occurs
				if (ghost->ghostType == 1)
				{
					ghost->position->X = 355.0f;
					ghost->position->Y = 355.0f;
				}
				else if (ghost->ghostType == 2)
				{
					ghost->position->X = 579.0f;
					ghost->position->Y = 355.0f;
				}
				else if (ghost->ghostType == 3)
				{
					ghost->position->X = 355.0f;
					ghost->position->Y = 483.0f;
				}
				else if (ghost->ghostType == 4)
				{
					ghost->position->X = 579.0f;
					ghost->position->Y = 483.0f;
				}
			srand(time(NULL));
			ghost->direction = rand() % 4;
			ghost->isDead = true;
			Audio::Play(_capture);
			return;
			}
			else if (mapInput[y][x] != '.' && mapInput[y][x] != ' ' && mapInput[y][x] != 'O')
			{
				if (ghost->HitBox->Intersects(Rect(currentWallRect)) && (ghost->isBeingSucked == false))
				{
					//cout << "Hit Wall and Being Not Sucked" << endl;
					if (ghost->direction == 0)
					{
						ghost->position->X -= ghost->speed * elapsedTime;
						srand(time(NULL));
						ghost->direction = rand() % (3 - 2 + 1) + 2;
						/*cout << ghost->direction << endl;*/
					}
					else if (ghost->direction == 1)
					{
						ghost->position->X += ghost->speed * elapsedTime;
						srand(time(NULL));
						ghost->direction = rand() %(3 - 2 + 1) + 2;
						/*cout << ghost->direction << endl;*/
					}
					else if (ghost->direction == 2) {
						ghost->position->Y += ghost->speed * elapsedTime;
						srand(time(NULL));
						ghost->direction = rand() % 2;
						/*cout << ghost->direction << endl;*/
					}
					else if (ghost->direction == 3) {
						ghost->position->Y -= ghost->speed * elapsedTime;
						srand(time(NULL));
						ghost->direction = rand() % 2;
						/*cout << ghost->direction << endl;*/
					}
				}
			}
		}
	}
}

void Pacman::CheckGhostCollision(MovingEnemy* ghost)
{

	float currentPacX = _pacman->_pacmanPosition->X;
	float currentPacy = _pacman->_pacmanPosition->Y;
	int currentPacw = _pacman->_pacmanSourceRect->Width;
	int currentPach = _pacman->_pacmanSourceRect->Height;
	Rect currentPacRect = Rect(currentPacX, currentPacy, currentPacw, currentPach);

	ghost->HitBox->X = ghost->position->X;
	ghost->HitBox->Y = ghost->position->Y;
	ghost->HitBox->Width = ghost->sourceRect->Width;
	ghost->HitBox->Height = ghost->sourceRect->Height;

	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			float currentWallX = _map[y][x]->_wallPosition->X;
			float currentWally = _map[y][x]->_wallPosition->Y;
			int currentWallw = _map[y][x]->_wallRect->Width;
			int currentWallh = _map[y][x]->_wallRect->Height;
			Rect currentWallRect = Rect(currentWallX, currentWally, (currentWallw), currentWallh);

			if (ghost->HitBox->Intersects(Rect(currentPacRect)))
			{
				/*cout << "True Current Ghost Rect = " << _ghosts[0]->position->X << " " << _ghosts[0]->position->Y << " " << _ghosts[0]->sourceRect->Width << " " << _ghosts[0]->sourceRect->Height << endl;
				cout << "Pacman Rect = " << currentPacX << " " << currentPacy << " " << " " << currentPacw << " " << currentPach << endl;*/
				if (ghost->isScared == false)
				{
					//cout << "Pacman Died" << endl;
					_pacman->ghostBuster = false;
					_pacman->_lives -= 1;
					_pacman->dead = true;
					_pacman->_pacmanFrame = 0;
					Audio::Pause(_theme);
					Audio::Stop(_siren);
					Audio::Play(_deathNoise);
					return;
				}
				else if (ghost->isScared == true)
				{
					//cout << "Ghost Died" << endl;
					ghost->isDead = true;
					Audio::Play(_capture);

					if (ghost->ghostType == 1)
					{
						ghost->position->X = 355.0f;
						ghost->position->Y = 355.0f;
					}
					else if (ghost->ghostType == 2)
					{
						ghost->position->X = 579.0f;
						ghost->position->Y = 355.0f;
					}
					else if (ghost->ghostType == 3)
					{
						ghost->position->X = 355.0f;
						ghost->position->Y = 483.0f;
					}
					else if (ghost->ghostType == 4)
					{
						ghost->position->X = 579.0f;
						ghost->position->Y = 483.0f;
					}

					srand(time(NULL));
					ghost->direction = rand() % 4;
					ghost->isBeingSucked = false;
					if (ghost->ghostType == 1)
					{
						ghost->texture->Load("Textures/RedGhost.png", false);
					}
					else if (ghost->ghostType == 2)
					{
						ghost->texture->Load("Textures/GhostBlue2.png", false);
					}
					else if (ghost->ghostType == 3)
					{
						ghost->texture->Load("Textures/PinkGhost.png", false);
					}
					else if (ghost->ghostType == 4)
					{
						ghost->texture->Load("Textures/OrangeGhost.png", false);
					}
					ghost->isScared = false;
					return;
				}
				/*else
				{
					cout << "Ghost Died" << endl;
					_ghosts[0]->isDead = true;
					_ghosts[0]->position->X = 355.0f;
					_ghosts[0]->position->Y = 355.0f;
					_ghosts[0]->isBeingSucked = false;
					_ghosts[0]->isScared = false;
					return;
				}*/
			}
			else if (ghost->HitBox->Intersects(Rect(currentWallRect)) && (ghost->HitBox->Intersects(Rect(currentPacRect)))) //Stops double collisions
			{
				//cout << "Intersect Both" << endl;
				ghost->isDead = true;
				Audio::Play(_capture);
				if (ghost->ghostType == 1)
				{
					ghost->position->X = 355.0f;
					ghost->position->Y = 355.0f;
				}
				else if (ghost->ghostType == 2)
				{
					ghost->position->X = 579.0f;
					ghost->position->Y = 355.0f;
				}
				else if (ghost->ghostType == 3)
				{
					ghost->position->X = 355.0f;
					ghost->position->Y = 483.0f;
				}
				else if (ghost->ghostType == 4)
				{
					ghost->position->X = 579.0f;
					ghost->position->Y = 483.0f;
				}
				srand(time(NULL));
				ghost->direction = rand() % 4;
				ghost->isBeingSucked = false;
				if (ghost->ghostType == 1)
				{
					ghost->texture->Load("Textures/RedGhost.png", false);
				}
				else if (ghost->ghostType == 2)
				{
					ghost->texture->Load("Textures/GhostBlue2.png", false);
				}
				else if (ghost->ghostType == 3)
				{
					ghost->texture->Load("Textures/PinkGhost.png", false);
				}
				else if (ghost->ghostType == 4)
				{
					ghost->texture->Load("Textures/OrangeGhost.png", false);
				}
				ghost->isScared = false;
				return;
			}
		}
	}
	
	
}

void Pacman::CheckPacGhost(MovingEnemy* ghost)
{
	_pacman->Box->X = _pacman->_pacmanPosition->X;
	_pacman->Box->Y = _pacman->_pacmanPosition->Y;
	_pacman->Box->Width = _pacman->_pacmanSourceRect->Width;
	_pacman->Box->Height = _pacman->_pacmanSourceRect->Height;

	float currentGhostX = ghost->position->X;
	float currentGhosty = ghost->position->Y;
	int currentGhostw = ghost->sourceRect->Width;
	int currentGhosth = ghost->sourceRect->Height;
	Rect currentGhostRect = Rect(currentGhostX, currentGhosty, currentGhostw, currentGhosth);


	if (_pacman->Box->Intersects(Rect(currentGhostRect)))
	{
		if (ghost->isScared == false)
		{
			//cout << "Pacman Died" << endl;
			_pacman->ghostBuster = false;
			_pacman->_lives -= 1;
			_pacman->dead = true;
			_pacman->_pacmanFrame = 0;
			Audio::Pause(_theme);
			Audio::Stop(_siren);
			Audio::Play(_deathNoise);
			return;
		}
		else if (ghost->isScared == true)
		{
			//cout << "Ghost Died" << endl;
			ghost->isDead = true;
			Audio::Play(_capture);
			srand(time(NULL));
			ghost->direction = rand() % 4;

			if (ghost->ghostType == 1)
			{
				ghost->position->X = 355.0f;
				ghost->position->Y = 355.0f;
			}
			else if (ghost->ghostType == 2)
			{
				ghost->position->X = 579.0f;
				ghost->position->Y = 355.0f;
			}
			else if (ghost->ghostType == 3)
			{
				ghost->position->X = 355.0f;
				ghost->position->Y = 483.0f;
			}
			else if (ghost->ghostType == 4)
			{
				ghost->position->X = 579.0f;
				ghost->position->Y = 483.0f;
			}

			ghost->isBeingSucked == false;
			if (ghost->ghostType == 1)
			{
				ghost->texture->Load("Textures/RedGhost.png", false);
			}
			else if (ghost->ghostType == 2)
			{
				ghost->texture->Load("Textures/GhostBlue2.png", false);
			}
			else if (ghost->ghostType == 3)
			{
				ghost->texture->Load("Textures/PinkGhost.png", false);
			}
			else if (ghost->ghostType == 4)
			{
				ghost->texture->Load("Textures/OrangeGhost.png", false);
			}
			ghost->isScared == false;
			return;
		}
		/*else
		{
			cout << "Ghost Died" << endl;
			ghost->isDead = true;
			ghost->position->X = 355.0f;
			ghost->position->Y = 355.0f;
			ghost->isBeingSucked == false;
			ghost->isScared == false;
		}*/
	}
}

void Pacman::CheckPacWallCollision(int elapsedTime)
{
	char mapInput[ScreenHeight][ScreenWidth];
	char ch;
	fstream fin("Map4.txt", fstream::in);
	for (int y = 0; y < 29; y++) {
		for (int x = 0; x < 29; x++) {
			char ch = fin.get();
			if (ch != '\n') mapInput[y][x] = ch;
		}
	}

	_pacman->Box->X = _pacman->_pacmanPosition->X;
	_pacman->Box->Y = _pacman->_pacmanPosition->Y;
	_pacman->Box->Width = _pacman->_pacmanSourceRect->Width;
	_pacman->Box->Height = _pacman->_pacmanSourceRect->Height;

	for (y = 0; y < 29; y++)
	{
		for (x = 0; x < 29; x++)
		{
			float currentWallX = _map[y][x]->_wallPosition->X;
			float currentWally = _map[y][x]->_wallPosition->Y;
			int currentWallw = _map[y][x]->_wallRect->Width;
			int currentWallh = _map[y][x]->_wallRect->Height;
			Rect currentWallRect = Rect(currentWallX, currentWally, (currentWallw), currentWallh);

			if (mapInput[y][x] != '.' && mapInput[y][x] != ' ' && mapInput[y][x] != 'O')
			{
				if (_pacman->Box->Intersects(Rect(currentWallRect)))
				{
					if (_pacman->_pacmanDirection == 0)
					{
						_pacman->_pacmanPosition->X = _pacman->_pacmanPosition->X - _pacman->_cPacmanSpeed * elapsedTime;
					}
					else if (_pacman->_pacmanDirection == 2)
					{
						_pacman->_pacmanPosition->X = _pacman->_pacmanPosition->X + _pacman->_cPacmanSpeed * elapsedTime;
					}
					else if (_pacman->_pacmanDirection == 1)
					{
						_pacman->_pacmanPosition->Y = _pacman->_pacmanPosition->Y - _pacman->_cPacmanSpeed * elapsedTime;
					}
					else if (_pacman->_pacmanDirection == 3)
					{
						_pacman->_pacmanPosition->Y = _pacman->_pacmanPosition->Y + _pacman->_cPacmanSpeed * elapsedTime;
					}
				}
			}
		}
	}
}

void Pacman::CheckMunchieCollision()
{
	_pacman->Box->X = _pacman->_pacmanPosition->X;
	_pacman->Box->Y = _pacman->_pacmanPosition->Y;
	_pacman->Box->Width = _pacman->_pacmanSourceRect->Width;
	_pacman->Box->Height = _pacman->_pacmanSourceRect->Height;

	for (i = 0; i < _munchieCount; i++)
	{
		float currentMunchX = _munchies[i]->_position->X;
		float currentMunchy = _munchies[i]->_position->Y;
		int currentMunchw = _munchies[i]->_rect->Width;
		int currentMunchh = _munchies[i]->_rect->Height;
		Rect currentMunchRect = Rect(currentMunchX, currentMunchy, (currentMunchw), currentMunchh);

		if (_pacman->Box->Intersects(Rect(currentMunchRect)))
		{
			_munchies[i]->_position->X = 1000.0f;
			_score->_scoreInt += 50;
			Audio::Play(_pop);
		}
	}
}

void Pacman::CheckPackCollision()
{
	_pacman->Box->X = _pacman->_pacmanPosition->X;
	_pacman->Box->Y = _pacman->_pacmanPosition->Y;
	_pacman->Box->Width = _pacman->_pacmanSourceRect->Width;
	_pacman->Box->Height = _pacman->_pacmanSourceRect->Height;

	for (i = 0; i < _protonCount; i++)
	{
		float currentPackX = _protonPacks[i]->_position->X;
		float currentPacky = _protonPacks[i]->_position->Y;
		int currentPackw = _protonPacks[i]->_rect->Width;
		int currentPackh = _protonPacks[i]->_rect->Height;
		Rect currentPackRect = Rect(currentPackX, currentPacky, (currentPackw), currentPackh);

		if (_pacman->Box->Intersects(Rect(currentPackRect)))
		{
			_protonPacks[i]->_position->X = 1000.0f;
			Audio::Play(_packStart);
			_pacman->ghostBuster =true;
			Audio::Stop(_siren);
			Audio::Resume(_theme);
			ResetTimer();
		}
	}
}

void Pacman::GhostBuster(MovingEnemy* ghost, int elapsedTime)
{
	const float suckSpeed = 1.0f;
	int pacmanPosx = _pacman->_pacmanPosition->X / 32;
	int pacmanPosy = _pacman->_pacmanPosition->Y / 32;
	int ghostPosy = ghost->position->Y / 32;
	int ghostPosx = ghost->position->X / 32;
	
	
	float ghostfcy = (ghost->position->Y -10)/ 32;
	float ghostfcx = (ghost->position->X -10)/ 32;
	float pacmanfcx = (_pacman->_pacmanPosition->X -10)/ 32;
	float pacmanfcy = (_pacman->_pacmanPosition->Y -10)/ 32;

	float ghostffy = (ghost->position->Y +10)/ 32;
	float ghostffx = (ghost->position->X +10)/ 32;
	float pacmanffx = (_pacman->_pacmanPosition->X +10)/ 32;
	float pacmanffy = (_pacman->_pacmanPosition->Y +10) / 32;

	int ghostcx = ceil(ghostfcx);
	int ghostyx = ceil(ghostfcy);
	int paccx = ceil(pacmanfcx);
	int paccy = ceil(pacmanfcy);

	int ghostflx = floor(ghostffx);
	int ghostfly = floor(ghostffy);
	int pacflx = floor(pacmanffx);
	int pacfly = floor(pacmanffy);



	if (_pacman->ghostBuster == true && _pacman->_pacmanPosition != ghost->position && pacmanPosx == ghostPosx) //Top Suck
	{
		if (pacmanPosy - ghostPosy < 0 && _pacman->_pacmanDirection == 1)
		{
			while (paccx == ghostcx)
			{
				//cout << "Pacman over ghost" << endl;
				for (i = ghostPosy - 1; i >= pacmanPosy; i--)
				{
					if (_map[i][pacmanPosx]->isWall == true)
					{
						//cout << "Wall" << endl;
						ghost->isBeingSucked = false;
						return;
					}
					else
					{
						ghost->isBeingSucked = true;
						SoundEffectState state = _suck->GetState();
						if (state != SoundEffectState::PLAYING)
						{
							Audio::Play(_suck);
						}
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhostS.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2S.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhostS.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhostS.png", false);
						}
						ghost->direction = 2;
						//cout << "Moving up" << endl;
						ghost->position->Y -= suckSpeed * elapsedTime;
						CheckGhostCollision(ghost);
						return;
					}
				}
			}
		}
		else if (pacmanPosy - ghostPosy > 0 && _pacman->_pacmanDirection == 3)
		{
			while (paccx == ghostcx)
			{
				//cout << "Pacman under ghost" << endl;
				for (i = ghostPosy + 1; i <= pacmanPosy; i++)
				{
					if (_map[i][pacmanPosx]->isWall == true)
					{
						//cout << "wall" << endl;
						ghost->isBeingSucked = false;
						ghost->position->X -= 0.1f;
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhost.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhost.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhost.png", false);
						}
						return;
					}
					else
					{
						ghost->isBeingSucked = true;
						SoundEffectState state = _suck->GetState();
						if (state != SoundEffectState::PLAYING)
						{
							Audio::Play(_suck);
						}
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhostS.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2S.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhostS.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhostS.png", false);
						}
						ghost->direction = 3;
						//cout << "Moving Down" << endl;
						ghost->position->Y += suckSpeed * elapsedTime;
						CheckGhostCollision(ghost);
						return;
					}
				}
			}
		}
	}
	else if (_pacman->ghostBuster == true && _pacman->_pacmanPosition != ghost->position && pacmanPosy == ghostPosy) //Side Suck
	{
		if (pacmanPosx - ghostPosx < 0 && _pacman->_pacmanDirection == 0)
		{
			while (pacfly == ghostfly)
			{
				//cout << "Pacman left ghost" << endl;
				for (i = ghostPosx - 1; i >= pacmanPosx; i--)
				{
					if (_map[pacmanPosy][i]->isWall == true)
					{
						//cout << "Wall" << endl;
						ghost->isBeingSucked = false;
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhost.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhost.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhost.png", false);
						}
						return;
					}
					else
					{
						ghost->isBeingSucked = true;
						SoundEffectState state = _suck->GetState();
						if (state != SoundEffectState::PLAYING)
						{
							Audio::Play(_suck);
						}
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhostS.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2S.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhostS.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhostS.png", false);
						}
						ghost->direction = 1;
						//cout << "Moving right" << endl;
						ghost->position->X -= suckSpeed * elapsedTime;
						CheckGhostCollision(ghost);
						return;
					}
				}
			}	
		}
		else if (pacmanPosx - ghostPosx > 0 && _pacman->_pacmanDirection == 2)
		{
			while (pacfly == ghostfly)
			{
				//cout << "Pacman right ghost" << endl;
				for (i = ghostPosx + 1; i <= pacmanPosx; i++)
				{
					if (_map[pacmanPosy][i]->isWall == true)
					{
						//cout << "wall" << endl;
						ghost->isBeingSucked = false;
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhost.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhost.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhost.png", false);
						}
						return;
					}
					else
					{
						ghost->isBeingSucked = true;
						SoundEffectState state = _suck->GetState();
						if (state != SoundEffectState::PLAYING)
						{
							Audio::Play(_suck);
						}
						if (ghost->ghostType == 1)
						{
							ghost->texture->Load("Textures/RedGhostS.png", false);
						}
						else if (ghost->ghostType == 2)
						{
							ghost->texture->Load("Textures/GhostBlue2S.png", false);
						}
						else if (ghost->ghostType == 3)
						{
							ghost->texture->Load("Textures/PinkGhostS.png", false);
						}
						else if (ghost->ghostType == 4)
						{
							ghost->texture->Load("Textures/OrangeGhostS.png", false);
						}
						ghost->direction = 0;
						//cout << "Moving Right" << endl;
						ghost->position->X += suckSpeed * elapsedTime;
						CheckGhostCollision(ghost);
						return;
					}
				}
			}			
		}
	}
	else if (_pacman->ghostBuster == true && pacmanPosx == ghostPosx && pacmanPosy == ghostPosy)
	{
		ghost->isDead = true;
		Audio::Play(_capture);
		return;
	}
	ghost->isBeingSucked = false;
	if (ghost->ghostType == 1)
	{
		ghost->texture->Load("Textures/RedGhost.png", false);
	}
	else if (ghost->ghostType == 2)
	{
		ghost->texture->Load("Textures/GhostBlue2.png", false);
	}
	else if (ghost->ghostType == 3)
	{
		ghost->texture->Load("Textures/PinkGhost.png", false);
	}
	else if (ghost->ghostType == 4)
	{
		ghost->texture->Load("Textures/OrangeGhost.png", false);
	}
	return;
}


void CheckCollision()
{

}

void Pacman::CheckTimer(double time)
{
	_time->storedTime -= time;
}

void Pacman::ResetTimer()
{
	_time->storedTime = 6;
}

void Pacman::CheckWin()
{
	for (i = 0; i < _munchieCount; i++)
	{
		if (_munchies[i]->_position->X == 1000.0f && _munchies[i]->_eaten == false)
		{
			_munchiesEaten++;
			_munchies[i]->_eaten = true;
			//cout << _munchiesEaten;
		}
	}

	if (_munchiesEaten == _munchieCount)
	{
		_gameWin = true;
		Audio::Play(_win);
	}
}

void Pacman::PacmanDeath(int elapsedTime) {
	//Pacman Animation
	
	_pacman->_pacmanCurrentFrameTime += elapsedTime;

	if (_pacman->_pacmanCurrentFrameTime > _pacman->_FrameTime)
	{
		_pacman->_pacmanSourceRect->X = _pacman->_pacmanSourceRect->Width * _pacman->_pacmanFrame;
		Sleep(100);
		_pacman->_pacmanFrame++;
	}
	
	_pacman->_pacmanSourceRect->Y = _pacman->_pacmanSourceRect->Height * _pacman->_pacmanDirection;
}

//void Pacman::UpdatePacman(int elapsedTime) {
//	//Pacman Animation
//	_pacman->_pacmanCurrentFrameTime += elapsedTime;
//
//	if (_pacman->_pacmanCurrentFrameTime > _pacman->_FrameTime)
//	{
//		_pacman->_pacmanFrame++;
//
//		if (_pacman->_pacmanFrame >= 2)
//			_pacman->_pacmanFrame = 0;
//
//		_pacman->_pacmanCurrentFrameTime = 0;
//	}
//	_pacman->_pacmanSourceRect->X = _pacman->_pacmanSourceRect->Width * _pacman->_pacmanFrame;
//	_pacman->_pacmanSourceRect->Y = _pacman->_pacmanSourceRect->Height * _pacman->_pacmanDirection;

void Pacman::ResetPositions()
{
	for (i = 0; i < GHOSTCOUNT; i++)
	{
		if (i == 0)
		{
			//Red Ghost is 0
			_ghosts[i]->position = new Vector2(355.0f, 355.0f);
		}
		else if (i == 1)
		{
			 //Blue Ghosts is 1
			_ghosts[i]->position = new Vector2(579.0f, 355.0f);
		}
		else if (i == 2)
		{
			//Pink Ghost is 2
			_ghosts[i]->position = new Vector2(355.0f, 483.0f);
		}
		else if (i == 3)
		{
			//Orange Ghost is 3
			_ghosts[i]->position = new Vector2(579.0f, 483.0f);
		}

	}
	_pacman->_pacmanPosition = new Vector2(704.0f, 448.0f);
}

void Pacman::StoreScore()
{
	ofstream outScorefile("HighScore/HighScore.txt");

	// Check if the file was opened.
	if (!outScorefile.is_open())
	{
		cout << "File could not be opened" << endl;
	}
	
	//Write high score to file
	outScorefile << _score->_highScore;

	// Close the file.
	outScorefile.close();
}

void Pacman::ReadSetScore()
{
	int scoreToStore = 0;
	ifstream inScorefile("HighScore/HighScore.txt");

	// Check if the file was opened.
	if (inScorefile.is_open())
	{
		while (inScorefile >> scoreToStore)
		{
			//Store current high score
			inScorefile >> scoreToStore;
			_score->_highScore = scoreToStore;
		}	
		// Close the file.
		inScorefile.close();
	}
	else
	{
		cout << "Unable to open file";
	}
}