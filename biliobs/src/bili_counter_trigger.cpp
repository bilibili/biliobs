#include "bili_counter_trigger.h"
#include "../libobs/util/threading.h"

BiliCounterTrigger::BiliCounterTrigger(std::function<void()>&& action)
	: triggerAction(action)
	, count(0)
{
	Inc();
}

void BiliCounterTrigger::StartPendingAction()
{
	Inc();
}

void BiliCounterTrigger::FinishPendingAction()
{
	Dec();
}

BiliCounterTrigger* BiliCounterTrigger::Create(std::function<void()>&& action)
{
	return new BiliCounterTrigger(std::move(action));
}

void BiliCounterTrigger::Activate()
{
	Dec();
}

void BiliCounterTrigger::Inc()
{
	os_atomic_inc_long(&count);
}

void BiliCounterTrigger::Dec()
{
	if (os_atomic_dec_long(&count) == 0)
	{
		triggerAction();
		delete this;
	}
}
