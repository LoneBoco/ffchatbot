#include <utility>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

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
			c.AccessLevel = 0;

			QJsonObject o = i.value().toObject();
			auto army = o.find("army");
			auto lastseen = o.find("lastseen");
			auto accesslevel = o.find("accesslevel");
			if (army != o.end())
				c.Army = army.value().toString();
			if (lastseen != o.end())
				c.LastSeen = (time_t)lastseen.value().toDouble();
			if (accesslevel != o.end())
				c.AccessLevel = (int32_t)accesslevel.value().toDouble();

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
			QJsonValue army(i->second.Army);
			QJsonValue lastseen((int)i->second.LastSeen);
			QJsonValue accesslevel((int)i->second.AccessLevel);

			QJsonObject data;
			data.insert("army", army);
			data.insert("lastseen", lastseen);
			data.insert("accesslevel", accesslevel);

			out.insert(i->first, data);
		}

		QJsonDocument doc(out);
		f.write(doc.toJson());

		f.close();
	}
}

TCharIter CharacterManager::AddNewCharacter(const QString& name, const QString& army)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i != _characters.end())
		return i;

	SCharacterDetails c;
	c.Name = uname;
	c.Army = army;
	c.LastSeen = 0;
	c.AccessLevel = 0;

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

void CharacterManager::SetLastSeen(const QString& name, const QString& army, time_t lastseen)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		i = AddNewCharacter(uname, army);

	i->second.LastSeen = lastseen;
}

bool CharacterManager::SetAccessLevel(const QString& name, int32_t access)
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		return false;

	i->second.AccessLevel = access;
	return true;
}

int CharacterManager::GetAccessLevel(const QString &name) const
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		return 0;

	return i->second.AccessLevel;
}

QStringList CharacterManager::GetInactives(int days, const QString& army) const
{
	QStringList ret;

	for (auto i = _characters.begin(); i != _characters.end(); ++i)
	{
		if (i->second.Army.toUpper() != army.toUpper())
			continue;

		QDateTime lastseen = QDateTime::fromTime_t(i->second.LastSeen);
		if (lastseen.addDays(days) > QDateTime::currentDateTime())
			continue;

		ret.append(i->second.Name);
	}

	return ret;
}

QString CharacterManager::GetInfo(const QString& name) const
{
	QString uname = name.toUpper();
	auto i = _characters.find(uname);
	if (i == _characters.end())
		return "Player not found.";

	QDateTime lastseen = QDateTime::fromTime_t(i->second.LastSeen);

	QString ret;
	QTextStream(&ret) << "[" << i->second.Army << "] " << i->second.Name
					  << ": Access " << i->second.AccessLevel
					  << ", last seen: " << lastseen.toString(Qt::TextDate);

	return ret;
}
