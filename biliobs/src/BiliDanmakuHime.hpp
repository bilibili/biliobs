#ifndef BILIDANMAKUHIME_H
#define BILIDANMAKUHIME_H

#include <functional>
#include "../common/BiliJsonHelper.hpp"
#include "third_party/w32-pthreads/pthread.h"


#ifdef WIN32

typedef _W64 unsigned int socket_t;

#else

#pragma error ("what to include?")
typedef int socket_t;

#endif

class BiliDanmakuHime
{
	BiliDanmakuHime(const BiliDanmakuHime&) = delete;
	BiliDanmakuHime& operator = (const BiliDanmakuHime&) = delete;

public:
	typedef std::function<void()> OnDisconnectedProcT;
	typedef std::function<void(const BiliJsonPtr&)> OnJsonProcT;
	typedef std::function<void(int)> OnAudienceCountProcT;
	
	BiliDanmakuHime(int pRoomId, int pUId);
	~BiliDanmakuHime();

	//void SetOnDisconnectedProc(OnDisconnectedProcT&& proc);
	void SetOnJsonProc(OnJsonProcT&& proc);
	void SetOnAudienceCountProc(OnAudienceCountProcT&& proc);

	bool Start();
	void Stop();

private:
	static bool DanmakuThread(void* p);
	static void* DanmakuThreadWrapper(void* p);

	socket_t danmakuSocket;
	int roomId;
	int uId;

	bool isStopping;

	pthread_t networkThread;

	//OnDisconnectedProcT onDisconnected;
	OnJsonProcT onJson;
	OnAudienceCountProcT onAudienceCount;
};

#endif
