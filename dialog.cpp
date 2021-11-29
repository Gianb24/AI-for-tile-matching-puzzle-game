// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#include "config.h"
#include "dialog.h"
#include "game.h"

#include <locale>

bool Dialog::isValidPlayerName(QString Name) {
	unsigned char CurrentCharacter;

	for (int i = 0; i < Name.size(); i++) {
		CurrentCharacter = Name.at(i).toLatin1();

		if (!isalnum(CurrentCharacter))
			return false;
	}

	return true;
}

Button::Button(const QString text, QWidget* parent)
	: QAbstractButton(parent)
{
	buttonText = text;
}

void Button::paintEvent(QPaintEvent* e)
{
	buttonIcon = new QPixmap(":/res/images/boxbutton2.png");
	QPainter painter(this);
	painter.drawPixmap(0, 0, buttonIcon->scaled(size()));
	QFont f(DIALOG_FONT, 10);
	painter.setFont(f);
	painter.drawText(QRect(0, 0, 96, 32), Qt::AlignCenter, buttonText);
}

Dialog::Dialog(QString str, QGraphicsView* parent)
{
	dialogType = str;
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setFixedSize(520, 360);
	move(parent->x() + 50, parent->y() + 50);
	if (dialogType == "loadGrid")
		dialogLoadGrid();
	else if (dialogType == "exitGame")
		dialogExitGame();
	else if (dialogType == "newGame")
		dialogNewGame();
	else dialogShowOption();
}

void Dialog::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	QPixmap pix(":/res/images/dialogbox2.png");

	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.drawPixmap(0, 0, pix.scaled(size()));

	QFont f(DIALOG_FONT, 16, QFont::Bold);

	painter.setFont(f);
	painter.setPen(Qt::white);

	if (dialogType == QString("loadGrid"))
		painter.drawText(QRect(115, 100, 520, 360), Qt::AlignTop,
			QString("A previous match was found.\nDo you want to restore it?"));
	else if (dialogType == QString("exitGame"))
		painter.drawText(QRect(115, 100, 520, 360), Qt::AlignTop,
			QString("Do you want to save the\ncurrent match?"));
	else if (dialogType == QString("newGame"))
		painter.drawText(QRect(115, 100, 520, 360), Qt::AlignTop,
			QString("Enter your nickname:"));
	else
	{
		QFont g(DIALOG_FONT, 16, QFont::Bold);
		QFont h(DIALOG_FONT, 14);

		unsigned int ScoreSpacing = 70;
		unsigned int ScoreSpacingIncrement;
		QString RankingPlace;
		QString Score;

		for (unsigned int i = 0; i <= RANKING_PLACES; i++) {
			if (i == 0) {
				painter.setFont(g);
				ScoreSpacingIncrement = 30;
			}
			else {
				painter.setFont(h);
				ScoreSpacingIncrement = 25;
			}

			RankingPlace = QString::number(i + 1);
			Score = QString::number(Game::getInstance().highscoreValue[i]);

			switch (RankingPlace.back().toLatin1())
			{
			case '1':
				RankingPlace += QString("st");
				break;
			case '2':
				RankingPlace += QString("nd");
				break;
			case '3':
				RankingPlace += QString("rd");
				break;
			default:
				RankingPlace += QString("th");
				break;
			}

			RankingPlace += QString(":");

			while (Score.size() < 10)
				Score = QString("0") + Score;

			painter.drawText(QRect(125, ScoreSpacing, 520, 100), Qt::AlignTop,
				RankingPlace);
			painter.drawText(QRect(180, ScoreSpacing, 520, 100), Qt::AlignTop,
				Game::getInstance().highscoreName[i]);

			if (i == 0)
				painter.setFont(h);

			painter.drawText(QRect(240, ScoreSpacing, 520, 100), Qt::AlignTop,
				QString("|"));

			if (i == 0)
				painter.setFont(g);

			painter.drawText(QRect(270, ScoreSpacing, 520, 100), Qt::AlignTop,
				Score);

			ScoreSpacing += ScoreSpacingIncrement;
		}

		ScoreSpacing += 50;

		painter.drawText(QRect(285, ScoreSpacing, 520, 100), Qt::AlignTop,
			QString("Name: ") + Game::getInstance().name);
	}
}

