#include "raylib.h"
#include <stdio.h>
#include <string.h>

#define ROWS 20
#define COLS 10
#define BLOCK_SIZE 30
#define MAX_SCORES 3
#define SCORE_FILE "scores.txt"
#define PLAYER_NAME_MAX 24

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 700

int board[ROWS][COLS] = {0};
int score = 0;
int gameover = 0;
int level = 1;
int nextpiecetype = 0;
int muted = 0;

/* -------------------- Game states -------------------- */
typedef enum {
    STATE_MENU,
    STATE_DIFFICULTY,
    STATE_HOW_TO_PLAY,
    STATE_CREDITS,
    STATE_LEADERBOARD,
    STATE_NAME_INPUT,
    STATE_PLAYING,
    STATE_SCORE_RESULT
} GameState;

GameState gameState = STATE_MENU;

/* -------------------- Difficulty -------------------- */
typedef enum {
    EASY,
    MEDIUM,
    HARD
} Difficulty;

Difficulty difficulty = MEDIUM;

const char *DifficultyName(Difficulty d)
{
    if (d == EASY) return "EASY";
    if (d == HARD) return "HARD";
    return "MEDIUM";
}

/* -------------------- Piece -------------------- */
typedef struct
{
    int shape[4][4];
    int x;
    int y;
    int type;
    Color color;
} Piece;

