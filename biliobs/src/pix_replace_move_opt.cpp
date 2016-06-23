#include "pix_replace_move_opt.h"
#include <assert.h>

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

PixReplaceMoveOpt::PixReplaceMoveOpt(QObject *parent) : QObject(parent)
{
    movie_timer_ = 0;

    fade_stride_ = 5;
    radius_stride_ = 10;
    move_stride_ = 10;
    tear_stride_ = 10;
}

void PixReplaceMoveOpt::setPixmaps(const QPixmap &old, const QPixmap &replaced)
{
    old_pix_ = old;
    replaced_pix_ = replaced;
}

void PixReplaceMoveOpt::startMovie()
{
    assert(!movie_timer_);
    on_show_ = false;

    stride_count_ = 0;

    cur_strategy_ = rand();
//    cur_strategy_ = UP_SCROLL;
//    qDebug() << cur_strategy_;
    switch (cur_strategy_) {
    case FADE:
        movie_timer_ = startTimer(50);
        break;
//    case TEAR:
//        movie_timer_ = startTimer(50);
//        break;
    //case CIRCLE_DISSLOVE:
    //    on_disslove_all_ = false;
    //    movie_timer_ = startTimer(50);
    //    break;
    case UP_SCROLL:
        movie_timer_ = startTimer(50);
        scale_replaced_pix_ = replaced_pix_.scaled(old_pix_.size());
        break;
    default:
        break;
    }
}

void PixReplaceMoveOpt::endMovie()
{
    if (movie_timer_) {
        killTimer(movie_timer_);
        movie_timer_ = 0;
    }
}

bool PixReplaceMoveOpt::isWorking() const
{
    return movie_timer_ != 0;
}

void PixReplaceMoveOpt::timerEvent(QTimerEvent *)
{
    switch (cur_strategy_) {
    case FADE:
        doFade();
        break;
//    case TEAR:

//        break;
    //case CIRCLE_DISSLOVE:
    //    doCircleDisslove();
    //    break;
    case UP_SCROLL:
        doScroll();
        break;
    default:
        break;
    }
}

PixReplaceMoveOpt::Strategy PixReplaceMoveOpt::rand() const
{
    return (PixReplaceMoveOpt::Strategy)(::rand() % PixReplaceMoveOpt::STRATEGY_SIZE);
}

void PixReplaceMoveOpt::doFade()
{


    stride_count_++;
    if (!on_show_) {
        QPixmap ret_pix(old_pix_.size());
        ret_pix.fill(Qt::transparent);
        QPainter painter(&ret_pix);

        int tmp = stride_count_ * fade_stride_;
        painter.setOpacity(1 - ((double) tmp / (double)100));
        painter.drawPixmap(0, 0, old_pix_);

        emit onTransform(ret_pix);
        if (100 <= tmp) {
            on_show_ = true;
            stride_count_ = 0;
        }
    } else {
        int tmp = stride_count_ * fade_stride_;
        if (tmp >= 100) {
            emit workFinished();

            killTimer(movie_timer_);
            movie_timer_ = 0;

            return;
        }

        QPixmap ret_pix(replaced_pix_.size());
        ret_pix.fill(Qt::transparent);
        QPainter painter(&ret_pix);

        painter.setOpacity((double) tmp / (double)100);
        painter.drawPixmap(0, 0, replaced_pix_);

        emit onTransform(ret_pix);
    }
}

//    void doTear();
void PixReplaceMoveOpt::doCircleDisslove()
{

    if (!on_show_) {
        QPixmap ret_pix(old_pix_.size());
        ret_pix.fill(Qt::transparent);
        QPainter painter(&ret_pix);

        if (!on_disslove_all_) {
            stride_count_++;
            int tmp = stride_count_ * fade_stride_;
  //        painter.setOpacity(1 - ((double) tmp / (double)100));
            painter.drawPixmap(0, 0, old_pix_);
            painter.setCompositionMode(QPainter::CompositionMode_Clear);

            QPainterPath path;
            path.addEllipse(QPointF(ret_pix.width() / 2, ret_pix.height() / 2),
                            (double)tmp / (double) 100 * ret_pix.width() / 2,
                            (double)tmp / (double) 100 * ret_pix.height() / 2);

            painter.fillPath(path, Qt::transparent);

            emit onTransform(ret_pix);
            if (100 <= tmp) {
                on_disslove_all_ = true;
                stride_count_ = 0;

            }
        } else {
            on_show_ = true;
            emit onTransform(ret_pix);

        }
    } else {

        int tmp = stride_count_ * fade_stride_;
        if (tmp >= 100) {
            emit workFinished();

            killTimer(movie_timer_);
            movie_timer_ = 0;

            return;
        }

        QPixmap ret_pix(replaced_pix_.size());
        ret_pix.fill(Qt::transparent);
        QPainter painter(&ret_pix);

        painter.drawPixmap(0, 0, replaced_pix_);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOut);

        QPainterPath path;
        path.addEllipse(QPointF(ret_pix.width() / 2, ret_pix.height() / 2),
                        (double)(100 - tmp) / (double) 100 * ret_pix.width() / 2,
                        (double)(100 - tmp) / (double) 100 * ret_pix.height() / 2);
        painter.fillPath(path, Qt::transparent);

        emit onTransform(ret_pix);

        stride_count_++;
    }
}

void PixReplaceMoveOpt::doScroll()
{
    stride_count_++;

    int tmp = stride_count_ * fade_stride_;

    if (tmp >= 100) {
        emit workFinished();

        killTimer(movie_timer_);
        movie_timer_ = 0;

        return;
    }

    QPixmap ret_pix(old_pix_.size());
    ret_pix.fill(Qt::transparent);
    QPainter painter(&ret_pix);

    painter.drawPixmap(0,
                       -((double)tmp / (double)100 * ret_pix.height()),
                       old_pix_);

    painter.drawPixmap(0,
                       ((double)(100 - tmp) / (double)100 * ret_pix.height()),
                       scale_replaced_pix_);

    emit onTransform(ret_pix);

}

