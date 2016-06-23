#ifndef SHADOWDLG_H
#define SHADOWDLG_H

#include <QDialog>

class ShadowDlg : public QDialog {

	Q_OBJECT
public:
	ShadowDlg(QWidget *parent = 0);
	~ShadowDlg();

	void closeEvent(QCloseEvent *e);
	void mSetSolidWid(QWidget *sw, QSize swSize);

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);

private:
	bool mIsPressed;
	QPoint mPoint;
	QWidget *mSolidWid;
};

#endif // SHADOWDLG_H