void Dialog::dialogLoadGrid()
{
	Button* buttonYes = new Button("Yes", this);
	Button* buttonNo = new Button("No", this);

	buttonYes->setGeometry(140, 230, 90, 30);
	buttonYes->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
	buttonNo->setGeometry(280, 230, 90, 30);
	buttonNo->setStyleSheet(DIALOG_BUTTON_STYLESHEET);

	connect(buttonNo, &QAbstractButton::clicked, this, &QDialog::close);
	connect(buttonYes, &QAbstractButton::clicked, [=]
		{
			Game::getInstance().loadGame();
			close();
		});
}

void Dialog::dialogExitGame()
{
	Button* buttonYes = new Button("Yes", this);
	Button* buttonNo = new Button("No", this);

	buttonYes->setGeometry(140, 230, 90, 30);
	buttonYes->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
	buttonNo->setGeometry(280, 230, 90, 30);
	buttonNo->setStyleSheet(DIALOG_BUTTON_STYLESHEET);

	connect(buttonNo, &QAbstractButton::clicked, [=]
		{
			//QFile::remove("save.json");
			close();
		});
	connect(buttonYes, &QAbstractButton::clicked, [=] {Game::getInstance().saveGame(); close(); });
}

void Dialog::dialogShowOption()
{
	Button* buttonCancel = new Button("Cancel", this);
	Button* buttonResetRanking = new Button("ResetRanking", this);
	Button* buttonChangeName = new Button("ChangeName", this);

	buttonCancel->setGeometry(320, 215, 90, 30);
	buttonCancel->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
	buttonResetRanking->setGeometry(218, 215, 100, 30);
	buttonResetRanking->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
	buttonChangeName->setGeometry(115, 215, 100, 30);
	buttonChangeName->setStyleSheet(DIALOG_BUTTON_STYLESHEET);

	connect(buttonCancel, &QAbstractButton::clicked, [=] {close(); emit closeDialog(); });
	connect(buttonResetRanking, &QAbstractButton::clicked, [=]
		{
			Game::getInstance().resetRanking();
			update();
		});
	connect(buttonChangeName, &QAbstractButton::clicked, [=] 
		{		
			QLineEdit* text = new QLineEdit(this);
			text->setGeometry(115, 250, 120, 30);
			text->clear();
			text->setPlaceholderText(QString("Enter nickname..."));

			Button* newName = new Button("", this);
			newName->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
			newName->setGeometry(240, 250, 30, 30);

			connect(text, &QLineEdit::textChanged, [=]
				{
					if (text->text().size() < MinimumNameLength || text->text().size() > MaximumNameLength || !isValidPlayerName(text->text()))
						newName->hide();
					else
						newName->show();
				});

			connect(newName, &QAbstractButton::clicked, [=]
				{
					Game::getInstance().name = text->text();

					text->hide();
					newName->hide();
					update();
				});

			text->show();
		});
}

void Dialog::dialogNewGame()
{
	Button* buttonStart = new Button("Start", this);
	buttonStart->setGeometry(280, 230, 90, 30);
	buttonStart->setStyleSheet(DIALOG_BUTTON_STYLESHEET);
	buttonStart->hide();

	QLineEdit* text = new QLineEdit(this);
	text->setGeometry(115, 150, 120, 30);
	text->show();

	connect(text, &QLineEdit::textChanged, [=]
		{
			if (text->text().size() < MinimumNameLength || text->text().size() > MaximumNameLength || !isValidPlayerName(text->text()))
				buttonStart->hide();
			else
				buttonStart->show();
		});

	connect(buttonStart, &QAbstractButton::clicked, [=]
		{
			Game::getInstance().name = text->text();
			Game::getInstance().startGame();
			close();
		});
}
