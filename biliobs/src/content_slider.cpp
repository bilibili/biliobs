
#include "content_slider.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>

#include <assert.h>
#include <QPainter>
#include <qdebug.h>
#include <qpainterpath.h>

ContentSlider::ContentSlider(QWidget *parent /*= 0*/)
        :QWidget(parent), slider_min_size_(21)
{
    view_size_ = content_size_ = 0;

    setStep(20);
}

bool ContentSlider::valid() const
{
    return (view_size_ < content_size_);
}

void ContentSlider::setViewAndContentSize(int v, int c)
{
    assert(c >= 0);
    assert(v >= 0);

    bool pre_state = valid();

    content_size_ = c;
    view_size_ = v;

    //qDebug() << "view port" << view_size_;
    //qDebug() << "content port" << content_size_;

    if (valid()) {
        double ratio = (double)view_size_ / (double)content_size_;

        if (!pre_state) {
            position_ = 1;
        }


        if (ratio > 1)
            slider_size_ = height();
        else if (ratio < 0)
            slider_size_ = slider_min_size_;
        else {
            int hint = ratio * height();

            if (hint < slider_min_size_) {
                if (slider_size_ == slider_min_size_)
                    return;
            } else if (hint >= height())
                slider_size_ = height();
            else
                slider_size_ = hint;
        }

        update();

    }
}

void ContentSlider::setPostion(double pos)
{
    assert(pos <= 1.0);
    assert(pos >= 0);

    if (!valid()) return;

    position_ = pos;
    update();
}

int ContentSlider::positionInt() const
{
    if (!valid()) return -1;

    return position_ * (content_size_ - view_size_) + view_size_ / 2;
}

void ContentSlider::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    if (!valid() || e->buttons() != Qt::LeftButton) {
        ignore_mouse_press_ = true;
        return;
    }

    ignore_mouse_press_ = false;

    int pos_int = position_ * (height() - slider_size_) + slider_size_ / 2;
    int mouse_pos = e->pos().y();
    if (mouse_pos >= pos_int - slider_size_ / 2
            && mouse_pos <= pos_int + slider_size_ / 2)
        /*on slider*/
        press_off_ = pos_int - mouse_pos;
    else
        if (mouse_pos - slider_size_ / 2 >= 0
                && mouse_pos + slider_size_ / 2 <= height()) {
            press_off_ = 0;
            position_ = (double)(mouse_pos - slider_size_ / 2) / (double)(height() - slider_size_);
            emit postionChanged(position_);
            update();
        } else
            if (mouse_pos - slider_size_ / 2 < 0) {
                press_off_ = slider_size_ / 2 - mouse_pos;
                press_off_ = 0;
                update();
            } else {
                press_off_ = (height() - slider_size_ / 2) - mouse_pos;
                press_off_ = 1;
                update();
            }

    //qDebug() << press_off_;


        

}

void ContentSlider::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
    if (ignore_mouse_press_)
        return;

    if (!valid()) return;

    int pos_int = e->pos().y() + press_off_;
    if (pos_int - slider_size_ / 2 < 0) {
        if (0 != position_) {
            position_ = 0;
            emit postionChanged(position_);
            update();
        }
    } else if (pos_int + slider_size_ / 2 > height()) {
        if (1 != position_) {
            position_ = 1;
            emit postionChanged(position_);
            update();
        }
    } else {
        position_ = (double)(pos_int - slider_size_ / 2) / (double)(height() - slider_size_);
        emit postionChanged(position_);

        update();
    }

}

void ContentSlider::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
    if (ignore_mouse_press_)
        return;
    if (!valid()) return;
}

void ContentSlider::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();
    mousePressEvent(e);
}

void ContentSlider::resizeEvent(QResizeEvent *e)
{
    e->accept();
    if (!valid()) return;

    double ratio = (double)view_size_ / (double)content_size_;

    if (ratio > 1)
        slider_size_ = height();
    else if (ratio < 0)
        slider_size_ = slider_min_size_;
    else {
        int hint = ratio * height();

        if (hint < slider_min_size_)
            slider_size_ = slider_min_size_;
        else if (hint >= height())
            slider_size_ = height();
        else
            slider_size_ = hint;
    }
}

void ContentSlider::paintEvent(QPaintEvent *e)
{
    if (!valid()) return;

    int slider_begin_pos = position_ * (height() - slider_size_);

    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    //painter.fillRect(0, slider_begin_pos, width(), slider_size_, Qt::green);

    QPainterPath round_rect;
    round_rect.addRoundedRect(0, slider_begin_pos, width(), slider_size_, 4, 4);
    painter.fillPath(round_rect, QColor(238, 238, 238));
}


INPUT_HANDLE_DEF(ContentSlider)

void ContentSlider::stepUp()
{
    int content_pos = position_ * (content_size_ - view_size_);

    int dst = content_pos - getStep();

    if (dst < 0) {
        if (0 != dst) {
            position_ = 0;
            emit postionChanged(position_);
            update();
        }

    } else {
        position_ = (double)dst / (double)(content_size_ - view_size_);

        if (position_ < 0)
            position_ = 0;
        else if (position_ > 1)
            position_ = 1;

        emit postionChanged(position_);
        update();
    }
}

void ContentSlider::stepDn()
{
    int content_pos = position_ * (content_size_ - view_size_) + view_size_;

    int dst = content_pos + getStep();

    if (dst > content_size_) {
        if (1 != position_) {
            position_ = 1;

            emit postionChanged(position_);
            update();
        }
    } else {
        position_ = (double)(dst - view_size_) / (double)(content_size_ - view_size_);


        if (position_ < 0)
            position_ = 0;
        else if (position_ > 1)
            position_ = 1;

        emit postionChanged(position_);
        update();
    }
}


