// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#include <QKeyEvent>
#include <QMouseEvent>
#include <QFileInfo>

#include "scene.h"
#include "game.h"
#include "gem.h"
#include "dialog.h"

Scene::Scene(QGraphicsView* parent)
	: QGraphicsView(parent)
{
	scene = new QGraphicsScene(0, 0, 640, 480);
	scene->addPixmap(QPixmap(":/res/images/bg-base.png"));
	setScene(scene);
	setRenderHint(QPainter::Antialiasing);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFixedSize(640, 480);

	newGame = new QGraphicsPixmapItem(QPixmap(":/res/images/newgame-pushed.png"));
	addButton(scene, newGame, 16, 140);
	normal = new QGraphicsPixmapItem(QPixmap(":/res/images/normal-pushed.png"));
	addButton(scene, normal, 16, 185);
	options = new QGraphicsPixmapItem(QPixmap(":/res/images/options-pushed.png"));
	addButton(scene, options, 16, 240);
	timeTrial = new QGraphicsPixmapItem(QPixmap(":/res/images/timetrial-pushed.png"));
	addButton(scene, timeTrial, 94, 185);
	ai = new QGraphicsPixmapItem(QPixmap(":/res/images/ai-pushed.png"));
	addButton(scene, ai, 16, 405);
	hint = new QGraphicsPixmapItem(QPixmap(":/res/images/hint.png"));
	addButton(scene, hint, 65, 305);
	hintBox = new QGraphicsRectItem(65, 305, 52, 72); //fix hint button
	hintBox->setOpacity(0);
	scene->addItem(hintBox);
	hintBox->setAcceptHoverEvents(true);

	normalSelected = new QGraphicsPixmapItem(QPixmap(":/res/images/normal-selected.png"));
	addButton(scene, normalSelected, 16, 185);
	showButton(normalSelected);
	timeTrialSelected = new QGraphicsPixmapItem(QPixmap(":/res/images/timetrial-selected.png"));
	addButton(scene, timeTrialSelected, 94, 185);

	initailizeScoreDigits();
	initializeProgressbar();

	Game::getInstance();
	connect(this, SIGNAL(hintRequested()), &Game::getInstance(), SLOT(giveHint())); //Used the old syntax to fix connect to the singleton Game
	connect(this, &Scene::newGameTriggered, [=]
		{
			if (Game::getInstance().name == "")
			{
				Dialog* newGameDialog = new Dialog(QString("newGame"), this);
				newGameDialog->setModal(true);
				newGameDialog->show();
			}
			else
				Game::getInstance().startGame();
		});

	if (QFileInfo::exists("save.json"))
	{
		Dialog* loadDialog = new Dialog(QString("loadGrid"), this);
		loadDialog->setModal(true);
		loadDialog->show();
	}

	//sceneSounds = new Sounds();

	show(); //The window should be displayed on the screen only after everything has been loaded.
}

void Scene::mousePressEvent(QMouseEvent* event)
{
	QGraphicsView::mousePressEvent(event);
	if (newGame->isUnderMouse())
		showButton(newGame);
	else if (ai->isUnderMouse() && Game::getInstance().isRunning())
		showButton(ai);
	else if (timeTrial->isUnderMouse())
		showButton(timeTrial);
	else if (options->isUnderMouse())
		showButton(options);
	else if (normal->isUnderMouse())
		showButton(normal);
}

void Scene::mouseReleaseEvent(QMouseEvent* event)
{
	showButton(newGame, false);
	showButton(timeTrial, false);
	showButton(options, false);
	showButton(normal, false);

	if (newGame->isUnderMouse())
	{
		emit newGameTriggered();
		if (isAiModeOn)
		{
			showButton(ai, false);
			isAiModeOn = false;
			Game::getInstance().name = tmpName;
			Game::getInstance().terminate();
		}
	}
	else if (timeTrial->isUnderMouse())
	{
		showButton(normalSelected, false);
		showButton(timeTrialSelected);
		Sounds::getInstance()->play("click");
	}
	else if (normal->isUnderMouse())
	{
		showButton(normalSelected);
		showButton(timeTrialSelected, false);
		Sounds::getInstance()->play("click");
	}
	else if (options->isUnderMouse())
	{
		Sounds::getInstance()->play("options3");

		Dialog* option = new Dialog("", this);

		option->setModal(true);
		option->show();

		connect(option, &Dialog::closeDialog, [=] {delete option; }); //fix to clear RAM
	}
	else if (ai->isUnderMouse() && Game::getInstance().isRunning())
	{
		if (isAiModeOn)
		{
			showButton(ai, false);
			isAiModeOn = false;
			Game::getInstance().name = tmpName;
			Game::getInstance().terminate();
			Game::getInstance().enableGems();
		}
		else
		{
			showButton(ai);
			isAiModeOn = true;
			tmpName = Game::getInstance().name;
			Game::getInstance().name = QString("AI");
			Game::getInstance().disableGems();
			Game::getInstance().start();
		}
		Sounds::getInstance()->play("click");
	}
	else if (hintBox->isUnderMouse() && Game::getInstance().getScore() >= 5 && Game::getInstance().isRunning() && !isAiModeOn)
	{
		if (!Game::getInstance().hintRequested)
		{
			emit(Game::getInstance().scoreChanged(-5));
			emit hintRequested();

			Game::getInstance().hintRequested = true;
			Sounds::getInstance()->play("hint");
		}
	}
}

