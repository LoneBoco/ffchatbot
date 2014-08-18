#include "xmppclient.h"
#include "charactermanager.h"
#include "connectionmanager.h"
#include "prefixmanager.h"

#include "QXmppMucManager.h"
#include "QXmppUtils.h"

#include <QJsonDocument>
#include <QDateTime>
#include <random>


XmppClient::XmppClient(QObject *parent)
	: QXmppClient(parent),
	  _server("chat.firefallthegame.com"),
	  _jidserver("firefall.chat.firefallthegame.com"),
	  _confserver("conference.firefall.chat.firefallthegame.com")
{
	bool check;
	Q_UNUSED(check);

	check = QObject::connect(this, SIGNAL(connected()), SLOT(clientConnected()));
	Q_ASSERT(check);

	check = QObject::connect(this, SIGNAL(messageReceived(QXmppMessage)), SLOT(messageReceived(QXmppMessage)));
	Q_ASSERT(check);

	check = QObject::connect(this, SIGNAL(presenceReceived(QXmppPresence)), SLOT(presenceReceived(QXmppPresence)));
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

void XmppClient::connect(const QString& character, const QString& secret)
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

void XmppClient::add_channel(const QString& channel)
{
	QXmppMucRoom* room = nullptr;

	// See if this room already exists.
	for (auto r: _muc_manager->rooms())
	{
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

void XmppClient::remove_channel(const QString& channel)
{
	QXmppMucRoom* room = nullptr;

	// Find the room.
	for (auto r: _muc_manager->rooms())
	{
		if (QXmppUtils::jidToUser(r->jid()).toUpper() == channel.toUpper())
		{
			room = r;
			break;
		}
	}

	// If the room doesn't exist, just return.
	if (room == nullptr)
		return;

	// Leave.
	room->leave();
}

void XmppClient::send_pm(const QString& jid, const QString& msg)
{
	QString msg2 = msg;
	while (msg2.count() != 0)
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
		body.setValue(msg2.left(256));
		msg2 = msg2.remove(0, 256);

		// Link them together.
		message.appendChild(body);

		// Send the element.
		sendElement(message);
	}
}

void XmppClient::send_to_all(const QString& msg)
{
	QStringList sl;
	QString msg2 = msg;
	while (msg2.count() != 0)
	{
		sl.append(msg2.left(256));
		msg2 = msg2.remove(0, 256);
	}

	// Loop through each room relaying the message.
	for (auto r: _muc_manager->rooms())
	{
		for (QString s: sl)
			r->sendMessage(s);
	}

	ConnectionManager::Instance().SendMessage(_character, msg);
}

void XmppClient::send_to_room(const QString& msg, QXmppMucRoom* room)
{
	if (room == nullptr)
		return;

	// Break into 256 character chunks.
	QStringList sl;
	QString msg2 = msg;
	while (msg2.count() != 0)
	{
		sl.append(msg2.left(256));
		msg2 = msg2.remove(0, 256);
	}

	// Relay each message.
	for (QString s: sl)
		room->sendMessage(s);
}

QString XmppClient::roll_dice(const QString& dice, const QString& from)
{
	QStringList list = dice.split('d', QString::SkipEmptyParts);
	if (list.length() == 2)
	{
		bool n_ok, s_ok;
		int n = list[0].toInt(&n_ok);
		int s = list[1].toInt(&s_ok);

		if (n < 0 || n > 100)
			n_ok = false;
		if (s < 0 || s > 255)
			s_ok = false;

		if (n_ok && s_ok)
		{
			std::mt19937 gen;
			gen.seed(QDateTime::currentMSecsSinceEpoch());
			std::uniform_int_distribution<char> r(1, s);

			QString rolls;

			int result = 0;
			for (int i = 0; i < n; ++i)
			{
				int d = r(gen);
				result += d;
				if (i != 0) rolls.append(", ");
				rolls.append(QString::number(d));
			}

			return from + " rolled: " + QString::number(result) + " (" + rolls + ")";
		}
	}

	return QString("dice: Invalid dice parameters.");
}

//
// SLOTS
//

void XmppClient::clientConnected()
{
	QString c = ConnectionManager::Instance().GetCharacterChannel(_character);
	add_channel(c);
}

void XmppClient::presenceReceived(const QXmppPresence& presence)
{
	QString from = QXmppUtils::jidToResource(presence.from());
}

void XmppClient::messageReceived(const QXmppMessage& message)
{
	// This function is only for normal messages.
	if (message.type() != QXmppMessage::Normal)
		return;

	// See who it is from.  If it is from ourself, don't respond to it.
	QString from = QXmppUtils::jidToResource(message.from());
	if (from.isEmpty() || from.toUpper() == _character.toUpper())
		return;

	QString m = message.body();

	// Check for AFK replies.
	// This avoids an infinite loop where it keeps responding to the AFK notification.
	if (m == "<AFK> I am away from my keyboard")
		return;

	// Check for a help command.
	if (m.startsWith("help", Qt::CaseInsensitive))
	{
		if (m == "help")
		{
			QString msg = "Commands (use help <command> for detailed help): version, listonline/listusers, listrooms, listinactive, join, leave, roll";
			send_pm(message.from(), msg);
		}
		else if (m == "help version")
			send_pm(message.from(), "Gets the FFChatBot version information.");
		else if (m == "help listonline" || m == "help listusers")
			send_pm(message.from(), "Gets a list of all online users.");
		else if (m == "help listrooms")
			send_pm(message.from(), "Gets a list of all joined rooms.");
		else if (m == "help join")
			send_pm(message.from(), "join <room> <alias>: Joins the given room.");
		else if (m == "help leave")
			send_pm(message.from(), "leave <alias>: Leaves the given room.");
		else if (m == "help roll")
			send_pm(message.from(), "roll xdy: Rolls x number of y-sided dice. (ex, 1d6).  Can be used publically.");
		else if (m == "help listinactive")
			send_pm(message.from(), "listinactive <days>: Lists players who haven't logged in for the given number of days.");
		return;
	}

	// Version listing.
	if (m == "version")
	{
		send_pm(message.from(), QString("FFChatBot version: ") + VERSION);
		return;
	}

	// Returns the player listing.
	if (m == "listonline" || m == "listusers")
	{
		send_pm(message.from(), _getLoginMessage());
		return;
	}

	// Returns a list of all watched rooms.
	if (m == "listrooms")
	{
		send_pm(message.from(), "Watched rooms: " + PrefixManager::Instance().get_all_prefixes().join(", "));
		return;
	}

	// Join a room.
	if (m.startsWith("join ", Qt::CaseInsensitive))
	{
		QStringList list = m.split(' ', QString::SkipEmptyParts);
		if (list.length() != 3)
			return;

		PrefixManager::Instance().add_prefix(list[2], list[1]);
		add_channel(list[1]);
		return;
	}

	// Leave a room.
	if (m.startsWith("leave ", Qt::CaseInsensitive))
	{
		QStringList list = m.split(' ', QString::SkipEmptyParts);
		if (list.length() != 2)
			return;

		QString c = PrefixManager::Instance().remove_prefix(list[1]);
		remove_channel(c);
		return;
	}

	// Roll dice.
	if (m.startsWith("roll ", Qt::CaseInsensitive))
	{
		send_pm(message.from(), roll_dice(m.mid(5).trimmed(), from));
		return;
	}

	// List inactive players.
	if (m.startsWith("listinactive ", Qt::CaseInsensitive))
	{
		int days = m.mid(13).trimmed().toInt();
		if (days < 1) days = 1;

		QString list("Users inactive for ");
		list.append(QString::number(days));
		if (days == 1) list.append(" day: ");
		else list.append(" days: ");

		QStringList inactives = CharacterManager::Instance().GetInactives(days);
		for (QString c: inactives)
			list.append(c + ", ");
		list = list.remove(list.length() - 2, 2);

		send_pm(message.from(), list);
		return;
	}

	// Invalid command.
	send_pm(message.from(), "Invalid command.  Use help for a command list.");
}

void XmppClient::muc_messageReceived(const QXmppMessage& message)
{
	QXmppMucRoom *room = qobject_cast<QXmppMucRoom*>(sender());
	if (!room)
		return;

	// This function is only for group chats.
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
	for (auto r: _muc_manager->rooms())
	{
		// Ignore the room that the message is from.
		if (r == room)
			continue;

		send_to_room(msg, r);
	}
	ConnectionManager::Instance().SendMessage(_character, msg);

	QString m = message.body();

	// See if it is a public command.
	if (m.startsWith("!roll "))
	{
		send_to_all(roll_dice(m.mid(6).trimmed(), from));
	}

	if (m == "!listusers" || m == "!listonline")
	{
		send_to_all(_getLoginMessage());
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

	// Record the last seen time of this user.
	CharacterManager::Instance().SetLastSeen(user, QDateTime::currentDateTime().toTime_t());

	QString prefix = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(room->jid()));
	QString message = "[" + prefix + "] " + user + " has logged in.";

	// Loop through each room informing of the user's status.
	for (auto r: _muc_manager->rooms())
	{
		// Ignore the room that just sent the message.
		if (r == room)
			continue;

		send_to_room(message, r);
	}
	ConnectionManager::Instance().SendMessage(_character, message);

	// Send the user a list of everybody online.
	send_pm(jid, _getLoginMessage());
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

	// Record the last seen time of this user.
	CharacterManager::Instance().SetLastSeen(user, QDateTime::currentDateTime().toTime_t());

	QString prefix = PrefixManager::Instance().get_prefix(QXmppUtils::jidToUser(room->jid()));
	QString message = "[" + prefix + "] " + user + " has signed out.";

	// Loop through each room informing of the user's status.
	for (auto r: _muc_manager->rooms())
	{
		// Ignore the room that just sent the message.
		if (r == room)
			continue;

		send_to_room(message, r);
	}
	ConnectionManager::Instance().SendMessage(_character, message);
}

void XmppClient::muc_error(const QXmppStanza::Error& err)
{
	if (err.condition() == QXmppStanza::Error::Conflict)
	{
		// We are already signed into this channel.  Disconnect and try again in 10 seconds.
		this->disconnectFromServer();
		_reconnect_timer.start(10000);
	}
}

void XmppClient::muc_error_timeout()
{
	_reconnect_timer.stop();
	this->connect(_character, _secret);
}

QString XmppClient::_getLoginMessage() const
{
	QString message;
	int total_users = 0;
	for (auto r: _muc_manager->rooms())
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
				if (cu.toUpper() == _character.toUpper())
					continue;

				message.append(cu);
				message.append(", ");
			}
			message.remove(message.length() - 2, 2);
		}
	}
	message.insert(0, "Users online [" + QString::number(total_users) + "]");
	return message;
}
