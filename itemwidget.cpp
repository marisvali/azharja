#include "itemwidget.h"
#include <QVBoxLayout>
#include <QTabWidget>

ItemWidget::ItemWidget(const Data::Item& item, QFont font)
{
    mItemID = item.mID;
    
    mNeed = new QLineEdit();
    mNeed->setObjectName("Need");
    
    auto journalAnswer = new QTabWidget();
    
    auto tabJournal = new QWidget();
    journalAnswer->addTab(tabJournal, "Journal");
    
    auto tabAnswer = new QWidget();
    journalAnswer->addTab(tabAnswer, "Answer");
    
    QVBoxLayout* tabJournalLayout = new QVBoxLayout(tabJournal);
    mJournal = new ScintillaEditCustom(font);
    tabJournalLayout->addWidget(mJournal);
    mJournal->setObjectName("Journal");
    
    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEditCustom(font);
    tabAnswerLayout->addWidget(mAnswer);
    mAnswer->setObjectName("Answer");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mNeed);
    layout->addWidget(journalAnswer);
    
    if (mItemID >= 0)
    {
        mNeed->setText(item.mNeed);
        mJournal->setText(item.mJournal.toUtf8().data());
        mAnswer->setText(item.mAnswer.toUtf8().data());
    }
    
    connect(journalAnswer, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
            
    if (mAnswer->length() == 0 && mJournal->length() != 0)
        journalAnswer->setCurrentIndex(0);
    else
        journalAnswer->setCurrentIndex(1);
}

bool ItemWidget::IsEmpty()
{
    return ItemID() == -1 &&
           mNeed->text() == "" &&
           mJournal->length() == 0 &&
           mAnswer->length() == 0;
}

void ItemWidget::TabChanged(int index)
{
    if (index == 0)
        this->setFocusProxy(mJournal);
    else
        this->setFocusProxy(mAnswer);
}
