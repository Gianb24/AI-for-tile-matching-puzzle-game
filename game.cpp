// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#if Debug
	#include <QDebug>
#endif

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariant>
#include <QFileInfo>

#include "game.h"
#include "dialog.h"
#include "config.h"

#define RANKING_PLACES 4

void Game::saveGrid()
{
	for (int i = 0; i < 64; i++)
		bkpGrid[i / 8][i % 8]->setColor(gemGrid[i / 8][i % 8]->getColor());
}

Game::Game()
	: MT(std::chrono::high_resolution_clock::now().time_since_epoch().count())
{
	highscoreName.resize(5);
	highscoreValue.resize(5);
	resetRanking();

	if (QFileInfo::exists("ranking.dat"))
		loadRanking();

	gemGrid.resize(8);
	for (int i = 0; i < 8; i++)
		gemGrid[i].resize(8);

	bkpGrid.resize(8);
	for (int i = 0; i < 8; i++)
		bkpGrid[i].resize(8);

	timer = new QTimer();
	timer->setInterval(GAME_FPS);
	state = State::READY;

	std::random_device rd;
	std::mt19937 MTinit{ rd() };

	connect(this, &Game::scoreChanged, this, &Game::incrementScore);
	connect(this, &Game::scoreChanged, [=] {Scene::getInstance().displayScore(); });
	connect(this, &Game::makeMove, this, &Game::aiMove);
}

void Game::aiMove(int pos, char moveTo)
{
	swapGem(pos / 8, pos % 8);

	if (moveTo == 'r')
		swapGem(pos / 8, (pos % 8) + 1);
	else if (moveTo == 'l')
		swapGem(pos / 8, (pos % 8) - 1);
	else if (moveTo == 'u')
		swapGem((pos / 8) - 1, pos % 8);
	else if (moveTo == 'd')
		swapGem((pos / 8) + 1, pos % 8);
}

int Game::generateRandInt(int Minimum, int Maximum)
{
	std::uniform_int_distribution<int> dist(Minimum, Maximum);
	return dist(MT);
}

void Game::startGame()
{
	#if Debug
		qDebug() << "Start";
	#endif

	if (state == State::PAUSE)
	{
		timer->start();
		state = State::RUNNING;
	}
	else
	{
		if (firstGame)
		{
			fillGrid();
			firstGame = false;
		}
		if (state == State::READY)
		{
			state = State::RUNNING;
			timer->start();
		}
		else if (state == State::GAME_OVER || state == State::RUNNING)
		{
			state = State::GAME_OVER;
			reset();
		}

		score = 0;
		emit scoreChanged(0);
		hintRequested = false;

		Sounds::getInstance()->play("getready");
	}
}

void Game::pause()
{
	#if Debug
		qDebug() << "Pause";
	#endif

	state = State::PAUSE;

	timer->stop();
}

void Game::reset()
{
	#if Debug
		qDebug() << "Reset";
	#endif

	if (state == State::GAME_OVER)
		do
		{
			for (int i = 0; i < 64; i++)
				initializeGem(i / 8, i % 8);

			correctGrid();
		} while (countMoves() == 0);

		state = State::READY;

		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				gemGrid[i][j]->setOpacity(1);

		startGame();

		Scene::getInstance().getScene()->update();
}

void Game::gameOver()
{
	#if Debug
		qDebug() << "Game Over";
	#endif

	state = State::GAME_OVER;

	if (hideGemAtEnd)
		for (int i = 0; i < 64; i++)
			gemGrid[i / 8][i % 8]->setOpacity(0);

	updateRanking(score, name, 0);

	Scene::getInstance().showButton(Scene::getInstance().ai, false);

	Sounds::getInstance()->play("gameover");
}

void Game::fillGrid()
{
	int m, n;

	m = 13;

	do {
		for (int i = 0; i < 8; i++) {
			n = 200;

			for (int j = 0; j < 8; j++)
			{
				gemGrid[i][j] = new Gem(i, j);
				bkpGrid[i][j] = new Gem(i, j);

				gemGrid[i][j]->isMatched = false;
				gemGrid[i][j]->isHint = false;

				gemGrid[i][j]->setPos(n, m);

				gemGrid[i][j]->setColor(static_cast<Gem::gemColor>(generateRandInt(0, 6)));
				gemGrid[i][j]->initGemPixmap();

				Scene::getInstance().getScene()->addItem(gemGrid[i][j]);

				connect(timer, &QTimer::timeout, gemGrid[i][j], &Gem::animate);
				connect(gemGrid[i][j], &Gem::gemClicked, this, &Game::swapGem);

				n += 52;
			}

			m += 52;
		}

		correctGrid();

	} while (countMoves() == 0);
}

