#include "BiliOBSUtility.hpp"
#include "BiliGlobalDatas.hpp"

#define CfgPath(v) QString("%1\\%2").arg(QString::fromStdString(gBili_mid)).arg(v).toUtf8().data()
#define BILIOBS_DIR "bilibili\\"
#define SCENE_CONFIGFILE CfgPath("sceneConfig.json")
#define AUDIODEVICE_CONFIGFILE CfgPath("audioDevices.json")
#define FRONTEND_HOTKEY_CONFIGFILE CfgPath("frontendHotkeys.json")
#define LOGIN_CONFIGFILE BILIOBS_DIR "loginInfo.json"

#include "HotkeyManager.h"

#include <sstream>
#include <algorithm>
#include <string.h>
#include <iterator>
#include "../libobs/util/platform.h"

#include <qwidget.h>
#include <qdesktopwidget.h>
#include <QPushButton>
#include <sstream>
#include <iomanip>
#include <regex>
#include <functional>

#include "../src_obs/obs-app-api.hpp"
#include "BiLiApp.h"
#include "../biliapi/IBiliApi.h"
#include "BiLiOBSMainWid.h"
#include "bili_obs_source_helper.h"
#include "HotkeyTriggeredNotice.h"

#include "../libobs/obs-hotkey.h"

#include "knuth_morris_pratt.hpp"
#include <qdesktopwidget.h>

const char* BILI_HOTKEY_MUTE = "biliobs.mute-source";
const char* BILI_HOTKEY_PUSHTOTALK = "biliobs.push-to-talk";

static void* ShowNotice(QString icon, QString text)
{
	(new HotkeyTriggeredNotice(icon, text))->show();
	return 0;
}

class SourceMuteHotkeySinker
{
	static QString tr(const char* s) { return QApplication::translate("SourceMuteHotkeySinker", s, 0); }

public:
	void Triggered(obs_source_t* src, bool pressed)
	{
		if (pressed)
		{
			bool ismuted = obs_source_muted(src);

			const char* sourceId = obs_source_get_id(src);

			BiliThreadWorker::TaskT showNoticeWidget;
			if (strcmp(sourceId, "wasapi_input_capture") == 0)
			{
				if (ismuted)
					showNoticeWidget = std::bind(ShowNotice, QString(":/HotkeyTriggeredNotice/mic-unmute"), tr("microphone unmuted"));
				else
					showNoticeWidget = std::bind(ShowNotice, QString(":/HotkeyTriggeredNotice/mic-mute"), tr("microphone muted"));
			}
			else if (strcmp(sourceId, "wasapi_output_capture") == 0)
			{
				if (ismuted)
					showNoticeWidget = std::bind(ShowNotice, QString(":/HotkeyTriggeredNotice/system-unmute"), tr("system sound unmuted"));
				else
					showNoticeWidget = std::bind(ShowNotice, QString(":/HotkeyTriggeredNotice/system-mute"), tr("system sound muted"));
			}

			assert(static_cast<bool>(showNoticeWidget));

			App()->mGetMainWindow()->mInvokeProcdure(std::move(showNoticeWidget));

			obs_source_set_muted(src, !ismuted);
		}
	}
};

class SourcePressTalkHotkeySinker
{
	static QString tr(const char* s) { return QApplication::translate("SourcePressTalkHotkeySinker", s, 0); }

public:
	void Triggered(obs_source_t* src, bool pressed)
	{
		if (pressed)
		{
			obs_source_set_muted(src, false);
		}
		else
		{
			obs_source_set_muted(src, true);
		}
	}
};

void EnableMuteHotkeySupport()
{
	HotkeyManager::GetInstance()->Register(BILI_HOTKEY_MUTE, CreateSourceHotkeyCallback((SourceMuteHotkeySinker*)0, &SourceMuteHotkeySinker::Triggered));
}

struct FindSourceHotkeyData
{
	obs_hotkey_id id; //output
	obs_source_t* src;
	std::string hotkeyName;
};

