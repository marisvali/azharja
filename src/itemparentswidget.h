#ifndef ITEMPARENTSWIDGET_H
#define ITEMPARENTSWIDGET_H

#include <QListWidget>
#include <QWidget>

class ItemParentsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemParentsWidget(QWidget* parent = nullptr);

    QListWidget* mList = nullptr;

signals:
};

#endif  // ITEMPARENTSWIDGET_H
