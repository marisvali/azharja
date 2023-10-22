#include "itemparentswidget.h"
#include <QVBoxLayout>
#include <QLabel>

ItemParentsWidget::ItemParentsWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto label = new QLabel("Parents");
    mList = new QListWidget();
    layout->addWidget(label);
    layout->addWidget(mList);
}
