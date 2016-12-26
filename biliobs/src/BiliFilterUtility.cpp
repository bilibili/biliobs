#include "BiliFilterUtility.hpp"
#include "bili_obs_source_helper.h"
#include "../libobs/obs.hpp"
#include <vector>
#include <string>
#include <memory>
#include <list>
#include <unordered_set>

class FilterBackupData
{
	FilterBackupData(obs_source_t* filter)
	{
		filterId = obs_source_get_id(filter);
		filterName = obs_source_get_name(filter);
		filterSettings = obs_data_create();

		obs_data_t* settings = obs_source_get_settings(filter);
		obs_data_apply(filterSettings, settings);
		obs_data_release(settings);
	}

	FilterBackupData(const FilterBackupData&) = delete;
	FilterBackupData& operator = (const FilterBackupData&) = delete;
public:
	~FilterBackupData()
	{
		if (filterSettings)
			obs_data_release(filterSettings);
	}

	static FilterBackupData* FromFilter(obs_source_t* source, const char* filterName)
	{
		obs_source_t* filter = obs_source_get_filter_by_name(source, filterName);
		if (filter)
		{
			FilterBackupData* r = new FilterBackupData(filter);
			obs_source_release(filter);
			return r;
		}
		else
			return 0;
	}

	static FilterBackupData* FromFilter(obs_source_t* filter)
	{
		return new FilterBackupData(filter);
	}

	bool RestoreToSource(obs_source_t* source)
	{
		obs_source_t* filter = obs_source_get_filter_by_name(source, filterName.c_str());
		if (!filter)
		{
			filter = obs_source_create(OBS_SOURCE_TYPE_FILTER, filterId.c_str(), filterName.c_str(), filterSettings, 0);
			obs_source_filter_add(source, filter);
			obs_source_release(filter);
			return true;
		}
		else
		{
			obs_source_update(filter, filterSettings);
			return true;
		}
	}

	const std::string& GetFilterId() const 
	{
		return filterId;
	}

	const std::string& GetFilterName() const
	{
		return filterName;
	}
private:
	std::string filterId;
	std::string filterName;
	obs_data_t* filterSettings;
};

typedef std::unique_ptr<FilterBackupData> PFilterBackupData;

class BiliFiltersBackupData
{
	BiliFiltersBackupData(const BiliFiltersBackupData&) = delete;
	BiliFiltersBackupData& operator = (const BiliFiltersBackupData&) = delete;
public:
	BiliFiltersBackupData(obs_source_t* src)
	{
		for (OBSSource& filter : OBSEnumFilters(src))
		{
			filtersBackupData.push_back(PFilterBackupData(FilterBackupData::FromFilter(filter)));
		}
	}

	bool RestoreToSource(obs_source_t* source)
	{
		std::unordered_set<std::string> existFiltersBefore;

		//注：现在还不会恢复顺序！
		for (PFilterBackupData& x : filtersBackupData)
		{
			x->RestoreToSource(source);
			existFiltersBefore.insert(x->GetFilterName());
		}

		//删除多出来的filter
		std::unordered_set<std::string> existFiltersAfter;
		for (OBSSource& filter : OBSEnumFilters(source))
		{
			existFiltersAfter.insert(obs_source_get_name(filter));
		}

		for (const std::string& filterName : existFiltersAfter)
		{
			//如果保存的时候没的现在有了，就删掉
			if (existFiltersBefore.find(filterName) == existFiltersBefore.end())
			{
				obs_source_t* filter = obs_source_get_filter_by_name(source, filterName.c_str());
				if (filter)
					obs_source_filter_remove(source, filter);
				obs_source_release(filter);
			}
		}

		return true;
	}
private:
	std::vector<PFilterBackupData> filtersBackupData;
};

BiliFiltersBackup::~BiliFiltersBackup()
{
	delete m_Data;
}

BiliFiltersBackup::BiliFiltersBackup(BiliFiltersBackupData* data)
	: m_Data(data)
{
}

BiliFiltersBackup* bili_backup_source_filter_settings(obs_source_t* src)
{
	return new BiliFiltersBackup(new BiliFiltersBackupData(src));
}

bool bili_restore_source_filter_settings(obs_source_t* src, BiliFiltersBackup* data)
{
	return data->GetData()->RestoreToSource(src);
}