static bool find_source_hotkey_proc(void *data, obs_hotkey_id id, obs_hotkey_t *key)
{
	FindSourceHotkeyData* p = static_cast<FindSourceHotkeyData*>(data);
	if (obs_hotkey_get_registerer_type(key) == OBS_HOTKEY_REGISTERER_SOURCE
		&& p->hotkeyName == obs_hotkey_get_name(key))
	{
		obs_weak_source_t* weakSrc = static_cast<obs_weak_source_t*>(obs_hotkey_get_registerer(key));
		obs_source_t* source = obs_weak_source_get_source(weakSrc);
		if (source)
		{
			if (source == p->src)
			{
				p->id = obs_hotkey_get_id(key);
				obs_source_release(source);
				return false;
			}
		}
	}
	return true;
}

struct FindFrontendHotkeyData
{
	obs_hotkey_id id; //output
	std::string hotkeyName;
};

obs_hotkey_id bili_get_source_hotkey_id(obs_source_t* source, const char* hotkeyName)
{
	FindSourceHotkeyData fshd;
	fshd.hotkeyName = hotkeyName;
	fshd.src = source;
	fshd.id = ~(obs_hotkey_id());
	obs_enum_hotkeys(find_source_hotkey_proc, &fshd);
	return fshd.id;
}

static bool find_frontend_hotkey_proc(void* data, obs_hotkey_id id, obs_hotkey_t* key)
{
	FindFrontendHotkeyData* p = static_cast<FindFrontendHotkeyData*>(data);
	if (obs_hotkey_get_registerer_type(key) == OBS_HOTKEY_REGISTERER_FRONTEND)
	{
		if (p->hotkeyName == obs_hotkey_get_name(key))
		{
			p->id = obs_hotkey_get_id(key);
			return false;
		}
	}
	return true;
}

obs_hotkey_id bili_get_frontend_hotkey_id(const char* hotkeyName)
{
	FindFrontendHotkeyData fshd;
	fshd.hotkeyName = hotkeyName;
	fshd.id = ~(obs_hotkey_id());
	obs_enum_hotkeys(find_frontend_hotkey_proc, &fshd);
	return fshd.id;
}

bool BiliConfigFile::SaveSceneData(obs_data_t* data)
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), SCENE_CONFIGFILE);
	return obs_data_save_json_safe(data, &savePathData[0], "tmp", "bak");
}

obs_data_t* BiliConfigFile::LoadSceneData()
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), SCENE_CONFIGFILE);
	return obs_data_create_from_json_file(&savePathData[0]);
}


bool BiliConfigFile::SaveAudioDeviceConfig(obs_data_t* data)
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), AUDIODEVICE_CONFIGFILE);
	return obs_data_save_json_safe(data, &savePathData[0], "tmp", "bak");
}

obs_data_t* BiliConfigFile::LoadAudioDeviceConfig()
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), AUDIODEVICE_CONFIGFILE);
	return obs_data_create_from_json_file(&savePathData[0]);
}

bool BiliConfigFile::SaveFrontendHotkeys(obs_data_t* data)
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), FRONTEND_HOTKEY_CONFIGFILE);
	return obs_data_save_json_safe(data, &savePathData[0], "tmp", "bak");
}

obs_data_t* BiliConfigFile::LoadFrontendHotkeys()
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), FRONTEND_HOTKEY_CONFIGFILE);
	return obs_data_create_from_json_file(&savePathData[0]);
}

std::string BiliConfigFile::GetLoginConfigPath()
{
	std::vector<char> savePathData(512);
	GetUserDataPath(&savePathData[0], savePathData.size(), LOGIN_CONFIGFILE);
	return &savePathData[0];
}





obs_data_t* BiliSceneConfig::Get()
{
	obs_data_t*	      result;
	obs_data_array_t* sourceArray;
	obs_source_t*     currentScene;
	const char*       currentSceneName;

	result = obs_data_create();

	//保存所有来源，包括场景和场景元素
	sourceArray = obs_save_sources();

	//当前场景和当前场景名
	currentScene = obs_get_output_source(0);
	currentSceneName = obs_source_get_name(currentScene);

	obs_data_set_string(result, "currentScene", currentSceneName);
	obs_data_set_array(result, "sources", sourceArray);

	obs_data_array_release(sourceArray);
	obs_source_release(currentScene);

	return result;
}

