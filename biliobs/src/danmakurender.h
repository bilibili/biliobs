#ifndef DANMAKURENDER_H
#define DANMAKURENDER_H

#include <QObject>
#include "tasknamespace.h"

class QTimer;
class QPixmap;
class DanmakuCreate;
class DanmakuMove;
class DanmakuRender : public QObject {
	Q_OBJECT

public:
	DanmakuRender(QObject *parent = 0);
	~DanmakuRender();

	static DanmakuRender *getInst_();
	DanmakuCreate *dmCreater_;
	DanmakuMove *dmMover_;

	int staySecond_;
	void changedStayTime_(int second);
	void setOpacityDecreaseSpeed_();

signals:
	void sglTopDM();
public slots:
	void sltRecvToRenderDM(int dmIndex);

private:
	static DanmakuRender *inst_;
	QTimer *dmOpacityTimer_;
	QTimer *dmSideTopTimer_;
	QList<int> toRenderDMIndexL;
	void renderDM_();

	float opacityDecreaseSpeed_;
};

#endif // DANMAKURENDER_H
