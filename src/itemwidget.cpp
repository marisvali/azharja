#include "itemwidget.h"

#include <QTabBar>
#include <QTabWidget>
#include <QVBoxLayout>

ItemWidget::ItemWidget(Item& item, QFont font) : mItem(item)
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

    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEditCustom(font);
    tabAnswerLayout->addWidget(mAnswer);
    mAnswer->setObjectName("Answer");

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
    connect(mJournal, SIGNAL(notifyChange()), this, SLOT(UpdateJournal()));
    connect(mAnswer, SIGNAL(notifyChange()), this, SLOT(UpdateAnswer()));
    connect(mNeed, SIGNAL(textChanged(QString)), this, SLOT(UpdateNeed(QString)));

    if (mAnswer->length() == 0 && mJournal->length() != 0)
        mJournalAnswer->setCurrentIndex(0);
    else
        mJournalAnswer->setCurrentIndex(1);

    SetTabTextColors();
}

ItemWidget::~ItemWidget()
{
    if (!mDeleteItem)
        SaveToMemoryGuaranteed();

    if (mItem.IsEmpty() || mDeleteItem)
    {
        mItem.GetData().DeleteItem(mItem.ID());
        emit ItemDeleted();
    }
}

bool ItemWidget::IsEmpty() { return mNeed->text() == "" && mJournal->length() == 0 && mAnswer->length() == 0; }

void ItemWidget::TabChanged(int index)
{
    if (index == 0)
        this->setFocusProxy(mJournal);
    else
        this->setFocusProxy(mAnswer);
}

void ItemWidget::UpdateJournal()
{
    mDirtyJournal = true;
    SetTabTextColors();
}

void ItemWidget::UpdateAnswer()
{
    mDirtyAnswer = true;
    SetTabTextColors();
}

void ItemWidget::UpdateNeed(const QString&) { mDirtyNeed = true; }

void ItemWidget::SetTabTextColors()
{
    mJournalAnswer->tabBar()->setTabTextColor(0, mJournal->length() == 0 ? QColor(Qt::GlobalColor::gray) : QColor(Qt::GlobalColor::black));

    mJournalAnswer->tabBar()->setTabTextColor(1, mAnswer->length() == 0 ? QColor(Qt::GlobalColor::gray) : QColor(Qt::GlobalColor::black));
}

bool ItemWidget::SaveToMemoryTry()
{
    if (mDirtyNeed)
        if (mItem.SetNeed(mNeed->text()))
        {
            mDirtyNeed = false;
            emit NeedChanged();
        }

    if (mDirtyJournal)
        if (mItem.SetJournal(mJournal->getText(mJournal->length() + 1)))
            mDirtyJournal = false;

    if (mDirtyAnswer)
        if (mItem.SetAnswer(mAnswer->getText(mAnswer->length() + 1)))
            mDirtyAnswer = false;

    return !mDirtyNeed && !mDirtyJournal && !mDirtyAnswer;
}

void ItemWidget::SaveToMemoryGuaranteed()
{
    while (!this->SaveToMemoryTry()) QThread::msleep(50);
}

void ItemWidget::MarkItemForDeletion() { mDeleteItem = true; }

void ItemWidget::SwitchTabs()
{
    if (mJournalAnswer->currentIndex() == 0)
        mJournalAnswer->setCurrentIndex(1);
    else
        mJournalAnswer->setCurrentIndex(0);
}
