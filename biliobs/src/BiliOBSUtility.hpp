#ifndef BILIOBSUTILITY_H
#define BILIOBSUTILITY_H

#include <string>
#include <util/config-file.h>
#include <obs.h>
#include <stdint.h>
#include <vector>

class BiliConfigFile
{
public:
	static bool SaveSceneData(obs_data_t* data);
	static obs_data_t* LoadSceneData();

	static bool SaveAudioDeviceConfig(obs_data_t* data);
	static obs_data_t* LoadAudioDeviceConfig();

	static bool SaveFrontendHotkeys(obs_data_t* data);
	static obs_data_t* LoadFrontendHotkeys();

	static std::string GetLoginConfigPath();
};

class BiliSceneConfig
{
public:
	//不包含order！
	static obs_data_t* Get();
	static void Set(obs_data_t* data);
};

class BiliAudioDeviceConfig
{
public:
	static obs_data_t* Get();
	static void Set(obs_data_t* data);
	
	static void mResetAudioDevice(const char *sourceId, const char *deviceId, const char *deviceDesc, int channel);

	static void mCreateFirstRunSources();


public:
	static void checkHasAudioDevicesFirst();
	static bool hasAudioInputDevice() { if (!checked_) checkHasAudioDevicesFirst();  return has_input_audio_device_; }
	static bool hasDesktopAudioDevice() { if (!checked_) checkHasAudioDevicesFirst(); return has_desktop_audio_device_; }
private:
	static bool checked_;
	static bool has_desktop_audio_device_;
	static bool has_input_audio_device_;
	
private:
	static bool HasAudioDevices(const char *source_id);

	static void initAudioInputDevice();
	static void initDesktopAudioDevice();
};

class BiliFrontendHotkeyConfig
{
public:
	static obs_data_t* SaveFrontendHotkeys();
	static bool LoadFrontendHotkeys(obs_data_t* frontendHotkeyData);
};

bool RestartWithoutAutoLogin();

class QWidget;
void move_widget_to_center(QWidget *child, QWidget *parent);

class QPoint;
int get_pos_desktop_index(const QPoint &ps);

class QPushButton;

void SetPushButtonBackgroundColor(QPushButton* btn, uint32_t color);

void SaveHttpLogToFile(const std::string& url, const std::vector<char>& content);

bool CheckUseCustomPushStream(config_t* config, std::string* pPushStreamServer = 0, std::string* pPushStreamPath = 0);

inline void SaveHttpLogToFile(const std::string& url, const std::string& errMsg)
{
	SaveHttpLogToFile(url, std::vector<char>(errMsg.begin(), errMsg.end()));
}

std::wstring BiliGetFileVertion(std::wstring fullName);

void GetQSSWidthAndHeight(const char* qss, const char* widgetName, int& width, bool& isWidthGot, int& height, bool& isHeightGot);

obs_hotkey_id bili_get_source_hotkey_id(obs_source_t* source, const char* hotkeyName);
obs_hotkey_id bili_get_frontend_hotkey_id(const char* hotkeyName);
void EnableMuteHotkeySupport();

#endif
