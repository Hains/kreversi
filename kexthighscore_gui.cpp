/*
    This file is part of the KDE games library
    Copyright (C) 2001-2003 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QLayout>
#include <QTextStream>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTabWidget>
#include <QPushButton>
#include <QApplication>
#include <QFileDialog>
#include <QIcon>
#include <QtCore/QTemporaryFile>

#include <kmessagebox.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kguiitem.h>
#include "kexthighscore_internal.h"
#include "kexthighscore_gui.h"
#include "kexthighscore.h"
#include "kexthighscore_tab.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
ShowItem::ShowItem(QTreeWidget *list, bool highlight)
    : QTreeWidgetItem(list), _highlight(highlight)
{
//   kDebug(11001) ;
  if (_highlight) {
    for (int i=0; i < columnCount();i++) {
      setForeground(i, Qt::red);
    }
  }
}

//-----------------------------------------------------------------------------
ScoresList::ScoresList(QWidget *parent)
    : QTreeWidget(parent)
{
//   kDebug(11001) ;
    setSelectionMode(QTreeWidget::NoSelection);
//     setItemMargin(3);
    setAllColumnsShowFocus(true);
//     setSorting(-1);
    header()->setSectionsClickable(false);
    header()->setSectionsMovable(false);
}

void ScoresList::addHeader(const ItemArray &items)
{
//   kDebug(11001) ;
    addLineItem(items, 0, 0);
}

QTreeWidgetItem *ScoresList::addLine(const ItemArray &items,
                                   uint index, bool highlight)
{
//   kDebug(11001) ;
    QTreeWidgetItem *item = new ShowItem(this, highlight);
    addLineItem(items, index, item);
    return item;
}

void ScoresList::addLineItem(const ItemArray &items,
                             uint index, QTreeWidgetItem *line)
{
//   kDebug(11001) ;
    uint k = 0;
    for (int i=0; i<items.size(); i++) {
        const ItemContainer& container = *items[i];
        if ( !container.item()->isVisible() ) {
	  continue;
	}
        if (line) {
	  line->setText(k, itemText(container, index));
	  line->setTextAlignment(k, container.item()->alignment());
	}
        else {
	    headerItem()->setText(k, container.item()->label() );
            headerItem()->setTextAlignment(k, container.item()->alignment());
        }
        k++;
    }
    update();
}

//-----------------------------------------------------------------------------
HighscoresList::HighscoresList(QWidget *parent)
    : ScoresList(parent)
{
//   kDebug(11001) ;
}

QString HighscoresList::itemText(const ItemContainer &item, uint row) const
{
//   kDebug(11001) ;
    return item.pretty(row);
}

void HighscoresList::load(const ItemArray &items, int highlight)
{
//     kDebug(11001) ;
    clear();
    QTreeWidgetItem *line = 0;
    for (int j=items.nbEntries()-1; j>=0; j--) {
        QTreeWidgetItem *item = addLine(items, j, j==highlight);
        if ( j==highlight ) line = item;
    }
    scrollTo(indexFromItem(line));
}

//-----------------------------------------------------------------------------
HighscoresWidget::HighscoresWidget(QWidget *parent)
    : QWidget(parent),
      _scoresUrl(0), _playersUrl(0), _statsTab(0), _histoTab(0)
{
//     kDebug(11001) << ": HighscoresWidget";

    setObjectName( QLatin1String("show_highscores_widget" ));
    const ScoreInfos &s = internal->scoreInfos();
    const PlayerInfos &p = internal->playerInfos();

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(QApplication::fontMetrics().lineSpacing());

    _tw = new QTabWidget(this);
    connect(_tw, SIGNAL(currentChanged(int)), SLOT(tabChanged()));
    vbox->addWidget(_tw);

    // scores tab
    _scoresList = new HighscoresList(0);
    _scoresList->addHeader(s);
    _tw->addTab(_scoresList, i18n("Best &Scores"));

    // players tab
    _playersList = new HighscoresList(0);
    _playersList->addHeader(p);
    _tw->addTab(_playersList, i18n("&Players"));

    // statistics tab
    if ( internal->showStatistics ) {
        _statsTab = new StatisticsTab(0);
        _tw->addTab(_statsTab, i18n("Statistics"));
    }

    // histogram tab
    if ( p.histogram().size()!=0 ) {
        _histoTab = new HistogramTab(0);
        _tw->addTab(_histoTab, i18n("Histogram"));
    }
    load(-1);
}

void HighscoresWidget::changeTab(int i)
{
//   kDebug(11001) ;
    if ( i!=_tw->currentIndex() )
        _tw->setCurrentIndex(i);
}

void HighscoresWidget::load(int rank)
{
//   kDebug(11001) << rank;
    _scoresList->load(internal->scoreInfos(), rank);
    _playersList->load(internal->playerInfos(), internal->playerInfos().id());
    
    if (_statsTab) _statsTab->load();
    if (_histoTab) _histoTab->load();
}

//-----------------------------------------------------------------------------
HighscoresDialog::HighscoresDialog(int rank, QWidget *parent)
    : KPageDialog(parent), _rank(rank), _tab(0)
{
//     kDebug(11001) << ": HighscoresDialog";

    setWindowTitle( i18n("Highscores") );
// TODO    setButtons( Close|User1|User2 );
// TODO    setDefaultButton( Close );
    if ( internal->nbGameTypes()>1 )
        setFaceType( KPageDialog::Tree );
    else
        setFaceType( KPageDialog::Plain );
// TODO    setButtonGuiItem( User1, KGuiItem(i18n("Configure..."), QLatin1String( "configure" )) );
// TODO    setButtonGuiItem( User2, KGuiItem(i18n("Export...")) );
    connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
    connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );

    for (uint i=0; i<internal->nbGameTypes(); i++) {
        QString title = internal->manager.gameTypeLabel(i, Manager::I18N);
        QString icon = internal->manager.gameTypeLabel(i, Manager::Icon);
        HighscoresWidget *hsw = new HighscoresWidget(0);
        KPageWidgetItem *pageItem = new KPageWidgetItem( hsw, title);
        pageItem->setIcon( QIcon( BarIcon(icon, KIconLoader::SizeLarge) ) );
        addPage( pageItem );
        _pages.append(pageItem);
        connect(hsw, SIGNAL(tabChanged(int)), SLOT(tabChanged(int)));
    }

    connect(this, &KPageDialog::currentPageChanged,
            this, &HighscoresDialog::highscorePageChanged);
    setCurrentPage(_pages[internal->gameType()]);
}

void HighscoresDialog::highscorePageChanged(KPageWidgetItem* page, KPageWidgetItem* pageold)
{
    Q_UNUSED(pageold);
//   kDebug(11001) ;
    int idx = _pages.indexOf( page );
    Q_ASSERT(idx != -1);

    internal->hsConfig().readCurrentConfig();
    uint type = internal->gameType();
    bool several = ( internal->nbGameTypes()>1 );
    if (several)
        internal->setGameType(idx);
    HighscoresWidget *hsw = static_cast<HighscoresWidget*>(page->widget());
    hsw->load(uint(idx)==type ? _rank : -1);
    if (several) setGameType(type);
    hsw->changeTab(_tab);
}

//-----------------------------------------------------------------------------
LastMultipleScoresList::LastMultipleScoresList(
                            const QVector<Score> &scores, QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
//     kDebug(11001) << ": LastMultipleScoresList";

    const ScoreInfos &s = internal->scoreInfos();
    addHeader(s);
    for (int i=0; i<scores.size(); i++) addLine(s, i, false);
}

void LastMultipleScoresList::addLineItem(const ItemArray &si,
                                         uint index, QTreeWidgetItem *line)
{
//   kDebug(11001) ;
    uint k = 1; // skip "id"
    for (int i=0; i<si.size()-2; i++) {
        if ( i==3 ) k = 5; // skip "date"
        const ItemContainer& container = *si[k];
        k++;
        if (line) {
	  line->setText(i, itemText(container, index));
	  line->setTextAlignment(i, container.item()->alignment());
	}
        else {
	    headerItem()->setText(i, container.item()->label() );
            headerItem()->setTextAlignment(i, container.item()->alignment());
        }
    }
}

QString LastMultipleScoresList::itemText(const ItemContainer &item,
                                         uint row) const
{
//   kDebug(11001) ;
    QString name = item.name();
    if ( name==QLatin1String( "rank" ) )
        return (_scores[row].type()==Won ? i18n("Winner") : QString());
    QVariant v = _scores[row].data(name);
    if ( name==QLatin1String( "name" ) ) return v.toString();
    return item.item()->pretty(row, v);
}

//-----------------------------------------------------------------------------
TotalMultipleScoresList::TotalMultipleScoresList(
                            const QVector<Score> &scores, QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
//     kDebug(11001) << ": TotalMultipleScoresList";
    const ScoreInfos &s = internal->scoreInfos();
    addHeader(s);
    for (int i=0; i<scores.size(); i++) addLine(s, i, false);
}

void TotalMultipleScoresList::addLineItem(const ItemArray &si,
                                          uint index, QTreeWidgetItem *line)
{
//   kDebug(11001) ;
    const PlayerInfos &pi = internal->playerInfos();
    uint k = 1; // skip "id"
    for (uint i=0; i<4; i++) { // skip additional fields
        const ItemContainer *container;
        if ( i==2 ) container = pi.item(QLatin1String( "nb games" ));
        else if ( i==3 ) container = pi.item(QLatin1String( "mean score" ));
        else {
            container = si[k];
            k++;
        }

        if (line) {
	  line->setText(i, itemText(*container, index));
	  line->setTextAlignment(i, container->item()->alignment());
	}
        else {
            QString label =
                (i==2 ? i18n("Won Games") : container->item()->label());
	    headerItem()->setText(i, label );
            headerItem()->setTextAlignment(i, container->item()->alignment());
        }
    }
}

QString TotalMultipleScoresList::itemText(const ItemContainer &item,
                                          uint row) const
{
//   kDebug(11001) ;
    QString name = item.name();
    if ( name==QLatin1String( "rank" ) ) return QString::number(_scores.size()-row);
    else if ( name==QLatin1String( "nb games" ) )
        return QString::number( _scores[row].data(QLatin1String( "nb won games" )).toUInt() );
    QVariant v = _scores[row].data(name);
    if ( name==QLatin1String( "name" ) ) return v.toString();
    return item.item()->pretty(row, v);
}

//-----------------------------------------------------------------------------

} // namespace