void Game::swapGem(int row, int col)
{
	if (state == State::RUNNING)
	{
		if (firstGemSelected)
		{
			firstGemSelected = false;
			tmpRow = row;
			tmpCol = col;
		}
		else
		{
			if (tmpRow == row && tmpCol == col)
				firstGemSelected = true;
			else if (((tmpRow - row == -1 || tmpRow - row == 1) && (tmpCol - col == -1 || tmpCol - col == 1)) || ((tmpRow - row < -1 || tmpRow - row > 1) || (tmpCol - col < -1 || tmpCol - col > 1))) //check if swap is possible
			{
				#if Debug
					qDebug() << "Swap impossibile"; //Swap is not possible, because the gems are not consecutive
				#endif

				gemGrid[tmpRow][tmpCol]->initGemPixmap();
				gemGrid[tmpRow][tmpCol]->isSelected = false;

				gemGrid[row][col]->initGemPixmap();
				gemGrid[row][col]->isSelected = false;

				Scene::getInstance().getScene()->update();
				if (!Scene::getInstance().isAiActive())
					Sounds::getInstance()->play("bad");
			}
			else if ((tmpRow - row == -1 || tmpRow - row == 1 && tmpCol - col == 0) || (tmpCol - col == -1 || tmpCol - col == 1 && tmpRow - row == 0)) // Swap is possible
			{
				#if Debug
					qDebug() << "Swap consentito";
					qDebug() << "swap gem (" << tmpRow << "," << tmpCol << ") with gem (" << row << "," << col << ")";
				#endif

				gemGrid[tmpRow][tmpCol]->isSelected = false;
				gemGrid[row][col]->isSelected = false;

				switchGem(tmpRow, tmpCol, row, col);

				Scene::getInstance().getScene()->update();

				matchGrid();

				if (gemGrid[tmpRow][tmpCol]->isMatched || gemGrid[row][col]->isMatched)
				{
					parseMatchedGems();

					do
					{
						matchGrid();
						parseMatchedGems();
						matchGrid();
					} while (gridIsMatched);

					hintRequested = false;

					for (int i = 0; i < 8; i++)
						for (int j = 0; j < 8; j++)
							gemGrid[i][j]->isHint = false;

					if (!Scene::getInstance().isAiActive())
					Sounds::getInstance()->play("move");
				}
				else
				{
					switchGem(row, col, tmpRow, tmpCol, true, 0, true);

					#if Debug
						qDebug() << "reswap gem (" << row << "," << col << ") with gem (" << tmpRow << "," << tmpCol << ")";
					#endif
					if (!Scene::getInstance().isAiActive())
						Sounds::getInstance()->play("bad");
				}
			}
			firstGemSelected = true;
		}
		countMoves();
	}
}

int Game::swapGemAI(int row1, int col1, int row2, int col2)
{
	Gem::gemColor tmpColor;

	tmpColor = bkpGrid[row1][col1]->getColor();

	bkpGrid[row1][col1]->setColor(bkpGrid[row2][col2]->getColor());
	bkpGrid[row2][col2]->setColor(tmpColor);

	bool AIgridIsMatched = false;
	int aiPoints = 0;

	do {
		AIgridIsMatched = false;

		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++) {
				if (j < 6 && bkpGrid[i][j]->getColor() == bkpGrid[i][j + 1]->getColor() && bkpGrid[i][j]->getColor() == bkpGrid[i][j + 2]->getColor())
				{
					bkpGrid[i][j]->isMatched = true;
					bkpGrid[i][j + 1]->isMatched = true;
					bkpGrid[i][j + 2]->isMatched = true;
					aiPoints += stdPoints;
					AIgridIsMatched = true;
				}
				if (i < 6 && bkpGrid[i][j]->getColor() == bkpGrid[i + 1][j]->getColor() && bkpGrid[i][j]->getColor() == bkpGrid[i + 2][j]->getColor())
				{
					bkpGrid[i][j]->isMatched = true;
					bkpGrid[i + 1][j]->isMatched = true;
					bkpGrid[i + 2][j]->isMatched = true;
					aiPoints += stdPoints;
					AIgridIsMatched = true;
				}
			}

		for (int i = 0; i < 64; i++)
			if (bkpGrid[i / 8][i % 8]->isMatched)
			{
				bkpGrid[i / 8][i % 8]->isMatched = false;

				for (int j = i / 8; j > 0; j--)
				{
					Gem::gemColor tmpColor;
					tmpColor = bkpGrid[j][i % 8]->getColor();
					bkpGrid[j][i % 8]->setColor(bkpGrid[j - 1][i % 8]->getColor());
					bkpGrid[j - 1][i % 8]->setColor(tmpColor);
				}

				bkpGrid[0][i % 8]->isMatched = false;
				bkpGrid[0][i % 8]->setColor(static_cast<Gem::gemColor>(generateRandInt(0, 6)));
			}
	} while (AIgridIsMatched);

	return aiPoints;
}

bool Game::isRunning()
{
	if (state == State::RUNNING)
		return true;
	else
		return false;
}

bool Game::isGameOver()
{
	if (state == State::GAME_OVER)
		return true;
	else
		return false;
}

void Game::matchGrid(bool AlterScore)
{
	long long PointsPerMatch = stdPoints;

	gridIsMatched = false;

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			if (j < 6 && gemGrid[i][j]->getColor() == gemGrid[i][j + 1]->getColor() && gemGrid[i][j]->getColor() == gemGrid[i][j + 2]->getColor())
			{
				gemGrid[i][j]->isMatched = true;
				gemGrid[i][j + 1]->isMatched = true;
				gemGrid[i][j + 2]->isMatched = true;

				if (AlterScore)
					emit(scoreChanged(PointsPerMatch));

				gridIsMatched = true;
			}
			if (i < 6 && gemGrid[i][j]->getColor() == gemGrid[i + 1][j]->getColor() && gemGrid[i][j]->getColor() == gemGrid[i + 2][j]->getColor())
			{
				gemGrid[i][j]->isMatched = true;
				gemGrid[i + 1][j]->isMatched = true;
				gemGrid[i + 2][j]->isMatched = true;

				if (AlterScore)
					emit(scoreChanged(PointsPerMatch));

				gridIsMatched = true;
			}
		}
}

