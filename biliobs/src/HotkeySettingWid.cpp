#include "HotkeySettingWid.h"
#include "HotkeyItemWid.h"
#include "bili_obs_source_helper.h"
#include "BiliUIConfigSync.hpp"
#include "BiliOBSUtility.hpp"
#include "BiLiMsgDlg.h"
#include "HotkeyManager.h"
#include <algorithm>
#include <cstdint>
#include <utility>

#include <Windows.h>

#include "oper_tip_dlg_factory.h"
#include "oper_tip_dlg.h"

#include "system_ret_info_dlg.h"


#define PROPNAME_SOURCE "source"
#define PROPNAME_SRCTYPE "source_type"
#define PROPNAME_SRCNAME "source_name"
#define PROPNAME_HOTKEYNAME "hotkey_name"

static void hotkey_null_sinker(void *data, obs_hotkey_id id, bool pressed)
{
}

extern const char* BILI_HOTKEY_SWITCH_SCENE_NAME;
extern const char* BILI_HOTKEY_MUTE;

extern const char* BILI_HOTKEY_BROADCAST_NAME;
extern const char* BILI_HOTKEY_RECORD_NAME;

enum { SRCTYPE_MICROPHONE = 1, SRCTYPE_SYSTEMAUDIO, SRCTYPE_SCENE, SRCTYPE_FRONTEND };

static bool operator == (const obs_key_combination_t& lhs, const obs_key_combination_t& rhs)
{
	return memcmp(&lhs, &rhs, sizeof(obs_key_combination_t)) == 0;
}

static bool SceneComaprer(const OBSSource& lhs, const OBSSource& rhs)
{
	const char* lhsName = obs_source_get_name(lhs);
	const char* rhsName = obs_source_get_name(rhs);
	return strcmp(lhsName, rhsName) < 0;
}

struct IHotkeyPair
{
	IHotkeyPair()
		: hotkeyName("")
		, hotkey(obs_key_combination_t())
		, hotkeyId(obs_hotkey_id())
		, isChanged(false)
		, userData(0)
		, isLast(false)
	{
	}

	~IHotkeyPair() {}

	std::string hotkeyName; //都要，用于匹配快捷键名
	obs_key_combination_t hotkey; //load的时候是输出，save的时候是输入
	obs_hotkey_id hotkeyId; //load的时候是输出，save的时候没用到
	bool isChanged; //表示在save或者load里有没有操作过

	bool isLast;

	void* userData;

	virtual void* PrepareData(obs_hotkey_binding_t* binding) = 0;
	virtual void ReleaesData(void* data) = 0;
	virtual bool Match(void* data, obs_hotkey_binding_t* binding) = 0;
	virtual bool Load(void* data, obs_hotkey_binding_t* binding) = 0;
	virtual bool Save(void* data, obs_hotkey_binding_t* binding) = 0;
	virtual bool RegisterNew() = 0;
};

struct FrontendHotkeyPair : public IHotkeyPair
{
	FrontendHotkeyPair()
	{
	}

	void* PrepareData(obs_hotkey_binding_t* binding) override
	{
		obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);
		if (obs_hotkey_get_registerer_type(hotkeyObject) == OBS_HOTKEY_REGISTERER_FRONTEND)
		{
			return (void*)1;
		}
		else
			return 0;
	}

	void ReleaesData(void* data) override
	{
	}

	bool Match(void* data, obs_hotkey_binding_t* binding) override
	{
		if (isChanged == false)
		{
			obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);

			if (obs_hotkey_get_registerer_type(hotkeyObject) == OBS_HOTKEY_REGISTERER_FRONTEND)
			{
				if (hotkeyName == obs_hotkey_get_name(hotkeyObject))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool Load(void* data, obs_hotkey_binding_t* binding) override
	{
		if (Match(data, binding))
		{
			obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);
			hotkey = obs_hotkey_binding_get_key_combination(binding);
			hotkeyId = obs_hotkey_get_id(hotkeyObject);
			isChanged = true;
			return true;
		}
		else
			return false;
	}

	bool Save(void* data, obs_hotkey_binding_t* binding) override
	{
		if (Match(data, binding))
		{
			obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);
			obs_hotkey_id hotkeyId = obs_hotkey_get_id(hotkeyObject);
			obs_hotkey_load_bindings(hotkeyId, &hotkey, 1);
			isChanged = true;
			return true;
		}
		else
			return false;
	}

	bool RegisterNew() override
	{
		if (isChanged == false)
		{
			isChanged = true;
			HotkeyManager::GetInstance()->UnregisterFrontend(hotkeyName.c_str());

			if (hotkey.key != OBS_KEY_NONE)
			{
				obs_hotkey_id hotkeyId = HotkeyManager::GetInstance()->RegisterFrontend(hotkeyName.c_str());
				obs_hotkey_load_bindings(hotkeyId, &hotkey, 1);
			}
			return true;
		}
		else
			return false;
	}
};

