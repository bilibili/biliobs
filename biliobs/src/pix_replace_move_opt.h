#ifndef PIXREPLACEMOVEOPT_H
#define PIXREPLACEMOVEOPT_H

#include <QObject>

#include <QPixmap>

class PixReplaceMoveOpt : public QObject
{
    Q_OBJECT

public:
    PixReplaceMoveOpt(QObject *parent);

    void setPixmaps(const QPixmap &old, const QPixmap &replaced);
    void startMovie();
    void endMovie();

    bool isWorking() const;

signals:
    void onTransform(QPixmap pix);
    void workFinished();

private:
    enum Strategy {
        FADE = 0,
//        TEAR,
        //CIRCLE_DISSLOVE,
        UP_SCROLL,
        STRATEGY_SIZE
    };
private:
    void timerEvent(QTimerEvent *) override;

    Strategy rand() const;
    Strategy cur_strategy_;

    void doFade();
//    void doTear();
    void doCircleDisslove();
    void doScroll();
private:
    int movie_timer_;
    QPixmap old_pix_;
    QPixmap replaced_pix_;
    QPixmap scale_replaced_pix_;

    /*percent*/
    int fade_stride_;
    int radius_stride_;
    int move_stride_;
    int tear_stride_;

    int stride_count_;
    bool on_show_;

    bool on_disslove_all_;
};

#endif // PIXREPLACEMOVEOPT_H
