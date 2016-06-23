
#ifndef TIMER_ON_QT_H
#define TIMER_ON_QT_H

#include "timer_interface.h"
#include <qtimer.h>


class TimerOnQt : public TimerInterFace {
public:
    TimerOnQt();
    ~TimerOnQt();

private:
    void doStartTimer(int m_sec) override;
    void doStopTimer() override;

private:
    class TimerGenerator;

    void timerTriggered();
private:
    TimerGenerator *timer_generator_;
    int timer_id_;
};

class TimerOnQt::TimerGenerator : public QObject{
    Q_OBJECT

public:
    TimerGenerator();
    ~TimerGenerator();
public:
    void setTimerCb(void (TimerOnQt::*cb)(void), TimerOnQt *obj) { timer_cb_ = cb; cb_holder_ = obj; }

private:
    void timerEvent(QTimerEvent *e) override { if (cb_holder_) (cb_holder_->*timer_cb_)(); }

private:
    void (TimerOnQt::*timer_cb_)(void);
    TimerOnQt *cb_holder_;

};

#endif