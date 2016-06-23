#include "BiliRecordButtonOperator.hpp"

BiliRecordButtonOperator::BiliRecordButtonOperator(QPushButton* pRecordButton)
	: recordButton(pRecordButton)
	, status(IDLE)
{
	QObject::connect(recordButton, SIGNAL(clicked()), this, SLOT(OnRecordButtonClicked()));

	idleText = recordButton->text();
}

void BiliRecordButtonOperator::SetIdle()
{
	status = IDLE;
	recordButton->setText(idleText);
}

void BiliRecordButtonOperator::SetStarting()
{
	if (status == IDLE)
	{
		status = STARTING;
		recordButton->setText(tr("Starting"));
	}
}

void BiliRecordButtonOperator::SetRecording()
{
	if (status == STARTING || status == IDLE)
	{
		status = RECORDING;
		recordButton->setText(tr("Recording"));
	}
}

void BiliRecordButtonOperator::SetStopping()
{
	if (status == RECORDING)
	{
		status = STOPPING;
		recordButton->setText("Stopping");
	}
}

void BiliRecordButtonOperator::SetFailed()
{
	status = FAILED;
	recordButton->setText(tr("Failed"));
}

int  BiliRecordButtonOperator::GetStatus() const
{
	return status;
}

void BiliRecordButtonOperator::OnRecordButtonClicked()
{
	switch (status)
	{
	case IDLE:
		emit IdleClickedSignal();
		break;
	case STARTING:
		emit StartingClickedSignal();
		break;
	case RECORDING:
		emit RecordingClickedSignal();
		break;
	case STOPPING:
		emit StoppingClickedSignal();
		break;
	case FAILED:
		emit FailedClickedSignal();
		break;
	default:
		break;
	}
}

