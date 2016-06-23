#include "BiLiOBSMainWid.h"
#include "BiLiApp.h"
#include "BiliOBSUtility.hpp"
#include "bili_obs_source_helper.h"
#include "BiliGlobalDatas.hpp"


#define DESKTOP_AUDIO_1 Str("DesktopAudioDevice1")
#define DESKTOP_AUDIO_2 Str("DesktopAudioDevice2")
#define AUX_AUDIO_1     Str("AuxAudioDevice1")
#define AUX_AUDIO_2     Str("AuxAudioDevice2")
#define AUX_AUDIO_3     Str("AuxAudioDevice3")



static void LoadAudioDevice(const char *name, int channel, obs_data_t *parent) {

	obs_data_t *data = obs_data_get_obj(parent, name);
	if (!data)
		return;

	obs_source_t *source = obs_load_source(data);
	if (source) {
		obs_set_output_source(channel, source);
		obs_source_release(source);
	}

	obs_data_release(data);
}


static void SaveAudioDevice(const char *name, int channel, obs_data_t *parent) {

	obs_source_t *source = obs_get_output_source(channel);
	if (!source)
		return;

	obs_data_t *data = obs_save_source(source);

	obs_data_set_obj(parent, name, data);

	obs_data_release(data);
	obs_source_release(source);
}

static obs_data_t *GenerateSaveData(obs_data_array_t *sceneOrder) {

	obs_data_t       *saveData = obs_data_create();
	obs_data_array_t *sourcesArray = obs_save_sources();
	obs_source_t     *currentScene = obs_get_output_source(0);
	const char       *sceneName = obs_source_get_name(currentScene);

	const char *sceneCollection = config_get_string(App()->mGetGlobalConfig(),
		"Basic", "SceneCollection");

	SaveAudioDevice(DESKTOP_AUDIO_1, 1, saveData);
	SaveAudioDevice(DESKTOP_AUDIO_2, 2, saveData);
	SaveAudioDevice(AUX_AUDIO_1, 3, saveData);
	SaveAudioDevice(AUX_AUDIO_2, 4, saveData);
	SaveAudioDevice(AUX_AUDIO_3, 5, saveData);

	obs_data_set_string(saveData, "current_scene", sceneName);
	obs_data_set_array(saveData, "scene_order", sceneOrder);
	obs_data_set_string(saveData, "name", sceneCollection);
	obs_data_set_array(saveData, "sources", sourcesArray);
	obs_data_array_release(sourcesArray);
	obs_source_release(currentScene);

	return saveData;
}

void BiLiOBSMainWid::mSaveProjectDeferred() {

	if (mDisableSaving)
		return;

	if (!mProjectChanged)
		return;

	mProjectChanged = false;

	const char *sceneCollection = config_get_string(App()->mGetGlobalConfig(),
			"Basic", "SceneCollectionFile");
	char savePath[512];
	char fileName[512];
	int ret;

	if (!sceneCollection)
		return;

	ret = snprintf(fileName, 512, QString("%1\\%2").arg(QString::fromStdString(gBili_mid)).arg("basic/scenes/%s.json").toUtf8().data(), sceneCollection);
	if (ret <= 0)
		return;

	ret = GetUserDataPath(savePath, sizeof(savePath), fileName);
	if (ret <= 0)
		return;

	mSave(savePath);
}

void BiLiOBSMainWid::mSaveProject() {

	if (mDisableSaving)
		return;

	mProjectChanged = true;
	QMetaObject::invokeMethod(this, "mSaveProjectDeferred",
			Qt::QueuedConnection);
}


obs_data_array_t *BiLiOBSMainWid::mSaveSceneListOrder() {

	obs_data_array_t *sceneOrder = obs_data_array_create();

#if 1
	obs_source_t* currentScene = obs_get_output_source(0);
	std::list<std::string> tmpSceneOrder;

	//获取所有场景
	for (OBSSource& src : OBSEnumSources())
	{
		if (strcmp(obs_source_get_id(src), "scene") == 0)
			tmpSceneOrder.push_back(obs_source_get_name(src));
	}

	//当前场景放在第一个
	if (currentScene)
	{
		std::string currentSceneName = obs_source_get_name(currentScene);
		auto x = std::find(tmpSceneOrder.begin(), tmpSceneOrder.end(), currentSceneName);
		assert(x != tmpSceneOrder.end());
		tmpSceneOrder.erase(x);
		tmpSceneOrder.push_front(obs_source_get_name(currentScene));
	}

	//保存场景顺序
	for (auto& x : tmpSceneOrder) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name", x.c_str());
		obs_data_array_push_back(sceneOrder, data);
		obs_data_release(data);
	}

	obs_source_release(currentScene);
