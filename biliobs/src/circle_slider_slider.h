
#ifndef CIRCLE_SLIDER_SLIDER_H
#define CIRCLE_SLIDER_SLIDER_H
#include "slider_interface.h"

class CircleSliderSlider : public SliderInterface {
public:
	CircleSliderSlider(QWidget *parent = 0, Orientation orient = Orientation::HORIZONTAL);

	~CircleSliderSlider();


private:
    Orientation orientation_;
public:
    void setOrientation(Orientation direct)    { orientation_ = direct; } /*now no support for change*/
    Orientation orientation() const { return orientation_; }

private:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *e) override;

private:
    bool btn_pressed_;

private:
    void internalChangedInform() override;

private:
    int p2p_off_;
    int pos_on_move_;

private:
    const QPoint btnPos() const;
    int valToPixel(int val) const;
    int pixelToVal(int px) const;

private:
    void initPix();
    QPixmap *btn_pix_;
    QPixmap *btn_disable_pix_;
    QSize btn_size_;

};

#endif