int shapes[7][4][4] =
{
    /* I */
    {{1,1,1,1},{0,0,0,0},{0,0,0,0},{0,0,0,0}},
    /* O */
    {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
    /* T */
    {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
    /* S */
    {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
    /* Z */
    {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
    /* J */
    {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
    /* L */
    {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}
};

Color pieceColors[7] =
{
    VIOLET, YELLOW, PURPLE, GREEN, PINK, BLUE, ORANGE
};


/* -------------------- Leaderboard -------------------- */
typedef struct
{
    char name[PLAYER_NAME_MAX];
    int score;
} ScoreEntry;

ScoreEntry leaderboard[MAX_SCORES];
int leaderboardCount = 0;

char playerName[PLAYER_NAME_MAX] = "";
int playerNameLength = 0;
bool scoreSavedForCurrentGame = false;

void SortLeaderboard(void);

void ResetLeaderboardMemory(void)
{
    leaderboardCount = 0;
    for (int i = 0; i < MAX_SCORES; i++)
    {
        leaderboard[i].name[0] = '\0';
        leaderboard[i].score = 0;
    }
}

void SaveLeaderboard(void)
{
    FILE *file = fopen(SCORE_FILE, "w");
    if (!file) return;

    for (int i = 0; i < leaderboardCount && i < MAX_SCORES; i++)
        fprintf(file, "%s\t%d\n", leaderboard[i].name, leaderboard[i].score);

    fclose(file);
}

void LoadLeaderboard(void)
{
    ResetLeaderboardMemory();

    FILE *file = fopen(SCORE_FILE, "r");
    if (!file) return;

    char line[128];

    while (leaderboardCount < MAX_SCORES && fgets(line, sizeof(line), file))
    {
        char loadedName[PLAYER_NAME_MAX] = "";
        int loadedScore;
        char *tab = strrchr(line, '\t');

        if (tab)
        {
            *tab = '\0';
            if (sscanf(tab + 1, "%d", &loadedScore) != 1)
                continue;

            strncpy(loadedName, line, PLAYER_NAME_MAX - 1);
            loadedName[PLAYER_NAME_MAX - 1] = '\0';
        }
        else
        {
            /* Backward-compatible with the original: NAME SCORE format. */
            if (sscanf(line, "%23s %d", loadedName, &loadedScore) != 2)
                continue;
        }

        size_t len = strlen(loadedName);
        while (len > 0 && (loadedName[len - 1] == '\n' || loadedName[len - 1] == '\r'))
            loadedName[--len] = '\0';

        if (loadedName[0] == '\0') continue;

        strncpy(leaderboard[leaderboardCount].name, loadedName, PLAYER_NAME_MAX - 1);
        leaderboard[leaderboardCount].name[PLAYER_NAME_MAX - 1] = '\0';
        leaderboard[leaderboardCount].score = loadedScore;
        leaderboardCount++;
    }

    fclose(file);
}

void SortLeaderboard(void)
{
    for (int i = 0; i < leaderboardCount - 1; i++)
    {
        for (int j = i + 1; j < leaderboardCount; j++)
        {
            if (leaderboard[j].score > leaderboard[i].score)
            {
                ScoreEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
}

void AddScore(const char *name, int finalScore)
{
    if (finalScore <= 0) return;

    if (leaderboardCount < MAX_SCORES)
    {
        strncpy(leaderboard[leaderboardCount].name, name, PLAYER_NAME_MAX - 1);
        leaderboard[leaderboardCount].name[PLAYER_NAME_MAX - 1] = '\0';
        leaderboard[leaderboardCount].score = finalScore;
        leaderboardCount++;
    }
    else
    {
        SortLeaderboard();
        if (finalScore <= leaderboard[leaderboardCount - 1].score) return;

        strncpy(leaderboard[leaderboardCount - 1].name, name, PLAYER_NAME_MAX - 1);
        leaderboard[leaderboardCount - 1].name[PLAYER_NAME_MAX - 1] = '\0';
        leaderboard[leaderboardCount - 1].score = finalScore;
    }

    SortLeaderboard();
    SaveLeaderboard();
}

/* -------------------- Assets -------------------- */
Texture2D blockTexture = {0};
Font gamefont = {0};
Music music = {0};
Music gameoverMusic = {0};
Sound moveSound = {0};
Sound rotateSound = {0};
Sound lineSound = {0};
Sound dropSound = {0};
Sound gameoverSound = {0};

bool blockTextureLoaded = false;
bool fontLoaded = false;
bool musicLoaded = false;
bool gameoverMusicLoaded = false;
bool moveSoundLoaded = false;
bool rotateSoundLoaded = false;
bool lineSoundLoaded = false;
bool dropSoundLoaded = false;
bool gameoverSoundLoaded = false;

void PlayEffect(Sound *sound, bool loaded)
{
    if (!muted && loaded)
        PlaySound(*sound);
}

void SetGameMusicVolume(void)
{
    if (musicLoaded)
        SetMusicVolume(music, muted ? 0.0f : 0.45f);
    if (gameoverMusicLoaded)
        SetMusicVolume(gameoverMusic, muted ? 0.0f : 0.55f);
}

/* -------------------- Drawing -------------------- */
void DrawBlockWithShade(int x, int y, Color color)
{
    if (blockTextureLoaded)
    {
        DrawTexturePro(blockTexture,
                       (Rectangle){0, 0, (float)blockTexture.width, (float)blockTexture.height},
                       (Rectangle){(float)x, (float)y, BLOCK_SIZE, BLOCK_SIZE},
                       (Vector2){0, 0}, 0.0f, color);
        DrawRectangleLines(x, y, BLOCK_SIZE, BLOCK_SIZE, BLACK);
        return;
    }

    DrawRectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, color);

    Color lightShade = {
        (unsigned char)((color.r + 255) / 2),
        (unsigned char)((color.g + 255) / 2),
        (unsigned char)((color.b + 255) / 2),
        255
    };
    Color darkShade = {
        (unsigned char)(color.r / 2),
        (unsigned char)(color.g / 2),
        (unsigned char)(color.b / 2),
        255
    };

    DrawRectangle(x, y, BLOCK_SIZE, 5, lightShade);
    DrawRectangle(x, y + BLOCK_SIZE - 5, BLOCK_SIZE, 5, darkShade);
    DrawRectangleLines(x, y, BLOCK_SIZE, BLOCK_SIZE, BLACK);
}

void DrawPiece(Piece *piece)
{
    int startX = 50;
    int startY = 20;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (piece->shape[row][col])
            {
                int x = startX + (piece->x + col) * BLOCK_SIZE;
                int y = startY + (piece->y + row) * BLOCK_SIZE;
                DrawBlockWithShade(x, y, piece->color);
            }
        }
    }
}

void DrawNextBlockPreview(int type)
{
    int startX = 535;
    int startY = 440;

    DrawRectangleLinesEx((Rectangle){515, 420, 180, 150}, 3, LIGHTGRAY);
    DrawText("NEXT PIECE", 530, 390, 28, WHITE);

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (shapes[type][row][col])
                DrawBlockWithShade(startX + col * BLOCK_SIZE,
                                   startY + row * BLOCK_SIZE,
                                   pieceColors[type]);
        }
    }
}

void DrawBoard(void)
{
    int startX = 50;
    int startY = 20;

    DrawRectangleLinesEx((Rectangle){45, 15, COLS * BLOCK_SIZE + 10,
                                    ROWS * BLOCK_SIZE + 10},
                         5, (Color){0, 50, 100, 255});

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            int x = startX + col * BLOCK_SIZE;
            int y = startY + row * BLOCK_SIZE;

            if (board[row][col] != 0)
            {
                int type = board[row][col] - 1;
                if (type >= 0 && type < 7)
                    DrawBlockWithShade(x, y, pieceColors[type]);
            }
            else
            {
                DrawRectangle(x, y, BLOCK_SIZE, BLOCK_SIZE, DARKGRAY);
                DrawRectangleLines(x, y, BLOCK_SIZE, BLOCK_SIZE, BLACK);
            }
        }
    }
}

void DrawTextCentered(const char *text, int y, int fontSize, Color color)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, (SCREEN_WIDTH - width) / 2, y, fontSize, color);
}