void Game::parseMatchedGems() {
	for (int i = 0; i < 64; i++)
		if (gemGrid[i / 8][i % 8]->isMatched)
			swipeGem(i / 8, i % 8);
}

void Game::initializeGem(int row, int col)
{
	gemGrid[row][col]->isMatched = false;
	gemGrid[row][col]->setColor(static_cast<Gem::gemColor>(generateRandInt(0, 6)));
	gemGrid[row][col]->initGemPixmap();
}

void Game::swipeGem(int row, int col)
{
	gemGrid[row][col]->isMatched = false;

	for (int i = row; i > 0; i--)
		if (i > 1)
			switchGem(i, col, i - 1, col, true, ((i - 1) * SingleSwapAnimationDuration) + AdditionalSwapAnimationDelay);
		else
			switchGem(i, col, i - 1, col);

	initializeGem(0, col);
}

void Game::switchGem(int rowX, int colX, int rowY, int colY, bool Animate, int Delay, bool ReSwap)
{
	int Temp;

	if (rowX > rowY || colX > colY) {
		Temp = rowX;
		rowX = rowY;
		rowY = Temp;

		Temp = colX;
		colX = colY;
		colY = Temp;
	}

	Gem::gemColor tmpColor;
	tmpColor = gemGrid[rowX][colX]->getColor();

	if (Animate && !gemGrid[rowX][colX]->isBeingAnimated && !gemGrid[rowY][colY]->isBeingAnimated) {
		// Animation

		gemGrid[rowX][colX]->isBeingAnimated = true;
		gemGrid[rowY][colY]->isBeingAnimated = true;

		double GemX[2] = {
			gemGrid[rowX][colX]->x(),
			gemGrid[rowX][colX]->y()
		};

		double GemY[2] = {
			gemGrid[rowY][colY]->x(),
			gemGrid[rowY][colY]->y()
		};

		timerX[rowX][colX] = new QTimeLine(1000);
		timerY[rowY][colY] = new QTimeLine(1000);

		swapAnimationX[rowX][colX] = new QGraphicsItemAnimation;
		swapAnimationY[rowY][colY] = new QGraphicsItemAnimation;

		swapAnimationX[rowX][colX]->setItem(gemGrid[rowX][colX]);
		swapAnimationY[rowY][colY]->setItem(gemGrid[rowY][colY]);

		swapAnimationX[rowX][colX]->setTimeLine(timerX[rowX][colX]);
		swapAnimationY[rowY][colY]->setTimeLine(timerY[rowY][colY]);

		double RowStep = (double)(52) * ((double)(rowY - rowX) / (double)(SingleSwapAnimationDuration));
		double ColumnStep = (double)(52) * ((double)(colY - colX) / (double)(SingleSwapAnimationDuration));
		int AnimationDuration = SingleSwapAnimationDuration;

		if (ReSwap)
			AnimationDuration *= 2;

		int TotalAnimationDuration = AnimationDuration + Delay;

		// 200 + (rowY * 52) - [200 + (rowX * 52)] =
		// (rowY * 52) - (rowX * 52) =
		// 52 * (rowY - rowX)
		//
		// Analogamente
		//
		// 13 + (colY * 52) - [13 + (colX * 52)] =
		// (colY * 52) - (colX * 52) =
		// 52 * (colY - colX)
		//
		// c.v.d.

		for (int i = 0; i < Delay; i++) {
			swapAnimationX[rowX][colX]->setPosAt((double)(1) - ((double)(i) / (double)(TotalAnimationDuration)), QPointF(GemX[0], GemX[1]));
			swapAnimationY[rowY][colY]->setPosAt((double)(1) - ((double)(i) / (double)(TotalAnimationDuration)), QPointF(GemY[0], GemY[1]));
		}

		for (int i = 0; i <= AnimationDuration / (ReSwap ? 2 : 1); i++) {
			swapAnimationX[rowX][colX]->setPosAt((double)(1) - ((double)(i + Delay) / (double)(TotalAnimationDuration)), QPointF(GemX[0] + (ColumnStep * (double)(i)), GemX[1] + (RowStep * (double)(i))));
			swapAnimationY[rowY][colY]->setPosAt((double)(1) - ((double)(i + Delay) / (double)(TotalAnimationDuration)), QPointF(GemY[0] - (ColumnStep * (double)(i)), GemY[1] - (RowStep * (double)(i))));
		}

		connect(timerX[rowX][colX], &QTimeLine::finished, this, [=]
			{
				gemGrid[rowX][colX]->setPos(GemX[0], GemX[1]);
				gemGrid[rowX][colX]->isBeingAnimated = false;

				delete timerX[rowX][colX];
				delete swapAnimationX[rowX][colX];
			}
		);

		connect(timerY[rowY][colY], &QTimeLine::finished, this, [=]
			{
				gemGrid[rowY][colY]->setPos(GemY[0], GemY[1]);
				gemGrid[rowY][colY]->isBeingAnimated = false;

				delete timerY[rowY][colY];
				delete swapAnimationY[rowY][colY];
			}
		);

		timerX[rowX][colX]->start();
		timerY[rowY][colY]->start();
	}

	gemGrid[rowX][colX]->setColor(gemGrid[rowY][colY]->getColor());
	gemGrid[rowX][colX]->initGemPixmap();

	gemGrid[rowY][colY]->setColor(tmpColor);
	gemGrid[rowY][colY]->initGemPixmap();
}

