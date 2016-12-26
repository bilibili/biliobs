#include "BiliBroadcastButtonOperator.hpp"

BiliBroadcastButtonOperator::BiliBroadcastButtonOperator(QPushButton* pBroadcastButton)
	: broadcastButton(pBroadcastButton)
{
	QObject::connect(broadcastButton, SIGNAL(clicked()), this, SLOT(OnBroadcastButtonClicked()));
}

void BiliBroadcastButtonOperator::SetNotRetrived()
{
	broadcastButton->setText("\xe6\x9f\xa5\xe8\xaf\xa2\xe6\x88\xbf\xe9\x97\xb4\xe5\xa4\xb1\xe8\xb4\xa5\n\xe7\x82\xb9\xe5\x87\xbb\xe9\x87\x8d\xe8\xaf\x95"); //查询房间失败\n点击重试
	status = NO_PUSHSERVER;
}

void BiliBroadcastButtonOperator::SetWorkingInProgress()
{
	broadcastButton->setText("\xe8\xaf\xb7\xe7\xa8\x8d\xe5\x80\x99"); //请稍候
	status = WAITING_NETWORK_API;
}

void BiliBroadcastButtonOperator::SetNormalIdle()
{
	broadcastButton->setText("\xe5\xbc\x80\xe5\xa7\x8b\xe7\x9b\xb4\xe6\x92\xad"); //开始直播
	status = NORMAL_IDLE;
}

void BiliBroadcastButtonOperator::SetReconnecting()
{
	broadcastButton->setText("\xe9\x87\x8d\xe8\xbf\x9e\xe4\xb8\xad"); //重连中
	status = RECONNECTING;
}

void BiliBroadcastButtonOperator::SetBroadcasting()
{
	broadcastButton->setText("\xe5\x81\x9c\xe6\xad\xa2\xe7\x9b\xb4\xe6\x92\xad"); //停止直播
	status = BROADCASTING;
}

void BiliBroadcastButtonOperator::SetConnecting()
{
	if (status != BROADCASTING)
	{
		broadcastButton->setText("\xe6\xad\xa3\xe5\x9c\xa8\xe8\xbf\x9e\xe6\x8e\xa5"); //正在连接
		status = CONNECTING;
	}
}

void BiliBroadcastButtonOperator::SetDisconnecting()
{
	if (status != NORMAL_IDLE && status != WAITING_NETWORK_API)
	{
		broadcastButton->setText("\xe6\xad\xa3\xe5\x9c\xa8\xe6\x96\xad\xe5\xbc\x80"); //正在断开
		status = DISCONNECTING;
	}
}

int  BiliBroadcastButtonOperator::GetStatus() const
{
	return status;
}

void BiliBroadcastButtonOperator::OnBroadcastButtonClicked()
{
	if (status == NO_PUSHSERVER)
		emit RetryButtonClickedSignal();
	else if (status == WAITING_NETWORK_API)
		emit WorkingInProgressButtonClicked();
	else if (status == NORMAL_IDLE)
		emit StartBroadcastButtonClickedSignal();
	else if (status == BROADCASTING)
		emit StopBroadcastButtonClickedSignal();
	else if (status == RECONNECTING)
		emit ReconnectingClickedSignal();
	else if (status == CONNECTING)
		emit ConnectingClickedSignal();
	else if (status == DISCONNECTING)
		emit DisconnectingClickedSignal();
}

