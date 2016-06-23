
#ifndef TIMER_INTERFACE_H
#define TIMER_INTERFACE_H

class TimerInterFace {
public:
    TimerInterFace() : is_working_(false) {}
    virtual ~TimerInterFace() {}

public:
    virtual bool init() { return true; };
    virtual bool release() { return true; };

    void startTimer(void(*cb)(void *arg), void *arg, int m_sec) 
    {
        timer_cb_ = cb;  timer_cb_arg_ = arg;  is_working_ = true; doStartTimer(m_sec);
    }
    void stopTimer()  { is_working_ = false; doStopTimer(); } 

    bool isWorking() const { return is_working_; }

protected:
    virtual void doStartTimer(int m_sec) = 0;
    virtual void doStopTimer() = 0;

    void timerTriggerInform() { if (timer_cb_) timer_cb_(timer_cb_arg_); }
private:
    void (*timer_cb_)(void*);
    void *timer_cb_arg_;

    bool is_working_;

};

#endif