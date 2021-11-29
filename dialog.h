// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include <QDialog>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QAbstractButton>
#include <QLineEdit>
#include "scene.h" 

class Button : public QAbstractButton
{
	Q_OBJECT
public:
	explicit Button(const QString, QWidget* parent = nullptr);
private:
	QPixmap* buttonIcon;
	QString buttonText;
	void paintEvent(QPaintEvent* e);
};

class Dialog : public QDialog
{
	Q_OBJECT
public:
	explicit Dialog(QString, QGraphicsView* parent = nullptr);
private:
	QPixmap* pixmap;
	QString dialogType;
	bool changeNameVisible = false;
	bool isValidPlayerName(QString Name);
	void paintEvent(QPaintEvent* event);
	void dialogLoadGrid();
	void dialogExitGame();
	void dialogNewGame();
	void dialogShowOption();
signals:
	void closeDialog();
};