void Scene::mouseMoveEvent(QMouseEvent* event)
{
	if (hintBox->isUnderMouse() && !isAiModeOn)
		showButton(hint);
	else if (!hintBox->isUnderMouse())
		showButton(hint, false);
	QGraphicsView::mouseMoveEvent(event);
}

void Scene::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == KeyGameStart)
	{
		//Game::getInstance().startGame(); //Replaced with emit newGameTriggered
		if (Game::getInstance().isRunning())
			if (isAiModeOn)
			{
				showButton(ai, false);
				isAiModeOn = false;
				Game::getInstance().name = tmpName;
				Game::getInstance().terminate();
				Game::getInstance().enableGems();
			}
		emit newGameTriggered();
	}

	if (event->key() == KeyGamePause)
		Game::getInstance().pause();
}

void Scene::closeEvent(QCloseEvent* event)
{
	event->ignore();
	Game::getInstance().saveRanking();
	if (!Game::getInstance().firstGame && !Game::getInstance().isGameOver())
	{
		Dialog* closeDialog = new Dialog(QString("exitGame"), this);
		closeDialog->setModal(true);
		closeDialog->show();
	}
	event->accept();
}

void Scene::initailizeScoreDigits()
{
	_scoreDigitsFullMap = new QPixmap(":/res/images/scorefont.png");

	for (int i = 0; i < 10; i++)
		_scoreDigits[i] = _scoreDigitsFullMap->copy(i * 15, 0, 15, 22);

	displayScore();
	InitializedScore = true;
}

void Scene::displayScore()
{
	long long ScoreShow = Game::getInstance().getScore();
	long long NumberOfDigits = ScoreShow == 0 ? 1 : 0;

	while (ScoreShow > 0) {
		ScoreShow /= 10;
		NumberOfDigits++;
	}

	ScoreShow = Game::getInstance().getScore();

	long long PositionOffset = 175 - (7.5 * (10 - NumberOfDigits));

	for (int i = 0; i < PrecedentNumberOfScoreDigits; i++)
		delete _currentScoreDigits[i];

	for (int i = 0; i < NumberOfDigits; i++)
	{
		_currentScoreDigits[i] = new QGraphicsPixmapItem(_scoreDigits[ScoreShow % 10]);
		_currentScoreDigits[i]->setPos(PositionOffset - (20 + (15 * i)), 87);
		scene->addItem(_currentScoreDigits[i]);
		ScoreShow /= 10;
	}

	PrecedentNumberOfScoreDigits = NumberOfDigits;
}

void Scene::addButton(QGraphicsScene* scene, QGraphicsPixmapItem* button, int x, int y)
{
	button->setPos(x, y);
	button->setOpacity(0);
	scene->addItem(button);
}

void Scene::showButton(QGraphicsPixmapItem* button, bool show)
{
	if (show)
		button->setOpacity(1);
	else button->setOpacity(0);
}

Scene& Scene::getInstance()
{
	static Scene instance;
	return instance;
}

void Scene::initializeProgressbar()
{
	_progressBarFull = new QPixmap(":res/images/titlegreenbar2.png");

	setProgressbarPercent(0);

	updateProgressbar();
}

void Scene::setProgressbarPercent(double Amount) {
	ProgressBarPercent = Amount;

	updateProgressbar();
}

void Scene::updateProgressbar()
{
	if (ProgressBarPercent < 0)
		ProgressBarPercent = 0;
	else if (ProgressBarPercent > 1)
		ProgressBarPercent = 1;

	_progressBarCurrentValue = _progressBarFull->copy(0, 0, (int)(340 * ProgressBarPercent), 24);

	_progressBar = new QGraphicsPixmapItem(_progressBarCurrentValue);

	_progressBar->setPos(236, 445);

	if (ProgressBarPercent > 0)
		_progressBar->setOpacity(1);
	else
		_progressBar->setOpacity(0);

	scene->addItem(_progressBar);
}

bool Scene::isAiActive()
{
	if (isAiModeOn == false)
		return false;
	else return true;
}
