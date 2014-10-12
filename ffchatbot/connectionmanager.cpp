#include <utility>

#include <QFile>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

#include "connectionmanager.h"
#include "prefixmanager.h"

#include "xmppclient.h"
#include "QXmppMucManager.h"
#include "QXmppUtils.h"


ConnectionManager* ConnectionManager::_instance = nullptr;


ConnectionManager::ConnectionManager()
{
}

void ConnectionManager::Cleanup()
{
	for (auto i = _connections.begin(); i != _connections.end(); ++i)
	{
		std::shared_ptr<XmppClient> client = i->second.Client;
		client->disconnectFromServer();
	}

	_connections.clear();
}

bool ConnectionManager::LoadMOTD(const QString& file)
{
	QFile f(file);
	if (f.open(QIODevice::ReadOnly))
	{
		// Load the data from the settings file.
		QByteArray filedata = f.readAll();
		f.close();

		MOTD = QString(filedata);

		return true;
	}

	return false;
}

bool ConnectionManager::SaveMOTD(const QString& file) const
{
	QFile f(file);
	if (f.open(QIODevice::WriteOnly))
	{
		f.write(MOTD.toUtf8());
		f.close();

		return true;
	}

	return false;
}

int ConnectionManager::LoadCharacters(const QString &file)
{
	int loaded = 0;

	QFile f(file);
	if (f.open(QIODevice::ReadOnly))
	{
		// Load the data from the settings file.
		QByteArray filedata = f.readAll();
		f.close();

		// Convert to a QJsonDocument object.
		QJsonDocument json = QJsonDocument::fromJson(filedata);
		if (json.isNull())
			return 0;

		// Read some of our settings.
		QJsonObject obj = json.object();
		for (auto i = obj.begin(); i != obj.end(); ++i)
		{
			SConnectionDetails c;
			c.Name = i.key();

			QJsonObject o = i.value().toObject();
			auto guid = o.find("GUID");
			auto email = o.find("E-Mail");
			auto password = o.find("Password");
			auto channel = o.find("Channel");

			if (channel == o.end())
				continue;

			// Get the GUID.
			if (guid != o.end())
				c.GUID = guid.value().toString();
			else if (email != o.end() && password != o.end())
				c.GUID = buildGUID(email.value().toString(), password.value().toString());
			else continue;

			// Get our channel info.
			QJsonObject chanobj = channel.value().toObject();
			QStringList k = chanobj.keys();
			QString prefix = k.at(0);
			QString chan = chanobj[prefix].toString();

			// Save the channel and prefix.
			c.Channel = chan;
			c.Prefix = prefix;
			PrefixManager::Instance().add_prefix(prefix, chan);

			// Create our XmppClient!
			std::shared_ptr<XmppClient> client(new XmppClient());
			c.Client = client;

			// Load the MUC (multi-user channel) extension and get our channel list.
			client->load_muc_extension();

			// Set up the XMPP network.
			client->connect(c.Name, c.GUID);

			_connections.insert(std::make_pair(c.Name.toLower(), c));
			++loaded;
		}

		return loaded;
	}

	return 0;
}

QString ConnectionManager::GetCharacterChannel(const QString& name)
{
	auto i = _connections.find(name.toLower());
	if (i == _connections.end())
		return QString();

	return i->second.Channel;
}

void ConnectionManager::SendMessage(const QString& from, const QString& msg)
{
	QString _from = from.toLower();
	for (auto i = _connections.begin(); i != _connections.end(); ++i)
	{
		if (_from.toLower() == i->first.toLower())
			continue;

		std::shared_ptr<XmppClient> client = i->second.Client;
		client->send_to_all(msg);
	}
}

QStringList ConnectionManager::BuildLoginMessage() const
{
	QStringList ret;

	// Construct user list.
	QString message;
	int total_users = 0;
	for (auto& kv: _connections)
	{
		const SConnectionDetails& detail = kv.second;
		auto ext = detail.Client->extensions();
		if (ext.count() == 0)
			continue;

		QXmppMucManager* muc = reinterpret_cast<QXmppMucManager*>(ext.at(0));
		if (muc == nullptr)
			continue;

		for (auto r: muc->rooms())
		{
			QString p = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(r->jid()));
			auto participants = r->participants();
			if (participants.length() > 1)
			{
				total_users += participants.length() - 1;
				message.append(". ");
				message.append("[" + p + "]: ");
				for (auto j = participants.begin(); j != participants.end(); ++j)
				{
					// Don't count myself.
					QString cu = QXmppUtils::jidToResource(*j);
					if (cu.toUpper() == detail.Name.toUpper())
						continue;

					message.append(cu);
					message.append(", ");
				}
				message.remove(message.length() - 2, 2);
			}
		}
	}

	// Add user list.
	message.insert(0, "Users online [" + QString::number(total_users) + "]");
	ret.append(message);

	// Add MOTD.
	ret.append(QString("[MOTD] " + MOTD));

	return ret;
}

QString ConnectionManager::buildGUID(const QString &email, const QString &password)
{
	// <email>-<pw>-red5salt-7nc9bsj4j734ughb8r8dhb8938h8by987c4f7h47b
	QByteArray secret = email.toUtf8() + "-" + password.toUtf8() + "-red5salt-7nc9bsj4j734ughb8r8dhb8938h8by987c4f7h47b";
	QByteArray hash = QCryptographicHash::hash(secret,
		QCryptographicHash::Sha1);

	return hash.toHex().toLower();
}