void BiliSceneConfig::Set(obs_data_t* data)
{
	obs_data_array_t* sourceArray;
	const char*       currentSceneName;

	currentSceneName = obs_data_get_string(data, "currentScene");
	sourceArray = obs_data_get_array(data, "sources");

	obs_load_sources(sourceArray);

	//恢复“当前scene”
	for (OBSSource& src : OBSEnumSources())
	{
		const char* sourceName = obs_source_get_name(src);
		if (strcmp(currentSceneName, sourceName) == 0)
		{
			obs_set_output_source(0, src);
			break;
		}
	}

	obs_data_array_release(sourceArray);
}




static struct {
	const char* deviceName;
	int deviceChannel;
} audioDeviceList[] = 
{
	"DesktopAudioDevice1", 1,
//	"DesktopAudioDevice2", 2,
	"AuxAudioDevice1",     3,
//	"AuxAudioDevice2",     4,
//	"AuxAudioDevice3",     5,
};

bool BiliAudioDeviceConfig::checked_ = false;
bool BiliAudioDeviceConfig::has_desktop_audio_device_;
bool BiliAudioDeviceConfig::has_input_audio_device_;

obs_data_t* BiliAudioDeviceConfig::Get()
{
	obs_data_t* result;

	result = obs_data_create();
	if (result)
	{
		int deviceCount = sizeof(audioDeviceList) / sizeof(*audioDeviceList);
		for (int i = 0; i < deviceCount; ++i)
		{
			obs_source_t* source = obs_get_output_source(audioDeviceList[i].deviceChannel);
			if (source)
			{
				obs_data_t* savedData = obs_save_source(source);
				obs_data_set_obj(result, audioDeviceList[i].deviceName, savedData);
				obs_source_release(source);
				obs_data_release(savedData);
			}
		}
	}

	return result;
}

void BiliAudioDeviceConfig::Set(obs_data_t* data)
{
	if (!data) {
		mCreateFirstRunSources();
	}

	bool hasInputAudio = hasAudioInputDevice();
	bool hasDesktopAudio = hasDesktopAudioDevice();

	bool has_input_audio_data;
	bool has_desktop_audio_data;

	bool isNewSource = false;

	int deviceCount = sizeof(audioDeviceList) / sizeof(*audioDeviceList);
	for (int i = 0; i < deviceCount; ++i)
	{
		obs_source_t* outputSrc = obs_get_output_source(audioDeviceList[i].deviceChannel);
		if (outputSrc)
			isNewSource = false;
		else
			isNewSource = true;

		obs_data_t* savedData = obs_data_get_obj(data, audioDeviceList[i].deviceName);
		if (savedData)
		{
			if (1 == audioDeviceList[i].deviceChannel) {
				/*desktop audio data*/
				if (!hasDesktopAudio)
					goto SKIP;
			} else if (3 == audioDeviceList[i].deviceChannel) {
				/*input_audio data*/
				if (!hasInputAudio)
					goto SKIP;
			}

			if (isNewSource) //不存在，创建
			{
				obs_source_t* savedSource = obs_load_source(savedData);
				if (savedSource)
				{
					obs_set_output_source(audioDeviceList[i].deviceChannel, savedSource);
					obs_source_release(savedSource);
				}
			}
			else //存在，只更新
			{
				obs_data_t* settings = obs_data_get_obj(savedData, "settings");
				if (settings)
				{
					obs_source_update(outputSrc, settings);
					obs_data_release(settings);
				}
			}
SKIP:
			obs_data_release(savedData);
		} else {
			if (1 == audioDeviceList[i].deviceChannel) {
				if (hasDesktopAudio)
				{
					initDesktopAudioDevice();
				}
			} else if (3 == audioDeviceList[i].deviceChannel) {
				if (hasInputAudio)
				{
					initAudioInputDevice();
				}
			}
		}

		obs_source_release(outputSrc);

		//添加快捷键支持
		if (isNewSource)
		{
			outputSrc = obs_get_output_source(audioDeviceList[i].deviceChannel);
			if (outputSrc)
			{
				HotkeyManager::GetInstance()->RegisterSource(outputSrc, BILI_HOTKEY_MUTE);
			}
		}
	}
}

