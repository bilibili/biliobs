#ifndef SLIDER_INPUT_ACCEPT_MACRO_H
#define SLIDER_INPUT_ACCEPT_MACRO_H

#define INPUT_HANDLE_DEC \
    protected:\
        int step_;\
    public: \
        void setStep(int st) { step_ = st; } \
        int getStep() const { return step_; } \
            \
        virtual void stepUp(); \
        virtual void stepDn(); \
    private: \
        void wheelEvent(QWheelEvent *e) override; \
        void keyPressEvent(QKeyEvent *) override;


#define INPUT_HANDLE_DEF(CLASS) \
    void CLASS::wheelEvent(QWheelEvent *e) \
    { \
        if (e->angleDelta().y() > 0) \
            stepUp(); \
        else if (e->angleDelta().y() < 0) \
            stepDn(); \
        e->accept(); \
    } \
    void CLASS::keyPressEvent(QKeyEvent *e) \
    { \
        if (1 == e->count()) { \
            if (Qt::Key_Up == e->key()) { \
                stepUp(); \
                e->accept(); \
            } else if (Qt::Key_Down == e->key()) { \
                stepDn(); \
                e->accept(); \
            } \
        } \
    }


#endif // SLIDER_INPUT_ACCEPT_MACRO_H

