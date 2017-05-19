/*
    This file is part of the KDE games library
    Copyright (C) 2001-2004 Nicolas Hadacek (hadacek@kde.org)

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


#include <QFile>
#include <QLayout>
#include <qdom.h>
#include <QTextStream>
#include <QVector>
#include <QCryptographicHash>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <kuser.h>
#include <kmessagebox.h>
#include <kcodecs.h>

#include "kexthighscore.h"
#include "kexthighscore_internal.h"
#include "kexthighscore_gui.h"


// TODO Decide if want to support
// a build time HIGHSCORE_DIRECTORY or not
// #include <config-highscore.h>

namespace KExtHighscore
{

//-----------------------------------------------------------------------------
const char ItemContainer::ANONYMOUS[] = "_";
const char ItemContainer::ANONYMOUS_LABEL[] = I18N_NOOP("anonymous");

ItemContainer::ItemContainer()
    : _item(0)
{}

ItemContainer::~ItemContainer()
{
    delete _item;
}

void ItemContainer::setItem(Item *item)
{
    delete _item;
    _item = item;
}

QString ItemContainer::entryName() const
{
    if ( _subGroup.isEmpty() ) return _name;
    return _name + QLatin1Char( '_' ) + _subGroup;
}

QVariant ItemContainer::read(uint i) const
{
    Q_ASSERT(_item);

    QVariant v = _item->defaultValue();
    if ( isStored() ) {
        internal->hsConfig().setHighscoreGroup(_group);
        v = internal->hsConfig().readPropertyEntry(i+1, entryName(), v);
    }
    return _item->read(i, v);
}

QString ItemContainer::pretty(uint i) const
{
    Q_ASSERT(_item);
    return _item->pretty(i, read(i));
}

void ItemContainer::write(uint i, const QVariant &value) const
{
    Q_ASSERT( isStored() );
    Q_ASSERT( internal->hsConfig().isLocked() );
    internal->hsConfig().setHighscoreGroup(_group);
    internal->hsConfig().writeEntry(i+1, entryName(), value);
}

uint ItemContainer::increment(uint i) const
{
    uint v = read(i).toUInt() + 1;
    write(i, v);
    return v;
}

//-----------------------------------------------------------------------------
ItemArray::ItemArray()
    : _group(QLatin1String( "" )), _subGroup(QLatin1String( "" )) // no null groups
{}

ItemArray::~ItemArray()
{
    for (int i=0; i<size(); i++) delete at(i);
}

int ItemArray::findIndex(const QString &name) const
{
    for (int i=0; i<size(); i++)
        if ( at(i)->name()==name ) return i;
    return -1;
}

const ItemContainer *ItemArray::item(const QString &name) const
{
    QLoggingCategory::setFilterRules(QStringLiteral("games.highscore.debug = true"));
    
    int i = findIndex(name);
    if ( i==-1 ) qCCritical(GAMES_EXTHIGHSCORE) << "no item named \"" << name
                                << "\"";
    return at(i);
}

ItemContainer *ItemArray::item(const QString &name)
{
    QLoggingCategory::setFilterRules(QStringLiteral("games.highscore.debug = true"));
  
    int i = findIndex(name);
    if ( i==-1 ) qCCritical(GAMES_EXTHIGHSCORE) << "no item named \"" << name
                                << "\"";
    return at(i);
}

void ItemArray::setItem(const QString &name, Item *item)
{
    QLoggingCategory::setFilterRules(QStringLiteral("games.highscore.debug = true"));
  
    int i = findIndex(name);
    if ( i==-1 ) qCCritical(GAMES_EXTHIGHSCORE) << "no item named \"" << name
                                << "\"";
    bool stored = at(i)->isStored();
    bool canHaveSubGroup = at(i)->canHaveSubGroup();
    _setItem(i, name, item, stored, canHaveSubGroup);
}

void ItemArray::addItem(const QString &name, Item *item,
                        bool stored, bool canHaveSubGroup)
{
    QLoggingCategory::setFilterRules(QStringLiteral("games.highscore.debug = true"));
    
    if ( findIndex(name)!=-1 )
        qCCritical(GAMES_EXTHIGHSCORE) << "item already exists \"" << name << "\"";

    append(new ItemContainer);
    //at(i) = new ItemContainer;
    _setItem(size()-1, name, item, stored, canHaveSubGroup);
}

void ItemArray::_setItem(uint i, const QString &name, Item *item,
                         bool stored, bool canHaveSubGroup)
{
    at(i)->setItem(item);
    at(i)->setName(name);
    at(i)->setGroup(stored ? _group : QString());
    at(i)->setSubGroup(canHaveSubGroup ? _subGroup : QString());
}

void ItemArray::setGroup(const QString &group)
{
    Q_ASSERT( !group.isNull() );
    _group = group;
    for (int i=0; i<size(); i++)
        if ( at(i)->isStored() ) at(i)->setGroup(group);
}

void ItemArray::setSubGroup(const QString &subGroup)
{
    Q_ASSERT( !subGroup.isNull() );
    _subGroup = subGroup;
    for (int i=0; i<size(); i++)
        if ( at(i)->canHaveSubGroup() ) at(i)->setSubGroup(subGroup);
}

void ItemArray::read(uint k, Score &data) const
{
    for (int i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        data.setData(at(i)->name(), at(i)->read(k));
    }
}

void ItemArray::write(uint k, const Score &data, uint nb) const
{
    for (int i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        for (uint j=nb-1; j>k; j--)  at(i)->write(j, at(i)->read(j-1));
        at(i)->write(k, data.data(at(i)->name()));
    }
}

void ItemArray::exportToText(QTextStream &s) const
{
    for (uint k=0; k<nbEntries()+1; k++) {
        for (int i=0; i<size(); i++) {
            const Item *item = at(i)->item();
            if ( item->isVisible() ) {
                if ( i!=0 ) s << '\t';
                if ( k==0 ) s << item->label();
                else s << at(i)->pretty(k-1);
            }
        }
        s << endl;
    }
}

//-----------------------------------------------------------------------------
class ScoreNameItem : public NameItem
{
 public:
    ScoreNameItem(const ScoreInfos &score, const PlayerInfos &infos)
        : _score(score), _infos(infos) {}

    QString pretty(uint i, const QVariant &v) const {
        uint id = _score.item(QStringLiteral( "id" ))->read(i).toUInt();
        if ( id==0 ) return NameItem::pretty(i, v);
        return _infos.prettyName(id-1);
    }

 private:
    const ScoreInfos  &_score;
    const PlayerInfos &_infos;
};

//-----------------------------------------------------------------------------
ScoreInfos::ScoreInfos(uint maxNbEntries, const PlayerInfos &infos)
    : _maxNbEntries(maxNbEntries)
{
    addItem(QStringLiteral( "id" ), new Item((uint)0));
    addItem(QStringLiteral( "rank" ), new RankItem, false);
    addItem(QStringLiteral( "name" ), new ScoreNameItem(*this, infos));
    addItem(QStringLiteral( "score" ), Manager::createItem(Manager::ScoreDefault));
    addItem(QStringLiteral( "date" ), new DateItem);
}

uint ScoreInfos::nbEntries() const
{
    uint i = 0;
    for (; i<_maxNbEntries; i++)
        if ( item(QStringLiteral( "score" ))->read(i)==item(QStringLiteral( "score" ))->item()->defaultValue() )
            break;
    return i;
}

//-----------------------------------------------------------------------------
const char *HS_ID              = "player id";

PlayerInfos::PlayerInfos()
{
    setGroup(QStringLiteral( "players" ));

    // standard items
    addItem(QStringLiteral( "name" ), new NameItem);
    Item *it = new Item((uint)0, i18n("Games Count"),Qt::AlignRight);
    addItem(QStringLiteral( "nb games" ), it, true, true);
    it = Manager::createItem(Manager::MeanScoreDefault);
    addItem(QStringLiteral( "mean score" ), it, true, true);
    it = Manager::createItem(Manager::BestScoreDefault);
    addItem(QStringLiteral( "best score" ), it, true, true);
    addItem(QStringLiteral( "date" ), new DateItem, true, true);
    it = new Item(QString(), i18n("Comment"), Qt::AlignLeft);
    addItem(QStringLiteral( "comment" ), it);

    // statistics items
    addItem(QStringLiteral( "nb black marks" ), new Item((uint)0), true, true); // legacy
    addItem(QStringLiteral( "nb lost games" ), new Item((uint)0), true, true);
    addItem(QStringLiteral( "nb draw games" ), new Item((uint)0), true, true);
    addItem(QStringLiteral( "current trend" ), new Item((int)0), true, true);
    addItem(QStringLiteral( "max lost trend" ), new Item((uint)0), true, true);
    addItem(QStringLiteral( "max won trend" ), new Item((uint)0), true, true);

	currentPlayerName();

}

void PlayerInfos::currentPlayerName()
{

	QString username = KUser().loginName();

	internal->hsConfig().setHighscoreGroup(QStringLiteral("players"));
	for (uint i=0; ;i++) {
		if ( !internal->hsConfig().hasEntry(i+1, QStringLiteral("name")) ) {
			_newPlayer = true;
			_id = i;
			break;
		}
		if ( internal->hsConfig().readEntry(i+1, QStringLiteral("name"))==username ) {
			_newPlayer = false;
			_id = i;
			return;
		}
}

    ConfigGroup cg;
    _oldLocalPlayer = cg.hasKey(HS_ID);
    _oldLocalId = cg.readEntry(HS_ID).toUInt();   
  
     
        cg.writeEntry(HS_ID, _id);
        item(QStringLiteral( "name" ))->write(_id, username);     

    _bound = true;
    internal->hsConfig().writeAndUnlock();
}

void PlayerInfos::createHistoItems(const QVector<uint> &scores, bool bound)
{
    Q_ASSERT( _histogram.size()==0 );
    _bound = bound;
    _histogram = scores;
    for (int i=1; i<histoSize(); i++)
        addItem(histoName(i), new Item((uint)0), true, true);
}

uint PlayerInfos::nbEntries() const
{
    internal->hsConfig().setHighscoreGroup(QStringLiteral( "players" ));
    const QStringList list = internal->hsConfig().readList(QStringLiteral( "name" ), -1);
    return list.count();
}

QString PlayerInfos::histoName(int i) const
{
    const QVector<uint> &sh = _histogram;
    Q_ASSERT( i<sh.size() || (_bound || i==sh.size()) );
    if ( i==sh.size() )
        return QStringLiteral( "nb scores greater than %1").arg(sh[sh.size()-1]);
    return QStringLiteral( "nb scores less than %1").arg(sh[i]);
}

int PlayerInfos::histoSize() const
{
     return _histogram.size() + (_bound ? 0 : 1);
}

void PlayerInfos::submitScore(const Score &score) const
{
    // update counts
    uint nbGames = item(QStringLiteral( "nb games" ))->increment(_id);
    switch (score.type()) {
    case Lost:
        item(QStringLiteral( "nb lost games" ))->increment(_id);
        break;
    case Won: break;
    case Draw:
        item(QStringLiteral( "nb draw games" ))->increment(_id);
        break;
    };

    // update mean
    if ( score.type()==Won ) {
        uint nbWonGames = nbGames - item(QStringLiteral( "nb lost games" ))->read(_id).toUInt()
                        - item(QStringLiteral( "nb draw games" ))->read(_id).toUInt()
                        - item(QStringLiteral( "nb black marks" ))->read(_id).toUInt(); // legacy
        double mean = (nbWonGames==1 ? 0.0
                       : item(QStringLiteral( "mean score" ))->read(_id).toDouble());
        mean += (double(score.score()) - mean) / nbWonGames;
        item(QStringLiteral( "mean score" ))->write(_id, mean);
    }

    // update best score
    Score best = score; // copy optional fields (there are not taken into account here)
    best.setScore( item(QStringLiteral( "best score" ))->read(_id).toUInt() );
    if ( best<score ) {
        item(QStringLiteral( "best score" ))->write(_id, score.score());
        item(QStringLiteral( "date" ))->write(_id, score.data(QStringLiteral( "date" )).toDateTime());
    }

    // update trends
    int current = item(QStringLiteral( "current trend" ))->read(_id).toInt();
    switch (score.type()) {
    case Won: {
        if ( current<0 ) current = 0;
        current++;
        uint won = item(QStringLiteral( "max won trend" ))->read(_id).toUInt();
        if ( (uint)current>won ) item(QStringLiteral( "max won trend" ))->write(_id, current);
        break;
    }
    case Lost: {
        if ( current>0 ) current = 0;
        current--;
        uint lost = item(QStringLiteral( "max lost trend" ))->read(_id).toUInt();
        uint clost = -current;
        if ( clost>lost ) item(QStringLiteral( "max lost trend" ))->write(_id, clost);
        break;
    }
    case Draw:
        current = 0;
        break;
    }
    item(QStringLiteral( "current trend" ))->write(_id, current);

    // update histogram
    if ( score.type()==Won ) {
        const QVector<uint> &sh = _histogram;
        for (int i=1; i<histoSize(); i++)
            if ( i==sh.size() || score.score()<sh[i] ) {
                item(histoName(i))->increment(_id);
                break;
            }
    }
}

bool PlayerInfos::isNameUsed(const QString &newName) const
{
    if ( newName==name() ) return false; // own name...
    for (uint i=0; i<nbEntries(); i++)
        if ( newName.toLower()==item(QStringLiteral( "name" ))->read(i).toString().toLower() ) return true;
    if ( newName==i18n(ItemContainer::ANONYMOUS_LABEL) ) return true;
    return false;
}

//-----------------------------------------------------------------------------
ManagerPrivate::ManagerPrivate(uint nbGameTypes, Manager &m)
    : manager(m), showStatistics(false), showDrawGames(false),
      trackLostGames(false), trackDrawGames(false),
      showMode(Manager::ShowForHigherScore),
      _first(true), _nbGameTypes(nbGameTypes), _gameType(0)
{}

void ManagerPrivate::init(uint maxNbEntries)
{
    _hsConfig = new KHighscore(false, 0);
    _playerInfos = new PlayerInfos;
    _scoreInfos = new ScoreInfos(maxNbEntries, *_playerInfos);
}

ManagerPrivate::~ManagerPrivate()
{
    delete _scoreInfos;
    delete _playerInfos;
    delete _hsConfig;
}

Score ManagerPrivate::readScore(uint i) const
{
    Score score(Won);
    _scoreInfos->read(i, score);
    return score;
}

int ManagerPrivate::rank(const Score &score) const
{
    uint nb = _scoreInfos->nbEntries();
    uint i = 0;
	for (; i<nb; i++)
        if ( readScore(i)<score ) break;
	return (i<_scoreInfos->maxNbEntries() ? (int)i : -1);
}

void ManagerPrivate::convertToGlobal()
{
    // read old highscores
    KHighscore *tmp = _hsConfig;
    _hsConfig = new KHighscore(true, 0);
    QVector<Score> scores(_scoreInfos->nbEntries());
    for (int i=0; i<scores.count(); i++)
        scores[i] = readScore(i);

    // commit them
    delete _hsConfig;
    _hsConfig = tmp;
    _hsConfig->lockForWriting();
    for (int i=0; i<scores.count(); i++)
        if ( scores[i].data(QStringLiteral( "id" )).toUInt()==_playerInfos->oldLocalId()+1 )
            submitLocal(scores[i]);
    _hsConfig->writeAndUnlock();
}

void ManagerPrivate::setGameType(uint type)
{
    if (_first) {
        _first = false;
        if ( _playerInfos->isNewPlayer() ) {
            // convert legacy highscores
            for (uint i=0; i<_nbGameTypes; i++) {
                setGameType(i);
                manager.convertLegacy(i);
            }

#ifdef HIGHSCORE_DIRECTORY
            if ( _playerInfos->isOldLocalPlayer() ) {
                // convert local to global highscores
                for (uint i=0; i<_nbGameTypes; i++) {
                    setGameType(i);
                    convertToGlobal();
                }
            }
#endif
        }
    }

    Q_ASSERT( type<_nbGameTypes );
    _gameType = qMin(type, _nbGameTypes-1);
    QString str = QStringLiteral( "scores" );
    QString lab = manager.gameTypeLabel(_gameType, Manager::Standard);
    if ( !lab.isEmpty() ) {
        _playerInfos->setSubGroup(lab);
        str += QLatin1Char( '_' ) + lab;
    }
    _scoreInfos->setGroup(str);
}

void ManagerPrivate::checkFirst()
{
    if (_first) setGameType(0);
}

int ManagerPrivate::submitScore(const Score &ascore,
                                QWidget *widget)
{
    checkFirst();

    Score score = ascore;
    score.setData(QStringLiteral( "id" ), _playerInfos->id() + 1);
    score.setData(QStringLiteral( "date" ), QDateTime::currentDateTime());

    int rank = -1;
    if ( _hsConfig->lockForWriting(widget) ) { // no GUI when locking

        // commit locally
        _playerInfos->submitScore(score);
        if ( score.type()==Won ) rank = submitLocal(score);
        _hsConfig->writeAndUnlock();
    }
    return rank;
}

int ManagerPrivate::submitLocal(const Score &score)
{
    int r = rank(score);
    if ( r!=-1 ) {
        uint nb = _scoreInfos->nbEntries();
        if ( nb<_scoreInfos->maxNbEntries() ) nb++;
        _scoreInfos->write(r, score, nb);
    }
    return r;
}

void ManagerPrivate::exportHighscores(QTextStream &s)
{
    uint tmp = _gameType;

    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        if ( _nbGameTypes>1 ) {
            if ( i!=0 ) s << endl;
            s << "--------------------------------" << endl;
            s << "Game type: "
              << manager.gameTypeLabel(_gameType, Manager::I18N) << endl;
            s << endl;
        }
        s << "Players list:" << endl;
        _playerInfos->exportToText(s);
        s << endl;
        s << "Highscores list:" << endl;
        _scoreInfos->exportToText(s);
    }

    setGameType(tmp);
}

} // namespace