struct FrontendHotkeyPairEnd : public FrontendHotkeyPair
{
	FrontendHotkeyPairEnd()
	{
		assert(sizeof(FrontendHotkeyPairEnd) == sizeof(FrontendHotkeyPair));
		isLast = true;
	}
};

struct SourceHotkeyPair : public FrontendHotkeyPair
{
	SourceHotkeyPair()
		: source(nullptr)
	{
	}

	obs_source_t* source; //都要

	void* PrepareData(obs_hotkey_binding_t* binding) override
	{
		obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);
		if (obs_hotkey_get_registerer_type(hotkeyObject) == OBS_HOTKEY_REGISTERER_SOURCE)
		{
			obs_weak_source_t* weakSrc = static_cast<obs_weak_source_t*>(obs_hotkey_get_registerer(hotkeyObject));
			obs_source_t* src = obs_weak_source_get_source(weakSrc);
			return src;
		}
		else
			return 0;
	}

	void ReleaesData(void* data) override
	{
		if (data != 0)
			obs_source_release(static_cast<obs_source_t*>(data));
	}

	bool Match(void* data, obs_hotkey_binding_t* binding) override
	{
		if (isChanged == false)
		{
			obs_hotkey_t* hotkeyObject = obs_hotkey_binding_get_hotkey(binding);

			if (obs_hotkey_get_registerer_type(hotkeyObject) == OBS_HOTKEY_REGISTERER_SOURCE)
			{
				if (hotkeyName == obs_hotkey_get_name(hotkeyObject))
				{
					obs_source_t* pSource = static_cast<obs_source_t*>(data);
					if (pSource == source)
						return true;
				}
			}
		}

		return false;
	}

	bool RegisterNew() override
	{
		if (isChanged == false)
		{
			isChanged = true;
			//有可能是要删掉的……会进这里
			HotkeyManager::GetInstance()->UnregisterSource(source, hotkeyName.c_str());

			if (hotkey.key != OBS_KEY_NONE)
			{
				obs_hotkey_id hotkeyId = HotkeyManager::GetInstance()->RegisterSource(source, hotkeyName.c_str());
				obs_hotkey_load_bindings(hotkeyId, &hotkey, 1);
			}
			return true;
		}
		else
			return false;
	}
};

struct SourceHotkeyPairEnd : public SourceHotkeyPair
{
	SourceHotkeyPairEnd()
	{
		isLast = true;
	}
};

static bool LoadHotkeyCallback(void *data, size_t idx, obs_hotkey_binding_t* binding)
{
	IHotkeyPair** pairs = static_cast<IHotkeyPair**>(data);

	void* userdata = (*pairs)->PrepareData(binding);
	if (userdata)
	{
		IHotkeyPair** x = pairs;
		while (!(*x)->isLast)
		{
			if ((*x)->Load(userdata, binding))
			{
				break;
			}
			++x;
		}
		(*pairs)->ReleaesData(userdata);
	}
	return true;
}

static bool SaveHotkeyCallback(void *data, size_t idx, obs_hotkey_binding_t* binding)
{
	IHotkeyPair** pairs = static_cast<IHotkeyPair**>(data);

	void* userdata = (*pairs)->PrepareData(binding);
	if (userdata)
	{
		IHotkeyPair** x = pairs;
		while (!(*x)->isLast)
		{
			if ((*x)->Save(userdata, binding))
			{
				break;
			}
			++x;
		}
		(*pairs)->ReleaesData(userdata);
	}
	return true;
}

