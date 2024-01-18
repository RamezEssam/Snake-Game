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
    struct SnakeBlock  * head;
    struct SnakeBlock  * tail;
    Color color;
    int length;
}Snake;

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


void displaySnake(Snake * snake){
    struct SnakeBlock * p = snake->head;
    while(p != NULL){
        DrawRectangleRec(p->block, snake->color);
        p = p->next;
   }
}




void moveBlock(struct SnakeBlock * block, Snake * snake, int direction){

    block->prev_state = block->block;

    if (block == snake->head){
        
        if (block->direction == RIGHT && direction == LEFT) {
            block->block.x += block->block.width;
        } else if (block->direction == RIGHT && direction == UP) {
            block->block.y -= block->block.width;
            block->direction = direction;
        } else if (block->direction == RIGHT && direction == DOWN) {
            block->block.y += block->block.width;
            block->direction = direction;
        } else if(block->direction == RIGHT && direction == RIGHT){
            block->block.x += block->block.width;
            block->direction = direction;


        }else if (block->direction == LEFT && direction == RIGHT) {
            block->block.x -= block->block.width;
        }else if(block->direction == LEFT && direction == UP){
            block->block.y -= block->block.width;
            block->direction = direction;
        }else if(block->direction == LEFT && direction == DOWN){
            block->block.y += block->block.width;
            block->direction = direction;
        }else if(block->direction == LEFT && direction == LEFT){
            block->block.x -= block->block.width;
            block->direction = direction;


        }else if(block->direction == UP && direction == UP){
            block->block.y -= block->block.width;
            block->direction = direction;
        }else if(block->direction == UP && direction == LEFT){
            block->block.x -= block->block.width;
            block->direction = direction;
        }else if(block->direction == UP && direction == RIGHT){
            block->block.x += block->block.width;
            block->direction = direction;
        }else if(block->direction == UP && direction == DOWN){
            block->block.y -= block->block.width;
        
        
        
        }else if(block->direction == DOWN && direction == UP){
            block->block.y += block->block.width;
        }else if(block->direction == DOWN && direction == LEFT){
            block->block.x -= block->block.width;
            block->direction = direction;
        }else if(block->direction == DOWN && direction == RIGHT){
            block->block.x += block->block.width;
            block->direction = direction;
        }else if(block->direction == DOWN && direction == DOWN){
            block->block.y += block->block.width;
            block->direction = direction;
        }
    } else{
        block->block = block->prev->prev_state;
    }
    
    }



void moveSnake(Snake * snake, int direction){
    struct SnakeBlock * p = snake->head;
    while(p != NULL){
        moveBlock(p, snake, direction);
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

    return (snake->head->block.x <= 0) || (snake->head->block.x >= GetScreenWidth()) || (snake->head->block.y <= 0) || (snake->head->block.y >= GetScreenHeight());
}


int main(){
    srand(time(NULL));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME);

    SetTargetFPS(30);

    Vector2 midpoint = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};

    Snake * snake = initSnake(1, LEFT, BLUE, 10, midpoint);

    Fruit fruit = generateFruit(6, ORANGE);

    int score = 0;

    bool start_game = true;


    // Main game loop
    while (!WindowShouldClose()) {  // Detect window close button or ESC key



        while(!hasCollidedWithSelf(snake) && !hasCollidedWithBoundary(snake) && start_game){
                // Update
            
            moveSnake(snake, snake->head->direction);
            if(IsKeyPressed(KEY_DOWN)){
                moveSnake(snake, DOWN);
            }
            if(IsKeyPressed(KEY_UP)){

                moveSnake(snake, UP);
            }
            if(IsKeyPressed(KEY_LEFT)){

                moveSnake(snake, LEFT);
            }
            if(IsKeyPressed(KEY_RIGHT)){

                moveSnake(snake, RIGHT);
            }

            if(CheckCollisionCircleRec(fruit.center,(float)fruit.radius, snake->head->block)){
                growSnake(snake);
                fruit = generateFruit(fruit.radius, fruit.color);
                score ++;
            }
            
            // Draw
            BeginDrawing();

            ClearBackground(DARKGRAY);    // Clear background color

            DrawCircle(fruit.center.x, fruit.center.y, fruit.radius, fruit.color);

            DrawText(TextFormat("Score: %d", score), 10, 10, 25, GREEN);

            displaySnake(snake);


            EndDrawing();

        }

        start_game = false;

        BeginDrawing();

        displaySnake(snake);

        DrawText(TextFormat(GAME_OVER), midpoint.x-280, midpoint.y-50, 80, RED);

        DrawText(TextFormat(START_SCRIPT), midpoint.x-280, midpoint.y+70, 20, BEIGE);

        EndDrawing();

        if(IsKeyPressed(KEY_SPACE)){
            free(snake);
            start_game = true;
            snake = initSnake(1, LEFT, BLUE, 10, midpoint);
            fruit = generateFruit(6, ORANGE);
            score = 0;
        }
        
    }

    // De-Initialization
    CloseWindow();  // Close window and OpenGL context
}