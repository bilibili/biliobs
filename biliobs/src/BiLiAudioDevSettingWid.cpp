#include "BiLiAudioDevSettingWid.h"
#include <QLabel>
#include "circle_slider_slider.h"
#include "spc_vol_slider.h"
#include "qmessagebox.h"
#include <qdebug.h>

#include "bili_effect_widget.h"

#define VOLSLIDER_WIDTH	8
#define VOLSLIDER_MIN	0
#define VOLSLIDER_MAX	100

static const char *gainFilterId = "gain_filter";
static const char *noiseGateFilterId = "noise_gate_filter";
AudioDevControl::AudioDevControl(OBSSource source)
	:mSource(source)
	, gainFilter_(nullptr)
	, noiseGateFilter_(nullptr)
	, mOBSFader(obs_fader_create(OBS_FADER_CUBIC))
	, obs_volmeter_(obs_volmeter_create(OBS_FADER_CUBIC)) {

	mSetupUI();
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	mIsMuted = obs_source_muted(mSource);
	signal_handler_connect(obs_fader_get_signal_handler(mOBSFader),
		"volume_changed", OBSVolumeChanged, this);
	signal_handler_connect(obs_source_get_signal_handler(mSource),
		"mute", OBSVolumeMuted, this);

    connect(mSpcSlider, SIGNAL(valueChanged(int)), this, SLOT(mSltSliderChanged(int)));

	obs_fader_attach_source(mOBSFader, mSource);
	mSltVolumeChanged();
}

AudioDevControl::~AudioDevControl(){

	signal_handler_disconnect(obs_fader_get_signal_handler(mOBSFader),
		"volume_changed", OBSVolumeChanged, this);

	signal_handler_disconnect(obs_source_get_signal_handler(mSource),
		"mute", OBSVolumeMuted, this);

	obs_fader_destroy(mOBSFader);
	obs_volmeter_destroy(obs_volmeter_);
}

