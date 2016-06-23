#ifndef RESTRICT_TEXT_AREA_H
#define RESTRICT_TEXT_AREA_H

#include "slider_input_accept_macro.h"
#include "barrage_history_macro.h"

#define CONTENT_WIDTH 235
#define SLIDER_WIDTH 8
#define PAINT_INTERVAL (1000 / 30)

#include <QWidget>
class QTextDocument;

class ContentSlider;

class RestrictTextArea : public QWidget {
    Q_OBJECT
public:
    RestrictTextArea(QWidget *parent = 0);

public:
    void addItem(const QString &);
    void addItems(const QString *begin, int num);
    void replace(const QString *begin, int num);

    void beginAddTransaction() { on_add_transaction_ = true; }
    void endAddTransaction() { on_add_transaction_ = false; doContentUpdate(); }
private:
    bool on_add_transaction_;

private:
    void addItemimpl(const QString &);
    void doContentUpdate();

private slots:
    void positionChanged(double pos);

private:
    bool eventFilter(QObject *o, QEvent *e);

private:
    void init();
private:
    static void docAddHtml(QTextDocument *doc, const QString &);

    template <class T>
    static void swapPtr(T **p1, T **p2) { swapPtr((void**)p1, (void**)p2); }
    static void swapPtr(void **p1, void **p2);

private:
    void timerEvent(QTimerEvent *e);
private:
    bool need_repaint_;

private:
    QTextDocument *doc0_;
    int *doc0_item_heights_;
    QTextDocument *doc1_;
    int *doc1_item_heights_;

    int array[2][LIMIT_PER_PAGE + 1];

    int item_count_;

    ContentSlider *slider_;
    QWidget *content_wgt_;
    

INPUT_HANDLE_DEC
};

#endif
