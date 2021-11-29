// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#include <QTimer>

#include "gem.h"
#include "game.h"
#include "scene.h"
#include "config.h"

Gem::Gem(int row, int col, QObject* parent)
{
	rowIndex = row;
	columnIndex = col;

	spriteImage = new QPixmap(":/res/images/gem-sprite.png");

	setAcceptHoverEvents(true);
}

void Gem::animate()
{
	if (isEnabled) {
		if (isHover || isSelected || (isHint && Game::getInstance().hintRequested))
		{
			currentFrameWidth += spriteRectWidth;

			if (currentFrameWidth >= spriteWidth)
				currentFrameWidth = 0;
			update(0, 0, spriteRectWidth, spriteRectHeight);
		}
	}
}

QRectF Gem::boundingRect() const
{
	return QRectF(0, 0, spriteRectWidth, spriteRectHeight);
}

void Gem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->drawPixmap(0, 0, *spriteImage, currentFrameWidth, currentFrameHeight, spriteRectWidth, spriteRectHeight);
}

QString Gem::getStringColor() const
{
	if (color == gemColor::YELLOW)
		return QString("YELLOW");
	else if (color == gemColor::WHITE)
		return QString("WHITE");
	else if (color == gemColor::BLUE)
		return QString("BLUE");
	else if (color == gemColor::RED)
		return QString("RED");
	else if (color == gemColor::PURPLE)
		return QString("PURPLE");
	else if (color == gemColor::ORANGE)
		return QString("ORANGE");
	else if (color == gemColor::GREEN)
		return QString("GREEN");

	return QString("");
}

void Gem::setStringColor(QString str)
{
	if (str == QString("YELLOW"))
		color = gemColor::YELLOW;
	else if (str == QString("WHITE"))
		color = gemColor::WHITE;
	else if (str == QString("BLUE"))
		color = gemColor::BLUE;
	else if (str == QString("RED"))
		color = gemColor::RED;
	else if (str == QString("PURPLE"))
		color = gemColor::PURPLE;
	else if (str == QString("ORANGE"))
		color = gemColor::ORANGE;
	else if (str == QString("GREEN"))
		color = gemColor::GREEN;
}

void Gem::initGemPixmap()
{
	currentFrameWidth = 0;
	currentFrameHeight = 0;
	QString s_color = QVariant::fromValue(color).toString();

	if (s_color == "YELLOW")
		currentFrameHeight = spriteRectHeight;
	else if (s_color == "WHITE")
		currentFrameHeight = spriteRectHeight * 3;
	else if (s_color == "BLUE")
		currentFrameHeight = spriteRectHeight * 5;
	else if (s_color == "RED")
		currentFrameHeight = spriteRectHeight * 7;
	else if (s_color == "PURPLE")
		currentFrameHeight = spriteRectHeight * 9;
	else if (s_color == "ORANGE")
		currentFrameHeight = spriteRectHeight * 11;
	else if (s_color == "GREEN")
		currentFrameHeight = spriteRectHeight * 13;
}

void Gem::setColor(gemColor newColor) {
	color = newColor;
}

void Gem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (isEnabled) {
		if (isUnderMouse() && Game::getInstance().isRunning())
			if (!isSelected)
			{
				isSelected = true;
				currentFrameHeight -= spriteRectHeight;
				#if Debug
					qDebug() << "selected gem (" << rowIndex << "," << columnIndex << ")" << " of color" << color;
				#endif
			}
			else
			{
				isSelected = false;
				currentFrameHeight += spriteRectHeight;
			}
		emit gemClicked(rowIndex, columnIndex);
	}
}

void Gem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	if (isEnabled) {
		QGraphicsItem::hoverEnterEvent(event);
		isHover = true;
	}
}

void Gem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	if (isEnabled) {
		QGraphicsItem::hoverEnterEvent(event);
		isHover = false;
		if (!isSelected)
			currentFrameWidth = 0;
	}
}