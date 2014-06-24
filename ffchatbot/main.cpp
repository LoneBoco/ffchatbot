#include "mainwindow.h"
#include "xmppclient.h"
#include "prefixmanager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <string>
#include "QXmppMucManager.h"
#include <signal.h>


QJsonDocument settings;
XmppClient client;


int main(int argc, char *argv[]);
void terminate(int sig);


int main(int argc, char *argv[])
{
	// Create our object.
	QCoreApplication a(argc, argv);
	signal(SIGABRT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGINT, terminate);

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
			return 2;
	}
	else return 1;

	// Read some of our settings.
	QJsonObject obj = settings.object();
	QString character = obj["Character"].toString();
	QString guid = obj["GUID"].toString();

	// Load the MUC (multi-user channel) extension and get our channel list.
	client.load_muc_extension();
	auto chans = obj["Channels"];
	if (chans.isNull())
		return 3;

	// Load the channels into the channel manager.
	auto o = chans.toObject();
	PrefixManager::Instance().load_prefixes(o);

	// Set up the XMPP network.
	client.connect(character, guid);

	// Run.
	a.exec();

	return 0;
}

void terminate(int)
{
	client.disconnectFromServer();
	exit(0);
}