void DrawBackground(void)
{
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                           (Color){20, 20, 55, 255},
                           (Color){0, 0, 0, 255});
}

/* -------------------- Game mechanics -------------------- */
int IsValidPosition(Piece *piece)
{
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            if (!piece->shape[row][col]) continue;

            int boardX = piece->x + col;
            int boardY = piece->y + row;

            if (boardX < 0 || boardX >= COLS ||
                boardY < 0 || boardY >= ROWS)
                return 0;

            if (board[boardY][boardX] != 0)
                return 0;
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
            if (!piece->shape[row][col]) continue;

            int boardX = piece->x + col;
            int boardY = piece->y + row;

            if (boardX >= 0 && boardX < COLS &&
                boardY >= 0 && boardY < ROWS)
                board[boardY][boardX] = piece->type + 1;
        }
    }
}

int ClearLine(void)
{
    int cleared = 0;

    for (int row = ROWS - 1; row >= 0; row--)
    {
        int full = 1;

        for (int col = 0; col < COLS; col++)
        {
            if (board[row][col] == 0)
            {
                full = 0;
                break;
            }
        }

        if (full)
        {
            cleared++;

            for (int r = row; r > 0; r--)
                for (int col = 0; col < COLS; col++)
                    board[r][col] = board[r - 1][col];

            for (int col = 0; col < COLS; col++)
                board[0][col] = 0;

            row++; /* re-check shifted row */
        }
    }

    return cleared;
}

void SpawnPiece(Piece *piece)
{
    int type = nextpiecetype;
    nextpiecetype = GetRandomValue(0, 6);

    piece->type = type;
    piece->x = 3;
    piece->y = 0;
    piece->color = pieceColors[type];

    memcpy(piece->shape, shapes[type], sizeof(piece->shape));
}

void RestartGame(Piece *piece)
{
    memset(board, 0, sizeof(board));
    score = 0;
    level = 1;
    gameover = 0;
    nextpiecetype = GetRandomValue(0, 6);
    SpawnPiece(piece);
}