void BiliAudioDeviceConfig::mResetAudioDevice(const char *sourceId, const char *deviceId,
	const char *deviceDesc, int channel) {

	obs_source_t *source;
	obs_data_t *settings;
	bool same = false;

	source = obs_get_output_source(channel);
	if (source) {
		settings = obs_source_get_settings(source);
		const char *curId = obs_data_get_string(settings, "device_id");

		same = (strcmp(curId, deviceId) == 0);

		obs_data_release(settings);
		obs_source_release(source);
	}

	if (!same)
		obs_set_output_source(channel, nullptr);

	if (!same && strcmp(deviceId, "disabled") != 0) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_string(settings, "device_id", deviceId);
		source = obs_source_create(OBS_SOURCE_TYPE_INPUT,
			sourceId, deviceDesc, settings, nullptr);
		obs_data_release(settings);

		obs_set_output_source(channel, source);
		obs_source_release(source);
	}
}

void BiliAudioDeviceConfig::mCreateFirstRunSources()
{
	if (hasDesktopAudioDevice())
		initDesktopAudioDevice();

	if (hasAudioInputDevice())
		initAudioInputDevice();

}

void BiliAudioDeviceConfig::checkHasAudioDevicesFirst()
{
	assert(!checked_);
	checked_ = true;

	has_input_audio_device_ = HasAudioDevices(App()->mInputAudioSource());
	has_desktop_audio_device_ = HasAudioDevices(App()->mOutputAudioSource());
}

bool BiliAudioDeviceConfig::HasAudioDevices(const char *source_id) {
	const char *output_id = source_id;
	obs_properties_t *props = obs_get_source_properties(
		OBS_SOURCE_TYPE_INPUT, output_id);
	size_t count = 0;

	if (!props)
		return false;

	obs_property_t *devices = obs_properties_get(props, "device_id");
	if (devices)
		count = obs_property_list_item_count(devices);

	obs_properties_destroy(props);

	return count != 0;
}

void BiliAudioDeviceConfig::initAudioInputDevice()
{
	
	mResetAudioDevice(App()->mInputAudioSource(), "default",
		Str("AuxDevice1"), 3);
}

void BiliAudioDeviceConfig::initDesktopAudioDevice()
{
	mResetAudioDevice(App()->mOutputAudioSource(), "default",
		Str("DesktopDevice1"), 1);
}


void SetPushButtonBackgroundColor(QPushButton* btn, uint32_t color)
{
	std::stringstream qssBuf;
	uint8_t* tmp = reinterpret_cast<uint8_t*>(&color); //r g b a
	qssBuf << "QPushButton { background-color: #"
		<< std::setfill('0') << std::setw(2) << std::hex << (uint16_t)tmp[3]
		<< std::setfill('0') << std::setw(2) << std::hex << (uint16_t)tmp[0]
		<< std::setfill('0') << std::setw(2) << std::hex << (uint16_t)tmp[1]
		<< std::setfill('0') << std::setw(2) << std::hex << (uint16_t)tmp[2];
	std::string qssString = qssBuf.str() + "; }";
	btn->setStyleSheet(qssString.c_str());
}


bool CheckUseCustomPushStream(config_t* config, std::string* pPushStreamServer, std::string* pPushStreamPath)
{
	const char* pushServerType = config_get_string(config, "AdvOut", "PushServerType");
	if (!pushServerType)
		return false;
	if (strcmp(pushServerType, "pushToCustomRadio") == 0)
	{
		const char* pushStreamServer = config_get_string(config, "AdvOut", "PushStreamServer");
		const char* pushStreamPath = config_get_string(config, "AdvOut", "PushStreamPath");

		if (!pushStreamServer) pushStreamServer = "";
		if (!pushStreamPath) pushStreamPath = "";

		if (pPushStreamServer)
			*pPushStreamServer = pushStreamServer;
		if (pPushStreamPath)
			*pPushStreamPath = pushStreamPath;

		return true;
	}
	else
		return false;
}

#ifdef WIN32
#include <windows.h>
#include <Shlwapi.h>
#include <shlobj.h>
#include <string>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")

bool RestartWithoutAutoLogin()
{
	std::wstring buf;
	buf.resize(MAX_PATH);

	int len = GetModuleFileNameW(0, &buf[0], MAX_PATH);
	buf.resize(len);

	if ((size_t)ShellExecuteW(NULL, NULL, buf.c_str(), L"--disable-auto-login", NULL, SW_SHOWNORMAL) > 32)
		return true;
	else
		return false;
}

