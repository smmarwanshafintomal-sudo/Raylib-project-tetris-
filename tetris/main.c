#include "raylib.h"
#include <stdio.h>

#define ROWS 20
#define COLS 10
#define BLOCK_SIZE 30

int board[ROWS][COLS] = {0};

int score=0;
int gameover=0;
int level=1;
int gamestarted = 0;

typedef struct
{
    int shape[4][4];
    int x;
    int y;
    Color color;
} Piece;



int shapes[7][4][4] =
{
    // I
    {
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // O
    {
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // T
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // S
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // Z
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // J
    {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },

    // L
    {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }
};


// color for each piece
Color pieceColors[7] =
{
    VIOLET,     // I
    YELLOW,     // O
    PURPLE,     // T
    GREEN,      // S
    PINK,       // Z
    BLUE,       // J
    ORANGE      // L
};





void DrawPiece(Piece *piece)
{
    int startX = 50;
    int startY = 20;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (piece->shape[row][col] == 1)
            {
                int x = startX +
                        (piece->x + col) * BLOCK_SIZE;

                int y = startY +
                        (piece->y + row) * BLOCK_SIZE;

                DrawRectangle(
                    x,
                    y,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    piece->color
                );

                DrawRectangleLines(
                    x,
                    y,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    BLACK
                );
            }
        }
    }
}





Color lockedpiececolor={220,0,0,255};

void DrawBoard(void)
{
    int startX = 50;
    int startY = 20;

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            int x = startX + col * BLOCK_SIZE;
            int y = startY + row * BLOCK_SIZE;

            if (board[row][col] == 1)
            {
                DrawRectangle(
                    x,
                    y,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    lockedpiececolor
                );
            }

            else
            {
                DrawRectangle(
                    x,
                    y,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    DARKGRAY
                );
            }

            DrawRectangleLines(
                x,
                y,
                BLOCK_SIZE,
                BLOCK_SIZE,
                BLACK
            );
        }
    }
}






int IsValidPosition(Piece *piece)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (piece->shape[row][col] == 1)
            {
                int boardX = piece->x + col;
                int boardY = piece->y + row;

                if (boardX < 0)
                {
                    return 0;
                }

                if (boardX >= COLS)
                {
                    return 0;
                }

                if (boardY < 0)
                {
                    return 0;
                }

                if (boardY >= ROWS)
                {
                    return 0;
                }

                if (board[boardY][boardX] == 1)
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}





void LockPiece(Piece *piece)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (piece->shape[row][col] == 1)
            {
                int boardX = piece->x + col;
                int boardY = piece->y + row;

                if (boardX >= 0 &&
                    boardX < COLS &&
                    boardY >= 0 &&
                    boardY < ROWS)
                {
                    board[boardY][boardX] = 1;
                }
            }
        }
    }
}





// clear the line
int ClearLine()
{ 
    int clearedline=0;
    for(int row=0;row<ROWS;row++)
    {
        int isfull=1;
        for(int col=0;col<COLS;col++)
        {
            if(board[row][col]==0)
            isfull=0;
        }
        if(isfull==1)
        {
            clearedline++;
            for(int r=row;r>0;r--)
            {
                for(int col=0;col<COLS;col++)
                board[r][col]=board[r-1][col];
            }
            for(int col=0;col<COLS;col++)
            board[0][col]=0;
        }

    }
    return clearedline;
}





// spawning a new piece
void SpawnPiece(Piece *piece)
{
    int type = GetRandomValue(0, 6);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            piece->shape[row][col] =
                shapes[type][row][col];
        }
    }
    piece->x = 3;
    piece->y = 0;

    piece->color = pieceColors[type];
}




// restart the game
void RestartGame(Piece *piece)
{
    // Clear the board
    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            board[row][col] = 0;
        }
    }

    // Reset game variables
    score = 0;
    level = 1;
    gameover = 0;

    // Spawn a new piece
    SpawnPiece(piece);
}




void RotatePiece(Piece *piece)
{
    int rotated[4][4] = {0};
    int original[4][4] = {0};
    int originalX = piece->x;
    int originalY = piece->y;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            original[row][col] = piece->shape[row][col];
        }
    }

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            rotated[col][3 - row] = piece->shape[row][col];
        }
    }

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            piece->shape[row][col] = rotated[row][col];
        }
    }

    if (!IsValidPosition(piece))
    {
        for (int row = 0; row < 4; row++)
        {
            for (int col = 0; col < 4; col++)
            {
                piece->shape[row][col] = original[row][col];
            }
        }

        piece->x = originalX;
        piece->y = originalY;

        for (int offset = -1; offset <= 1; offset++)
        {
            piece->x = originalX + offset;

            if (IsValidPosition(piece))
            {
                return;
            }
        }

        piece->x = originalX;
        piece->y = originalY;
    }
}



