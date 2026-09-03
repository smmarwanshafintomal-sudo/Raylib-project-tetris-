#include "raylib.h"

#define ROWS 20
#define COLS 10
#define BLOCK_SIZE 30


// BOARD


int board[ROWS][COLS] = {0};



// PIECE STRUCT--

typedef struct
{
    int shape[4][4];
    int x;
    int y;
} Piece;



// ALL 7 TETRIS SHAPES


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



// DRAW CURRENT PIECE
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
                    PURPLE
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


// DRAW BOARD

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

            // Locked block
            if (board[row][col] == 1)
            {
                DrawRectangle(
                    x,
                    y,
                    BLOCK_SIZE,
                    BLOCK_SIZE,
                    PURPLE
                );
            }
            // Empty cell
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

            // Cell border
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


// CHECK VALID POSITION

int IsValidPosition(Piece *piece)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            // Only check occupied cells
            if (piece->shape[row][col] == 1)
            {
                int boardX = piece->x + col;
                int boardY = piece->y + row;

                // Left wall
                if (boardX < 0)
                {
                    return 0;
                }

                // Right wall
                if (boardX >= COLS)
                {
                    return 0;
                }

                // Top
                if (boardY < 0)
                {
                    return 0;
                }

                // Bottom
                if (boardY >= ROWS)
                {
                    return 0;
                }

                // Existing locked block
                if (board[boardY][boardX] == 1)
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}


// LOCK PIECE

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



// SPAWN RANDOM PIECE

void SpawnPiece(Piece *piece)
{
    // Pick random shape from 0 to 6
    int type = GetRandomValue(0, 6);

    // Copy selected shape into piece
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            piece->shape[row][col] =
                shapes[type][row][col];
        }
    }

    // Starting position
    piece->x = 3;
    piece->y = 0;
}


// ROTATE PIECE


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

        // small wall kick attempt
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


// MOVE PIECE DOWN

void MovePieceDown(Piece *piece)
{
    // Try moving down
    piece->y++;

    // If the new position is invalid
    if (!IsValidPosition(piece))
    {
        // Undo movement
        piece->y--;

        // Lock current piece
        LockPiece(piece);

        // Create a new random piece
        SpawnPiece(piece);
    }
}


// --------------------------------------------------
// MAIN
// --------------------------------------------------

int main(void)
{
    // --------------------------------------------------
    // WINDOW
    // --------------------------------------------------

    InitWindow(
        800,
        650,
        "MY NEW TETRIS"
    );

    SetTargetFPS(60);


    // --------------------------------------------------
    // RANDOM SEED
    // --------------------------------------------------

    SetRandomSeed(
        (unsigned int)GetTime()
    );


    // --------------------------------------------------
    // CURRENT PIECE
    // --------------------------------------------------

    Piece piece;

    SpawnPiece(&piece);


    // --------------------------------------------------
    // FALL TIMER
    // --------------------------------------------------

    float fallTimer = 0.0f;

    float fallSpeed = 0.5f;


    // --------------------------------------------------
    // GAME LOOP
    // --------------------------------------------------

while (!WindowShouldClose())
{
    // Timer
    fallTimer += GetFrameTime();


    // ==============================================
    // MOVE LEFT
    // ==============================================

    if (IsKeyPressed(KEY_LEFT))
    {
        piece.x--;

        if (!IsValidPosition(&piece))
        {
            piece.x++;
        }
    }


    // ==============================================
    // MOVE RIGHT
    // ==============================================

    if (IsKeyPressed(KEY_RIGHT))
    {
        piece.x++;

        if (!IsValidPosition(&piece))
        {
            piece.x--;
        }
    }


  
    // ROTATE


    if (IsKeyPressed(KEY_UP))
    {
        RotatePiece(&piece);
    }

\
    // SOFT DROP


    if (IsKeyPressed(KEY_DOWN))
    {
        MovePieceDown(&piece);
    }



    // AUTOMATIC FALLING


    if (fallTimer >= fallSpeed)
    {
        fallTimer = 0.0f;

        MovePieceDown(&piece);
    }


  
    // DRAW


    BeginDrawing();

    ClearBackground(BLACK);

    DrawText(
        "HELLO TETRIS",
        400,
        50,
        50,
        RED
    );

    DrawBoard();

    DrawPiece(&piece);

    EndDrawing();
}

    CloseWindow();

    return 0;
}
