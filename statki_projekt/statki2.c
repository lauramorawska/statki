#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BOARD_SIZE 10
#define NUM_SHIPS 10
#define EMPTY '-'
#define SHIP 'S'
#define HIT 'X'
#define MISS 'O'
#define SHIPS_VISIBLE 0

typedef struct {
	int x;
	int y;
	int size;
	int isVertical;
	int isSunk;
	int mastsAlive;
} Ship;

typedef struct {
	char board[BOARD_SIZE][BOARD_SIZE];
	Ship ships[NUM_SHIPS];
} Game;

void displayBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
	printf("   A B C D E F G H I J\n");
	for (int i = 0; i < BOARD_SIZE; i++) {
		printf("%2d ", i + 1);
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (SHIPS_VISIBLE == 0 && board[i][j] == SHIP) {
				printf("%c ", EMPTY);
				continue;
			}

			printf("%c ", board[i][j]);
		}
		printf("\n");
	}
}

void loadShipData(Ship* ship, char* line) {
	char* next_token = NULL;
	char seps[] = ";";

	ship->x = strtok_s(line, seps, &next_token)[0] - '0';
	ship->y = strtok_s(next_token, seps, &next_token)[0] - '0';
	ship->size = strtok_s(next_token, seps, &next_token)[0] - '0';
	ship->isVertical = strtok_s(next_token, seps, &next_token)[0] - '0';
}

int loadBoardFromFile(Game* game, int boardNumber) {
	FILE* file;
	fopen_s(&file, "Plansze.txt", "r");
	if (file == NULL) {
		printf("Nie mozna otworzyc pliku.\n");
		return 1;
	}

	int i = 0;
	int skip = 10 * (boardNumber - 1);
	char line[1024];
	while (fgets(line, 1024, file) && i < NUM_SHIPS) {
		if (skip-- > 0) {
			continue;
		}

		Ship* ship = &(game->ships[i++]);
		char* tmp = _strdup(line);
		loadShipData(ship, tmp);

		ship->isSunk = 0;
		ship->mastsAlive = ship->size;

		for (int i = 0; i < ship->size; i++) {
			if (ship->isVertical) {
				game->board[ship->y + i][ship->x] = SHIP;
			}
			else {
				game->board[ship->y][ship->x + i] = SHIP;
			}
		}

		free(tmp);
	}

	fclose(file);
	return 0;
}

void placeShip(Game* game, int index, int size, int isVertical, int x, int y) {
	Ship* ship = &(game->ships[index]);
	ship->x = x;
	ship->y = y;
	ship->size = size;
	ship->isVertical = isVertical;
	ship->isSunk = 0;
	ship->mastsAlive = size;

	for (int i = 0; i < size; i++) {
		if (isVertical) {
			game->board[y + i][x] = SHIP;
		}
		else {
			game->board[y][x + i] = SHIP;
		}
	}
}

int isFieldValid(Game* game, int x, int y) {
	if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
		return 0;
	}

	// check neighborhood - 3x3 fields square with (x,y) in the center
	for (int i = x - 1; i <= x + 1; i++) {
		for (int j = y - 1; j <= y + 1; j++) {
			// filed outside the board doesnt matter
			if (i < 0 || i >= BOARD_SIZE || j < 0 || j >= BOARD_SIZE) {
				continue;
			}

			if (game->board[j][i] == SHIP) {
				return 0;
			}
		}
	}

	return 1;
}

int isValidPlacement(Game* game, int size, int isVertical, int x, int y) {
	if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
		return 0;
	}

	for (int i = 0; i < size; i++) {
		// coordinates of next mast
		int check_x, check_y;
		check_x = isVertical ? x : x + i;
		check_y = isVertical ? y + i : y;

		if (isFieldValid(game, check_x, check_y) == 0) {
			return 0;
		}
	}

	return 1;
}

