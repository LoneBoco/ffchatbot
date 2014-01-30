#include "prefixmanager.h"
#include <utility>


PrefixManager* PrefixManager::_instance = nullptr;


PrefixManager::PrefixManager()
{
}

PrefixManager::~PrefixManager()
{
}

void PrefixManager::load_prefixes(QJsonObject& json)
{
	_channel_prefix.clear();
	for (auto i = json.begin(); i != json.end(); ++i)
	{
		_channel_prefix.insert(std::make_pair(i.value().toString(), i.key()));
	}
}

QString PrefixManager::get_prefix(QString channel)
{
	auto i = _channel_prefix.find(channel);
	if (i == _channel_prefix.end())
		return QString();
	return i->second;
}

void PrefixManager::add_prefix(QString prefix, QString channel)
{
	_channel_prefix.insert(std::make_pair(channel, prefix));
}

QString PrefixManager::remove_prefix(QString prefix)
{
	for (auto i = _channel_prefix.begin(); i != _channel_prefix.end(); ++i)
	{
		if (i->second == prefix)
		{
			QString ret = i->first;
			_channel_prefix.erase(i);
			return ret;
		}
	}
	return QString();
}

QStringList PrefixManager::get_all_prefixes()
{
	QStringList ret;
	for (auto i = _channel_prefix.begin(); i != _channel_prefix.end(); ++i)
	{
		ret.append(i->second);
	}
	return ret;
}
