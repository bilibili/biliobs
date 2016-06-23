#ifndef BILIRECORDBUTTONOPERATOR_H
#define BILIRECORDBUTTONOPERATOR_H

#include <QPushButton>
#include <QString>

class BiliRecordButtonOperator : public QObject
{
	Q_OBJECT

private:
	BiliRecordButtonOperator& operator = (const BiliRecordButtonOperator&) = delete;
	BiliRecordButtonOperator(const BiliRecordButtonOperator&) = delete;

public:
	enum { IDLE = 0, STARTING, RECORDING, STOPPING, FAILED };

private:
	QPushButton* recordButton;
	QString idleText;
	int status;

public:
	int GetStatus() const;

public:
	BiliRecordButtonOperator(QPushButton* pRecordButton);
	//~BiliRecordButtonOperator();

public slots :
	void SetIdle();
	void SetStarting();
	void SetRecording();
	void SetStopping();
	void SetFailed();

private slots :
	void OnRecordButtonClicked();

signals:
	void IdleClickedSignal();
	void StartingClickedSignal();
	void RecordingClickedSignal();
	void StoppingClickedSignal();
	void FailedClickedSignal();
};


#endif
