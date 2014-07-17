#include "mainwindow.h"
#include "xmppclient.h"
#include "prefixmanager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCryptographicHash>
#include <QString>
#include <string>
#include "QXmppMucManager.h"
#include <signal.h>


QJsonDocument settings;
XmppClient client;

enum class ERETURN
{
	OK = 0,
	NOCONFIG = 1,
	INVALIDCONFIG = 2,
	NOEMAILORPASSWORD = 3,
	NOCHANNELS = 4,

	COUNT
};

QCoreApplication* app;


int main(int argc, char *argv[]);
void terminate(int sig);


int main(int argc, char *argv[])
{
	// Create our object.
    app = new QCoreApplication(argc, argv);
	signal(SIGABRT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGINT, terminate);
	signal(SIGBREAK, terminate);

	// Open the settings file.
	QFile file("relayconfig.txt");
	if (file.open(QIODevice::ReadOnly))
	{
		// Load the data from the settings file.
		QByteArray filedata = file.readAll();
		file.close();

		// Convert to a QJsonDocument object.
		settings = QJsonDocument::fromJson(filedata);
		if (settings.isNull())
            return (int)ERETURN::INVALIDCONFIG;
	}
    else return (int)ERETURN::NOCONFIG;

	// Read some of our settings.
	QJsonObject obj = settings.object();
	QString character = obj["Character"].toString();
	QString guid;

	// Attempt to get the guid, or build it.
	auto guid_entry = obj.find("GUID");
	if (guid_entry != obj.end())
        guid = (*guid_entry).toString();
	else
	{
		auto email_entry = obj.find("E-Mail");
		auto password_entry = obj.find("Password");

		if (email_entry == obj.end() || password_entry == obj.end())
            return (int)ERETURN::NOEMAILORPASSWORD;

		// <email>-<pw>-red5salt-7nc9bsj4j734ughb8r8dhb8938h8by987c4f7h47b
        QByteArray email = (*email_entry).toString().toUtf8();
        QByteArray password = (*password_entry).toString().toUtf8();
        QByteArray secret = email + "-" + password + "-red5salt-7nc9bsj4j734ughb8r8dhb8938h8by987c4f7h47b";
        QByteArray hash = QCryptographicHash::hash(secret,
			QCryptographicHash::Sha1);

        hash = hash.toHex().toLower();
		guid = hash;
	}

	// Load the MUC (multi-user channel) extension and get our channel list.
	client.load_muc_extension();
	auto chans = obj["Channels"];
	if (chans.isNull())
        return (int)ERETURN::NOCHANNELS;

	// Load the channels into the channel manager.
	auto o = chans.toObject();
	PrefixManager::Instance().load_prefixes(o);

	// Set up the XMPP network.
	client.connect(character, guid);

	// Run.
    app->exec();
    delete app;

    return (int)ERETURN::OK;
}

void terminate(int)
{
	client.disconnectFromServer();
    app->quit();
    delete app;

	exit(0);
}