void Game::correctGrid()
{
	do {
		matchGrid();

		for (int i = 0; i < 64; i++)
			if (gemGrid[i / 8][i % 8]->isMatched)
				initializeGem(i / 8, i % 8);

		matchGrid();

	} while (gridIsMatched);
}

void Game::run()
{
	disableGems();

	unsigned NumberOfAiMoves;

	do
	{
		saveGrid();

		NumberOfAiMoves = AIcountMoves(true);

		#if Debug
			qDebug() << "AI Moves: " << NumberOfAiMoves;
		#endif

		if (NumberOfAiMoves > 0)
		{
			firstMoves = availableMoves;
			availableMoves.clear();

			for (auto m = firstMoves.begin(); m != firstMoves.end(); m++)
			{
				#if Debug
					qDebug() << m->first << " " << m->second;
				#endif
			}

			int bestMove = 0;

			char bestMoveTo = ' ';

			double bestMoveAvgScore = -std::numeric_limits<double>::infinity();
			double avgMovePoints = 0;

			for (auto m = firstMoves.begin(); m != firstMoves.end(); m++)
			{
				for (int i = 0; i < iterationPerMove; i++)
				{
					if (m->second == 'u')
						avgMovePoints += swapGemAI(m->first / 8, m->first % 8, (m->first / 8) - 1, m->first % 8);
					else if (m->second == 'd')
						avgMovePoints += swapGemAI(m->first / 8, m->first % 8, (m->first / 8) + 1, m->first % 8);
					else if (m->second == 'l')
						avgMovePoints += swapGemAI(m->first / 8, m->first % 8, m->first / 8, (m->first % 8) - 1);
					else if (m->second == 'r')
						avgMovePoints += swapGemAI(m->first / 8, m->first % 8, m->first / 8, (m->first % 8) + 1);

					//GAME N DIFFERENT MATCHES
					int movesMade = 0;
					double avgScore = 0;

					while (movesMade < treeHeight)
					{
						availableMoves.clear();

						if (AIcountMoves(true) != 0)
						{
							auto n = availableMoves.begin();
							std::advance(n, generateRandInt(0, availableMoves.size() - 1));

							if (n->second == 'u')
								avgScore += (swapGemAI(n->first / 8, n->first % 8, (n->first / 8) - 1, n->first % 8)) / (movesMade + 1);
							else if (n->second == 'd')
								avgScore += (swapGemAI(n->first / 8, n->first % 8, (n->first / 8) + 1, n->first % 8)) / (movesMade + 1);
							else if (n->second == 'l')
								avgScore += (swapGemAI(n->first / 8, n->first % 8, n->first / 8, (n->first % 8) - 1)) / (movesMade + 1);
							else if (n->second == 'r')
								avgScore += (swapGemAI(n->first / 8, n->first % 8, n->first / 8, (n->first % 8) + 1)) / (movesMade + 1);
						}
						else
							avgScore -= (penaltyPoints / (movesMade + 1));

						movesMade++;
					}

					availableMoves.clear();
					avgMovePoints += avgScore;

					movesMade = 0;
					avgScore = 0;

					//END GAME N DIFFERENT MATCHES
					saveGrid();
				}

				avgMovePoints /= iterationPerMove;

				if (avgMovePoints > bestMoveAvgScore)
				{
					bestMove = m->first;
					bestMoveTo = m->second;
					bestMoveAvgScore = avgMovePoints;
				}
			}

			emit makeMove(bestMove, bestMoveTo);

			#if Debug
				qDebug() << "Best Move = " << bestMove / 8 << " " << bestMove % 8 << " to " << bestMoveTo << " with avg move score: " << bestMoveAvgScore;
			#endif
		}
		else
			gameOver();

	} while (state != State::GAME_OVER);

	enableGems();
}

Game& Game::getInstance()
{
	static Game instance;
	return instance;
}

void Game::incrementScore(long long amount)
{
	score += amount;

	if (score > 9999999999)
		score = 9999999999;
	else if (score < 0)
		score = 0;
}

