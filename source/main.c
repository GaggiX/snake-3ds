// Simple citro2d untextured shape example
#include <citro2d.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

typedef enum
{
	Up,
	Down,
	Right,
	Left
} Direction;

typedef struct
{
	s32 x;
	s32 y;
} Cube;

struct LinkedCube
{
	Cube cube;
	struct LinkedCube *cube_back;
};

typedef struct
{
	struct LinkedCube *cubes;
	u32 len;
} Tail;

typedef struct
{
	Direction direction;
	Direction change_direction;
	Cube head;
	Tail tail;
} Snake;

u32 mod(s32 x, s32 N)
{
	return (x % N + N) % N;
}

u32 is_the_opp(Direction fst, Direction snd)
{
	switch (fst)
	{
	case Up:
		return snd == Down;
	case Down:
		return snd == Up;
	case Left:
		return snd == Right;
	case Right:
		return snd == Left;
	}
	return 1;
}

void move_snake(Snake *snake, s32 x, s32 y, Cube *food)
{
	Cube save_cube = snake->head;
	snake->head.x += x;
	snake->head.y += y;
	struct LinkedCube *new_cube = calloc(1, sizeof(struct LinkedCube));
	new_cube->cube = save_cube;
	new_cube->cube_back = snake->tail.cubes;

	if (mod(snake->head.x, SCREEN_WIDTH) != food->x || mod(snake->head.y, SCREEN_HEIGHT) != food->y)
	{
		struct LinkedCube *save_linkes_cube = snake->tail.cubes;
		for (size_t i = 0; i < snake->tail.len - 1; i++)
		{
			save_linkes_cube = save_linkes_cube->cube_back;
		}
		free(save_linkes_cube->cube_back);
		save_linkes_cube->cube_back = NULL;
	}
	else
	{
		u32 rand_coord = rand() % (40 * 24);
		food->x = (rand_coord % 40) * 10;
		food->y = (rand_coord / 40) * 10;
		snake->tail.len++;
	}

	snake->tail.cubes = new_cube;
}

//---------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	//---------------------------------------------------------------------------------
	// Init libs
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);
	srand(time(NULL));

	// Create screens
	C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
	u32 clrBlue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
	u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
	u32 clrClear = C2D_Color32(0x00, 0x00, 0x00, 0xFF);

	struct LinkedCube *fst_cube = calloc(1, sizeof(struct LinkedCube));
	struct LinkedCube *snd_cube = calloc(1, sizeof(struct LinkedCube));

	snd_cube->cube = (Cube){SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2};
	fst_cube->cube = (Cube){SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT / 2};
	fst_cube->cube_back = snd_cube;
	Tail tail = {fst_cube, 2};
	Snake snake = {Right, Right, {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, tail};

	u32 time = 0;
	u32 rand_coord = rand() % (40 * 24);
	Cube food = {(rand_coord % 40) * 10, (rand_coord / 40) * 10};

	// Main loop
	while (aptMainLoop())
	{
		time += 1;
		hidScanInput();

		u32 kDown = hidKeysDownRepeat();

		if (kDown & KEY_START)
		{
			break;
		}

		if (kDown & KEY_LEFT)
		{
			snake.change_direction = Left;
		}
		else if (kDown & KEY_RIGHT)
		{
			snake.change_direction = Right;
		}
		else if (kDown & KEY_UP)
		{
			snake.change_direction = Up;
		}
		else if (kDown & KEY_DOWN)
		{
			snake.change_direction = Down;
		}

		if (!(time % 5))
		{

			if (!is_the_opp(snake.direction, snake.change_direction))
			{
				snake.direction = snake.change_direction;
			}

			switch (snake.direction)
			{
			case Up:
				move_snake(&snake, 0, -10, &food);
				break;
			case Down:
				move_snake(&snake, 0, 10, &food);
				break;
			case Left:
				move_snake(&snake, -10, 0, &food);
				break;
			case Right:
				move_snake(&snake, 10, 0, &food);
				break;
			}
		}

		struct LinkedCube *save_linkes_cube = snake.tail.cubes;
		for (size_t i = 0; i < snake.tail.len; i++)
		{
			if (mod(snake.head.x, SCREEN_WIDTH) == mod(save_linkes_cube->cube.x, SCREEN_WIDTH) && mod(snake.head.y, SCREEN_HEIGHT) == mod(save_linkes_cube->cube.y, SCREEN_HEIGHT))
			{
				goto gameover;
			}
			save_linkes_cube = save_linkes_cube->cube_back;
		}

		printf("\x1b[2;1HLen: %ld\x1b[K", snake.tail.len + 1);

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(top, clrClear);
		C2D_SceneBegin(top);

		C2D_DrawRectangle(mod(snake.head.x, SCREEN_WIDTH), mod(snake.head.y, SCREEN_HEIGHT), 0, 10, 10, clrRed, clrRed, clrRed, clrRed);

		struct LinkedCube *navigate_cube = snake.tail.cubes;
		for (size_t i = 0; i < snake.tail.len; i++)
		{
			C2D_DrawRectangle(mod(navigate_cube->cube.x, SCREEN_WIDTH), mod(navigate_cube->cube.y, SCREEN_HEIGHT), 0, 10, 10, clrBlue, clrBlue, clrBlue, clrBlue);
			navigate_cube = navigate_cube->cube_back;
		}
		
		C2D_DrawRectangle(food.x, food.y, 0, 10, 10, clrGreen, clrGreen, clrGreen, clrGreen);

		C3D_FrameEnd(0);
	}

gameover:

	// Deinit libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
