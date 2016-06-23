#ifndef BILIMSGDLG_H
#define BILIMSGDLG_H

#define USE_NEW_MSGDLG

#ifndef USE_NEW_MSGDLG
#include <QDialog>
#include "ui_BiLiMsgDlg.h"

class BiLiMsgDlg : public QDialog
{
	Q_OBJECT

public:
	BiLiMsgDlg(QWidget *parent = 0);
	~BiLiMsgDlg();

	void mSetMsgTxtAndBtn(QString msgTxt, bool hasCancelBtn = true) ;
	void mSetTitle(QString titleStr) ;

	void mSetOkButtonText(QString str);
	void mSetCancelButtonText(QString str);
protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);

private:
	Ui::BiLiMsgDlg ui;

	bool mIsPress;
	QPoint mPoint;

private slots:
	void mSltOkBtn();
};

#else

#include "bili-icon-msgdlg.h"

class BiLiMsgDlg : public BiLiIconMsgDlg
{
	Q_OBJECT

public:
	BiLiMsgDlg(QWidget *parent = 0);
	~BiLiMsgDlg();

	void mSetMsgTxtAndBtn(QString msgTxt, bool hasCancelBtn = true);
	void mSetTitle(QString titleStr);

	void mSetOkButtonText(QString str);
	void mSetCancelButtonText(QString str);
};

#endif

#endif // BILIMSGDLG_H