unsigned Game::countMoves(bool aiMoves)
{
	unsigned retMOVES = 0;

	for (int i = 0; i < 64; i++)
		gemGrid[i / 8][i % 8]->isMatched = false;

	for (int i = 0; i < 64; i++)
	{
		//Horizontal

		// -
		// 0 # ##
		// +
		//

		if (i % 8 + 3 < 8 && (!gemGrid[i / 8][i % 8]->isMatched || !gemGrid[i / 8][i % 8 + 1]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 2]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 3]->getColor()) {
			retMOVES++;
			gemGrid[i / 8][i % 8]->isMatched = true;
			gemGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 0,'r' });
		}

		//
		// -
		// 0 ## #
		// +
		//
		if (i % 8 + 3 < 8 && (!gemGrid[i / 8][i % 8 + 2]->isMatched || !gemGrid[i / 8][i % 8 + 3]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 3]->getColor()) {
			retMOVES++;
			gemGrid[i / 8][i % 8 + 2]->isMatched = true;
			gemGrid[i / 8][i % 8 + 3]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 3,'l' });
		}

		//
		// - #
		// 0  ##
		// +
		//
		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!gemGrid[i / 8 - 1][i % 8]->isMatched || !gemGrid[i / 8][i % 8]->isMatched) && gemGrid[i / 8 - 1][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 1]->getColor() && gemGrid[i / 8 - 1][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 - 1][i % 8]->isMatched = true;
			gemGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 8,'d' });
		}

		//
		// -
		// 0  ##
		// + #
		//
		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!gemGrid[i / 8 + 1][i % 8]->isMatched || !gemGrid[i / 8][i % 8]->isMatched) && gemGrid[i / 8 + 1][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 1]->getColor() && gemGrid[i / 8 + 1][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 1][i % 8]->isMatched = true;
			gemGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 8,'u' });
		}

		//
		// -  #
		// 0 # #
		// +
		//
		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!gemGrid[i / 8 - 1][i % 8 + 1]->isMatched || !gemGrid[i / 8][i % 8 + 1]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 - 1][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 - 1][i % 8 + 1]->isMatched = true;
			gemGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 7,'d' });
		}

		//
		// -
		// 0 # #
		// +  #
		//
		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!gemGrid[i / 8 + 1][i % 8 + 1]->isMatched || !gemGrid[i / 8][i % 8 + 1]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 1][i % 8 + 1]->isMatched = true;
			gemGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 9,'u' });
		}

		//
		// -   #
		// 0 ##
		// +
		//
		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!gemGrid[i / 8 - 1][i % 8 + 2]->isMatched || !gemGrid[i / 8][i % 8 + 2]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 - 1][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 - 1][i % 8 + 2]->isMatched = true;
			gemGrid[i / 8][i % 8 + 2]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 6,'d' });
		}

		//
		// -
		// 0 ##
		// +   #
		//
		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!gemGrid[i / 8 + 1][i % 8 + 2]->isMatched || !gemGrid[i / 8][i % 8 + 2]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8 + 2]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 1][i % 8 + 2]->isMatched = true;
			gemGrid[i / 8][i % 8 + 2]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 10,'u' });
		}

		//Vertical

		//
		// 0 #
		//
		// 2 #
		// 3 #
		//
		if (i / 8 + 3 < 8 && (!gemGrid[i / 8][i % 8]->isMatched || !gemGrid[i / 8 + 1][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 2][i % 8]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 3][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8][i % 8]->isMatched = true;
			gemGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 0,'d' });
		}

		//
		// 0 #
		// 1 #
		//
		// 3 #
		//
		if (i / 8 + 3 < 8 && (!gemGrid[i / 8 + 2][i % 8]->isMatched || !gemGrid[i / 8 + 3][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 3][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 2][i % 8]->isMatched = true;
			gemGrid[i / 8 + 3][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 24,'u' });
		}

		// -0+
		//
		// #
		//  #
		//  #
		//
		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!gemGrid[i / 8][i % 8 - 1]->isMatched || !gemGrid[i / 8][i % 8]->isMatched) && gemGrid[i / 8][i % 8 - 1]->getColor() == gemGrid[i / 8 + 1][i % 8]->getColor() && gemGrid[i / 8][i % 8 - 1]->getColor() == gemGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8][i % 8 - 1]->isMatched = true;
			gemGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 1,'r' });
		}

		// -0+
		//
		//   #
		//  #
		//  #
		//
		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!gemGrid[i / 8][i % 8 + 1]->isMatched || !gemGrid[i / 8][i % 8]->isMatched) && gemGrid[i / 8][i % 8 + 1]->getColor() == gemGrid[i / 8 + 1][i % 8]->getColor() && gemGrid[i / 8][i % 8 + 1]->getColor() == gemGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8][i % 8 + 1]->isMatched = true;
			gemGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 1,'l' });
		}

		// -0+
		//
		//  #
		// #
		//  #
		//
		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!gemGrid[i / 8 + 1][i % 8 - 1]->isMatched || !gemGrid[i / 8 + 1][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8 - 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 1][i % 8 - 1]->isMatched = true;
			gemGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 7,'r' });
		}

		// -0+
		//
		//  #
		//   #
		//  #
		//
		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!gemGrid[i / 8 + 1][i % 8 + 1]->isMatched || !gemGrid[i / 8 + 1][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8 + 1]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 1][i % 8 + 1]->isMatched = true;
			gemGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 9,'l' });
		}

		// -0+
		//
		//  #
		//  #
		// #
		//
		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!gemGrid[i / 8 + 2][i % 8 - 1]->isMatched || !gemGrid[i / 8 + 2][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 2][i % 8 - 1]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 2][i % 8 - 1]->isMatched = true;
			gemGrid[i / 8 + 2][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 15,'r' });
		}

		// -0+
		//
		//  #
		//  #
		//   #
		//
		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!gemGrid[i / 8 + 2][i % 8 + 1]->isMatched || !gemGrid[i / 8 + 2][i % 8]->isMatched) && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 1][i % 8]->getColor() && gemGrid[i / 8][i % 8]->getColor() == gemGrid[i / 8 + 2][i % 8 + 1]->getColor()) {
			retMOVES++;
			gemGrid[i / 8 + 2][i % 8 + 1]->isMatched = true;
			gemGrid[i / 8 + 2][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 17,'l' });
		}
	}

	for (int i = 0; i < 64; i++)
		gemGrid[i / 8][i % 8]->isMatched = false;

	if (retMOVES == 0)
		gameOver();

	return retMOVES;
}

