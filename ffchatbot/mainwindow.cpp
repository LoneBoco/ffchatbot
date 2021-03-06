#include <QtNetwork/QNetworkAccessManager>
#include <QTimer>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>
#include <random>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectwindow.h"

const QString MainWindow::salt1 = QString("-red5salt-2239nknn234j290j09rjdj28fh8fnj234k");
const QString MainWindow::salt2 = QString("-red5salt-7nc9bsj4j734ughb8r8dhb8938h8by987c4f7h47b");

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QTimer::singleShot(0, this, SLOT(showConnectWindow()));
	network = std::make_shared<QNetworkAccessManager>(new QNetworkAccessManager());
	network->setCookieJar(new QNetworkCookieJar());

	this->userGUID = "0";
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_actionE_xit_triggered()
{
	this->close();
}

void MainWindow::showConnectWindow()
{
	ConnectWindow window;
	window.exec();

	QString user = window.getUsername();
	QString pass = window.getPassword();

	QByteArray toHash_a = (user + this->salt1).toUtf8();
	QByteArray toHash_b = (user + "-" + pass + this->salt2).toUtf8();

	// Hash A.
	QByteArray a_hash = QCryptographicHash::hash(toHash_a, QCryptographicHash::Sha1);

	// Hash B.
	QByteArray b_hash = QCryptographicHash::hash(toHash_b, QCryptographicHash::Sha1);

	// Convert hashes to login info.
	this->userID = a_hash.toBase64();
	this->userSecret = b_hash.toHex().toLower();

	// QUrl launcher_rqst("http://beta.firefallthegame.com/launcher/data");
	// QUrl character_rqst("https://beta.firefallthegame.com/game/character/list.json");
	QUrl character_rqst("https://clientapi-v01-uw2.firefallthegame.com/api/v2/characters/list");

	// Get the session cookie.
	/*
	QNetworkRequest cookie(launcher_rqst);
	auto reply_a = network->get(cookie);

	// Loop until we finish getting our data.
	QEventLoop loop;
	connect(reply_a, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	auto error_a = reply_a->errorString();
	*/

	// Get the character.
	QNetworkRequest character(character_rqst);
	character.setRawHeader("X-Red5-Signature", make_char_token("clientapi-v01-uw2.firefallthegame.com", "/api/v2/characters/list"));
	// character.setRawHeader("X-Red5-Signature", make_char_token("beta.firefallthegame.com", "/game/character/list.json"));
	auto reply = network->get(character);

	// Loop until we finish getting our data.
	QEventLoop loop2;
	connect(reply, SIGNAL(finished()), &loop2, SLOT(quit()));
	loop2.exec();

	auto error = reply->errorString();

	// Parse the information out of the reply.
	// {"characters":[{"id":123456,"character_guid":1231231231231231231,"account_id":12345,"name":"Nalin","unique_name":"NALIN","race":"human","gender":"male","loadout_id":null,"head_mainId":10004,"head_accAId":10089,"head_accBId":0,"voice_setId":1000,"is_active":true,"is_dev":false,"xdata":null,"deleted_at":null,"created_at":"2012-08-08T07:48:20+00:00","updated_at":"2014-02-01T04:22:31+00:00","time_played_secs":1374077,"title_id":160,"current_gender":"male"}],"is_dev":false}
	QByteArray replyData = reply->readAll();
	auto jdoc = QJsonDocument::fromJson(replyData);
	auto obj = jdoc.object();
	QString guid = obj["character"].toObject().value("guid").toString();
	QString name = obj["character"].toObject().value("name").toString();
}

QByteArray MainWindow::make_char_token(const QByteArray& host, const QByteArray& uri, const QByteArray& body)
{
	// Step 1: Hash the body.
	QByteArray A = QCryptographicHash::hash(body, QCryptographicHash::Sha1);

	// Step 2: Assemble B and xor each byte with 0x36.
	QByteArray B(this->userSecret);
	B.append(QByteArray(0x40 - B.length(), 0));
	for (int i = 0; i < 0x40; ++i)
		B.data()[i] ^= 0x36;

	// Step 3: Generate request string.
	std::default_random_engine gen;
	std::uniform_int_distribution<char> r;
	QByteArray nonce;
	for (int i = 0; i < 8; ++i)
		nonce.append(QString().sprintf("%02x", r(gen)));
	int time = QDateTime::currentDateTimeUtc().toTime_t();
	QByteArray req = "ver=2"
			+ "&tc=" + QString::number(time).toUtf8()
			+ "&nonce=" + nonce
			+ "&uid=" + QUrl::toPercentEncoding(this->userID)
			+ "&host=" + host
			+ "&path=" + QUrl::toPercentEncoding(uri)
			+ "&hbody=" + A.toHex().toLower()
			+ "&cid=" + this->userGUID;

	// Step 4: Generate hash of B + step 3.
	QByteArray C = QCryptographicHash::hash(B + req, QCryptographicHash::Sha1);

	// Step 5: Perform transform on secret (xor each byte with 0x5C, pad to 0x40).
	QByteArray D(this->userSecret);
	D.append(QByteArray(0x40 - D.length(), 0));
	for (int i = 0; i < 0x40; ++i)
		D.data()[i] ^= 0x5C;

	// Step 6: Hash transformed secret + step 4.
	QByteArray E = QCryptographicHash::hash(D + C, QCryptographicHash::Sha1);

	// Step 7: Form our final string.
	QByteArray ret = "Red5 " + E.toHex().toLower() + " " + req;

	return ret;
}

QByteArray MainWindow::bytes_to_hex(const QByteArray& bytes)
{
	int size = bytes.length();
	char* c = new char[size * 2];
	memset(c, 0, size * 2);

	int b;
	for (int i = 0; i < size; i++)
	{
		b = bytes[i] >> 4;
		c[i * 2] = (char)(55 + b + (((b - 10) >> 31) & -7));
		b = bytes[i] & 0xF;
		c[i * 2 + 1] = (char)(55 + b + (((b - 10) >> 31) & -7));
	}
	return QByteArray(c, size * 2);
}
