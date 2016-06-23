#include "PropertyDlgVolSliderWid.h"

#include <qslider.h>
#include <qlayout.h>
#include <qlabel.h>

#include "circle_slider_slider.h"


#include "obs.h"

const int _slider_val_min = 0;
const int _slider_val_max = 100;
const int _slider_width = 14;


PropertyDlgVolSliderWid::PropertyDlgVolSliderWid(obs_source_t* source) :
	obs_source_(source)
{
	setupUi();

	initVolControl();
}

PropertyDlgVolSliderWid::~PropertyDlgVolSliderWid()
{
	destoryControl();
}

int PropertyDlgVolSliderWid::getOriVal() const
{
	return ori_val_;
}

void PropertyDlgVolSliderWid::revert()
{
	onVolSliderValChanged(getOriVal());
}

int PropertyDlgVolSliderWid::getVolVal() const
{
	return vol_slider_->value();
}

void PropertyDlgVolSliderWid::setVolVal(int vol)
{
	onVolSliderValChanged(vol * 10);
	updateSliderVal(vol);
}

obs_source_t* PropertyDlgVolSliderWid::getSource() const
{
	return obs_source_;
}

void PropertyDlgVolSliderWid::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
}

void PropertyDlgVolSliderWid::setupUi()
{
	vol_slider_ = new CircleSliderSlider(this);
	vol_slider_->setObjectName("PropertyVOlSlider");
	vol_slider_->setRange(0, 100, 0);
	//vol_slider_->init();

	val_label_ = new QLabel();
    val_label_->setFixedWidth(34);
    val_label_->setAlignment(Qt::AlignCenter);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(vol_slider_);
	layout->addWidget(val_label_);
}

void PropertyDlgVolSliderWid::initVolControl()
{
	obs_fader_ = obs_fader_create(OBS_FADER_CUBIC);

	signal_handler_connect(obs_fader_get_signal_handler(obs_fader_),
		"volume_changed", OBSVolumeChanged, this);
	signal_handler_connect(obs_source_get_signal_handler(obs_source_),
		"mute", OBSVolumeMuted, this);

	connect(vol_slider_, SIGNAL(valueChanged(int)), this, SLOT(onVolSliderValChanged(int)));

	obs_fader_attach_source(obs_fader_, obs_source_);
	
	ori_val_ = (int)(obs_fader_get_deflection(obs_fader_) * 100.0f);
	updateSliderVal(ori_val_);
}

void PropertyDlgVolSliderWid::destoryControl()
{
	signal_handler_disconnect(obs_fader_get_signal_handler(obs_fader_),
		"volume_changed", OBSVolumeChanged, this);

	signal_handler_disconnect(obs_source_get_signal_handler(obs_source_),
		"mute", OBSVolumeMuted, this);

	obs_fader_destroy(obs_fader_);
}

void PropertyDlgVolSliderWid::onVolSliderValChanged(int vol)
{
	int offset = (((double)_slider_width / 2) / vol_slider_->width()) * double(100 * 10);

	vol_slider_->blockSignals(true);
	vol_slider_->setValue(vol);
	vol_slider_->blockSignals(false);

	val_label_->setText(QString::number(vol));

	obs_fader_set_deflection(obs_fader_, float(vol * 0.01));
}

void PropertyDlgVolSliderWid::updateSliderVal(int val)
{

	vol_slider_->blockSignals(true);

	vol_slider_->setValue(val);

	vol_slider_->blockSignals(false);
	
	val_label_->setText(QString::number(val));

	obs_fader_set_deflection(obs_fader_, float(val * 0.01));
	mSltVolumeMuted(_slider_val_min >= val);
}

void PropertyDlgVolSliderWid::mSltVolumeMuted(bool mute)
{
	if (obs_source_muted(obs_source_) != mute)
		obs_source_set_muted(obs_source_, mute);
}

void PropertyDlgVolSliderWid::updateVolFromObs()
{
	int val = (int)(obs_fader_get_deflection(obs_fader_) * 100.0f);

	updateSliderVal(val);
}

void PropertyDlgVolSliderWid::OBSVolumeChanged(void *data, calldata_t *calldata)
{
	static_cast<PropertyDlgVolSliderWid *>(data)->updateVolFromObs();
}

void PropertyDlgVolSliderWid::OBSVolumeMuted(void *data, calldata_t *calldata)
{
	static_cast<PropertyDlgVolSliderWid *>(data)->mSltVolumeMuted(calldata_bool(calldata, "muted"));
}