#ifndef PREFIXMANAGER_H
#define PREFIXMANAGER_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
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

	QString get_prefix(QString channel);
	void add_prefix(QString prefix, QString channel);
	QString remove_prefix(QString prefix);
	QStringList get_all_prefixes();

private:
	PrefixManager();

	static PrefixManager* _instance;

	std::map<QString, QString> _channel_prefix;
};

#endif // PREFIXMANAGER_H
