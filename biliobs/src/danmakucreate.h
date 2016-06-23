#ifndef DANMAKUCREATE_H
#define DANMAKUCREATE_H

#include <QObject>
#include <QHash>
#include <QMutex>
#include "tasknamespace.h" 

class DanmakuCreate : public QObject {

	Q_OBJECT

public:
	~DanmakuCreate();

	static DanmakuCreate *getInst_();
	QHash<int, TKNS::DMTask *> dmTaskHs_;
	uint totalDMCount_;
	uint totalDMHeight_;
	int dmAverageHeight_;
	QMutex dmHashMutex_;

	int danmakuFontSize_;
	int dmWidth_;

	float dmOpacity_;
	float dmOpacityIdle_;

signals:
	void sglPreparedDM(int index);
public slots:
	void sltRecvDM(QString dm);

private:
	DanmakuCreate(QObject *parent = 0);
	static DanmakuCreate *inst_;

	void setDMPix_(TKNS::DMTask *dmTk);

	QString danmakuStyle_;
	QString giftStyle_;
	QString laoyeStyle_;
	QString announceStyle_;
};

#endif // DANMAKUCREATE_H
