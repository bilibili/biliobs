#include "bili_obs_source_helper.h"

static bool enum_source_callback(void* param, obs_source_t* source)
{
	std::vector<OBSSource>& r = *static_cast<std::vector<OBSSource>*>(param);
	r.push_back(source);
	return true;
}

std::vector<OBSSource> OBSEnumSources()
{
	std::vector<OBSSource> r;
	obs_enum_sources(enum_source_callback, &r);
	return std::move(r);
}



static bool enum_sceneitem_callback(obs_scene_t* scene, obs_sceneitem_t* item, void* param)
{
	std::vector<OBSSceneItem>& r = *static_cast<std::vector<OBSSceneItem>*>(param);
	r.push_back(item);
	return true;
}

std::vector<OBSSceneItem> OBSEnumSceneItems(obs_scene_t* scene)
{
	std::vector<OBSSceneItem> r;
	obs_scene_enum_items(scene, enum_sceneitem_callback, &r);
	return std::move(r);
}


static void enum_filters_callback(obs_source_t* source, obs_source_t* filter, void* param)
{
	std::vector<OBSSource>& r = *static_cast<std::vector<OBSSource>*>(param);
	r.push_back(filter);
}

std::vector<OBSSource> OBSEnumFilters(obs_source_t* source)
{
	std::vector<OBSSource> r;
	obs_source_enum_filters(source, enum_filters_callback, &r);
	return std::move(r);
}