void randomPlacement(Game* game) {
	int sizes[NUM_SHIPS] = { 1, 1, 1, 1, 2, 2, 2, 3, 3, 4 };
	int isVertical, x, y;

	for (int i = 0; i < NUM_SHIPS; i++) {
		do {
			isVertical = rand() % 2;
			x = rand() % BOARD_SIZE;
			y = rand() % BOARD_SIZE;
		} while (!isValidPlacement(game, sizes[i], isVertical, x, y));

		placeShip(game, i, sizes[i], isVertical, x, y);
	}
}

void initializeBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j] = EMPTY;
		}
	}
}

int isHit(Ship* ship, int y, int x) {
	if (ship->isVertical) {
		if (ship->x != x) return 0;

		if (!(y >= ship->y && y < (ship->y + ship->size))) return 0;

		return 1;
	}
	else {
		if (ship->y != y) return 0;

		if (!(x >= ship->x && x < (ship->x + ship->size))) return 0;

		return 1;
	}
}

int checkHit(Game* game, int x, int y) {
	if (game->board[y][x] != SHIP) {
		return 0;
	}

	for (int i = 0; i < NUM_SHIPS; i++) {
		Ship* ship = &(game->ships[i]);
		if (ship->isSunk) {
			continue;
		}

		if (isHit(ship, y, x) == 0) {
			continue;
		}

		// isHit() == 1
		ship->mastsAlive--;
		if (ship->mastsAlive == 0) {
			ship->isSunk = 1;
			return 2;
		}

		return 1;
	}

	return 0;
}

int isGameOver(Game* game) {
	for (int i = 0; i < NUM_SHIPS; i++) {
		if (game->ships[i].isSunk == 0) {
			return 0;
		}
	}
	return 1;
}

void play(Game* game) {
	int shots = 0;
	int result = -1;
	while (!isGameOver(game)) {
		system("cls");
		printf("Witaj w grze w statki!\t\tstrzaly: %d\n", shots);
		printf("Zasady gry: Podawaj pole w formacie [litera][cyfra], np. A3, J10\n");

		switch (result) {
		case 2:
			printf("Trafiony zatopiony!\n");
			break;
		case 1:
			printf("Trafiony!\n");
			break;
		case 0:
			printf("Pudlo!\n");
			break;
		default:
			printf("\n");
			break;
		}
		displayBoard(game->board, 0);

		char input[4];
		printf("Podaj pole: ");
		scanf_s("%s", input, (unsigned int)sizeof(input));
		input[0] = toupper(input[0]);

		if (input[0] < 'A' || input[0] > 'J' || input[1] < '1' || input[1] > '9') {
			printf("Nieprawidlowe pole!\n");
			continue;
		}

		int x = input[0] - 'A';
		int y;
		if (input[2] == '\0') {
			y = input[1] - '1';
		}
		else {
			y = 9;
		}

		shots++;
		result = checkHit(game, x, y);

		if (result == 2) {
			game->board[y][x] = HIT;
		}
		else if (result == 1) {
			game->board[y][x] = HIT;
		}
		else if (game->board[y][x] == EMPTY) { // result == 0
			game->board[y][x] = MISS;
			//break; // DEBUG MODE -> strzel w puste pole by wygrac
		}
	}

	system("cls");
	displayBoard(game->board);
	printf("Gratulacje! Zatopiles wszystkie statki!\nWcisnij dowolny przycisk by grac dalej.\n");
	system("pause");
}

int main() {
	srand(time(NULL));

	Game game;
	int usedBoardsCount = 0;
	while (1) {
		initializeBoard(game.board);
		if (usedBoardsCount >= 10) {
			randomPlacement(&game);
			play(&game);
			continue;
		}

		char input[2];
		printf("Wylosowac plansze(1) czy wczytaj kolejna z pliku(2)?: ");
		scanf_s("%s", &input, (unsigned int)sizeof(input));
		if (input[0] == '1') {
			randomPlacement(&game);
			play(&game);
			continue;
		}

		if (input[0] == '2') {
			if (loadBoardFromFile(&game, 1) != 0) {
				printf("Nie udalo sie wczytac planszy z pliku. \n");
				return 1;
			}
			usedBoardsCount++;
			play(&game);
			continue;
		}

		system("cls");
		printf("Nieprawidlowa opcja!\n");
	}

	return 0;
}