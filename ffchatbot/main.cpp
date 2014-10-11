#include "mainwindow.h"
#include "xmppclient.h"
#include "charactermanager.h"
#include "connectionmanager.h"
#include "zonemanager.h"
#include "prefixmanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>

#include <string>
#include <signal.h>


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

#ifdef Q_OS_WIN32
	signal(SIGBREAK, terminate);
#endif

	// Open the settings file.
	int count = ConnectionManager::Instance().LoadCharacters("relayconfig.txt");
	if (count == 0)
		return (int)ERETURN::INVALIDCONFIG;

	// Load our zones.
	ZoneManager::Instance().LoadZones("zonelist.txt");

	// Load our saved user data.
	CharacterManager::Instance().LoadCharacters("userdata.txt");

	// Load MOTD.
	ConnectionManager::Instance().LoadMOTD("motd.txt");

	// Run.
    app->exec();
	delete app;

	// Cleanup.
	ConnectionManager::Instance().Cleanup();
	ConnectionManager::Instance().SaveMOTD("motd.txt");
	CharacterManager::Instance().SaveCharacters("userdata.txt");

    return (int)ERETURN::OK;
}

void terminate(int)
{
	app->quit();
    delete app;

	// Cleanup.
	ConnectionManager::Instance().Cleanup();
	ConnectionManager::Instance().SaveMOTD("motd.txt");
	CharacterManager::Instance().SaveCharacters("userdata.txt");

	exit(0);
}
