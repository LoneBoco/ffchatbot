#ifndef XMPPCLIENT_H
#define XMPPCLIENT_H

#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppRosterManager.h"
#include "QXmppMucManager.h"
#include <list>
#include <QTimer>

#define VERSION "[14.11.6.2]"

class XmppClient : public QXmppClient
{
	Q_OBJECT

public:
	XmppClient(QObject *parent = 0);
	~XmppClient();

	void load_muc_extension();

	void connect(const QString& character, const QString& secret);
	void add_channel(const QString& channel);
	void remove_channel(const QString& channel);

	void send_pm(const QString& jid, const QString& msg);
	void send_to_all(const QString& msg);
	void send_to_relay(const QString& msg);
	void send_to_room(const QString& msg, QXmppMucRoom* room);

	QString roll_dice(const QString& dice, const QString& from);

protected:
	QString _character;
	QString _secret;
	const QString _server;
	const QString _jidserver;
	const QString _confserver;
	QXmppMucManager* _muc_manager;
	QTimer _reconnect_timer;
	QTimer _save_timer;

public slots:
	void clientConnected();
	void presenceReceived(const QXmppPresence& presence);
	void messageReceived(const QXmppMessage& message);
	void muc_messageReceived(const QXmppMessage& message);
	void muc_userJoined(const QString& jid);
	void muc_userLeft(const QString& jid);
	void muc_error(const QXmppStanza::Error& err);
	void muc_error_timeout();
	void save_timeout();
};

#endif // XMPPCLIENT_H
