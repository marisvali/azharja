#include "itemparentswidget.h"

#include <QLabel>
#include <QVBoxLayout>


ItemParentsWidget::ItemParentsWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto label = new QLabel("Parents");
    mList = new QListWidget();
    layout->addWidget(label);
    layout->addWidget(mList);
}