void Game::saveGame() const
{
	QFile file("save.json");
	file.open(QFile::WriteOnly);

	QJsonObject recordObject;
	recordObject.insert("Name", QJsonValue::fromVariant(QVariant(name)));
	recordObject.insert("Score", QJsonValue::fromVariant(QVariant(score)));


	QJsonArray gridArray;

	for (int i = 0; i < 64; i++)
		gridArray.push_back(QJsonValue::fromVariant(QVariant(gemGrid[i / 8][i % 8]->getStringColor())));
	
	recordObject.insert("Grid", gridArray);

	QJsonDocument doc(recordObject);
	file.write(doc.toJson());
}

void Game::loadGame()
{
	fillGrid();
	QFile file("save.json");
	file.open(QFile::ReadOnly | QIODevice::Text);

	QByteArray saveData = file.readAll();
	file.close();
	QJsonDocument doc(QJsonDocument::fromJson(saveData));

	QJsonArray gridArray = doc["Grid"].toArray();
	int i = 0;
	for (int m = 0; m < 64; m++)
	{
		QString tmp = gridArray[i].toString();
		gemGrid[m / 8][m % 8]->setStringColor(tmp);
		gemGrid[m / 8][m % 8]->initGemPixmap();
		i++;
	}

	name = doc["Name"].toString();
	score = doc["Score"].toInt();
	Scene::getInstance().displayScore();
	

	firstGame = false;
	state = State::RUNNING;
	timer->start();
}

void Game::updateRanking(long long value, QString tmpName, int pos)
{
	if (pos == RANKING_PLACES)
	{
		if (value < highscoreValue[RANKING_PLACES])
			return;
		else if (value > highscoreValue[RANKING_PLACES])
		{
			highscoreValue[pos] = value;
			highscoreName[pos] = tmpName;
			return;
		}
	}
	if (value > highscoreValue[pos])
	{
		slideRanking(pos);
		highscoreValue[pos] = value;
		highscoreName[pos] = tmpName;
		return;
	}
	if (pos < RANKING_PLACES && value <= highscoreValue[pos])
	{
		pos++;
		return updateRanking(value, tmpName, pos);
	}

}

void Game::slideRanking(int pos)
{
	int last = RANKING_PLACES;
	do
	{
		highscoreValue[last] = highscoreValue[last - 1];
		highscoreName[last] = highscoreName[last - 1];
		last--;
	} while ((pos - last) != 0);
}

void Game::saveRanking() const
{
	QFile file("ranking.dat");
	file.open(QFile::WriteOnly);

	QJsonObject recordObject;

	QJsonArray rankingNameArray;
	for (int i = 0; i < 5; i++)
		rankingNameArray.push_back(QJsonValue::fromVariant(QVariant(highscoreName[i])));
	recordObject.insert("Ranking Name", rankingNameArray);

	QJsonArray rankingScoreArray;
	for (int i = 0; i < 5; i++)
		rankingScoreArray.push_back(QJsonValue::fromVariant(QVariant(highscoreValue[i])));
	recordObject.insert("Ranking Value", rankingScoreArray);

	QJsonDocument doc(recordObject);
	file.write(doc.toJson());
}

void Game::loadRanking()
{
	QFile file("ranking.dat");
	file.open(QFile::ReadOnly | QIODevice::Text);

	QByteArray saveData = file.readAll();
	file.close();
	QJsonDocument doc(QJsonDocument::fromJson(saveData));

	QJsonArray nameArray = doc["Ranking Name"].toArray();
	for (int i = 0; i < RANKING_PLACES + 1; i++)
	{
		QString tmp = nameArray[i].toString();
		highscoreName[i] = tmp;
	}

	QJsonArray valueArray = doc["Ranking Value"].toArray();
	for (int i = 0; i < RANKING_PLACES + 1; i++)
	{
		long long tmp = valueArray[i].toInt();
		highscoreValue[i] = tmp;
	}
}

long long Game::getScore() {
	return score;
}

void Game::resetRanking()
{
	for (int i = 0; i < 5; i++)
	{
		highscoreName[i] = "N/A";
		highscoreValue[i] = 0;
	}
}

