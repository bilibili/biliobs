#ifndef BILIAUDIODEVSETTINGWID_H
#define BILIAUDIODEVSETTINGWID_H

#include <QWidget>
#include "ui_BiLiAudioDevSettingWid.h"

#include "obs.hpp"
#include "rounded_widget.h"

class CustomSlider;
class QLabel;
class QSlider;
class SpcVolSlider;
class EffectWindowInterface;

namespace std {
	template<class _Ty,
	class _Alloc = allocator<_Ty> >
	class vector;
}

class AudioDevControl : public QWidget{

	Q_OBJECT

public:
	AudioDevControl(OBSSource source);
	~AudioDevControl();

	inline obs_source_t *mGetSource() const { return mSource; }

	QString mGetName() const ;
	void mSetName(const QString &newName);

private:
	QLabel *mAudioDevNameLab;
  SpcVolSlider *mSpcSlider;
	QLabel *mVolumeLab;

	OBSSource mSource;
	obs_fader_t *mOBSFader;
	obs_volmeter_t *obs_volmeter_;
	obs_source_t *gainFilter_;
	obs_source_t *noiseGateFilter_;
	bool mIsMuted;

	void mSetupUI();

	static void OBSVolumeChanged(void *data, calldata_t *calldata);
	static void OBSVolmeterChanged(void *data, calldata_t *calldata);
	static void OBSVolumeMuted(void *data, calldata_t *calldata);

public slots:
	void mSltVolumeChanged();
	void mSltVolmeterChanged(int vol);
	void mSltVolumeMuted(bool muted) ;

	void mSltSetMuted(bool checked);
	void mSltSliderChanged(int vol);
};

class BiLiAudioDevSettingWid : public RoundedWidget {

	Q_OBJECT

public:
	BiLiAudioDevSettingWid();
	~BiLiAudioDevSettingWid();

	void mReplaceItems(const std::vector<AudioDevControl*>& items);

protected:
    void wheelEvent(QWheelEvent *e)
    {
        RoundedWidget::wheelEvent(e);
    }

private:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent*) override;

    void resizeEvent(QResizeEvent *) override;

    void moveEvent(QMoveEvent *) override;
private:
	Ui::BiLiAudioDevSettingWid ui;

	int count_;

    EffectWindowInterface *effect_widget_;
};

#endif // BILIAUDIODEVSETTINGWID_H
