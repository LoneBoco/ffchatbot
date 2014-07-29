#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <map>

#include <QString>

struct SZoneDetails
{
	int ID;
	QString Name;
	std::vector<QString> Abbr;
};

class ZoneManager
{
public:
	static ZoneManager& Instance()
	{
		if (ZoneManager::_instance == nullptr)
			ZoneManager::_instance = new ZoneManager();
		return *ZoneManager::_instance;
	}

	void LoadZones(const QString& file);

	QString GetZone(const QString& test);

private:
	ZoneManager();

	static ZoneManager* _instance;

	std::map<int, SZoneDetails> _zones;
};

#endif // ZONEMANAGER_H