unsigned Game::AIcountMoves(bool aiMoves)
{
	unsigned retMOVES = 0;

	for (int i = 0; i < 64; i++)
		bkpGrid[i / 8][i % 8]->isMatched = false;

	for (int i = 0; i < 64; i++)
	{
		if (i % 8 + 3 < 8 && (!bkpGrid[i / 8][i % 8]->isMatched || !bkpGrid[i / 8][i % 8 + 1]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 2]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 3]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 0,'r' });
		}

		if (i % 8 + 3 < 8 && (!bkpGrid[i / 8][i % 8 + 2]->isMatched || !bkpGrid[i / 8][i % 8 + 3]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 3]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8][i % 8 + 2]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 3]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 3,'l' });
		}

		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 - 1][i % 8]->isMatched || !bkpGrid[i / 8][i % 8]->isMatched) && bkpGrid[i / 8 - 1][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 1]->getColor() && bkpGrid[i / 8 - 1][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 - 1][i % 8]->isMatched = true;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 8,'d' });
		}

		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 + 1][i % 8]->isMatched || !bkpGrid[i / 8][i % 8]->isMatched) && bkpGrid[i / 8 + 1][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 1]->getColor() && bkpGrid[i / 8 + 1][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 1][i % 8]->isMatched = true;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 8,'u' });
		}

		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 - 1][i % 8 + 1]->isMatched || !bkpGrid[i / 8][i % 8 + 1]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 - 1][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 - 1][i % 8 + 1]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 7,'d' });
		}

		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 + 1][i % 8 + 1]->isMatched || !bkpGrid[i / 8][i % 8 + 1]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 1][i % 8 + 1]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 1]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 9,'u' });
		}

		if (i / 8 - 1 >= 0 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 - 1][i % 8 + 2]->isMatched || !bkpGrid[i / 8][i % 8 + 2]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 - 1][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 - 1][i % 8 + 2]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 2]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 6,'d' });
		}

		if (i / 8 + 1 < 8 && i % 8 + 2 < 8 && (!bkpGrid[i / 8 + 1][i % 8 + 2]->isMatched || !bkpGrid[i / 8][i % 8 + 2]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8 + 2]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 1][i % 8 + 2]->isMatched = true;
			bkpGrid[i / 8][i % 8 + 2]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 10,'u' });
		}

		if (i / 8 + 3 < 8 && (!bkpGrid[i / 8][i % 8]->isMatched || !bkpGrid[i / 8 + 1][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 2][i % 8]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 3][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			bkpGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 0,'d' });
		}

		if (i / 8 + 3 < 8 && (!bkpGrid[i / 8 + 2][i % 8]->isMatched || !bkpGrid[i / 8 + 3][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 3][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 2][i % 8]->isMatched = true;
			bkpGrid[i / 8 + 3][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 24,'u' });
		}

		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!bkpGrid[i / 8][i % 8 - 1]->isMatched || !bkpGrid[i / 8][i % 8]->isMatched) && bkpGrid[i / 8][i % 8 - 1]->getColor() == bkpGrid[i / 8 + 1][i % 8]->getColor() && bkpGrid[i / 8][i % 8 - 1]->getColor() == bkpGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8][i % 8 - 1]->isMatched = true;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i - 1,'r' });
		}

		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!bkpGrid[i / 8][i % 8 + 1]->isMatched || !bkpGrid[i / 8][i % 8]->isMatched) && bkpGrid[i / 8][i % 8 + 1]->getColor() == bkpGrid[i / 8 + 1][i % 8]->getColor() && bkpGrid[i / 8][i % 8 + 1]->getColor() == bkpGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8][i % 8 + 1]->isMatched = true;
			bkpGrid[i / 8][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 1,'l' });
		}

		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!bkpGrid[i / 8 + 1][i % 8 - 1]->isMatched || !bkpGrid[i / 8 + 1][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8 - 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 1][i % 8 - 1]->isMatched = true;
			bkpGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 7,'r' });
		}

		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!bkpGrid[i / 8 + 1][i % 8 + 1]->isMatched || !bkpGrid[i / 8 + 1][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8 + 1]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 2][i % 8]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 1][i % 8 + 1]->isMatched = true;
			bkpGrid[i / 8 + 1][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 9,'l' });
		}

		if (i / 8 + 2 < 8 && i % 8 - 1 >= 0 && (!bkpGrid[i / 8 + 2][i % 8 - 1]->isMatched || !bkpGrid[i / 8 + 2][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 2][i % 8 - 1]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 2][i % 8 - 1]->isMatched = true;
			bkpGrid[i / 8 + 2][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 15,'r' });
		}

		if (i / 8 + 2 < 8 && i % 8 + 1 < 8 && (!bkpGrid[i / 8 + 2][i % 8 + 1]->isMatched || !bkpGrid[i / 8 + 2][i % 8]->isMatched) && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 1][i % 8]->getColor() && bkpGrid[i / 8][i % 8]->getColor() == bkpGrid[i / 8 + 2][i % 8 + 1]->getColor()) {
			retMOVES++;
			bkpGrid[i / 8 + 2][i % 8 + 1]->isMatched = true;
			bkpGrid[i / 8 + 2][i % 8]->isMatched = true;
			if (aiMoves)
				availableMoves.insert({ i + 17,'l' });
		}
	}

	for (int i = 0; i < 64; i++)
		bkpGrid[i / 8][i % 8]->isMatched = false;

	return retMOVES;
}

