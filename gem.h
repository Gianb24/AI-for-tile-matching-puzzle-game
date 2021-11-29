// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include "config.h"

#if Debug
	#include <QDebug>
#endif

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItem>
#include <QPixmap>
#include <QPainter>

class Gem : public QObject, public QGraphicsItem
{
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)
public:
	enum class gemColor { YELLOW, WHITE, BLUE, RED, PURPLE, ORANGE, GREEN };
	Q_ENUM(gemColor)

	explicit Gem(int, int, QObject* parent = nullptr);

	gemColor getColor() const {
		return color; 
	}
	QString getStringColor() const;
	void setColor(gemColor newColor);
	void setStringColor(QString);
	void initGemPixmap();

	bool isEnabled = true;
	bool isHover = false;
	bool isSelected = false;
	bool isMatched = false;
	bool isHint = false;
	bool isBeingAnimated = false;

	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

public slots:
	void animate();

signals:
	void gemClicked(int row, int col);
	void gemSwitched();

private:
	gemColor color;
	QPixmap* spriteImage = nullptr;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	QRectF boundingRect() const;

	static const int spriteWidth = 780;
	static const int spriteRectWidth = 52;
	static const int spriteRectHeight = 52;
	int currentFrameWidth = 0;
	int currentFrameHeight = 0;

	int rowIndex;
	int columnIndex;
};