void RotatePiece(Piece *piece)
{
    int rotated[4][4] = {0};
    int original[4][4];
    int originalX = piece->x;
    int originalY = piece->y;

    memcpy(original, piece->shape, sizeof(original));

    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            rotated[col][3 - row] = piece->shape[row][col];

    memcpy(piece->shape, rotated, sizeof(piece->shape));

    if (IsValidPosition(piece))
    {
        PlayEffect(&rotateSound, rotateSoundLoaded);
        return;
    }

    for (int offset = -1; offset <= 1; offset++)
    {
        piece->x = originalX + offset;
        if (IsValidPosition(piece))
        {
            PlayEffect(&rotateSound, rotateSoundLoaded);
            return;
        }
    }

    memcpy(piece->shape, original, sizeof(piece->shape));
    piece->x = originalX;
    piece->y = originalY;
}

void MovePieceDown(Piece *piece)
{
    piece->y++;

    if (!IsValidPosition(piece))
    {
        piece->y--;
        LockPiece(piece);
        PlayEffect(&dropSound, dropSoundLoaded);

        int cleared = ClearLine();

        if (cleared == 1) score += 100;
        else if (cleared == 2) score += 300;
        else if (cleared == 3) score += 500;
        else if (cleared == 4) score += 800;

        if (cleared > 0)
            PlayEffect(&lineSound, lineSoundLoaded);

        level = 1 + score / 200;
        SpawnPiece(piece);

        if (!IsValidPosition(piece))
            gameover = 1;
    }
}

float GetBaseFallSpeed(void)
{
    float speed;

    if (difficulty == EASY)
        speed = 0.75f;
    else if (difficulty == HARD)
        speed = 0.35f;
    else
        speed = 0.50f;

    speed -= (level - 1) * 0.05f;

    if (speed < 0.08f)
        speed = 0.08f;

    return speed;
}

/* -------------------- Game lifecycle -------------------- */
void StartGame(Piece *piece)
{
    RestartGame(piece);
    gameover = 0;
    gameState = STATE_PLAYING;

    if (gameoverMusicLoaded)
        StopMusicStream(gameoverMusic);

    if (musicLoaded)
    {
        SetGameMusicVolume();
        PlayMusicStream(music);
    }
}

/* -------------------- Menu drawing -------------------- */
void DrawMenu(int selected)
{
    DrawTextCentered("TETRIS", 85, 72, RED);
    DrawTextCentered("A COMPLETE TETRIS EXPERIENCE", 165, 24, LIGHTGRAY);

    const char *items[] = {
        "START GAME",
        "DIFFICULTY",
        "HOW TO PLAY",
        "LEADERBOARD",
        "CREDITS",
        "EXIT"
    };

    for (int i = 0; i < 6; i++)
    {
        int y = 225 + i * 55;
        Color c = (i == selected) ? GREEN : WHITE;
        DrawTextCentered(items[i], y, 34, c);
    }

    DrawTextCentered("Use UP/DOWN and ENTER to select", 580, 22, GRAY);
    DrawTextCentered(TextFormat("Difficulty: %s", DifficultyName(difficulty)),
                     615, 22, YELLOW);
}

void DrawDifficultyMenu(int selected)
{
    DrawTextCentered("SELECT DIFFICULTY", 90, 60, YELLOW);

    const char *items[] = {"EASY", "MEDIUM", "HARD"};
    for (int i = 0; i < 3; i++)
    {
        Color c = (i == selected) ? GREEN : WHITE;
        DrawTextCentered(items[i], 230 + i * 70, 40, c);
    }

    DrawTextCentered("UP/DOWN: select    ENTER: confirm    ESC: back", 550, 24, LIGHTGRAY);
}