void Game::giveHint()
{
	availableMoves.clear();
	countMoves(true);
	auto n = availableMoves.begin();
	if (randomHint)
		std::advance(n, generateRandInt(0, availableMoves.size() - 1));
	int moves = n->first;
	char movesTo = n->second;
	gemGrid[n->first / 8][n->first % 8]->isHint = true;
	Gem::gemColor tmpColor = gemGrid[n->first / 8][n->first % 8]->getColor();
	if (n->second == 'l')
	{
		if ((n->first % 8 - 3) >= 0 && gemGrid[n->first / 8][n->first % 8 - 2]->getColor() == tmpColor && gemGrid[n->first / 8][n->first % 8 - 3]->getColor() == tmpColor)
		{
			gemGrid[n->first / 8][n->first % 8 - 2]->isHint = true;
			gemGrid[n->first / 8][n->first % 8 - 3]->isHint = true;
		}
		else
		{
			if ((n->first / 8 - 1) >= 0 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 - 1][n->first % 8 - 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 - 1][n->first % 8 - 1]->isHint = true;
				if ((n->first / 8 - 2) >= 0 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 - 2][n->first % 8 - 1]->getColor() == tmpColor)
					gemGrid[n->first / 8 - 2][n->first % 8 - 1]->isHint = true;
			}
			if ((n->first / 8 + 1) < 8 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 + 1][n->first % 8 - 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 + 1][n->first % 8 - 1]->isHint = true;
				if ((n->first / 8 + 2) < 8 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 + 2][n->first % 8 - 1]->getColor() == tmpColor)
					gemGrid[n->first / 8 + 2][n->first % 8 - 1]->isHint = true;
			}
		}
	}
	else if (n->second == 'r')
	{
		if ((n->first % 8 + 3) < 8 && gemGrid[n->first / 8][n->first % 8 + 2]->getColor() == tmpColor && gemGrid[n->first / 8][n->first % 8 + 3]->getColor() == tmpColor)
		{
			gemGrid[n->first / 8][n->first % 8 + 2]->isHint = true;
			gemGrid[n->first / 8][n->first % 8 + 3]->isHint = true;
		}
		else
		{
			if ((n->first / 8 - 1) >= 0 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 - 1][n->first % 8 + 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 - 1][n->first % 8 + 1]->isHint = true;
				if ((n->first / 8 - 2) >= 0 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 - 2][n->first % 8 + 1]->getColor() == tmpColor)
					gemGrid[n->first / 8 - 2][n->first % 8 + 1]->isHint = true;
			}
			if ((n->first / 8 + 1) < 8 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 + 1][n->first % 8 + 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 + 1][n->first % 8 + 1]->isHint = true;
				if ((n->first / 8 + 2) < 8 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 + 2][n->first % 8 + 1]->getColor() == tmpColor)
					gemGrid[n->first / 8 + 2][n->first % 8 + 1]->isHint = true;
			}
		}
	}
	else if (n->second == 'u')
	{
		if ((n->first / 8 - 3) >= 0 && gemGrid[n->first / 8 - 2][n->first % 8]->getColor() == tmpColor && gemGrid[n->first / 8 - 3][n->first % 8]->getColor() == tmpColor)
		{
			gemGrid[n->first / 8 - 2][n->first % 8]->isHint = true;
			gemGrid[n->first / 8 - 3][n->first % 8]->isHint = true;
		}
		else
		{
			if ((n->first / 8 - 1) >= 0 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 - 1][n->first % 8 - 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 - 1][n->first % 8 - 1]->isHint = true;
				if ((n->first / 8 - 1) >= 0 && (n->first % 8 - 2) >= 0 && gemGrid[n->first / 8 - 1][n->first % 8 - 2]->getColor() == tmpColor)
					gemGrid[n->first / 8 - 1][n->first % 8 - 2]->isHint = true;
			}
			if ((n->first / 8 - 1) >= 0 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 - 1][n->first % 8 + 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 - 1][n->first % 8 + 1]->isHint = true;
				if ((n->first / 8 - 1) >= 0 && (n->first % 8 + 2) < 8 && gemGrid[n->first / 8 - 1][n->first % 8 + 2]->getColor() == tmpColor)
					gemGrid[n->first / 8 - 1][n->first % 8 + 2]->isHint = true;
			}
		}
	}
	else if (n->second == 'd')
	{
		if ((n->first / 8 + 3) < 8 && gemGrid[n->first / 8 + 2][n->first % 8]->getColor() == tmpColor && gemGrid[n->first / 8 + 3][n->first % 8]->getColor() == tmpColor)
		{
			gemGrid[n->first / 8 + 2][n->first % 8]->isHint = true;
			gemGrid[n->first / 8 + 3][n->first % 8]->isHint = true;
		}
		else
		{
			if ((n->first / 8 + 1) < 8 && (n->first % 8 - 1) >= 0 && gemGrid[n->first / 8 + 1][n->first % 8 - 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 + 1][n->first % 8 - 1]->isHint = true;
				if ((n->first / 8 + 1) < 8 && (n->first % 8 - 2) >= 0 && gemGrid[n->first / 8 + 1][n->first % 8 - 2]->getColor() == tmpColor)
					gemGrid[n->first / 8 + 1][n->first % 8 - 2]->isHint = true;
			}
			if ((n->first / 8 + 1) < 8 && (n->first % 8 + 1) < 8 && gemGrid[n->first / 8 + 1][n->first % 8 + 1]->getColor() == tmpColor)
			{
				gemGrid[n->first / 8 + 1][n->first % 8 + 1]->isHint = true;
				if ((n->first / 8 + 1) < 8 && (n->first % 8 + 2) < 8 && gemGrid[n->first / 8 + 1][n->first % 8 + 2]->getColor() == tmpColor)
					gemGrid[n->first / 8 + 1][n->first % 8 + 2]->isHint = true;
			}
		}
	}
	availableMoves.clear();

	hintRequested = false;
}

void Game::disableGems() {
	if (AreGemsEnabled) {
		changeGemsState(false);

		AreGemsEnabled = false;
	}
}

void Game::enableGems() {
	if (!AreGemsEnabled) {
		changeGemsState(true);

		AreGemsEnabled = true;
	}
}

void Game::changeGemsState(bool State) {
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			gemGrid[i][j]->isEnabled = State;
}