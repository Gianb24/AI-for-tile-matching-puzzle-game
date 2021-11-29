// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#include "sounds.h"

Sounds* Sounds::getInstance()
{
	static Sounds uniqueInstance;
	return &uniqueInstance;
}

Sounds::Sounds()
{
	sounds["gameover"] = new QSound(":/res/sounds/gameover.wav");
	sounds["getready"] = new QSound(":/res/sounds/getready.wav");
	sounds["options"] = new QSound(":/res/sounds/options.wav");
	sounds["options3"] = new QSound(":/res/sounds/options3.wav");
	sounds["bad"] = new QSound(":/res/sounds/bad2.wav");
	sounds["move"] = new QSound(":/res/sounds/combo32.wav");
	sounds["click"] = new QSound(":/res/sounds/click2.wav");
	sounds["hint"] = new QSound(":/res/sounds/hint.wav");
}

void Sounds::play(const std::string& id)
{
	if (sounds.find(id) != sounds.end())
		sounds[id]->play();
}