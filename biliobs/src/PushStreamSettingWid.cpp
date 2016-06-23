#include "PushStreamSettingWid.h"
#include <QFileDialog>

PushStreamSettingWid::PushStreamSettingWid(ConfigFile& configData_, bool isDisableCustomPushUrl, QWidget *parent)
	: QWidget(parent)
	, configData(configData_)
{
	ui.setupUi(this);

	QObject::connect(ui.pushServerType, SIGNAL(buttonToggled(QAbstractButton*, bool)), this, SLOT(serverTypeToggled(QAbstractButton*, bool)));

	LoadConfig();

	if (isDisableCustomPushUrl)
	{
		ui.pushServerEdit->setEnabled(false);
		ui.pushPathEdit->setEnabled(false);
	}
}

PushStreamSettingWid::~PushStreamSettingWid()
{

}

void PushStreamSettingWid::LoadConfig()
{
	DataToWidget(BILI_CONFIG_STRING(), ui.pushServerType, static_cast<config_t*>(configData), "AdvOut\0PushServerType");
	DataToWidget(BILI_CONFIG_STRING(), ui.pushPathEdit, static_cast<config_t*>(configData), "AdvOut\0PushStreamPath");
	DataToWidget(BILI_CONFIG_STRING(), ui.pushServerEdit, static_cast<config_t*>(configData), "AdvOut\0PushStreamServer");

	UpdateUI();
}

bool PushStreamSettingWid::SaveConfig()
{
	WidgetToData(BILI_CONFIG_STRING(), ui.pushServerType, static_cast<config_t*>(configData), "AdvOut\0PushServerType");
	WidgetToData(BILI_CONFIG_STRING(), ui.pushPathEdit, static_cast<config_t*>(configData), "AdvOut\0PushStreamPath");
	WidgetToData(BILI_CONFIG_STRING(), ui.pushServerEdit, static_cast<config_t*>(configData), "AdvOut\0PushStreamServer");

	return true;
}

void PushStreamSettingWid::UpdateUI()
{
	if (ui.pushToBiliRadio->isChecked())
	{
		ui.pushServerEdit->setEnabled(false);
		ui.pushPathEdit->setEnabled(false);
        ui.pushServerLabel->setEnabled(false);
        ui.pushPathLabel->setEnabled(false);
	}
	else if (ui.pushToCustomRadio->isChecked())
	{
		ui.pushServerEdit->setEnabled(true);
		ui.pushPathEdit->setEnabled(true);
        ui.pushServerLabel->setEnabled(true);
        ui.pushPathLabel->setEnabled(true);
	}
}

void PushStreamSettingWid::serverTypeToggled(QAbstractButton * button, bool checked)
{
	UpdateUI();
}
