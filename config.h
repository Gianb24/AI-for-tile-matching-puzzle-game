// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include <QKeyEvent>

#define RANKING_PLACES						4								// Sets the number of scores to be displayed in the ranking excluded the first one
#define GAME_FPS							25								// Sets the game FPS
const int SingleSwapAnimationDuration =		1250;							// How long the single swap animation takes, in milliseconds
const int AdditionalSwapAnimationDelay =	50;

#define Debug								1								// Sets debug version compilation on On/Off <---> 0 = Off, 1 = On
const bool randomHint =						true;							// If true hint is random, if false hint is first gem swappable
const bool hideGemAtEnd =					true;							// If true hide all gems at GameOver
const int iterationPerMove =				500;							// Number of Monte-Carlo iterations for each first move
const int treeHeight =						10;								// Number of moves after the first one
const int penaltyPoints =					30;								// Penalty for each game over iteraction
const int stdPoints =						10;

const int MinimumNameLength =				1;
const int MaximumNameLength =				16;

const int KeyGameStart =					Qt::Key_S;
const int KeyGamePause =					Qt::Key_P;

#define DIALOG_FONT							"Verdana"
#define DIALOG_LABEL_TEXT_COLOR				Qt::white
#define DIALOG_BUTTON_STYLESHEET			"Button {color: white;}"