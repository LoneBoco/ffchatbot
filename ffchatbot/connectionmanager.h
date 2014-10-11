#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <map>
#include <memory>

#include <QString>

class XmppClient;

struct SConnectionDetails
{
	QString Name;
	QString GUID;
	QString Channel;
	QString Prefix;
	std::shared_ptr<XmppClient> Client;
};

class ConnectionManager
{
public:
	static ConnectionManager& Instance()
	{
		if (ConnectionManager::_instance == nullptr)
			ConnectionManager::_instance = new ConnectionManager();
		return *ConnectionManager::_instance;
	}

	void Cleanup();

	bool LoadMOTD(const QString& file);
	bool SaveMOTD(const QString& file) const;

	int LoadCharacters(const QString& file);

	QString GetCharacterChannel(const QString& name);

	void SendMessage(const QString& from, const QString& msg);

	QString MOTD;

private:
	ConnectionManager();
	QString buildGUID(const QString& email, const QString& password);

	static ConnectionManager* _instance;

	std::map<QString, SConnectionDetails> _connections;
};

#endif // CONNECTIONMANAGER_H
