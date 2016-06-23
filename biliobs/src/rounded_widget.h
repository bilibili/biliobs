#ifndef ROUNDEDWIDGET_H
#define ROUNDEDWIDGET_H

#include <QWidget>

class RoundedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RoundedWidget(const QColor &background);

protected:
    void paintEvent(QPaintEvent *e);
private:
    QColor bgr_color_;

};

#endif // ROUNDEDWIDGET_H
