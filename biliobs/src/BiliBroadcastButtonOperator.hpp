#ifndef BILIBROADCASTBUTTONOPERATOR_H
#define BILIBROADCASTBUTTONOPERATOR_H

#include <QPushButton>

class BiliBroadcastButtonOperator : public QObject
{
	Q_OBJECT

private:
	BiliBroadcastButtonOperator& operator = (const BiliBroadcastButtonOperator&) = delete;
	BiliBroadcastButtonOperator(const BiliBroadcastButtonOperator&) = delete;
public:
	enum { NO_PUSHSERVER = 0, WAITING_NETWORK_API, NORMAL_IDLE, BROADCASTING, CONNECTING, DISCONNECTING, RECONNECTING };

private:
	QPushButton* broadcastButton;
	int status;

public:
	int GetStatus() const;

public:
	BiliBroadcastButtonOperator(QPushButton* pBroadcastButton);
	//~BiliBroadcastButtonOperator();

public slots :
	void SetNotRetrived();
	void SetWorkingInProgress();
	void SetNormalIdle();
	void SetBroadcasting();
	void SetReconnecting();
	void SetConnecting();
	void SetDisconnecting();

private slots :
	void OnBroadcastButtonClicked();

signals:
	void StartBroadcastButtonClickedSignal();
	void StopBroadcastButtonClickedSignal();
	void WorkingInProgressButtonClicked();
	void RetryButtonClickedSignal();
	void ReconnectingClickedSignal();
	void ConnectingClickedSignal();
	void DisconnectingClickedSignal();
};


#endif
