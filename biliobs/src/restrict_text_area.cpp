
#include "restrict_text_area.h"
#include "content_slider.h"
#include <QLayout>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QPainter>

#include <QTextDocument>
#include <QTextCursor>

#include <qdebug.h>


RestrictTextArea::RestrictTextArea(QWidget *parent /*= 0*/) : QWidget(parent)
{
    init();
}

void RestrictTextArea::addItem(const QString &str)
{
    addItemimpl(str);

    if (!on_add_transaction_)
        doContentUpdate();
}

void RestrictTextArea::addItems(const QString *begin, int num)
{
    if (num > LIMIT_PER_PAGE) {
        replace(begin, num);
        return;
    }

    for (int i = 0; i < num; i++)
        addItemimpl(begin[i]);

    if (!on_add_transaction_)
        doContentUpdate();
}
void RestrictTextArea::replace(const QString *begin, int num)
{
    int i;
    if (num > LIMIT_PER_PAGE) {
        item_count_ = LIMIT_PER_PAGE;
        i = num - LIMIT_PER_PAGE;
    } else {
        item_count_ = num;
        i = 0;
    }
    doc0_->clear();
    doc1_->clear();
    doc0_item_heights_[LIMIT_PER_PAGE] = 0;
    doc1_item_heights_[LIMIT_PER_PAGE] = 0;

    if (!begin) return;

    for (int j = 0; i < num; i++, j++) {
        docAddHtml(doc0_, begin[i]);
        doc0_item_heights_[j] = doc0_item_heights_[LIMIT_PER_PAGE];
        doc0_item_heights_[LIMIT_PER_PAGE] = doc0_->size().height();
    }

    if (!on_add_transaction_)
        doContentUpdate();
}

void RestrictTextArea::addItemimpl(const QString &str)
{
    if (LIMIT_PER_PAGE * 2 - 1 == item_count_) {

        swapPtr(&doc0_, &doc1_);
        swapPtr(&doc0_item_heights_, &doc1_item_heights_);

        doc1_->clear();

        item_count_ = LIMIT_PER_PAGE;
        docAddHtml(doc0_, str);


        doc1_item_heights_[LIMIT_PER_PAGE] = 0;
        doc0_item_heights_[LIMIT_PER_PAGE - 1] = doc0_item_heights_[LIMIT_PER_PAGE];
        doc0_item_heights_[LIMIT_PER_PAGE] = doc0_->size().height();

    }
    else {
        item_count_++;
        if (item_count_ > LIMIT_PER_PAGE) {
            docAddHtml(doc1_, str);
            doc1_item_heights_[item_count_ - LIMIT_PER_PAGE - 1] = doc1_item_heights_[LIMIT_PER_PAGE];
            doc1_item_heights_[LIMIT_PER_PAGE] = doc1_->size().height();

        }
        else {
            docAddHtml(doc0_, str);
            doc0_item_heights_[item_count_ - 1] = doc0_item_heights_[LIMIT_PER_PAGE];
            doc0_item_heights_[LIMIT_PER_PAGE] = doc0_->size().height();

        }
    }

    //
}

void RestrictTextArea::doContentUpdate()
{
    if (item_count_ > LIMIT_PER_PAGE)
        slider_->setViewAndContentSize(content_wgt_->height(),
                                       doc1_item_heights_[LIMIT_PER_PAGE]
                                       + (doc0_item_heights_[LIMIT_PER_PAGE]
                                          - doc0_item_heights_[item_count_ - LIMIT_PER_PAGE]
                                         )
                                      );
    else
        slider_->setViewAndContentSize(content_wgt_->height(), doc0_item_heights_[LIMIT_PER_PAGE]);

    need_repaint_ = true;
}

void RestrictTextArea::positionChanged(double pos)
{
    need_repaint_ = true;
}

