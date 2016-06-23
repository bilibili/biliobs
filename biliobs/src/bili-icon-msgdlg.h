#ifndef BILI_ICON_MSGDLG_H
#define BILI_ICON_MSGDLG_H

#include <QWidget>
#include "ui_bili-icon-msgdlg.h"
#include "bili_move_frameless_window.hpp"

class BiLiIconMsgDlg : public QDialog
{
	Q_OBJECT

public:
	BiLiIconMsgDlg(QWidget *parent = 0);
	~BiLiIconMsgDlg();

	void SetOkButtonText(const QString& text);
	void SetCancelButtonText(const QString& text);
	void SetLargeIcon(const QPixmap& largeIcon);
	void SetLargeIcon(const QString& url);
	void setWindowTitle(const QString& title);
	void SetSubTitle(const QString& title);
	void SetText(const QString& text);

public slots:
	void onOkButtonClicked();
	void onCancelButtonClicked();

protected:
	void paintEvent(QPaintEvent* pe) override;

	void mousePressEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

	MovableFramelessWindowUtil<QDialog> moveHelper;
private:
	Ui::BiLiIconMsgDlg ui;
};

#endif // BILI_ICON_MSGDLG_H