void AudioDevControl::mSetupUI(){

	mAudioDevNameLab = new QLabel(this);
	mAudioDevNameLab->setObjectName("AudioDevNameLab");
	mAudioDevNameLab->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignCenter);
	if (!strcmp(obs_source_get_name(mSource), "DesktopDevice1"))
		mAudioDevNameLab->setText(tr("DsktopDev1"));
	else if (!strcmp(obs_source_get_name(mSource), "AuxDevice1"))
		mAudioDevNameLab->setText(tr("AuxDev1"));
	else
		mAudioDevNameLab->setText(QString(tr("%1")).arg(obs_source_get_name(mSource)));

	mAudioDevNameLab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	//mAudioDevNameLab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	mAudioDevNameLab->setFixedWidth(66);


    mSpcSlider = new SpcVolSlider(this);
    mSpcSlider->setStep(10);
    mSpcSlider->setFixedSize(66, 116);
    mSpcSlider->setObjectName("VolSlider");


		signal_handler_connect(obs_volmeter_get_signal_handler(obs_volmeter_),
			"levels_updated", OBSVolmeterChanged, this);
		obs_volmeter_attach_source(obs_volmeter_, mSource);


	//add gain filter
	gainFilter_ = obs_source_get_filter_by_name(mSource, gainFilterId);
	if (!gainFilter_) {
		gainFilter_ = obs_source_create(OBS_SOURCE_TYPE_FILTER, gainFilterId, gainFilterId, 0, 0);
		if (gainFilter_){
			obs_source_filter_add(mSource, gainFilter_);
			obs_data_t *filterSettings = obs_source_get_settings(gainFilter_);
			obs_data_set_double(filterSettings, "db", 0.0f);
			obs_source_update(gainFilter_, filterSettings);
			obs_data_release(filterSettings);
			obs_source_release(gainFilter_);
		}
	}
	else{
		obs_data_t *filterSettings = obs_source_get_settings(gainFilter_);
		obs_data_set_double(filterSettings, "db", 0.0f);
		obs_source_update(gainFilter_, filterSettings);
		obs_data_release(filterSettings);
	}

	//add noise gate filter
	if (!QString::compare("wasapi_input_capture", obs_source_get_id(mSource), Qt::CaseInsensitive)) { 
		noiseGateFilter_ = obs_source_get_filter_by_name(mSource, noiseGateFilterId);
		if (!noiseGateFilter_){
			noiseGateFilter_ = obs_source_create(OBS_SOURCE_TYPE_FILTER, noiseGateFilterId, noiseGateFilterId, 0, 0);
			if (noiseGateFilter_){
				obs_source_filter_add(mSource, noiseGateFilter_);
				obs_source_set_enabled(noiseGateFilter_, false);

#if 0
				obs_data_t *filterSettings = obs_source_get_settings(noiseGateFilter_);

				obs_data_set_default_double(filterSettings, "open_threshold", -26.0f);
				obs_data_set_default_double(filterSettings, "close_threshold", -32.0f);
				obs_data_set_default_int   (filterSettings, "attack_time", 25);
				obs_data_set_default_int   (filterSettings, "hold_time", 200);
				obs_data_set_default_int   (filterSettings, "release_time", 150);

				obs_source_update(noiseGateFilter_, filterSettings);
				obs_data_release(filterSettings);
#endif

				obs_source_release(noiseGateFilter_);
			}
		}
		else {
			obs_source_set_enabled(noiseGateFilter_, true);
#if 0
			obs_data_t *filterSettings = obs_source_get_settings(noiseGateFilter_);

			obs_data_set_default_double(filterSettings, "open_threshold", -26.0f);
			obs_data_set_default_double(filterSettings, "close_threshold", -32.0f);
			obs_data_set_default_int(filterSettings, "attack_time", 25);
			obs_data_set_default_int(filterSettings, "hold_time", 200);
			obs_data_set_default_int(filterSettings, "release_time", 150);

			obs_source_update(noiseGateFilter_, filterSettings);
			obs_data_release(filterSettings);
#endif
		}
	}

	mVolumeLab = new QLabel(this);
	mVolumeLab->setObjectName("AudioSettingVolLab");
	mVolumeLab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	mVolumeLab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QHBoxLayout *layout_h = new QHBoxLayout();
	QSpacerItem *space_h_0 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	QSpacerItem *space_h_1 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	layout_h->addItem(space_h_0);

    layout_h->addWidget(mSpcSlider);
  //  mSpcSlider->show();
  //}
    
	layout_h->addItem(space_h_1);

	QSpacerItem *space_v_0 = new QSpacerItem(0, 7, QSizePolicy::Preferred, QSizePolicy::Fixed);
	QSpacerItem *space_v_1 = new QSpacerItem(0, 7, QSizePolicy::Preferred, QSizePolicy::Fixed);

	QVBoxLayout *HVayout = new QVBoxLayout(this);
	HVayout->setSpacing(0);
	HVayout->setContentsMargins(15, 0, 0, 0);
	HVayout->addWidget(mVolumeLab);
	HVayout->addItem(space_v_0);
	HVayout->addLayout(layout_h);
	HVayout->addItem(space_v_1);
	HVayout->addWidget(mAudioDevNameLab);
	setLayout(HVayout);
}

void AudioDevControl::OBSVolumeChanged(void *data, calldata_t *calldata) {

	Q_UNUSED(calldata);
	AudioDevControl *audioCtrl = static_cast<AudioDevControl *>(data);
	QMetaObject::invokeMethod(audioCtrl, "mSltVolumeChanged");
}

void AudioDevControl::OBSVolmeterChanged(void *data, calldata_t *calldata)
{
	AudioDevControl *audioCtrl = static_cast<AudioDevControl *>(data);

	float level = calldata_float(calldata, "level");
	float magnitude = calldata_float(calldata, "magnitude");
	float peak = calldata_float(calldata, "peak");
	bool muted = calldata_bool(calldata, "muted");

	//QMessageBox::information(0, QString::number(level), QString::number(peak));
	//audioCtrl->mSltVolmeterChanged(50);
	QMetaObject::invokeMethod(audioCtrl, "mSltVolmeterChanged", Q_ARG(int, level * 100));

}

void AudioDevControl::OBSVolumeMuted(void *data, calldata_t *calldata) {

	AudioDevControl *audioCtrl = static_cast<AudioDevControl *>(data);
	bool muted = calldata_bool(calldata, "muted");

	QMetaObject::invokeMethod(audioCtrl, "mSltVolumeMuted", Q_ARG(bool, muted));
}


QString AudioDevControl::mGetName() const {
	return mAudioDevNameLab->text();
}

void AudioDevControl::mSetName(const QString &newName) {
	mAudioDevNameLab->setText(newName);
}

