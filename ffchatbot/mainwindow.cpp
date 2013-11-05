#include <QtCore/QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QTimer::singleShot(0, this, SLOT(showConnectWindow()));
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
}
