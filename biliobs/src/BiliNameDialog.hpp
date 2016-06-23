#ifndef WINDOW_BILIBILI_NAME_HPP
#define WINDOW_BILIBILI_NAME_HPP

#include <QDialog>
#include <QString>
#include "bili_move_frameless_window.hpp"


namespace Ui {
	class BiliNameDialog;
};

class BiliNameDialog : public QDialog
{
	Q_OBJECT
public:
	explicit BiliNameDialog(QString& name, QWidget* parent = 0);
	~BiliNameDialog();

private slots:
	void on_xButton_clicked();
	void on_cancelButton_clicked();
	void on_okButton_clicked();

private:
	MovableFramelessWindowUtil<BiliNameDialog> moveWindowUtil;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

private:
	QString& nameString;
	Ui::BiliNameDialog* biliUi;
};

#endif // WINDOW_BILIBILI_NAME_HPP
