#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "include/raylib.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define WINDOW_NAME "Snake Game"
#define GAME_OVER "Game Over"
#define START_SCRIPT "Press the space key to start the game again"
#define SNAKE_MOVE_DELAY 0.07f
#define SNAKE_SPEED_UP 10 // speed up after every this number blocks increase in snake's length
#define SNAKE_MOVE_DELAY_SPEED_UP 0.009f // speed up by this much after SNAKE_SPEED_UP blocks increase in snake's length 
#define SNAKE_MIN_MOVE_DELAY 0.008f 

enum Orientation{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
};

struct SnakeBlock
{   
    Rectangle block;
    int direction;
    struct SnakeBlock * next;
    struct SnakeBlock * prev;
    Rectangle prev_state;
    
}SnakeBlock;

typedef struct Snake
{
    struct SnakeBlock * head;
    struct SnakeBlock * tail;
    Color color;
    int length;
    float moveTimer;   // Accrued time since last move
    float moveDelay;   // Time required between moves (e.g., 0.1 seconds)
} Snake;

typedef struct Fruit
{
    Vector2 center;
    Color color;
    int radius;
}Fruit;


Snake *initSnake(int length, int orientation, Color color, int thickness, Vector2 midpoint) {
    Snake *newSnake = (Snake *)malloc(sizeof(Snake));
    newSnake->length = length;
    newSnake->color = color;

    struct SnakeBlock *head = NULL;
    struct SnakeBlock *tail = NULL;

    for (int i = 0; i < length; ++i) {
        struct SnakeBlock *block = (struct SnakeBlock *)malloc(sizeof(SnakeBlock));
        Rectangle rec;

        if (orientation == RIGHT) {
            rec = (Rectangle){midpoint.x - (i * thickness), midpoint.y, thickness, thickness};
        } else if (orientation == LEFT) {
            rec = (Rectangle){midpoint.x + (i * thickness), midpoint.y, thickness, thickness};
        } else if (orientation == DOWN) {
            rec = (Rectangle){midpoint.x, midpoint.y - (i * thickness), thickness, thickness};
        } else if (orientation == UP) {
            rec = (Rectangle){midpoint.x, midpoint.y + (i * thickness), thickness, thickness};
        }

        block->block = rec;
        block->direction = orientation;
        block->prev_state = rec;
        block->next = NULL;

        if (head == NULL) {
            block->prev = NULL;
            head = block;
            tail = block;
        } else {
            tail->next = block;
            block->prev = tail;
            tail = block;
        }
    }

    newSnake->head = head;
    newSnake->tail = tail;

    return newSnake;
}

void deInitSnake(Snake *snake)
{
    // Safety check to prevent crashing if a NULL pointer is passed
    if (snake == NULL) return;

    struct SnakeBlock *p = snake->head;
    while (p != NULL) 
    {
        // Keep track of the next block before freeing the current one
        struct SnakeBlock *nextBlock = p->next;
        
        // Free the current block
        free(p);
        
        // Move to the next block in the chain
        p = nextBlock;
    }

    // Finally, free the main Snake structure container
    free(snake);
}


void displaySnake(Snake * snake){
    struct SnakeBlock * p = snake->head;
    while(p != NULL){
        DrawRectangleRec(p->block, snake->color);
        p = p->next;
   }
}






void moveSnake(Snake * snake, int direction) {
    // 1. Save states from back to front
    struct SnakeBlock * p = snake->tail;
    while(p != NULL) {
        p->prev_state = p->block;
        p = p->prev;
    }

    // 2. Move the head based on direction
    if (direction == RIGHT)      snake->head->block.x += snake->head->block.width;
    else if (direction == LEFT)  snake->head->block.x -= snake->head->block.width;
    else if (direction == UP)    snake->head->block.y -= snake->head->block.width;
    else if (direction == DOWN)  snake->head->block.y += snake->head->block.width;
    
    snake->head->direction = direction;

    // 3. Make body blocks follow the previous block's old position
    p = snake->head->next;
    while (p != NULL) {
        p->block = p->prev->prev_state;
        p = p->next;
    }
}

