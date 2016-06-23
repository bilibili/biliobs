
#include "timer_on_qt.h"
#include <qtimer.h>





TimerOnQt::TimerGenerator::TimerGenerator()
{
    cb_holder_ = 0;
}
TimerOnQt::TimerGenerator::~TimerGenerator()
{

}

TimerOnQt::TimerOnQt()
{
    timer_generator_ = new TimerGenerator();
    timer_generator_->setTimerCb(&TimerOnQt::timerTriggered, this);
}

TimerOnQt::~TimerOnQt()
{
    delete timer_generator_;
}


void TimerOnQt::doStartTimer(int m_sec)
{
    timer_id_ = timer_generator_->startTimer(m_sec);
}
void TimerOnQt::doStopTimer()
{
    timer_generator_->killTimer(timer_id_);
}

void TimerOnQt::timerTriggered()
{
    timerTriggerInform();
}