void SaveHttpLogToFile(const std::string& url, const std::vector<char>& content)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	std::vector<char> u8FileName(MAX_PATH * 2);
	std::vector<wchar_t> fileName(MAX_PATH);
	std::vector<wchar_t> fileDir(MAX_PATH);
	std::vector<wchar_t> filePath(MAX_PATH);

	GetUserDataPath(&u8FileName[0], u8FileName.size(), "");

	os_utf8_to_wcs(&u8FileName[0], strlen(&u8FileName[0]), &fileDir[0], fileDir.size());

	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintfW(&fileName[0], L"bilibili\\log\\%04d%02d%02d-%02d%02d%02d%03d.txt",
		static_cast<int>(st.wYear), static_cast<int>(st.wMonth), static_cast<int>(st.wDay),
		static_cast<int>(st.wHour), static_cast<int>(st.wMinute), static_cast<int>(st.wSecond), static_cast<int>(st.wMilliseconds));
	PathCombineW(&filePath[0], &fileDir[0], &fileName[0]);

	hFile = CreateFileW(&filePath[0], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD written;
		WriteFile(hFile, url.c_str(), url.size(), &written, NULL);
		WriteFile(hFile, "\r\n\r\n", 4, &written, NULL);
		if (content.empty() == false)
			WriteFile(hFile, &content[0], content.size(), &written, NULL);
		CloseHandle(hFile);
	}
}

#else
#error No implementment for this platform currently.
#endif

void move_widget_to_center(QWidget *child, QWidget *parent)
{
	assert(child);

	bool is_window = child->isWindow();
	QWidget *parent_widget = child->parentWidget();

	int child_w = child->width();
	int child_h = child->height();

	QPoint global_pos;

	if (parent) {
		QPoint parent_pos = parent->pos();
		int parent_w = parent->width();
		int parent_h = parent->height();

		if (!parent->isWindow()) {
			QWidget *wgt = parent->parentWidget();
			if (wgt)
				parent_pos = wgt->mapToGlobal(parent_pos);
		}

		global_pos = parent_pos + QPoint((parent_w - child_w) / 2, (parent_h - child_h) / 2);
	}
	else {
		QDesktopWidget *desktop = QApplication::desktop();
		int screenNum = desktop->screenCount();

		QRect screen_rect = desktop->screenGeometry();

		global_pos = { (screen_rect.width() - child->width()) / 2, (screen_rect.height() - child->height()) / 2} ;

		
		
	}


	if (!is_window && parent_widget)
		child->move(parent_widget->mapFromGlobal(global_pos));
	else
		child->move(global_pos-QPoint(-10, 20));
}

int get_pos_desktop_index(const QPoint &ps)
{
    int x = ps.x();
    QDesktopWidget *desktop = QApplication::desktop();
    int dsk_num = desktop->screenCount();

    for (int i = 0; i < dsk_num; i++) {
        if (x < desktop->screenGeometry(i).right())
            return i;
    }

    return dsk_num - 1;
}

void GetQSSWidthAndHeight(const char* qss, const char* selector, int& width, bool& isWidthGot, int& height, bool& isHeightGot)
{
	typedef knuth_morris_pratt<const char*> kmp_t;

	isWidthGot = false;
	isHeightGot = false;

	int selectorLen = strlen(selector);
	int qssLen = strlen(qss);
	kmp_t kmp(selector, selector + selectorLen);
	const char* found = kmp(qss, qss + qssLen);
	if (found - qss < qssLen)
	{
		const char* rangeBegin = found;
		const char* rangeEnd = strchr(found, '}');
		if (rangeEnd != 0)
		{
			std::regex whRegex("[ \\t]*" "(width|height)" "[ \\t]*:[ \\t]*" "(\\d+)" "[ \\t]*px[ \\t]*;" "[ \\t]*");
			std::cmatch whMatch;

			while (std::regex_search(rangeBegin, rangeEnd, whMatch, whRegex))
			{
				std::string keyName = whMatch[1].str();
				if (keyName == "width")
				{
					width = lexical_cast<int>(whMatch[2].str());
					isWidthGot = true;
				}
				else if (keyName == "height")
				{
					height = lexical_cast<int>(whMatch[2].str());
					isHeightGot = true;
				}
				rangeBegin = whMatch.suffix().first;
			}
		}
	}
	return;
}


