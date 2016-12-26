#ifndef BILICOUNTERTRIGGER_H
#define BILICOUNTERTRIGGER_H

#include <functional>

//一个带有计数的对象
//计数为0的时候触发事件
//用途：例如停止直播这种情况，要推流那边和弹幕机那边都停了以后才算停
//然而可能又不好确定哪个先哪个后，于是就（ry
class BiliCounterTrigger
{
	volatile long count;
	std::function<void()> triggerAction;

	BiliCounterTrigger(std::function<void()>&& action);

	void Inc();
	void Dec();
public:
	void StartPendingAction();
	void FinishPendingAction();
	void Activate();

	static BiliCounterTrigger* Create(std::function<void()>&& action);
};

#endif
