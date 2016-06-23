#ifndef BILIEFFECTWIDGET_H
#define BILIEFFECTWIDGET_H

#include <QWidget>
#include "effect_window_interface.h"

class BiliEffectWidget : public QWidget, public EffectWindowInterface
{
    Q_OBJECT
public:
    explicit BiliEffectWidget(QWidget *host);

    void doOnHostWidgetResize(int x, int y, int w, int height) override;
    void doOnHostWidgetVisibleStateChanged(bool visible) override;
    void doOnHostWidgetFocused() override;
    void doOnHostWidgetPositionChanged(int x, int y) override;

protected:
    void paintEvent(QPaintEvent *e);

private:
    int x_off_;
    int y_off_;
};

#endif // BILIEFFECTWIDGET_H