HotkeySettingWid::HotkeySettingWid(ConfigFile& config, QWidget *parent)
	: QWidget(parent)
	, mBasicConfig(config)
{
	ui.setupUi(this);

	obs_hotkey_set_callback_routing_func(hotkey_null_sinker, 0);
	obs_hotkey_enable_callback_rerouting(true);

    bool audio_not_null;
    bool scene_not_null = false;

	obs_source_t* microphoneSource = obs_get_output_source(3);
	if (microphoneSource)
	{
		HotkeyItemWid* microphone = new HotkeyItemWid(this);
		microphone->SetHotkeyName(tr("Mute/Unmute microphone"));
		microphone->setProperty(PROPNAME_SRCTYPE, SRCTYPE_MICROPHONE);
		microphone->setProperty(PROPNAME_SOURCE, qVPtr<obs_source_t>::toVariant(microphoneSource));
		microphone->setProperty(PROPNAME_HOTKEYNAME, BILI_HOTKEY_MUTE);
		ui.audioHotkeyListLayout->addWidget(microphone);
		obs_source_release(microphoneSource);

        audio_not_null = true;
    } else {
        audio_not_null = false;
    }

	obs_source_t* systemAudioSource = obs_get_output_source(1);
	if (systemAudioSource)
	{
        if (audio_not_null) {
            QSpacerItem *spacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
            ui.audioHotkeyListLayout->addItem(spacer);
        }

		HotkeyItemWid* systemAudio = new HotkeyItemWid(this);
		systemAudio->SetHotkeyName(tr("Mute/Unmute system audio"));
		systemAudio->setProperty(PROPNAME_SRCTYPE, SRCTYPE_SYSTEMAUDIO);
		systemAudio->setProperty(PROPNAME_SOURCE, qVPtr<obs_source_t>::toVariant(systemAudioSource));
		systemAudio->setProperty(PROPNAME_HOTKEYNAME, BILI_HOTKEY_MUTE);
		ui.audioHotkeyListLayout->addWidget(systemAudio);
		obs_source_release(systemAudioSource);
	}

	//场景切换部分
	//列出所有场景源并且按照名字排序
	std::vector<OBSSource> scenes;
	for (OBSSource source : OBSEnumSources())
	{
		if (strcmp(obs_source_get_id(source), "scene") == 0)
			scenes.push_back(source);
	}
	std::sort(scenes.begin(), scenes.end(), SceneComaprer);

	//添加控件列表
	for (OBSSource source : scenes)
	{
        if (scene_not_null) {
            QSpacerItem *spacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
            ui.sceneHotkeyListLayout->addItem(spacer);
        }
		HotkeyItemWid* item = new HotkeyItemWid(this);
		item->SetHotkeyName(tr("Switch to ") + obs_source_get_name(source));
		item->setProperty(PROPNAME_SRCTYPE, SRCTYPE_SCENE);
		item->setProperty(PROPNAME_SRCNAME, obs_source_get_name(source));
		item->setProperty(PROPNAME_SOURCE, qVPtr<obs_source_t>::toVariant(source));
		item->setProperty(PROPNAME_HOTKEYNAME, BILI_HOTKEY_SWITCH_SCENE_NAME);
		ui.sceneHotkeyListLayout->addWidget(item);

        scene_not_null = true;
	}

	//开关直播和录像
	struct
	{
		const char* hotkeyName;
		QString displayName;
	}
	broadcast_record_hotkey_names[] =
	{
		BILI_HOTKEY_BROADCAST_NAME, tr("Broadcast Start/Stop"),
		BILI_HOTKEY_RECORD_NAME, tr("Record Start/Stop"),
	};

	for (int i = 0; i < sizeof(broadcast_record_hotkey_names) / sizeof(*broadcast_record_hotkey_names); ++i)
	{
		if (i != 0)
		{
			QSpacerItem *spacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);
			ui.broadcastHotkeyListLayout->addItem(spacer);
		}

		HotkeyItemWid* item = new HotkeyItemWid(this);
		item->setProperty(PROPNAME_SRCTYPE, SRCTYPE_FRONTEND);
		item->setProperty(PROPNAME_HOTKEYNAME, broadcast_record_hotkey_names[i].hotkeyName);
		item->SetHotkeyName(broadcast_record_hotkey_names[i].displayName);
		ui.broadcastHotkeyListLayout->addWidget(item);
	}

	//读取各种配置
	// index代表当前读取的类型，不同类型的快捷键分别读取
	int index = 1;
	bool exitFlag = false;

	while (exitFlag == false)
	{
		std::vector<IHotkeyPair*> hotkeyPairs;

		switch (index)
		{
		case 1:
			for (HotkeyItemWid* hotkeyItem : GetHotkeyItems({ SRCTYPE_SCENE, SRCTYPE_MICROPHONE, SRCTYPE_SYSTEMAUDIO }))
			{
				SourceHotkeyPair* shp = new SourceHotkeyPair();
				shp->hotkeyName = hotkeyItem->property(PROPNAME_HOTKEYNAME).toString().toStdString();
				shp->source = qVPtr<obs_source_t>::toPtr(hotkeyItem->property(PROPNAME_SOURCE));
				shp->userData = hotkeyItem;

				hotkeyPairs.push_back(shp);
			}
			hotkeyPairs.push_back(new SourceHotkeyPair());

			break;

		case 2:
			//前端快捷键配置（开始和停止直播或录制）
			for (HotkeyItemWid* hotkeyItem : GetHotkeyItems({ SRCTYPE_FRONTEND }))
			{
				FrontendHotkeyPair* fhp = new FrontendHotkeyPair();
				fhp->hotkeyName = hotkeyItem->property(PROPNAME_HOTKEYNAME).toString().toStdString();
				fhp->userData = hotkeyItem;

				hotkeyPairs.push_back(fhp);
			}
			hotkeyPairs.push_back(new FrontendHotkeyPairEnd());

			break;

		case 3:
			exitFlag = true;
			break;

		}

		if (hotkeyPairs.size() > 0)
		{
			//读取配置
			obs_enum_hotkey_bindings(&LoadHotkeyCallback, &hotkeyPairs[0]);

			//显示在界面
			for (IHotkeyPair* hp : hotkeyPairs)
			{
				if (hp->userData != 0)
				{
					HotkeyItemWid* item = static_cast<HotkeyItemWid*>(hp->userData);
					item->SetHotkey(hp->hotkey);
				}
			}
		}
		//释放资源
		for (IHotkeyPair* hp : hotkeyPairs)
		{
			delete hp;
		}

		++index;
	}
}

