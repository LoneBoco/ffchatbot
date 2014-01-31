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

	// Load settings.
	QFile file("relayconfig.txt");
	if (!file.open(QIODevice::ReadOnly))
		return 1;
	QByteArray filedata = file.readAll();
	file.close();
	settings = QJsonDocument::fromJson(filedata);
	if (settings.isNull())
		return 1;

	QJsonObject obj = settings.object();
	QString character = obj["Character"].toString();
	QString guid = obj["GUID"].toString();

	client.load_muc_extension();
	auto chans = obj["Channels"];
	if (chans.isNull())
		return 1;

	// Load the channels into the channel manager.
	auto o = chans.toObject();
	PrefixManager::Instance().load_prefixes(o);

	// Set up the XMPP network.
	client.connect(character, guid);

	// Run.
	a.exec();
}

void terminate(int sig)
{
	client.disconnectFromServer();
	exit(0);
}
