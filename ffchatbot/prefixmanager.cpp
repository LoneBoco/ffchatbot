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
