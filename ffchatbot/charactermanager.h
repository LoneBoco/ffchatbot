#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

#include <map>
#include <ctime>
#include <stdint.h>

#include <QJsonDocument>
#include <QStringList>

struct SCharacterDetails
{
	QString Name;
	time_t LastSeen;
	uint32_t Zone;
};

typedef std::map<QString, SCharacterDetails>::iterator  TCharIter;

class CharacterManager
{
public:
	static CharacterManager& Instance()
	{
		if (CharacterManager::_instance == nullptr)
			CharacterManager::_instance = new CharacterManager();
		return *CharacterManager::_instance;
	}

	void LoadCharacters(const QString& file);
	void SaveCharacters(const QString& file);

	TCharIter AddNewCharacter(const QString& name);

	void SetLastSeen(const QString& name, time_t lastseen);
	void SetZone(const QString& name, uint32_t zone);

	QStringList GetInactives(int days);

protected:
	CharacterManager();

	static CharacterManager* _instance;

	std::map<QString, SCharacterDetails> _characters;
};

#endif // CHARACTERMANAGER_H