void MovePieceDown(Piece *piece)
{

    piece->y++;

    if (!IsValidPosition(piece))
    {
    
        piece->y--;

        LockPiece(piece);

        score=score+100*ClearLine();
        level=1+score/200;

        SpawnPiece(piece);

        if(!IsValidPosition(piece))
        {
            gameover=1;
        }
    }
}






//------------------------------------------------------------------------------------------------------------
int main(void)
{

    InitWindow(800,650,"MY NEW TETRIS");

    InitAudioDevice();

    Music music = LoadMusicStream("music_2.mp3");
    Music gameover_sound = LoadMusicStream("heavenly_gameover.mp3");
    gameover_sound.looping=0;

    SetTargetFPS(60);

    SetRandomSeed((unsigned int)GetTime());

    Piece piece;

    SpawnPiece(&piece);

    float fallTimer = 0.0f;
    float fallSpeed = 0.5f;

int gameover_sound_playback=1;

while (!WindowShouldClose())
{

    // the user will exit
    if (IsKeyPressed(KEY_X))
    {
        break;
    }

    // game start

    if (gamestarted == 0 && IsKeyPressed(KEY_ENTER))

       {
           gamestarted = 1;
           PlayMusicStream(music);
       }
     // game over and restart
       if (gameover == 1 && IsKeyPressed(KEY_R))
     {
         RestartGame(&piece);
         PlayMusicStream(music);
     }
     if(gameover==1&& gameover_sound_playback==1)
     {
        PlayMusicStream(gameover_sound);
        gameover_sound_playback=0;
     }
    // game starts
    if (gamestarted == 1 && gameover == 0)
{
    fallTimer += GetFrameTime();

    fallSpeed = 0.5 - (level - 1) * 0.05;

    if (fallSpeed < 0.1)
        fallSpeed = 0.1;
    
    UpdateMusicStream(music);

    if (!IsMusicStreamPlaying(music))
    {
        PlayMusicStream(music);
    }
    
    if (IsKeyPressed(KEY_LEFT))
    {
        piece.x--;

        if (!IsValidPosition(&piece))
        {
            piece.x++;
        }
    }

    if (IsKeyPressed(KEY_RIGHT))
    {
        piece.x++;

        if (!IsValidPosition(&piece))
        {
            piece.x--;
        }
    }

    if (IsKeyPressed(KEY_UP))
    {
        RotatePiece(&piece);
    }

    if (IsKeyPressed(KEY_DOWN))
    {
        MovePieceDown(&piece);
    }

    if (fallTimer >= fallSpeed)
    {
        fallTimer = 0.0f;
        MovePieceDown(&piece);
    }
}

    BeginDrawing();

    ClearBackground(BLACK);

    if (gamestarted == 0)
{
    DrawText("MY NEW TETRIS", 220, 150, 50, RED);
    DrawText("PRESS ENTER TO START", 220, 300, 30, GREEN);
}

    else if(gameover==1)
    {
        DrawText("GAME OVER!",250,200,50,RED);
        char arr[50];
        sprintf(arr,"SCORE=%d",score);
        DrawText(arr,250,255,35,BLUE);
        
        DrawText("PRESS R TO RESTART", 220, 320, 25, GREEN);

         StopMusicStream(music);
         UpdateMusicStream(gameover_sound);
    }

    else
   {
    DrawText(
        "HELLO TETRIS",
        400,
        50,
        50,
        RED
    );

    char arr[50];
    sprintf(arr,"SCORE=%d",score);
    DrawText(arr,400,150,40,BLUE);

    char brr[50];
    sprintf(brr,"LEVEL=%d",level);
    DrawText(brr,400,250,40,YELLOW);
     
    DrawText("Press X to exit",400,350,40,GREEN);
    

    DrawBoard();

    DrawPiece(&piece);
   }  


    EndDrawing();

}

    UnloadMusicStream(music);
    UnloadMusicStream(gameover_sound);

    CloseAudioDevice();

    CloseWindow();

    return 0;
}