void DrawHowToPlay(void)
{
    DrawTextCentered("HOW TO PLAY", 55, 55, YELLOW);

    DrawText("LEFT / RIGHT", 100, 155, 28, WHITE);
    DrawText("Move the falling piece", 370, 155, 28, LIGHTGRAY);

    DrawText("UP", 100, 205, 28, WHITE);
    DrawText("Rotate the piece", 370, 205, 28, LIGHTGRAY);

    DrawText("DOWN", 100, 255, 28, WHITE);
    DrawText("Soft drop", 370, 255, 28, LIGHTGRAY);

    DrawText("SPACE", 100, 305, 28, WHITE);
    DrawText("Fast drop", 370, 305, 28, LIGHTGRAY);

    DrawText("X", 100, 355, 28, WHITE);
    DrawText("Exit the game", 370, 355, 28, LIGHTGRAY);

    DrawText("Objective", 100, 430, 30, GREEN);
    DrawText("Complete horizontal lines to score points.", 250, 430, 25, LIGHTGRAY);
    DrawText("The level increases as your score rises.", 250, 465, 25, LIGHTGRAY);

    DrawTextCentered("ENTER / ESC: back to menu", 590, 25, YELLOW);
}

void DrawCredits(void)
{
    DrawTextCentered("CREDITS", 70, 60, YELLOW);

    DrawTextCentered("TETRIS", 180, 36, WHITE);
    DrawTextCentered("Programming & Game Design: S.M. Marwan Shafin Tomal", 240, 24, LIGHTGRAY);
    DrawTextCentered("Engine / Framework: raylib", 285, 24, LIGHTGRAY);
    DrawTextCentered("Font: Fredoka-Bold.ttf (external asset)", 330, 24, LIGHTGRAY);
    DrawTextCentered("Music: music.mp3 (external asset)", 375, 24, LIGHTGRAY);
    DrawTextCentered("Game Over Music: heavenly_gameover.mp3 (external asset)", 420, 24, LIGHTGRAY);
    DrawTextCentered("SFX / Texture: project assets", 465, 24, LIGHTGRAY);

    DrawTextCentered("See the project asset folder for the original asset sources/licenses.",
                     535, 20, GRAY);
    DrawTextCentered("ENTER / ESC: back to menu", 595, 25, YELLOW);
}

void DrawNameInput(void)
{
    DrawTextCentered("ENTER YOUR NAME", 105, 55, YELLOW);
    DrawTextCentered("Your score will be saved to the leaderboard", 175, 24, LIGHTGRAY);
    DrawRectangleLinesEx((Rectangle){180, 255, 540, 70}, 3, GREEN);

    const char *displayName = playerNameLength > 0 ? playerName : "_";
    DrawTextCentered(displayName, 272, 34, WHITE);

    DrawTextCentered("Type your name and press ENTER", 390, 27, LIGHTGRAY);
    DrawTextCentered("BACKSPACE: delete    ESC: cancel", 435, 23, GRAY);
}

void DrawScoreResult(void)
{
    DrawTextCentered("GAME OVER", 85, 70, RED);
    DrawTextCentered("YOUR RESULT", 175, 34, YELLOW);
    DrawTextCentered(playerName, 235, 38, WHITE);
    DrawTextCentered(TextFormat("SCORE: %d", score), 310, 50, BLUE);
    DrawTextCentered(TextFormat("LEVEL: %d", level), 370, 32, YELLOW);

    if (leaderboardCount > 0 && score >= leaderboard[0].score)
        DrawTextCentered("NEW HIGH SCORE!", 440, 30, GREEN);
    else
        DrawTextCentered("Your score has been saved.", 440, 26, LIGHTGRAY);

    DrawTextCentered("ENTER: view leaderboard", 520, 30, WHITE);
    DrawTextCentered("ESC: return to menu", 565, 25, GRAY);
}

void DrawLeaderboard(void)
{
    DrawTextCentered("LEADERBOARD", 65, 60, YELLOW);

    if (leaderboardCount == 0)
    {
        DrawTextCentered("No scores saved yet.", 250, 30, LIGHTGRAY);
    }
    else
    {
        for (int i = 0; i < leaderboardCount; i++)
        {
            int y = 150 + i * 42;
            DrawText(TextFormat("%2d.", i + 1), 220, y, 28, WHITE);
            DrawText(leaderboard[i].name, 300, y, 28, LIGHTGRAY);
            DrawText(TextFormat("%d", leaderboard[i].score), 560, y, 28, GREEN);
        }
    }

    DrawTextCentered("ENTER / ESC: back to menu", 625, 25, YELLOW);
}

