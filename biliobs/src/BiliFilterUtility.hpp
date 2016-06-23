#ifndef BILIFILTERUTILITY_H
#define BILIFILTERUTILITY_H

#include "../libobs/obs.h"
#include "BiliUIConfigSync.hpp"

class BiliFiltersBackupData;

class BiliFiltersBackup
{
	BiliFiltersBackupData* m_Data;

	BiliFiltersBackup(const BiliFiltersBackup&) = delete;
	BiliFiltersBackup& operator = (const BiliFiltersBackup&) = delete;
public:
	~BiliFiltersBackup();
	BiliFiltersBackup(BiliFiltersBackupData* data);

	BiliFiltersBackupData* GetData() { return m_Data; };
};

BiliFiltersBackup* bili_backup_source_filter_settings(obs_source_t* src);
bool bili_restore_source_filter_settings(obs_source_t* src, BiliFiltersBackup* data);

template<class QTWidgetT, class DataTypeT>
bool WidgetToFilterData(DataTypeT reserved, QTWidgetT* widget, obs_source_t* src, const char* filterName, const char* index)
{
	bool bRet = false;
	obs_source_t* filter = obs_source_get_filter_by_name(src, filterName);
	if (filter)
	{
		obs_data_t* filterSettings = obs_source_get_settings(filter);
		if (filterSettings)
		{
			bRet = WidgetToData(reserved, widget, filterSettings, index);
			obs_source_update(filter, filterSettings);
			obs_data_release(filterSettings);
		}
		obs_source_release(filter);
	}

	return bRet;
}

template<class QTWidgetT, class DataTypeT>
bool FilterDataToWidget(DataTypeT reserved, QTWidgetT* widget, obs_source_t* src, const char* filterName, const char* index)
{
	bool bRet = false;
	obs_source_t* filter = obs_source_get_filter_by_name(src, filterName);
	if (filter)
	{
		obs_data_t* filterSettings = obs_source_get_settings(filter);
		if (filterSettings)
		{
			bRet = DataToWidget(reserved, widget, filterSettings, index);
			obs_data_release(filterSettings);
		}
		obs_source_release(filter);
	}

	return bRet;
}

#endif
