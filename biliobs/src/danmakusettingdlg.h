#ifndef DANMAKUSETTINGDLG_H
#define DANMAKUSETTINGDLG_H

#include "advsetttingbasedlg.h"
#include "../libobs/util/config-file.h"

class DanmakuSettingDlg : public AdvSetttingBaseDlg{

	Q_OBJECT

public:
	DanmakuSettingDlg(config_t* basicConfig, QWidget *parent = 0);
	~DanmakuSettingDlg();

	int saveSetting_();
	int cancelSetting_();
	void show_();
	QPushButton *addBtn_();
	void addStackedPageWid_(int index, QWidget* wid = NULL);

signals:
	void OnSaveSignal(QVariant pConfig);
	void OnLoadSignal(QVariant pConfig);
	void OnCancelSignal(QVariant pConfig);

private:
	config_t* mBasicConfig;
};

#endif // DANMAKUSETTINGDLG_H