bool RestrictTextArea::eventFilter(QObject *o, QEvent *e)
{
    if (o == content_wgt_) {
        if (e->type() == QEvent::Paint) {
            QPainter painter(content_wgt_);
            e->accept();

            painter.fillRect(content_wgt_->rect(), Qt::white);

            if (0 == item_count_)
                return true;

            if (slider_->valid()) {
                int pos_int = slider_->positionInt() - content_wgt_->height() / 2;
                int off;

                if (item_count_ <= LIMIT_PER_PAGE) {

                    painter.setWindow(0, pos_int, content_wgt_->width(), content_wgt_->height());

                    doc0_->drawContents(&painter, QRect(0, pos_int, content_wgt_->width(), content_wgt_->height()));
                } else {
                    int begin = item_count_ - LIMIT_PER_PAGE;



                        
                    painter.setWindow(0, pos_int + doc0_item_heights_[begin], content_wgt_->width(), content_wgt_->height());
                    doc0_->drawContents(&painter, QRect(0, doc0_item_heights_[begin] + pos_int, content_wgt_->width(), content_wgt_->height()));
                    int consume = doc0_item_heights_[LIMIT_PER_PAGE] - doc0_item_heights_[begin] - pos_int;

                    if (consume < content_wgt_->height()) {
                        painter.setWindow(0, -consume, content_wgt_->width(), content_wgt_->height());

                        doc1_->drawContents(&painter, QRect(0, 0, content_wgt_->width(), content_wgt_->height() - consume));
                    }

                }


            } else {
                doc0_->drawContents(&painter, QRect(0, 0, content_wgt_->width(), content_wgt_->height()));
            }

            return true;
        } else if (e->type() == QEvent::Resize) {

            if (item_count_ > LIMIT_PER_PAGE) {

                slider_->setViewAndContentSize(content_wgt_->height(),
                                               doc1_item_heights_[LIMIT_PER_PAGE]
                                               + (doc0_item_heights_[LIMIT_PER_PAGE]
                                                       - doc0_item_heights_[LIMIT_PER_PAGE - (item_count_ - LIMIT_PER_PAGE)]
                                                       )
                                               );
            } else {

                slider_->setViewAndContentSize(content_wgt_->height(), doc0_item_heights_[LIMIT_PER_PAGE]);
            }

            return true;
        }

    }

    return QWidget::eventFilter(o, e);

}


void RestrictTextArea::init()
{
    doc0_ = new QTextDocument(this);
    doc1_ = new QTextDocument(this);

    doc0_->setTextWidth(CONTENT_WIDTH);
    doc1_->setTextWidth(CONTENT_WIDTH);

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    slider_ = new ContentSlider(this);
    slider_->setFixedWidth(SLIDER_WIDTH);

    content_wgt_ = new QWidget(this);
    content_wgt_->setFixedWidth(CONTENT_WIDTH);

    connect(slider_, &ContentSlider::postionChanged, this, &RestrictTextArea::positionChanged);

    main_layout->addWidget(content_wgt_);
    main_layout->addWidget(slider_);
    content_wgt_->installEventFilter(this);

    QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_layout->addItem(spacer);


    item_count_ = 0;
    doc0_item_heights_ = array[0];
    doc1_item_heights_ = array[1];
    doc0_item_heights_[LIMIT_PER_PAGE] = 0;
    doc1_item_heights_[LIMIT_PER_PAGE] = 0;

    on_add_transaction_ = false;

    startTimer(PAINT_INTERVAL);
    need_repaint_ = false;

    need_repaint_ = true;
}

void RestrictTextArea::docAddHtml(QTextDocument *doc, const QString &html)
{
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);

//    cursor.beginEditBlock();

    QTextBlockFormat format;
    format.setTopMargin(0);
    format.setBottomMargin(0);

    cursor.insertBlock(format, cursor.charFormat());
    cursor.insertHtml(html);
}

void RestrictTextArea::swapPtr(void **p1, void **p2)
{
    void *tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}

void RestrictTextArea::timerEvent(QTimerEvent *e)
{
    if (need_repaint_) {
        content_wgt_->update();
        need_repaint_ = false;
    }
        
}


INPUT_HANDLE_DEF(RestrictTextArea)

void RestrictTextArea::stepUp()
{
    if (slider_->valid())
        slider_->stepUp();

}

void RestrictTextArea::stepDn()
{
    if (slider_->valid())
        slider_->stepDn();
}
