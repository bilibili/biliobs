#ifndef EFFECTWINDOWINTERFACE_H
#define EFFECTWINDOWINTERFACE_H


class EffectWindowInterface
{

public:
    virtual void doOnHostWidgetResize(int x, int y, int w, int h) = 0;
    virtual void doOnHostWidgetVisibleStateChanged(bool visible) = 0;
    virtual void doOnHostWidgetFocused() = 0;
    virtual void doOnHostWidgetPositionChanged(int x, int y) = 0;

};

#endif // EFFECTWIDGETINTERFACE_H