void growSnake(Snake * snake){

    struct SnakeBlock *block = (struct SnakeBlock *)malloc(sizeof(SnakeBlock));

    block->next = NULL;
    block->block = snake->tail->prev_state;
    snake->tail->next = block;
    block->prev = snake->tail;
    snake->tail = block;
	
	if (++snake->length % SNAKE_SPEED_UP == 0)
	{
		snake->moveDelay -=  SNAKE_MOVE_DELAY_SPEED_UP;
		// Safety check: Prevent moveDelay from hitting 0 or going negative!
		if (snake->moveDelay < SNAKE_MIN_MOVE_DELAY) 
		{
			snake->moveDelay = SNAKE_MIN_MOVE_DELAY; 
		}
	}

}

Fruit generateFruit(int radius, Color color){

    int r_x = rand() % (GetScreenWidth() - 100 + 1);
    int r_y = rand() % (GetScreenHeight() - 100 + 1);

    return (Fruit){(Vector2){r_x, r_y}, color, radius};
}

bool hasCollidedWithSelf(Snake * snake){
    struct SnakeBlock * phead = snake->head;
    struct SnakeBlock * pfirst = phead->next;

    while(pfirst != NULL){
        if(CheckCollisionRecs(pfirst->block, phead->block)){
            return true;
        }
        pfirst = pfirst->next;
    }

    return false;

}

bool hasCollidedWithBoundary(Snake * snake){
    return (snake->head->block.x < 0) || 
           (snake->head->block.x + snake->head->block.width > GetScreenWidth()) || 
           (snake->head->block.y < 0) || 
           (snake->head->block.y + snake->head->block.height > GetScreenHeight());
}


int main(){
    srand(time(NULL));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME);
    SetTargetFPS(60);

    Vector2 midpoint = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    Snake * snake = initSnake(1, LEFT, BLUE, 10, midpoint);
    snake->moveTimer = 0.0f;
    snake->moveDelay = SNAKE_MOVE_DELAY;

    Fruit fruit = generateFruit(6, ORANGE);
    int score = 0;
    
    // Use a boolean or enum to track if the game is currently active
    bool isGameOver = false;

    // SINGLE MAIN GAME LOOP
    while (!WindowShouldClose()) {  // Now ESC and close buttons always work!

        // ==================== UPDATE PHASE ====================
        if (!isGameOver) {
            // 1. Parse Input
            if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) && snake->head->direction != UP) {
                snake->head->direction = DOWN;
            }
            if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) && snake->head->direction != DOWN) {
                snake->head->direction = UP;
            }
            if ((IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) && snake->head->direction != RIGHT) {
                snake->head->direction = LEFT;
            }
            if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && snake->head->direction != LEFT) {
                snake->head->direction = RIGHT;
            }
            
            // 2. Accumulate movement timer
            snake->moveTimer += GetFrameTime();
            if (snake->moveTimer >= snake->moveDelay) {
                moveSnake(snake, snake->head->direction);
                snake->moveTimer = 0.0f;
            }

            // 3. Fruit Collision
            if(CheckCollisionCircleRec(fruit.center, (float)fruit.radius, snake->head->block)){
                growSnake(snake);
                fruit = generateFruit(fruit.radius, fruit.color);
                score++;
            }

            // 4. Check Defeat Conditions
            if (hasCollidedWithSelf(snake) || hasCollidedWithBoundary(snake)) {
                isGameOver = true;
            }
        } 
        else {
            // We are in Game Over mode. Check for restart input.
            if(IsKeyPressed(KEY_SPACE)){
                deInitSnake(snake); // Note: ideally free body blocks too!
                snake = initSnake(1, LEFT, BLUE, 10, midpoint);
                snake->moveTimer = 0.0f;
                snake->moveDelay = SNAKE_MOVE_DELAY;
                fruit = generateFruit(6, ORANGE);
                score = 0;
                isGameOver = false; // Go back to playing state
            }
        }

        // ==================== DRAW PHASE ====================
        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (!isGameOver) {
            // Draw regular gameplay
            DrawCircle(fruit.center.x, fruit.center.y, fruit.radius, fruit.color);
            DrawText(TextFormat("Score: %d", score), 10, 10, 25, GREEN);
            displaySnake(snake);
        } 
        else {
            // Draw Game Over Screen
            displaySnake(snake);
            DrawText(TextFormat(GAME_OVER), midpoint.x-280, midpoint.y-50, 80, RED);
            DrawText(TextFormat(START_SCRIPT), midpoint.x-280, midpoint.y+70, 20, BEIGE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}