std::wstring BiliGetFileVertion(std::wstring fullName) {

	DWORD dwLen = 0;
	char* lpData = NULL;

	BOOL bSuccess = FALSE;
	QString fileInfomation;

	dwLen = GetFileVersionInfoSizeW(fullName.c_str(), 0);
	if (0 == dwLen)
		return L"";
	lpData = new char[dwLen + 1];
	ZeroMemory(lpData, dwLen + 1);

	bSuccess = GetFileVersionInfoW(fullName.c_str(), 0, dwLen, lpData);
	if (!bSuccess) {
		delete lpData;
		return L"";
	}

	LPWSTR lpBuffer = NULL;
	UINT uLen = 0;

	//language and code page
	bSuccess = VerQueryValueW(lpData,
		L"\\VarFileInfo\\Translation",
		(LPVOID*)&lpBuffer,
		&uLen);
	QString strTranslation, str1, str2;
	str1.setNum(*lpBuffer, 16);
	str1 = "000" + str1;
	strTranslation += str1.mid(str1.size() - 4, 4);
	str2.setNum(*(++lpBuffer), 16);
	str2 = "000" + str2;
	strTranslation += str2.mid(str2.size() - 4, 4);

	//version info
	QString code = "\\StringFileInfo\\" + strTranslation + "\\FileVersion";
	bSuccess = VerQueryValueW(lpData,
		code.toStdWString().c_str(),
		(LPVOID*)&lpBuffer,
		&uLen);
	if (!bSuccess) {
		delete lpData;
		return L"";
	}

	fileInfomation += QString::fromUtf16((ushort*)lpBuffer);
	delete[] lpData;
	return fileInfomation.toStdWString();
}


extern const char* BILI_HOTKEY_SWITCH_SCENE_NAME;
extern const char* BILI_HOTKEY_MUTE;
extern const char* BILI_HOTKEY_BROADCAST_NAME;
extern const char* BILI_HOTKEY_RECORD_NAME;

obs_data_t* BiliFrontendHotkeyConfig::SaveFrontendHotkeys()
{
	obs_data_t* obj = obs_data_create();

	obs_data_array_t* r = obs_data_array_create();
	const char* hotkeyNames[] = 
	{
		BILI_HOTKEY_SWITCH_SCENE_NAME,
		BILI_HOTKEY_MUTE,
		BILI_HOTKEY_BROADCAST_NAME,
		BILI_HOTKEY_RECORD_NAME
	};

	for (int i = 0; i < sizeof(hotkeyNames) / sizeof(*hotkeyNames); ++i)
	{
		obs_hotkey_id hotkeyId = bili_get_frontend_hotkey_id(hotkeyNames[i]);
		if (hotkeyId != ~obs_hotkey_id())
		{
			obs_data_array_t* hotkeyBindingData = obs_hotkey_save(hotkeyId);
			obs_data_t* currentObj = obs_data_create();
			obs_data_set_string(currentObj, "hotkey-name", hotkeyNames[i]);
			obs_data_set_array(currentObj, "hotkey-data", hotkeyBindingData);
			obs_data_array_push_back(r, currentObj);
			obs_data_release(currentObj);
			obs_data_array_release(hotkeyBindingData);
		}
	}

	obs_data_set_array(obj, "hotkeys", r);
	obs_data_array_release(r);

	return obj;
}

bool BiliFrontendHotkeyConfig::LoadFrontendHotkeys(obs_data_t* frontendHotkeyData)
{
	obs_data_array_t* data_array = obs_data_get_array(frontendHotkeyData, "hotkeys");
	int count = obs_data_array_count(data_array);
	for (int i = 0; i < count; ++i)
	{
		obs_data_t* currentObj = obs_data_array_item(data_array, i);
		if (currentObj != 0)
		{
			const char* hotkeyName = obs_data_get_string(currentObj, "hotkey-name");
			obs_data_array_t* hotkeyBindingData = obs_data_get_array(currentObj, "hotkey-data");

			obs_hotkey_id hotkeyId = HotkeyManager::GetInstance()->RegisterFrontend(hotkeyName);

			obs_hotkey_load(hotkeyId, hotkeyBindingData);

			obs_data_array_release(hotkeyBindingData);
			obs_data_release(currentObj);
		}
	}
	obs_data_array_release(data_array);

	return true;
}