HotkeySettingWid::~HotkeySettingWid()
{
	obs_hotkey_enable_callback_rerouting(false);
}

std::vector<HotkeyItemWid*> HotkeySettingWid::GetHotkeyItems()
{
	std::vector<HotkeyItemWid*> r;

	for (QObject* child : children())
	{
		HotkeyItemWid* hotkeyItem = qobject_cast<HotkeyItemWid*>(child);
		if (hotkeyItem)
		{
			r.push_back(hotkeyItem);
		}
	}

	return std::move(r);
}

std::vector<HotkeyItemWid*> HotkeySettingWid::GetHotkeyItems(std::initializer_list<int> types)
{
	std::vector<HotkeyItemWid*> r = GetHotkeyItems();
	
	auto i = r.begin();
	while (i != r.end())
	{
		int targetType = (*i)->property(PROPNAME_SRCTYPE).toInt();
		bool inList = false;
		for (int type : types)
		{
			if (type == targetType)
			{
				inList = true;
				break;
			}
		}

		if (inList == false)
		{
			i = r.erase(i);
		}
		else
			++i;
	}

	return std::move(r);
}

bool HotkeySettingWid::SaveConfig()
{
	//检查内部有没有重复
	std::list<obs_key_combination_t> hotkeyCheckList;
	for (HotkeyItemWid* hotkeyItem : GetHotkeyItems())
	{
		obs_key_combination_t hotkey = hotkeyItem->GetHotkey();
		if (hotkey.key != OBS_KEY_NONE)
		{
			if (std::find(hotkeyCheckList.begin(), hotkeyCheckList.end(), hotkey) != hotkeyCheckList.end())
			{
				//处理重复
				//BiLiMsgDlg msgDlg;
				//msgDlg.mSetMsgTxtAndBtn(tr("Duplicated hotkey detected."), false);
				//msgDlg.mSetTitle(tr("Error"));
				//msgDlg.exec();

                OperTipDlg *msgDlg = OperTipDlgFactory::makeDlg(OperTipDlgFactory::HOTKEY_DUPLICATE);
                msgDlg->exec();
                delete msgDlg;
				return false;
			}

			hotkeyCheckList.push_back(hotkey);
		}
	}

	//检查和系统有没有重复的
	static ATOM winHotkeyId = GlobalAddAtom(TEXT("for_hotkey_dup_chk_use"));
	for (obs_key_combination_t& hotkey : hotkeyCheckList)
	{
		DWORD winModifier = 0;
		if (hotkey.modifiers | INTERACT_SHIFT_KEY)
			winModifier |= MOD_SHIFT;
		if (hotkey.modifiers | INTERACT_CONTROL_KEY)
			winModifier |= MOD_CONTROL;
		if (hotkey.modifiers | INTERACT_ALT_KEY)
			winModifier |= MOD_ALT;

		if (RegisterHotKey(NULL, winHotkeyId, winModifier, obs_key_to_virtual_key(hotkey.key)) == 0)
		{
			//处理重复的
			//BiLiMsgDlg msgDlg;
			//msgDlg.mSetMsgTxtAndBtn(tr("Duplicated hotkey with other software."), false);
			//msgDlg.mSetTitle(tr("Error"));
			//msgDlg.exec();

            SystemRetInfoDlg msgDlg;
            msgDlg.setDetailInfo(tr("Duplicated hotkey with other software."));
            msgDlg.setSubTitle(tr("Error"));
            msgDlg.setTitle("");
            msgDlg.exec();

			return false;
		}

		UnregisterHotKey(NULL, winHotkeyId);
	}

	//去掉消息队列中可能出现的快捷键消息
	MSG msg;
	while (PeekMessage(&msg, 0, WM_HOTKEY, WM_HOTKEY, PM_REMOVE) > 0);

	//保存快捷键（更新或添加）
	std::vector<IHotkeyPair*> hotkeys;

	for (HotkeyItemWid* hotkeyItem : GetHotkeyItems({ SRCTYPE_SCENE, SRCTYPE_MICROPHONE, SRCTYPE_SYSTEMAUDIO }))
	{
		obs_key_combination_t hotkey = hotkeyItem->GetHotkey();
		SourceHotkeyPair* x = new SourceHotkeyPair();
		x->source = qVPtr<obs_source_t>::toPtr(hotkeyItem->property(PROPNAME_SOURCE));
		x->hotkey = hotkeyItem->GetHotkey();
		x->hotkeyName = hotkeyItem->property(PROPNAME_HOTKEYNAME).toString().toStdString();
		x->isChanged = false;

		hotkeys.push_back(x);
	}

	for (HotkeyItemWid* hotkeyItem : GetHotkeyItems({ SRCTYPE_FRONTEND }))
	{
		obs_key_combination_t hotkey = hotkeyItem->GetHotkey();
		FrontendHotkeyPair* x = new FrontendHotkeyPair();
		x->hotkey = hotkeyItem->GetHotkey();
		x->hotkeyName = hotkeyItem->property(PROPNAME_HOTKEYNAME).toString().toStdString();
		x->isChanged = false;

		hotkeys.push_back(x);
	}

	hotkeys.push_back(new FrontendHotkeyPairEnd());

	obs_enum_hotkey_bindings(&SaveHotkeyCallback, &hotkeys[0]);

	//如果寻找过程中仍然没法更新的，就尝试（如果有的话就先删掉然后）添加……
	for (IHotkeyPair* x : hotkeys)
	{
		if (x->isChanged == false)
		{
			x->RegisterNew();
		}
	}

	for (IHotkeyPair* x : hotkeys)
		delete x;

	return true;
}
