// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#include <QApplication>
#include <QIcon>
#include "scene.h"
#include "sounds.h"

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	a.setWindowIcon(QIcon(":/res/images/bejeweled-icon.png"));
	a.setApplicationDisplayName("Bejeweled");
	
	Sounds::getInstance();
	Scene::getInstance();
	return a.exec();
}
