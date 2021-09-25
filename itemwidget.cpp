#include "itemwidget.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTabBar>

ItemWidget::ItemWidget(Data::Item& item, QFont font): mItem(item)
{
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
    connect(mJournal, SIGNAL(notifyChange()), this, SLOT(UpdateJournal()));
    
    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEditCustom(font);
    tabAnswerLayout->addWidget(mAnswer);
    mAnswer->setObjectName("Answer");
    connect(mAnswer, SIGNAL(notifyChange()), this, SLOT(UpdateAnswer()));
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mNeed);
    layout->addWidget(mJournalAnswer);
    
    if (mItem.ID() >= 0)
    {
        mNeed->setText(item.Need());
        mJournal->setText(item.Journal().toUtf8().data());
        mJournal->emptyUndoBuffer();
        mAnswer->setText(item.Answer().toUtf8().data());
        mAnswer->emptyUndoBuffer();
    }
    
    connect(mJournalAnswer, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
            
    if (mAnswer->length() == 0 && mJournal->length() != 0)
        mJournalAnswer->setCurrentIndex(0);
    else
        mJournalAnswer->setCurrentIndex(1);
    
    UpdateJournal();
    UpdateAnswer();
}

bool ItemWidget::IsEmpty()
{
    return mNeed->text() == "" &&
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

void ItemWidget::UpdateJournal()
{
    mJournalAnswer->tabBar()->setTabTextColor(0, mJournal->length() == 0 ? 
                                                  QColor(Qt::GlobalColor::gray) :
                                                  QColor(Qt::GlobalColor::black));
    mDirtyJournal = true;
}

void ItemWidget::UpdateAnswer()
{
    mJournalAnswer->tabBar()->setTabTextColor(1, mAnswer->length() == 0 ? 
                                                  QColor(Qt::GlobalColor::gray) :
                                                  QColor(Qt::GlobalColor::black));
    mDirtyAnswer = true;
}

void ItemWidget::UpdateNeed()
{
    mDirtyNeed = true;
}