#else
	for (int i = 0; i < ui->scenes->count(); i++) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name",
				QT_TO_UTF8(ui->scenes->item(i)->text()));
		obs_data_array_push_back(sceneOrder, data);
		obs_data_release(data);
	}
#endif

	return sceneOrder;
}

void BiLiOBSMainWid::mSave(const char *file) 
{
	SaveScene();
	SaveAudioDeviceConfig();
	SaveFrontendHotkeys();
	mBasicConfig.Save();
#if 0
	obs_data_array_t *sceneOrder = mSaveSceneListOrder();
	obs_data_t *saveData  = GenerateSaveData(sceneOrder);

	if (!obs_data_save_json_safe(saveData, file, "tmp", "bak"))
		blog(LOG_ERROR, "Could not save scene data to %s", file);

	obs_data_release(saveData);
	obs_data_array_release(sceneOrder);
#endif
}


void BiLiOBSMainWid::SaveScene()
{
	obs_data_t* sceneData;

	std::list<obs_scene_t*> scenes;

	//移除未使用的source
	for (OBSSource& source : OBSEnumSources())
	{
		obs_scene_t* scene = obs_scene_from_source(source);
		if (scene)
			scenes.push_back(scene);
	}

	std::list<obs_source_t*> sourcesToRemove;

	//遍历所有source，找出有在场景中出现的，其他删掉
	for (OBSSource& source : OBSEnumSources())
	{
		if (obs_scene_from_source(source) == 0)
		{
			//如果是场景那么就不执行这里面的代码，只对场景元素执行里面的代码
			bool isReferred = false;

			//遍历每个场景里的全部source，看看有没有当前的
			for (obs_scene_t* scene : scenes)
			{
				for (OBSSceneItem& sceneItem : OBSEnumSceneItems(scene))
				{
					obs_source_t* sceneItemSource = obs_sceneitem_get_source(sceneItem);
					if (sceneItemSource == source)
					{
						//找到了，标记找到，并停止当前查找
						isReferred = true;
						break;
					}
				}

				//如果已经找到，就不需要在下一个场景中找
				if (isReferred)
					break;
			}

			//没被引用的放到待删除里
			if (isReferred == false)
				sourcesToRemove.push_back(source);
		}
	}

	//删
	for (auto x : sourcesToRemove)
		obs_source_remove(x);


	//保存
	sceneData = BiliSceneConfig::Get();
	if (sceneData)
	{
		BiliConfigFile::SaveSceneData(sceneData);
		obs_data_release(sceneData);
	}
}


void BiLiOBSMainWid::mLoadSceneListOrder(obs_data_array_t *array) {

	size_t num = obs_data_array_count(array);

	for (size_t i = 0; i < num; i++) {
		obs_data_t *data = obs_data_array_item(array, i);
		const char *name = obs_data_get_string(data, "name");

		// UI Secens ListWidget
		//ReorderItemByName(ui->scenes, name, (int)i);

		obs_data_release(data);
	}
}


void BiLiOBSMainWid::mLoad(const char *file)
{
	LoadScene();
	LoadAudioDeviceConfig();
	LoadFrontendHotkeys();

#if 0
	if (!file || !os_file_exists(file)) {
		blog(LOG_INFO, "No scene file found, creating default scene");
		mCreateDefaultScene(true);
		mSaveProject();
		return;
	}

	mDisableSaving++;

	obs_data_t *data = obs_data_create_from_json_file_safe(file, "bak");
	if (!data) {
		mDisableSaving--;
		blog(LOG_ERROR, "Failed to load '%s', creating default scene",
				file);
		mCreateDefaultScene(true);
		mSaveProject();
		return;
	}

	mClearSceneData();

	obs_data_array_t *sceneOrder = obs_data_get_array(data, "scene_order");
	obs_data_array_t *sources    = obs_data_get_array(data, "sources");
	const char       *sceneName = obs_data_get_string(data,
			"current_scene");

	const char *curSceneCollection = config_get_string(
			App()->mGetGlobalConfig(), "Basic", "SceneCollection");

	obs_data_set_default_string(data, "name", curSceneCollection);

	const char       *name = obs_data_get_string(data, "name");
	obs_source_t     *curScene;

	if (!name || !*name)
		name = curSceneCollection;

	LoadAudioDevice(DESKTOP_AUDIO_1, 1, data);
	LoadAudioDevice(DESKTOP_AUDIO_2, 2, data);
	LoadAudioDevice(AUX_AUDIO_1,     3, data);
	LoadAudioDevice(AUX_AUDIO_2,     4, data);
	LoadAudioDevice(AUX_AUDIO_3,     5, data);

	obs_load_sources(sources);

	if (sceneOrder)
		mLoadSceneListOrder(sceneOrder);

	curScene = obs_get_source_by_name(sceneName);
	obs_set_output_source(0, curScene);
	obs_source_release(curScene);

	obs_data_array_release(sources);
	obs_data_array_release(sceneOrder);

	std::string file_base = strrchr(file, '/') + 1;
	file_base.erase(file_base.size() - 5, 5);

	config_set_string(App()->mGetGlobalConfig(), "Basic", "SceneCollection",
			name);
	config_set_string(App()->mGetGlobalConfig(), "Basic", "SceneCollectionFile",
			file_base.c_str());

	obs_data_release(data);

	mCleanupUnusedSources();

	mDisableSaving--;
#endif
}

