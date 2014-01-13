#include "xmppclient.h"
#include "prefixmanager.h"
#include "QXmppMucManager.h"
#include "QXmppUtils.h"
#include <QJsonDocument>

// From main.cpp
extern QJsonDocument settings;

XmppClient::XmppClient(QObject *parent)
	: QXmppClient(parent),
	  _server("chat.red5studios.com"),
	  _jidserver("firefall.chat.red5studios.com"),
	  _confserver("conference.firefall.chat.red5studios.com")
{	
	bool check;
	Q_UNUSED(check);

	check = QObject::connect(this, SIGNAL(connected()), SLOT(clientConnected()));
	Q_ASSERT(check);

	check = QObject::connect(this, SIGNAL(messageReceived(QXmppMessage)), SLOT(messageReceived(QXmppMessage)));
	Q_ASSERT(check);

	check = QObject::connect(&_reconnect_timer, SIGNAL(timeout()), SLOT(muc_error_timeout()));
	Q_ASSERT(check);
}

XmppClient::~XmppClient()
{
}

void XmppClient::load_muc_extension()
{
	_muc_manager = new QXmppMucManager;
	this->addExtension(_muc_manager);
}

void XmppClient::connect(QString character, QString secret)
{
	_character = character;
	_secret = secret;

	QString JID = character.toUpper() + "@" + _jidserver;
	QXmppConfiguration& config = this->configuration();
	config.setResource("firefall");
	config.setHost(_server);
	config.setJid(JID);
	config.setPassword(secret);
	config.setAutoReconnectionEnabled(true);

	this->logger()->setLoggingType(QXmppLogger::StdoutLogging);	
	this->connectToServer(config);
}

void XmppClient::add_channel(QString channel)
{
	QXmppMucRoom* room = nullptr;

	// See if this room already exists.
	auto rooms = _muc_manager->rooms();
	for (auto i = rooms.begin(); i != rooms.end(); ++i)
	{
		QXmppMucRoom* r = *i;
		if (QXmppUtils::jidToUser(r->jid()).toUpper() == channel.toUpper())
		{
			room = r;
			break;
		}
	}

	// Add a new room.
	if (!room)
	{
		room = _muc_manager->addRoom(channel + "@" + _confserver);

		bool check;
		Q_UNUSED(check);

		check = QObject::connect(room, SIGNAL(messageReceived(QXmppMessage)), SLOT(muc_messageReceived(QXmppMessage)));
		Q_ASSERT(check);

		check = QObject::connect(room, SIGNAL(participantAdded(QString)), SLOT(muc_userJoined(QString)));
		Q_ASSERT(check);

		check = QObject::connect(room, SIGNAL(participantRemoved(QString)), SLOT(muc_userLeft(QString)));
		Q_ASSERT(check);

		check = QObject::connect(room, SIGNAL(error(QXmppStanza::Error)), SLOT(muc_error(QXmppStanza::Error)));
		Q_ASSERT(check);
	}

	room->setNickName(_character.toUpper());
	room->join();
}

void XmppClient::send_pm(QString jid, QString msg)
{
	// Message element.
	QXmppElement message;
	message.setTagName("message");
	message.setAttribute("xml:lang", "*");
	message.setAttribute("to", jid);
	message.setAttribute("from", this->configuration().jid());
	message.setAttribute("type", "normal");
	message.setAttribute("displayname", QXmppUtils::jidToResource(jid));

	// Body element.
	QXmppElement body;
	body.setTagName("body");
	body.setValue(msg);

	// Link them together.
	message.appendChild(body);

	// Send the element.
	sendElement(message);
}

//
// SLOTS
//

void XmppClient::clientConnected()
{
	QJsonObject obj = settings.object();
	auto chans = obj["Channels"];
	if (chans.isNull())
		return;

	// Register each channel.
	auto o = chans.toObject();
	for (auto i = o.begin(); i != o.end(); ++i)
	{
		add_channel(i.value().toString());
	}
}

void XmppClient::messageReceived(const QXmppMessage& message)
{
}

void XmppClient::muc_messageReceived(const QXmppMessage& message)
{
	QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
	if (!room)
		return;

	// Only do group chats for now.
	if (message.type() != QXmppMessage::GroupChat)
		return;

	// See who it is from.  If it is from ourself, don't relay it.
	QString from = QXmppUtils::jidToResource(message.from());
	if (from.isEmpty() || from.toUpper() == _character.toUpper())
		return;

	// Determine the channel prefix.
	auto prefix = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(room->jid()));
	QString msg = "[" + prefix + "] " + from + ": " + message.body();

	// Loop through each room relaying the message.
	auto rooms = _muc_manager->rooms();
	for (auto i = rooms.begin(); i != rooms.end(); ++i)
	{
		// Ignore the room that the message is from.
		if ((*i) == room)
			continue;

		QXmppMucRoom* r = *i;
		r->sendMessage(msg);
	}
}

void XmppClient::muc_userJoined(const QString& jid)
{
	QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
	if (!room)
		return;

	QString user = QXmppUtils::jidToResource(jid);

	// Ignore ourself.
	if (user.toUpper() == _character.toUpper())
		return;

	QString prefix = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(room->jid()));
	QString message = "[" + prefix + "] " + user + " has logged in.";

	// Loop through each room informing of the user's joining.
	auto rooms = _muc_manager->rooms();
	for (auto i = rooms.begin(); i != rooms.end(); ++i)
	{
		// Ignore the room that just sent the message.
		if ((*i) == room)
			continue;

		QXmppMucRoom* r = *i;
		r->sendMessage(message);
	}

	// Send the user a list of everybody online.
	message.clear();
	int total_users = 0;
	for (auto i = rooms.begin(); i != rooms.end(); ++i)
	{
		QXmppMucRoom* r = *i;

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
				if (cu.toUpper() == _character.toUpper())
					continue;

				message.append(cu);
				message.append(", ");
			}
			message.remove(message.length() - 2, 2);
		}
	}
	message.insert(0, "Users online [" + QString::number(total_users) + "]");

	send_pm(jid, message);
}

void XmppClient::muc_userLeft(const QString& jid)
{
	QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
	if (!room)
		return;

	QString user = QXmppUtils::jidToResource(jid);

	// Ignore myself.
	if (user.toUpper() == _character.toUpper())
		return;

	QString prefix = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(room->jid()));
	QString message = "[" + prefix + "] " + user + " has signed out.";

	// Loop through each room informing of the user's joining.
	auto rooms = _muc_manager->rooms();
	for (auto i = rooms.begin(); i != rooms.end(); ++i)
	{
		// Ignore the room that just sent the message.
		if ((*i) == room)
			continue;

		QXmppMucRoom* room = *i;
		room->sendMessage(message);
	}
}

void XmppClient::muc_error(const QXmppStanza::Error& err)
{
	if (err.condition() == QXmppStanza::Error::Conflict)
	{
		// We are already signed into this channel.  Disconnect and try again.
		this->disconnectFromServer();
		_reconnect_timer.start(10000);
	}
}

void XmppClient::muc_error_timeout()
{
	_reconnect_timer.stop();
	this->connect(_character, _secret);
}
