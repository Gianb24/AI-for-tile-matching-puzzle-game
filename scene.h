// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include "config.h"
#include "sounds.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneHoverEvent>

class Scene : public QGraphicsView
{
	Q_OBJECT

signals:
	void newGameTriggered();
	void hintRequested();

public slots:
	void displayScore();

public:
	static Scene& getInstance();
	Scene(Scene const&) = delete;
	void operator=(Scene const&) = delete;
	QGraphicsScene* getScene() { return scene; }
	void showButton(QGraphicsPixmapItem* button, bool show = true);
	QGraphicsPixmapItem* ai;
	bool isAiActive();

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	void closeEvent(QCloseEvent* event);

private:
	explicit Scene(QGraphicsView* parent = nullptr);
	QGraphicsScene* scene;
	QGraphicsPixmapItem* newGame;
	QGraphicsPixmapItem* normal;
	QGraphicsPixmapItem* normalSelected;
	QGraphicsPixmapItem* timeTrial;
	QGraphicsPixmapItem* timeTrialSelected;
	QGraphicsPixmapItem* hint;
	QGraphicsPixmapItem* options;
	QGraphicsRectItem* hintBox;

	void initializeProgressbar();
	void setProgressbarPercent(double Amount);
	void updateProgressbar();
	void addButton(QGraphicsScene* scene, QGraphicsPixmapItem* button, int x, int y);
	void initailizeScoreDigits();
	QPixmap* _scoreDigitsFullMap;
	QPixmap _scoreDigits[10];
	QGraphicsPixmapItem* _currentScoreDigits[10];
	long long PrecedentNumberOfScoreDigits = 0;
	bool InitializedScore;
	bool isAiModeOn = false;
	QPixmap* _progressBarFull;
	QPixmap _progressBarCurrentValue;
	QGraphicsPixmapItem* _progressBar;
	double ProgressBarPercent;
	QString tmpName;
};