void BiLiOBSMainWid::loadImgAtNoItems()
{
	/*获取分辨率*/
	uint width = config_get_uint(mBasicConfig, "Video", "BaseCX");
	uint height = config_get_uint(mBasicConfig, "Video", "BaseCY");


	sceneListWidgetOperator->loadImgOnNoItems(width, height);
}

void BiLiOBSMainWid::LoadScene()
{
	obs_data_t* sceneData = BiliConfigFile::LoadSceneData();
	if (sceneData)
	{
		BiliSceneConfig::Set(sceneData);
		obs_data_release(sceneData);
	}

	//如果加载进来的场景不够3个，就补到三个
	std::string firstAddedScene;
	int sceneCount = 0;
	for (OBSSource& src : OBSEnumSources())
	{
		if (strcmp(obs_source_get_id(src), "scene") == 0)
		{
			++sceneCount;
		}
	}

	int nextSceneIndex = 1;
	for (; sceneCount < 3; ++sceneCount)
	{
	restartByNewName:
		std::string sceneName = tr("Scene %1").arg(nextSceneIndex).toUtf8().data();

		obs_source_t* existedSource = obs_get_source_by_name(sceneName.c_str());
		if (existedSource != 0)
		{
			obs_source_release(existedSource);
			++nextSceneIndex;
			goto restartByNewName;
		}

		if (firstAddedScene.empty())
		{
			firstAddedScene = sceneName;
		}

		obs_scene_t* scene = obs_scene_create(sceneName.c_str());
		obs_source_t* sceneSource = obs_scene_get_source(scene);
		obs_add_source(sceneSource);
		obs_scene_release(scene);
	}

	obs_source_t* currentOutputSource = obs_get_output_source(0);
	if (!currentOutputSource)
	{
		if (!firstAddedScene.empty())
		{
			currentOutputSource = obs_get_source_by_name(firstAddedScene.c_str());
			if (currentOutputSource)
			{
				obs_set_output_source(0, currentOutputSource);
				obs_source_release(currentOutputSource);
			}
		}
	}
	else
	{
		obs_source_release(currentOutputSource);
	}

	//更新列表控件
	sceneListWidgetOperator->NotifyCurrentSceneChanged();
}

void BiLiOBSMainWid::LoadAudioDeviceConfig()
{
	obs_data_t* data = BiliConfigFile::LoadAudioDeviceConfig();
	BiliAudioDeviceConfig::Set(data);
	if (data) {
		obs_data_release(data);
	}
}

void BiLiOBSMainWid::SaveAudioDeviceConfig()
{
	obs_data_t* data = BiliAudioDeviceConfig::Get();
	if (data)
	{
		BiliConfigFile::SaveAudioDeviceConfig(data);
		obs_data_release(data);
	}
}

void BiLiOBSMainWid::SaveFrontendHotkeys()
{
	obs_data_t* frontendHotkeyData = BiliFrontendHotkeyConfig::SaveFrontendHotkeys();
	BiliConfigFile::SaveFrontendHotkeys(frontendHotkeyData);
	obs_data_release(frontendHotkeyData);
}

void BiLiOBSMainWid::LoadFrontendHotkeys()
{
	obs_data_t* frontendHotkeyData = BiliConfigFile::LoadFrontendHotkeys();
	if (frontendHotkeyData)
	{
		BiliFrontendHotkeyConfig::LoadFrontendHotkeys(frontendHotkeyData);
		obs_data_release(frontendHotkeyData);
	}
}
