#ifndef PREFIXMANAGER_H
#define PREFIXMANAGER_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <map>
#include <set>

class PrefixManager
{
public:
	~PrefixManager();

	static PrefixManager& Instance()
	{
		if (PrefixManager::_instance == nullptr)
			PrefixManager::_instance = new PrefixManager();
		return *PrefixManager::_instance;
	}

	void load_prefixes(QJsonObject& json);
	QString get_prefix(QString channel);

private:
	PrefixManager();

	static PrefixManager* _instance;

	std::map<QString, QString> _channel_prefix;
};

#endif // PREFIXMANAGER_H
