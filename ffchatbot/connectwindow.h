#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include <QDialog>

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QDialog
{
	Q_OBJECT

public:
	explicit ConnectWindow(QWidget *parent = 0);
	~ConnectWindow();

	QString getUsername() const;
	QString getPassword() const;

private:
	Ui::ConnectWindow *ui;
};

#endif // CONNECTWINDOW_H
