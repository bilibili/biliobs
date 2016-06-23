#ifndef CONTENT_SLIDER_H
#define CONTENT_SLIDER_H

#include "slider_input_accept_macro.h"

#include <QWidget>
/*make sure the argument valid personally*/

class ContentSlider : public QWidget {
    Q_OBJECT

public:
    explicit ContentSlider(QWidget *parent = 0);

public:
    bool valid() const;

public:
    void setViewAndContentSize(int v, int c);
private:
    int content_size_;
    int view_size_;

private:
    double position_;  /*valid val: 0 to 1*/
public:
    void setPostion(double pos);
    int positionInt() const;
    double positionDouble() const { return position_; }
signals:
    void postionChanged(double pos);


private:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
private:
    bool ignore_mouse_press_;
    int press_off_;

private:
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
private:
    const int slider_min_size_;
    int slider_size_;

INPUT_HANDLE_DEC

};

#endif
