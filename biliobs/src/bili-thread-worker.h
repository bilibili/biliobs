#ifndef BILIWORKERTHREAD_H
#define BILIWORKERTHREAD_H

#include <functional>
#include <queue>
#include <utility>
#include <list>

#include "third_party/w32-pthreads/pthread.h"
#include "third_party/w32-pthreads/semaphore.h"

class BiliThreadWorker
{
public:
	typedef std::function<void*()> TaskT;

private:
	BiliThreadWorker(const BiliThreadWorker&);
	BiliThreadWorker& operator= (const BiliThreadWorker&);

	std::list<std::function<int()> > freeResources;

	pthread_t workerThread;
	sem_t queueSemaphore;

	volatile bool isThreadRunning;
	pthread_cond_t isThreadRunningCondVar;
	pthread_mutex_t isThreadRunningMutex;

	void* stopSignal; //this address returned from taskT means the thread should stop

	static void* WorkerProc(void* userData);
	void WorkerProcImpl();

	std::queue<TaskT> taskQueue;
	pthread_mutex_t taskQueueMutex;
public:
	BiliThreadWorker();
	~BiliThreadWorker();

	bool Start();
	bool Stop();

	void AddTask(TaskT&& task);
};

#endif
