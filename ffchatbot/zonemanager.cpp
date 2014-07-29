#include <utility>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include "zonemanager.h"


ZoneManager* ZoneManager::_instance = nullptr;


ZoneManager::ZoneManager()
{
}

void ZoneManager::LoadZones(const QString& file)
{
	_zones.clear();

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
			SZoneDetails c;
			c.ID = i.key().toInt();

			QJsonObject o = i.value().toObject();
			auto name = o.find("name");
			auto abbr = o.find("abbr");
			if (name != o.end())
				c.Name = name.value().toString();
			if (abbr != o.end())
			{
				QJsonArray abbrs = abbr.value().toArray();
				for (QJsonValue v: abbrs)
				{
					c.Abbr.push_back(v.toString());
				}
			}

			_zones.insert(std::make_pair(c.ID, c));
		}
	}
}

QString ZoneManager::GetZone(const QString &test)
{
	for (auto i = _zones.begin(); i != _zones.end(); ++i)
	{
		std::vector<QString>& abbr = i->second.Abbr;
		for (QString s: abbr)
		{
			if (s == test)
				return i->second.Name;
		}
	}

	return QString();
}
