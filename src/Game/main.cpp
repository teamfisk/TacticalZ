#include "Game.h"

int main(int argc, char* argv[])
{
	Game game(argc, argv);
	while (game.Running()) {
		game.Tick();
	}

	return 0;
}