#ifndef DANMAKUMOVE_H
#define DANMAKUMOVE_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include "tasknamespace.h"

class DanmakuCreate;
class QTimer;
class DanmakuMove : public QObject {
	Q_OBJECT

public:
	~DanmakuMove();
	static DanmakuMove *getInst_();

	QPoint dmUpPos_;
	QSize dmSideWidSize_;
	uint startPaintDMIndex_;
	QPixmap *dmSideWidBKPixCache_;
	DanmakuCreate *dmCreater_;

	QPixmap *standByPix_;
	void drawStandBy_();

	bool isNeedUpdateStandBy_;
private:
	DanmakuMove(QObject *parent = 0);
	static DanmakuMove *inst_;

	QTimer *dmMoveUpTimer_;
	void moveDM_();

signals:
	void sglUpdateDMPix();
	void sglSendRenderDM(int dmIndex);

public slots:
	void sltRecvPreparedDM(int index);
	
};

#endif // DANMAKUMOVE_H
