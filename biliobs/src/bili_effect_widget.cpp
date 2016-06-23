#include "bili_effect_widget.h"
#include <qpainter.h>

#include <Windows.h>

#include <assert.h>
#include <qdebug.h>

BiliEffectWidget::BiliEffectWidget(QWidget *host) : QWidget(host)
{
    x_off_ = 6;
    y_off_ = 6;

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool | Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground);

    //HWND hwnd = (HWND)winId();
    //SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

}

void BiliEffectWidget::doOnHostWidgetResize(int x, int y, int w, int h)
{
    move(x - x_off_, y - y_off_);
    resize(x_off_ * 2 + w, y_off_ * 2 + h);
    repaint();
}

void BiliEffectWidget::doOnHostWidgetVisibleStateChanged(bool visible)
{
    setVisible(visible);
}

void BiliEffectWidget::doOnHostWidgetFocused()
{
    raise();
}

void BiliEffectWidget::doOnHostWidgetPositionChanged(int x, int y)
{
    move(x - x_off_, y - y_off_);
    show();
    //repaint();
}


void BiliEffectWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    //painter.setCompositionMode(QPainter::CompositionMode_Clear);
    //painter.fillRect(rect(), Qt::transparent);

    //painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    QPen p = painter.pen();
    p.setWidth(1);
    p.setWidthF(0.5);


    int h = height();
    int w = width();

    assert(6 == x_off_);
    assert(6 == y_off_);

    p.setColor(QColor(63, 63, 63, 30));
    painter.setPen(p);
    painter.drawRoundedRect(0,
                            0,
                            width() - 1 * 2 + 1,
                            height() - 1 * 2 + 1,
                            x_off_,
                            y_off_);

    //
    p.setColor(QColor(63, 63, 63, 60));
    painter.setPen(p);
    painter.drawRoundedRect(1,
                            1,
                            width() - 2 * 2 + 1,
                            height() - 2 * 2 + 1,
                            x_off_,
                            y_off_);
   

    p.setColor(QColor(63, 63, 63, 90));
    painter.setPen(p);
    painter.drawRoundedRect(2,
                            2,
                            width() - 3 * 2 + 1,
                            height() - 3 * 2 + 1,
                            x_off_,
                            y_off_);


    p.setColor(QColor(63, 63, 63, 120));
    painter.setPen(p);
    painter.drawRoundedRect(3,
                            3,
                            width() - 4 * 2 + 1,
                            height() - 4 * 2 + 1,
                            x_off_,
                            y_off_);

    p.setColor(QColor(63, 63, 63, 150));
    painter.setPen(p);
    painter.drawRoundedRect(4,
                            4,
                            width() - 5 * 2 + 1,
                            height() - 5 * 2 + 1,
                            x_off_,
                            y_off_);

    p.setColor(QColor(63, 63, 63, 180));
    painter.setPen(p);
    painter.drawRoundedRect(5,
                            5,
                            width() - 6 * 2 + 1,
                            height() - 6 * 2 + 1,
                            x_off_,
                            y_off_);

}
