#include <utility>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>

#include "charactermanager.h"


CharacterManager* CharacterManager::_instance = nullptr;


CharacterManager::CharacterManager()
{
}

void CharacterManager::LoadCharacters(const QString& file)
{
	_characters.clear();

	QFile f(file);
	if (f.open(QIODevice::ReadOnly))
	{
		// Load the data from the settings file.
		QByteArray filedata = f.readAll();
		f.close();

		// Convert to a QJsonDocument object.
		QJsonDocument json = QJsonDocument::fromJson(filedata);
		if (json.isNull())
			return;

		QJsonObject obj = json.object();
		for (auto i = obj.begin(); i != obj.end(); ++i)
		{
			SCharacterDetails c;
			c.Name = i.key();
			c.LastSeen = 0;
			c.Zone = 0;

			QJsonObject o = i.value().toObject();
			auto lastseen = o.find("lastseen");
			auto zone = o.find("zone");
			if (lastseen != o.end())
				c.LastSeen = (time_t)lastseen.value().toDouble();
			if (zone != o.end())
				c.Zone = (uint32_t)zone.value().toDouble();

			_characters.insert(std::make_pair(c.Name, c));
		}
	}
}

void CharacterManager::SaveCharacters(const QString& file)
{
	// Make a backup.
	QFile::remove(file + ".bak");
	QFile::copy(file, file + ".bak");

	// Open the settings file.
	QFile f(file);
	if (f.open(QIODevice::WriteOnly))
	{
		QJsonObject out;

		for (TCharIter i = _characters.begin(); i != _characters.end(); ++i)
		{
			QJsonValue lastseen((int)i->second.LastSeen);
			QJsonValue zone((int)i->second.Zone);

			QJsonObject data;
			data.insert("lastseen", lastseen);
			data.insert("zone", zone);

			out.insert(i->first, data);
		}

		QJsonDocument doc(out);
		f.write(doc.toJson());

		f.close();
	}
}

TCharIter CharacterManager::AddNewCharacter(const QString& name)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i != _characters.end())
		return i;

	SCharacterDetails c;
	c.Name = uname;
	c.LastSeen = 0;
	c.Zone = 0;

	std::pair<TCharIter, bool> ret = _characters.insert(std::make_pair(uname, c));
	return ret.first;
}

bool CharacterManager::RemoveCharacter(const QString& name)
{
	auto i = _characters.find(name.toUpper());
	if (i != _characters.end())
	{
		_characters.erase(i);
		return true;
	}

	return false;
}

void CharacterManager::SetLastSeen(const QString& name, time_t lastseen)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		i = AddNewCharacter(uname);

	i->second.LastSeen = lastseen;
}

void CharacterManager::SetZone(const QString& name, uint32_t zone)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		i = AddNewCharacter(uname);

	i->second.Zone = zone;
}

QStringList CharacterManager::GetInactives(int days)
{
	QStringList ret;

	for (auto i = _characters.begin(); i != _characters.end(); ++i)
	{
		QDateTime lastseen = QDateTime::fromTime_t(i->second.LastSeen);
		if (lastseen.addDays(days) > QDateTime::currentDateTime())
			continue;

		ret.append(i->second.Name);
	}

	return ret;
}
