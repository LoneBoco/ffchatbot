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
	QString Army;
	time_t LastSeen;
	int32_t AccessLevel;
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

	TCharIter AddNewCharacter(const QString& name, const QString& army);
	bool RemoveCharacter(const QString& name);

	void SetLastSeen(const QString& name, const QString& army, time_t lastseen);

	bool SetAccessLevel(const QString& name, int32_t access);
	int GetAccessLevel(const QString& name) const;

	QStringList GetInactives(int days, const QString& army) const;
	QString GetInfo(const QString& name) const;

protected:
	CharacterManager();

	static CharacterManager* _instance;

	std::map<QString, SCharacterDetails> _characters;
};

#endif // CHARACTERMANAGER_H