void DrawPlaying(Piece *piece)
{
    DrawText("TETRIS", 400, 45, 42, RED);
    DrawText(TextFormat("SCORE: %d", score), 400, 120, 32, BLUE);
    DrawText(TextFormat("LEVEL: %d", level), 400, 165, 32, YELLOW);
    DrawText(TextFormat("DIFFICULTY: %s", DifficultyName(difficulty)),
             400, 210, 24, GREEN);

    DrawText("M: mute/unmute", 400, 255, 22, LIGHTGRAY);
    DrawText("X: exit", 400, 285, 22, LIGHTGRAY);

    DrawBoard();
    DrawPiece(piece);
    DrawNextBlockPreview(nextpiecetype);
}





















//Main
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "WELCOME TO TETRIS");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    SetRandomSeed((unsigned int)GetTime());

    if (FileExists("Fredoka-Bold.ttf"))
    {
        gamefont = LoadFont("Fredoka-Bold.ttf");
        fontLoaded = true;
    }


    if (FileExists("music.mp3"))
    {
        music = LoadMusicStream("music.mp3");
        music.looping = true;
        musicLoaded = true;
    }

    if (FileExists("heavenly_gameover.mp3"))
    {
        gameoverMusic = LoadMusicStream("heavenly_gameover.mp3");
        gameoverMusic.looping = false;
        gameoverMusicLoaded = true;
    }

    LoadLeaderboard();

    Piece piece;
    nextpiecetype = GetRandomValue(0, 6);
    SpawnPiece(&piece);

    float fallTimer = 0.0f;
    int menuSelection = 0;
    int difficultySelection = (int)difficulty;

    while (!WindowShouldClose())
    {
        /* -------------------- Global controls -------------------- */
        if (IsKeyPressed(KEY_M))
{
    muted = !muted;
    SetGameMusicVolume();
}

        if (gameState == STATE_MENU)
        {
            if (IsKeyPressed(KEY_DOWN))
                menuSelection = (menuSelection + 1) % 6;
            if (IsKeyPressed(KEY_UP))
                menuSelection = (menuSelection + 5) % 6;

            if (IsKeyPressed(KEY_ENTER))
            {
                if (menuSelection == 0)
                {
                    playerName[0] = '\0';
                    playerNameLength = 0;
                    scoreSavedForCurrentGame = false;
                    gameState = STATE_NAME_INPUT;
                }
                else if (menuSelection == 1)
                {
                    difficultySelection = (int)difficulty;
                    gameState = STATE_DIFFICULTY;
                }
                else if (menuSelection == 2)
                    gameState = STATE_HOW_TO_PLAY;
                else if (menuSelection == 3)
                    gameState = STATE_LEADERBOARD;
                else if (menuSelection == 4)
                    gameState = STATE_CREDITS;
                else if (menuSelection == 5)
                    break;
            }
        }
        else if (gameState == STATE_DIFFICULTY)
        {
            if (IsKeyPressed(KEY_DOWN))
                difficultySelection = (difficultySelection + 1) % 3;
            if (IsKeyPressed(KEY_UP))
                difficultySelection = (difficultySelection + 2) % 3;

            if (IsKeyPressed(KEY_ENTER))
            {
                difficulty = (Difficulty)difficultySelection;
                gameState = STATE_MENU;
            }
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = STATE_MENU;
        }
        else if (gameState == STATE_HOW_TO_PLAY ||
                 gameState == STATE_CREDITS ||
                 gameState == STATE_LEADERBOARD)
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
                gameState = STATE_MENU;
        }
        else if (gameState == STATE_NAME_INPUT)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if (key >= 32 && key <= 126 && playerNameLength < PLAYER_NAME_MAX - 1)
                {
                    playerName[playerNameLength++] = (char)key;
                    playerName[playerNameLength] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && playerNameLength > 0)
                playerName[--playerNameLength] = '\0';

            if (IsKeyPressed(KEY_ENTER) && playerNameLength > 0)
            {
                StartGame(&piece);
                fallTimer = 0.0f;
                scoreSavedForCurrentGame = false;
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                playerName[0] = '\0';
                playerNameLength = 0;
                gameState = STATE_MENU;
            }
        }
        else if (gameState == STATE_PLAYING)
        {
            if (IsKeyPressed(KEY_X))
            {
                if (musicLoaded) StopMusicStream(music);
                gameState = STATE_MENU;
            }

            if (musicLoaded)
            {
                UpdateMusicStream(music);
                SetGameMusicVolume();
            }

            if (IsKeyPressed(KEY_LEFT))
            {
                piece.x--;
                if (!IsValidPosition(&piece)) piece.x++;
                else PlayEffect(&moveSound, moveSoundLoaded);
            }

            if (IsKeyPressed(KEY_RIGHT))
            {
                piece.x++;
                if (!IsValidPosition(&piece)) piece.x--;
                else PlayEffect(&moveSound, moveSoundLoaded);
            }

            if (IsKeyPressed(KEY_UP))
                RotatePiece(&piece);

            if (IsKeyPressed(KEY_DOWN))
            {
                MovePieceDown(&piece);
                fallTimer = 0.0f;
            }

            float fallSpeed = GetBaseFallSpeed();
            if (IsKeyDown(KEY_SPACE))
                fallSpeed = 0.035f;

            fallTimer += GetFrameTime();
            if (fallTimer >= fallSpeed)
            {
                fallTimer = 0.0f;
                MovePieceDown(&piece);
            }

            if (gameover)
            {
                if (musicLoaded) StopMusicStream(music);

                if (gameoverMusicLoaded)
                {
                    SetGameMusicVolume();
                    PlayMusicStream(gameoverMusic);
                }

                PlayEffect(&gameoverSound, gameoverSoundLoaded);

                if (!scoreSavedForCurrentGame)
                {
                    AddScore(playerName, score);
                    scoreSavedForCurrentGame = true;
                }

                gameState = STATE_SCORE_RESULT;
            }
        }
        else if (gameState == STATE_SCORE_RESULT)
        {
            if (gameoverMusicLoaded)
                UpdateMusicStream(gameoverMusic);

            if (IsKeyPressed(KEY_ENTER))
            {
                if (gameoverMusicLoaded) StopMusicStream(gameoverMusic);
                gameState = STATE_LEADERBOARD;
            }
            else if (IsKeyPressed(KEY_ESCAPE))
            {
                if (gameoverMusicLoaded) StopMusicStream(gameoverMusic);
                gameState = STATE_MENU;
            }
        }

        BeginDrawing();
        DrawBackground();

        switch (gameState)
        {
            case STATE_MENU:          DrawMenu(menuSelection); break;
            case STATE_DIFFICULTY:   DrawDifficultyMenu(difficultySelection); break;
            case STATE_HOW_TO_PLAY:  DrawHowToPlay(); break;
            case STATE_CREDITS:      DrawCredits(); break;
            case STATE_LEADERBOARD:  DrawLeaderboard(); break;
            case STATE_NAME_INPUT:   DrawNameInput(); break;
            case STATE_PLAYING:      DrawPlaying(&piece); break;
            case STATE_SCORE_RESULT: DrawScoreResult(); break;
        }

        if (muted)
            DrawText("MUTED", 780, 20, 20, RED);

        EndDrawing();
    }

    if (musicLoaded) UnloadMusicStream(music);
    if (gameoverMusicLoaded) UnloadMusicStream(gameoverMusic);
    if (moveSoundLoaded) UnloadSound(moveSound);
    if (rotateSoundLoaded) UnloadSound(rotateSound);
    if (lineSoundLoaded) UnloadSound(lineSound);
    if (dropSoundLoaded) UnloadSound(dropSound);
    if (gameoverSoundLoaded) UnloadSound(gameoverSound);
    if (blockTextureLoaded) UnloadTexture(blockTexture);
    if (fontLoaded) UnloadFont(gamefont);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
