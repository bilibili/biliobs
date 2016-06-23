#include "AudioDeviceSettingWid.h"

#include "obs.h"
#include "BiLiApp.h"
#include "BiliUIConfigSync.hpp"
#include "BiliFilterUtility.hpp"
#include <vector>
#include <QComboBox>
#include <QStyledItemDelegate>

static bool AddAudioChannelByName(int channel, const char* sourceName, QComboBox* comboBox)
{
	//Ìî³äÁÐ±í
	obs_properties_t* props = obs_get_source_properties(OBS_SOURCE_TYPE_INPUT, sourceName);
	if (!props)
		return false;

	DataToWidget(BILI_PROP_LIST_STRING(), comboBox, props, "device_id");
	obs_properties_destroy(props);

	//¼ÓÔØÅäÖÃ
	obs_source_t* source = obs_get_output_source(channel);
	if (!source)
		return false;

	obs_data_t* sourceSetting = obs_source_get_settings(source);
	DataToWidget(BILI_DATA_STRING(), comboBox, sourceSetting, "device_id");

	if (!strcmp(sourceName, App()->mInputAudioSource())) {
		QCheckBox *noiseReduceCB = qVPtr<QCheckBox>::toPtr(comboBox->property("NoiseReduceCB"));
		if (!DataToWidget(BILI_DATA_BOOL(), noiseReduceCB, sourceSetting, comboBox->currentText().toUtf8().data()))
			noiseReduceCB->setChecked(false);
	}

	obs_data_release(sourceSetting);
	obs_source_release(source);

	return true;
}

static bool SaveAudioChannelSettings(int channel, QComboBox* comboBox)
{
	//±£´æÅäÖÃ
	obs_source_t* source = obs_get_output_source(channel);
	if (!source)
		return false;

	obs_data_t *sourceSetting = obs_source_get_settings(source);
	obs_data_t *newSourceSetting = obs_data_create();
	obs_data_apply(newSourceSetting, sourceSetting);

	WidgetToData(BILI_DATA_STRING(), comboBox, newSourceSetting, "device_id");
	if (!strcmp(obs_source_get_id(source), App()->mInputAudioSource())) {
		QCheckBox *noiseReduceCB = qVPtr<QCheckBox>::toPtr(comboBox->property("NoiseReduceCB"));
		WidgetToData(BILI_DATA_BOOL(), noiseReduceCB, newSourceSetting, comboBox->currentText().toUtf8().data());

		obs_source_t *noiseGateFilter = obs_source_get_filter_by_name(source, "noise_gate_filter");
		obs_source_set_enabled(noiseGateFilter, noiseReduceCB->isChecked());
		obs_source_release(noiseGateFilter);
	}

	obs_source_update(source, newSourceSetting);

	obs_data_release(newSourceSetting);
	obs_data_release(sourceSetting);
	obs_source_release(source);
	return true;
}

static const char* forcemono_filter_name = "forcemono_filter";

static obs_source_t* CreateOrGetForceMonoFilter(obs_source_t* source)
{
	if (!source)
		return 0;

	obs_source_t* filter = obs_source_get_filter_by_name(source, forcemono_filter_name);
	if (filter)
		return filter;
	else
	{
		obs_source_t* forcemonoFilter = obs_source_create(OBS_SOURCE_TYPE_FILTER, forcemono_filter_name, forcemono_filter_name, 0, 0);
		if (forcemonoFilter)
		{
			obs_source_filter_add(source, forcemonoFilter);
		}
		return forcemonoFilter;
	}
}

AudioDeviceSettingWid::AudioDeviceSettingWid(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.NRDescLabel->hide();
	ui.NRLabel->hide();
	ui.NRSlider->hide();
	ui.NRValueLabel->hide();

	QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
	ui.inputDeviceList->setItemDelegate(itemDelegate);
	ui.outputDeviceList->setItemDelegate(itemDelegate);

	ui.inputDeviceList->setProperty("NoiseReduceCB", qVPtr<QCheckBox>::toVariant(ui.NoiseReduceCB) );

	AddAudioChannelByName(1, App()->mOutputAudioSource(), ui.outputDeviceList);
	AddAudioChannelByName(3, App()->mInputAudioSource(), ui.inputDeviceList);
	
	obs_source_t* source = obs_get_output_source(3);
	if (source)
	{
		obs_source_t* forcemonoFilter = CreateOrGetForceMonoFilter(source);
		obs_source_release(forcemonoFilter);

		FilterDataToWidget(BILI_DATA_STRING(), ui.mixdownModeRadios, source, forcemono_filter_name, "mode");
		obs_source_release(source);
	}

	QObject::connect(ui.inputDeviceList, &QComboBox::currentTextChanged, [this](QString txt){
		obs_source_t* source = obs_get_output_source(3);
		if (!source)
			return false;

		obs_data_t* sourceSetting = obs_source_get_settings(source);
		if ( !DataToWidget(BILI_DATA_BOOL(), ui.NoiseReduceCB, sourceSetting, txt.toUtf8().data()) )
			ui.NoiseReduceCB->setChecked(false);

		obs_data_release(sourceSetting);
		obs_source_release(source);
	});
}

AudioDeviceSettingWid::~AudioDeviceSettingWid() {

}

bool AudioDeviceSettingWid::SaveConfig() {

	ui.inputDeviceList->setProperty("NoiseReduceCB", qVPtr<QCheckBox>::toVariant(ui.NoiseReduceCB) );

	obs_source_t* source = obs_get_output_source(3);
	if (source)
	{
		obs_source_t* forcemonoFilter = CreateOrGetForceMonoFilter(source);
		obs_source_release(forcemonoFilter);

		WidgetToFilterData(BILI_DATA_STRING(), ui.mixdownModeRadios, source, forcemono_filter_name, "mode");
		obs_source_release(source);
	}

	SaveAudioChannelSettings(1, ui.outputDeviceList);
	SaveAudioChannelSettings(3, ui.inputDeviceList);

	return true;
}
