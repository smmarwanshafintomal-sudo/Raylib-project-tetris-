# Raylib Tetris

A classic arcade-style Tetris game built with C and the raylib graphics library. This project includes a playable game loop, score tracking, difficulty levels, leaderboard persistence, sound effects, and a menu system.

## Features

- Falling tetromino gameplay with classic Tetris rules
- Multiple difficulty modes: Easy, Medium, Hard
- Score system and level progression
- Next-piece preview
- Pause and mute controls
- Name input and persistent local leaderboard
- Menu screens for start, difficulty, leaderboard, credits, and help
- Audio and sound effects using raylib audio APIs

## Project Structure

Raylib-project-tetris-
├── README.md
├── tetris/
│   ├── Makefile
│   ├── main.c
│   ├── main
│   ├── music.mp3
│   ├── music_2.mp3
│   ├── heavenly_gameover.mp3
│   ├── rotate.wav
│   ├── line.wav
│   ├── drop.wav
│   ├── Fredoka-Bold.ttf
│   ├── Gameover.mp3
│   └── scores.txt
└── .git/


## Controls

- Left / Right Arrow: move piece
- Up Arrow: rotate piece
- Down Arrow: soft drop
- Space: fast drop
- P: pause / resume
- M: mute / unmute
- X: exit to menu
- Enter: confirm menu actions / submit name
- Escape: back / cancel

## Gameplay

The player must complete full horizontal rows to clear them and earn points. As the score increases, the level rises and the falling speed becomes faster. The game ends when a new piece cannot be placed on the board.

## Leaderboard

The game stores the top scores in `tetris/scores.txt`, so player scores persist between runs.

## Credits

This project was created by S M Marwan Shafin Tomal and Tayeaba under the supervisor of Mohammad Sadat Hossain Sir.