void AudioDevControl::mSltVolumeChanged() {

	int vol = (int)(obs_fader_get_deflection(mOBSFader)*100.0f);

    mSpcSlider->setValue(vol);


	
	mVolumeLab->setText(QString("%1%").arg(vol));

	obs_fader_set_deflection(mOBSFader, float(vol*0.01));
	mSltVolumeMuted(vol?false:true);
}

void AudioDevControl::mSltVolmeterChanged(int vol) {

	/*int vol = (int)(obs_fader_get_deflection(mOBSFader)*100.0f);
	mVolumeLab->setText(QString("%1%").arg(vol));*/


   mSpcSlider->setDynVol(vol);

}

void AudioDevControl::mSltVolumeMuted(bool muted) {

	if (mIsMuted != muted){
		mIsMuted = muted;
		mSltSetMuted(mIsMuted);
	}
}

void AudioDevControl::mSltSetMuted(bool checked){
	obs_source_set_muted(mSource, checked);
	mIsMuted = checked;
}

void AudioDevControl::mSltSliderChanged(int vol) {


  mSpcSlider->setValue(vol);

	mVolumeLab->setText(QString("%1%").arg(vol));

	obs_fader_set_deflection(mOBSFader, float(vol*0.01));
	mSltSetMuted(!vol);

	float gainDB = (vol>100)?float((vol - 100.0) * 3 / 40.0):0.0f;
	//float gainDB = (vol>100)?float((vol - 100.0) * 3 / 20.0):0.0f;
	//float gainDB = (vol>100)?float((vol - 100.0) * 2.5 / 20.0):0.0f;
	//float gainDB = (vol>100)?float((vol - 100.0) / 10.0):0.0f;
	if (gainFilter_){
		obs_data_t* filterSettings = obs_source_get_settings(gainFilter_);
		obs_data_set_double(filterSettings, "db", gainDB);
		obs_source_update(gainFilter_, filterSettings);
		obs_data_release(filterSettings);
	}
}

//////////////////////////////////////////////////////////////////////////////////

BiLiAudioDevSettingWid::BiLiAudioDevSettingWid()
    : RoundedWidget(QColor(255, 255, 255)), count_(0) {

	ui.setupUi(this);
	setFixedWidth(90);
	ui.VolScrollAreaWidContents->setLayout(new QHBoxLayout());

	ui.VolScrollAreaWidContents->layout()->setContentsMargins(0, 15, 15, 15);

    BiliEffectWidget *effect = new BiliEffectWidget(0);
    effect_widget_ = effect;
    setParent(effect);
    setWindowFlags(windowFlags() | Qt::Popup);

    
}

BiLiAudioDevSettingWid::~BiLiAudioDevSettingWid() {

}

void BiLiAudioDevSettingWid::mReplaceItems(const std::vector<AudioDevControl*>& items)
{
	std::vector<AudioDevControl*> item_copy = items;

	QLayout *layout = ui.VolScrollAreaWidContents->layout();

	while (layout->count())
		layout->removeItem(layout->itemAt(0));

	std::vector<AudioDevControl*>::const_iterator it = items.begin();

	while (items.end() != it)
		layout->addWidget(*it++);

	setFixedWidth(90 * layout->count() + 5);
}

void BiLiAudioDevSettingWid::showEvent(QShowEvent *)
{
    //effect_widget_->doOnHostWidgetVisibleStateChanged(true);

    effect_widget_->doOnHostWidgetPositionChanged(frameGeometry().x(), frameGeometry().y());
}
void BiLiAudioDevSettingWid::hideEvent(QHideEvent*)
{
    effect_widget_->doOnHostWidgetVisibleStateChanged(false);
}

void BiLiAudioDevSettingWid::resizeEvent(QResizeEvent *)
{
    //if (first_resize_) {
    //    //if (pos() == QPoint(0, 0))
        effect_widget_->doOnHostWidgetResize(frameGeometry().x(), frameGeometry().y(), frameGeometry().width(), frameGeometry().height());
        //else


    //    first_resize_ = false;
    //}
    //else
    //    effect_widget_->doOnHostWidgetResize(frameGeometry().x(), frameGeometry().y(), frameGeometry().width(), frameGeometry().height());
}

void BiLiAudioDevSettingWid::moveEvent(QMoveEvent *)
{
    effect_widget_->doOnHostWidgetPositionChanged(frameGeometry().x(), frameGeometry().y());

    //qDebug() << frameGeometry().topLeft();
}

