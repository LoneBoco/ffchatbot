#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QtNetwork/QNetworkAccessManager>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_actionE_xit_triggered();
	void showConnectWindow();

private:
	Ui::MainWindow *ui;
	std::shared_ptr<QNetworkAccessManager> network;

	QByteArray userID;
	QByteArray userSecret;

	static const QString salt1;
	static const QString salt2;

	QByteArray make_char_token(const QByteArray& host, const QByteArray& uri, const QByteArray& body = "");
	QByteArray bytes_to_hex(const QByteArray& bytes);
};

#endif // MAINWINDOW_H
