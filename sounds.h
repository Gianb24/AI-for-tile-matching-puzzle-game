// Università degli Studi di Cassino e del Lazio Meridionale
//
// Corso di Tecniche di Programmazione
//
// Prof. Alessandro Bria
//
// Anno Accademico 2020-2021


#pragma once

#include <QSound>

#include <map>
#include <string>

class Sounds
{
private:
	std::map< std::string, QSound* > sounds;

public:
	Sounds();

	static Sounds* getInstance();
	void play(const std::string& id);
};