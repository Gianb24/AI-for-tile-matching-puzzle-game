// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include <QGraphicsItemAnimation>
#include <QTimeLine>
#include <QGraphicsScene>
#include <QTimer>
#include <QVector>
#include <QThread>

#include <chrono>
#include <random>
#include <map>

#include "scene.h"
#include "gem.h"

class Game : public QThread
{
	Q_OBJECT

public slots:
	void pause();
	void reset();
	void startGame();
	void gameOver();
	void swapGem(int row, int col);
	int swapGemAI(int row1, int col1, int row2, int col2);
	void incrementScore(long long amount);
	void resetRanking();
	void updateRanking(long long, QString, int pos);
	void aiMove(int pos, char moveTo);
	void giveHint();

signals:
	void scoreChanged(long long amount);
	void makeMove(int pos, char moveTo);

private:
	enum class State { READY, RUNNING, PAUSE, GAME_OVER };
	Game();

	State state;
	QVector<QVector<Gem*>> gemGrid;
	QVector<QVector<Gem*>> bkpGrid;
	bool firstGemSelected = true;
	int tmpRow;
	int tmpCol;

	void fillGrid();
	void correctGrid();

	QTimeLine* timerX[8][8];
	QTimeLine* timerY[8][8];

	QGraphicsItemAnimation* swapAnimationX[8][8];
	QGraphicsItemAnimation* swapAnimationY[8][8];

	void initializeGem(int row, int col);
	unsigned countMoves(bool aiMoves = false);
	unsigned AIcountMoves(bool aiMoves = false);
	void matchGrid(bool AlterScore = true);
	void parseMatchedGems();
	bool gridIsMatched;
	void swipeGem(int row, int col);
	void switchGem(int rowX, int colX, int rowY, int colY, bool Animate = false, int Delay = 0, bool ReSwap = false);
	void slideRanking(int);
	long long score;

	void run();
	std::multimap<int, char> firstMoves;
	std::multimap<int, char> availableMoves;
	void saveGrid();

	bool AreGemsEnabled = true;
	void changeGemsState(bool State);

	int generateRandInt(int Minimum, int Maximum);
	std::mt19937 MT;

public:
	static Game& getInstance();
	Game(Game const&) = delete;
	void operator=(Game const&) = delete;

	bool firstGame = true;
	bool hintRequested = false;
	QString name = "";
	QTimer* timer;
	bool isRunning();
	bool isGameOver();
	void saveGame() const;
	void loadGame();
	void saveRanking() const;
	void loadRanking();
	long long getScore();
	QVector<long long> highscoreValue;
	QVector<QString> highscoreName;

	void disableGems();
	void enableGems();
};
