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
    mJournal = new ScintillaEdit();
    mJournal->styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    mJournal->styleSetSize(STYLE_DEFAULT, font.pointSize());
    mJournal->setWrapMode(1);
    for (int idx = 0; idx < mJournal->margins(); ++idx)
        mJournal->setMarginWidthN(idx, 0);
    mJournal->setUseTabs(false);
    mJournal->setTabIndents(true);
    mJournal->setIndent(4);
    
    tabJournalLayout->addWidget(mJournal);
    mJournal->setObjectName("Journal");
    
    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEdit();
    mAnswer->styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    mAnswer->styleSetSize(STYLE_DEFAULT, font.pointSize());
    mAnswer->setWrapMode(1);
    for (int idx = 0; idx < mAnswer->margins(); ++idx)
        mAnswer->setMarginWidthN(idx, 0);
    mAnswer->setUseTabs(false);
    mAnswer->setTabIndents(true);
    mAnswer->setIndent(4);
    
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
