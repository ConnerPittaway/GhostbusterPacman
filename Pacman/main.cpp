#include "Pacman.h"

//Entry Point for Application
int main(int argc, char* argv[]) {
	
	int Count = 236;
	int ProtonAmount = 4;

	Pacman* game = new Pacman(argc, argv, Count, ProtonAmount);
}