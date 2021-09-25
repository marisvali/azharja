#include "itemwidget.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTabBar>

ItemWidget::ItemWidget(const Data::Item& item, QFont font)
{
    mItemID = item.mID;
    
    mNeed = new QLineEdit();
    mNeed->setObjectName("Need");
    
    mJournalAnswer = new QTabWidget();
    
    auto tabJournal = new QWidget();
    mJournalAnswer->addTab(tabJournal, "Journal");
    
    auto tabAnswer = new QWidget();
    mJournalAnswer->addTab(tabAnswer, "Answer");
    
    QVBoxLayout* tabJournalLayout = new QVBoxLayout(tabJournal);
    mJournal = new ScintillaEditCustom(font);
    tabJournalLayout->addWidget(mJournal);
    mJournal->setObjectName("Journal");
    connect(mJournal, SIGNAL(notifyChange()), this, SLOT(UpdateTabTitleText()));
    
    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEditCustom(font);
    tabAnswerLayout->addWidget(mAnswer);
    mAnswer->setObjectName("Answer");
    connect(mAnswer, SIGNAL(notifyChange()), this, SLOT(UpdateTabTitleText()));
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mNeed);
    layout->addWidget(mJournalAnswer);
    
    if (mItemID >= 0)
    {
        mNeed->setText(item.mNeed);
        mJournal->setText(item.mJournal.toUtf8().data());
        mAnswer->setText(item.mAnswer.toUtf8().data());
    }
    
    connect(mJournalAnswer, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
            
    if (mAnswer->length() == 0 && mJournal->length() != 0)
        mJournalAnswer->setCurrentIndex(0);
    else
        mJournalAnswer->setCurrentIndex(1);
    
    UpdateTabTitleText();
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

void ItemWidget::UpdateTabTitleText()
{
    mJournalAnswer->tabBar()->setTabTextColor(0, mJournal->length() == 0 ? 
                                                  QColor(Qt::GlobalColor::gray) :
                                                  QColor(Qt::GlobalColor::black));
    mJournalAnswer->tabBar()->setTabTextColor(1, mAnswer->length() == 0 ? 
                                                  QColor(Qt::GlobalColor::gray) :
                                                  QColor(Qt::GlobalColor::black));
}
