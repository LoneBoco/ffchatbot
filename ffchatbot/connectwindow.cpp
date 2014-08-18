#include "connectwindow.h"
#include "ui_connectwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ConnectWindow)
{
	ui->setupUi(this);
}

ConnectWindow::~ConnectWindow()
{
	delete ui;
}

QString ConnectWindow::getUsername() const
{
	return ui->editUsername->text();
}

QString ConnectWindow::getPassword() const
{
	return ui->editPassword->text();
}
