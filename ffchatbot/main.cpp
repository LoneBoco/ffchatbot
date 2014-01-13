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

QJsonDocument settings;
bool relayMode = false;

int main(int argc, char *argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		std::string t = argv[i];
		if (t == "-relay")
			relayMode = true;
	}

	// Create our object.
	QObject* a = nullptr;
	if (!relayMode)
		a = new QApplication(argc, argv);
	else a = new QCoreApplication(argc, argv);

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

	XmppClient client;
	client.load_muc_extension();
	if (relayMode)
	{
		auto chans = obj["Channels"];
		if (chans.isNull())
			return 1;

		// Register each channel with the XmppClient.
		auto o = chans.toObject();
		/*
		for (auto i = o.begin(); i != o.end(); ++i)
		{
			client.add_channel(i.value().toString());
		}
		*/

		// Load the channels into the channel manager.
		PrefixManager::Instance().load_prefixes(o);
	}

	// Set up the XMPP network.
	client.connect(character, guid);

	// Set up the window.
	MainWindow* w = nullptr;
	if (!relayMode)
	{
		w = new MainWindow();
		w->show();
	}

	// Run.
	if (!relayMode)
		return reinterpret_cast<QApplication*>(a)->exec();
	else return reinterpret_cast<QCoreApplication*>(a)->exec();
}
