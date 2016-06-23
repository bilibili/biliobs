#ifndef DANMAKUWID_H
#define DANMAKUWID_H

#include <QWidget>
#include <QHash>

#include "ui_danmakuwid.h"

#define HAS_LOCKDM		1

#if HAS_LOCKDM
class SuspendLockWid;
#endif

class QSystemTrayIcon;
class DanmakuCreate;
class DanmakuMove;
class DanmakuRender;
class QCheckBox;
class QPushButton;
class QLabel;
class DanmakuWid : public QWidget {

	Q_OBJECT

public:
	DanmakuWid(QWidget *parent = 0);
	~DanmakuWid();

	static QHash<QString, QObject *> objects_;
	void setDMParams_();
	void initDMThreads_();
	void startDMThreads_();
	void releaseObjects_();

	void changeOpacity_(int opacity);
	void changeStayTime_(int second);
	void changeFontSize_(QString fontSize);

	void initBKPix_();
	void setupStateWidLaout_();
	void setNetState_(int state);
	void setNetUpSpeed_(int speed);
	void setFrameLostRate_(float rate);
	void setNumOfAudience_(int num);
	void setNumOfFans_(int num);

	QWidget *dmWid_;
	QPixmap *bkPix_;
	QPixmap *bkPixCache_;

signals:
	void sglSendDM(QString);
	void sglSettingsDM();
	void sglUpdateStandByPix();
public slots:
	void sltUpdateDMPix() ;

protected:
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *e);
	virtual bool eventFilter(QObject *o, QEvent *e);

    void paintEvent(QPaintEvent *e) override;

private:
	Ui::DanmakuWid ui;

	DanmakuCreate *dmCreater_;
	DanmakuMove *dmMover_;
	DanmakuRender *dmRender_;

	QLabel *colorLab_;
	QLabel *uploadSpeedLab_;
	QLabel *uploadSpeedVolLab_;
	QLabel *frameLostRateLab_;
	QLabel *frameLostRateVolLab_;
	QLabel *audienceLab_;
	QLabel *audienceVolLab_;
	QLabel *fansLab_;
	QLabel *fansVolLab_;
	QLabel *switchLab_;
	QCheckBox *switchCB_;
	QPushButton *settingBtn_;
	QPushButton *topBtn_;
	QPushButton *lockBtn_;
	QPushButton *minBtn_;
	QLabel *dragScaleLab_;

	bool isTop_;
	bool isPress_;
	QPoint pressPoint_;
	bool lockTrans_;
	bool canDragScale_;
	QTimer *checkTransMouseTimer_;
	bool transMouse_();
	void setDragCursorShape_(QEvent *e);

	QSystemTrayIcon *trayicon_;

#if HAS_LOCKDM
	SuspendLockWid *suspendLockWid_;
	void setLockTransState_();
	QRect getLockBtnGeometry_();
#endif
};

#endif // DANMAKUWID_H
