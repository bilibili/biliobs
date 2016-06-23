#ifndef BILI_OBS_SOURCE_HELPER_H
#define BILI_OBS_SOURCE_HELPER_H

#include "../libobs/obs.h"
#include "../libobs/obs.hpp"

#include <vector>

std::vector<OBSSource> OBSEnumSources();
std::vector<OBSSceneItem> OBSEnumSceneItems(obs_scene_t* scene);
std::vector<OBSSource> OBSEnumFilters(obs_source_t* source);

#endif
