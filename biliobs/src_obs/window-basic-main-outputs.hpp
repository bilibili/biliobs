#pragma once

class BiLiOBSMainWid;

struct BasicOutputHandler {
	OBSOutput              fileOutput;
	OBSOutput              streamOutput;
	bool                   streamingActive = false;
	bool                   recordingActive = false;
	bool                   delayActive = false;
	BiLiOBSMainWid         *biliMain;

	OBSSignal              startRecording;
	OBSSignal              stopRecording;
	OBSSignal              startingRecording;
	OBSSignal              stoppingRecording;
	OBSSignal              startStreaming;
	OBSSignal              stopStreaming;
	OBSSignal              streamDelayStarting;
	OBSSignal              streamDelayStopping;

	inline BasicOutputHandler(BiLiOBSMainWid *main_) : biliMain(main_) {}

	virtual ~BasicOutputHandler() {};

	virtual bool StartStreaming(obs_service_t *service) = 0;
	virtual bool StartRecording() = 0;
	virtual void StopStreaming() = 0;
	virtual void ForceStopStreaming() = 0;
	virtual void StopRecording() = 0;
	virtual bool StreamingActive() const = 0;
	virtual bool RecordingActive() const = 0;

	virtual void Update() = 0;

	inline bool Active() const
	{
		return streamingActive || recordingActive || delayActive;
	}
};

BasicOutputHandler *CreateSimpleOutputHandler(BiLiOBSMainWid *main);
BasicOutputHandler *CreateAdvancedOutputHandler(BiLiOBSMainWid *main);
