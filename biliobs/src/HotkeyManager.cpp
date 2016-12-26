#include "HotkeyManager.h"
#include "BiliOBSUtility.hpp"

HotkeyManager* HotkeyManager::instance_ = 0;

void HotkeyManager::InitializeInstance()
{
	if (!instance_)
		instance_ = new HotkeyManager();
}

void HotkeyManager::UninitializeInstance()
{
	if (instance_)
		delete instance_;
	instance_ = 0;
}

HotkeyManager* HotkeyManager::GetInstance()
{
	return instance_;
}

bool HotkeyManager::Register(const char* hotkeyName, PHotkeyCallback&& callback)
{
	callbacks_.erase(hotkeyName);
	auto x = callbacks_.insert(std::make_pair(std::string(hotkeyName), PHotkeyCallback()));
	if (x.second == true)
		x.first->second = std::move(callback);
	return x.second;
}

obs_hotkey_id HotkeyManager::RegisterSource(obs_source_t* src, const char* hotkeyName)
{
	auto x = callbacks_.find(hotkeyName);
	if (x != callbacks_.end())
	{
		obs_hotkey_id hotkeyId = bili_get_source_hotkey_id(src, hotkeyName);
		if (hotkeyId != ~obs_hotkey_id())
			obs_hotkey_unregister(hotkeyId);
		return obs_hotkey_register_source(src, hotkeyName, hotkeyName, &HotkeyManager::RawHotkeyCallback, static_cast<IHotkeyCallback*>(x->second.get()));
	}
	else
		return ~obs_hotkey_id();
}

obs_hotkey_id HotkeyManager::RegisterFrontend(const char* hotkeyName)
{
	auto x = callbacks_.find(hotkeyName);
	if (x != callbacks_.end())
	{
		obs_hotkey_id hotkeyId = bili_get_frontend_hotkey_id(hotkeyName);
		if (hotkeyId != ~obs_hotkey_id())
			obs_hotkey_unregister(hotkeyId);
		return obs_hotkey_register_frontend(hotkeyName, hotkeyName, &HotkeyManager::RawHotkeyCallback, static_cast<IHotkeyCallback*>(x->second.get()));
	}
	else
		return ~obs_hotkey_id();
}

struct MatchHotkeyUserDataHelper
{
	std::string hotkeyName;
	std::vector<obs_hotkey_id> result;
};

bool HotkeyManager::UnregisterSource(obs_source_t* src, const char* hotkeyName)
{
	obs_hotkey_id hotkeyId = bili_get_source_hotkey_id(src, hotkeyName);
	if (hotkeyId == ~obs_hotkey_id())
		return false;
	else
	{
		obs_hotkey_unregister(hotkeyId);
		return true;
	}
}

bool HotkeyManager::UnregisterFrontend(const char* hotkeyName)
{
	obs_hotkey_id hotkeyId = bili_get_frontend_hotkey_id(hotkeyName);
	if (hotkeyId == ~obs_hotkey_id())
		return false;
	else
	{
		obs_hotkey_unregister(hotkeyId);
		return true;
	}
}

static bool MatchHotkeyKeyName(void *data, obs_hotkey_id id, obs_hotkey_t *key)
{
	MatchHotkeyUserDataHelper* p = static_cast<MatchHotkeyUserDataHelper*>(data);
	if (p->hotkeyName == obs_hotkey_get_name(key))
		p->result.push_back(id);
	return true;
}

bool HotkeyManager::Unregister(const char* hotkeyName)
{
	//解除快捷键注册
	MatchHotkeyUserDataHelper param;
	param.hotkeyName = hotkeyName;
	obs_enum_hotkeys(MatchHotkeyKeyName, &param);

	for (obs_hotkey_id& x : param.result)
		obs_hotkey_unregister(x);

	//移除对应表里的
	HotkeyMap::iterator r = callbacks_.find(hotkeyName);
	callbacks_.erase(r);
	return r != callbacks_.end();
}

void HotkeyManager::RawHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed)
{
	IHotkeyCallback* callback = static_cast<IHotkeyCallback*>(data);
	switch (obs_hotkey_get_registerer_type(hotkey))
	{
		case OBS_HOTKEY_REGISTERER_FRONTEND:
			callback->InvokeFrontend(pressed);
			break;

		case OBS_HOTKEY_REGISTERER_SOURCE:
		{
			obs_weak_source_t* weakSrc = static_cast<obs_weak_source_t*>(obs_hotkey_get_registerer(hotkey));
			obs_source_t* src = obs_weak_source_get_source(weakSrc);
			if (src)
			{
				callback->InvokeSource(src, pressed);
				obs_source_release(src);
			}
			break;
		}

		case OBS_HOTKEY_REGISTERER_OUTPUT:
		case OBS_HOTKEY_REGISTERER_SERVICE:
		case OBS_HOTKEY_REGISTERER_ENCODER:
			break;
	}
}
