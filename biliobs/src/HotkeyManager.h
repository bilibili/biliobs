#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H

#include <memory>
#include <unordered_map>
#include <string>
#include "../libobs/obs.h"

class HotkeyNotImplementException
{
};

class IHotkeyCallback
{
public:
    ~IHotkeyCallback() {};
    virtual void InvokeSource(obs_source_t* src, bool pressed) { throw HotkeyNotImplementException(); }
	virtual void InvokeOutput(obs_output_t* output, bool pressed) { throw HotkeyNotImplementException(); }
	virtual void InvokeService(obs_service_t* service, bool pressed) { throw HotkeyNotImplementException(); }
	virtual void InvokeEncoder(obs_encoder_t* encoder, bool pressed) { throw HotkeyNotImplementException(); }
	virtual void InvokeFrontend(bool pressed) { throw HotkeyNotImplementException(); } //for frontend
};

typedef std::unique_ptr<IHotkeyCallback> PHotkeyCallback;


template<class CallbackObject, class CallbackMethod>
class SourceHotkeyCallback : public IHotkeyCallback
{
    CallbackObject object_;
    CallbackMethod method_;
    
public:
    SourceHotkeyCallback(CallbackObject object, CallbackMethod method)
        : object_(object)
        , method_(method)
    {
    }

	void InvokeSource(obs_source_t* src, bool pressed) override
    {
		(object_->*method_)(src, pressed);
    }
};

template<class CallbackObject, class CallbackMethod>
inline PHotkeyCallback CreateSourceHotkeyCallback(CallbackObject object, CallbackMethod method)
{
	return PHotkeyCallback(new SourceHotkeyCallback<CallbackObject, CallbackMethod>(object, method));
}


template<class CallbackObject, class CallbackMethod>
class FrontendHotkeyCallback : public IHotkeyCallback
{
	CallbackObject object_;
	CallbackMethod method_;
	
public:
	FrontendHotkeyCallback(CallbackObject object, CallbackMethod method)
		: object_(object)
		, method_(method)
	{
	}

	void InvokeFrontend(bool pressed) override
	{
		(object_->*method_)(pressed);
	}
};

template<class CallbackObject, class CallbackMethod>
inline PHotkeyCallback CreateFrontendHotkeyCallback(CallbackObject object, CallbackMethod method)
{
	return PHotkeyCallback(new FrontendHotkeyCallback<CallbackObject, CallbackMethod>(object, method));
}


class HotkeyManager
{
    static HotkeyManager* instance_;
	typedef std::unordered_map<std::string, PHotkeyCallback> HotkeyMap;
	HotkeyMap callbacks_;
    
    static void RawHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);
    
public:
    static void InitializeInstance();
    static void UninitializeInstance();
    static HotkeyManager* GetInstance();
    
    bool Register(const char* hotkeyName, PHotkeyCallback&& callback);
	bool Unregister(const char* hotkeyName);

	obs_hotkey_id RegisterSource(obs_source_t* src, const char* hotkeyName);
	bool UnregisterSource(obs_source_t* src, const char* hotkeyName);

	bool UnregisterFrontend(const char* hotkeyName);
	obs_hotkey_id RegisterFrontend(const char* hotkeyName);
};

#endif // HOTKEYMANAGER_H
