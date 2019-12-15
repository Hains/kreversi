/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

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

#ifndef KEXTHIGHSCORE_GUI_H
#define KEXTHIGHSCORE_GUI_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QList>
#include <QTreeWidget>

#include <KPageDialog>

#include "kexthighscore.h"

class QTabWidget;

namespace KExtHighscore
{

class ItemContainer;
class ItemArray;
class Score;
class AdditionalTab;

//-----------------------------------------------------------------------------
class ShowItem : public QTreeWidgetItem
{
 public:
    ShowItem(QTreeWidget *, bool highlight);

 private:
    bool _highlight;
};

class ScoresList : public QTreeWidget
{
 Q_OBJECT
 public:
    explicit ScoresList(QWidget *parent);

    void addHeader(const ItemArray &);

 protected:
    QTreeWidgetItem *addLine(const ItemArray &, uint index, bool highlight);
    virtual QString itemText(const ItemContainer &, uint row) const = 0;

 private:
    virtual void addLineItem(const ItemArray &, uint index,
                             QTreeWidgetItem *item);
};

//-----------------------------------------------------------------------------
class HighscoresList : public ScoresList
{
 Q_OBJECT
 public:
    explicit HighscoresList(QWidget *parent);

    void load(const ItemArray &, int highlight);

 protected:
    QString itemText(const ItemContainer &, uint row) const override;
};

class HighscoresWidget : public QWidget
{
 Q_OBJECT
 public:
    explicit HighscoresWidget(QWidget *parent);

    void load(int rank);

 signals:
    void tabChanged(int i);

 public slots:
    void changeTab(int i);

 private slots:
    void tabChanged() { emit tabChanged(_tw->currentIndex()); }

 private:
    QTabWidget     *_tw = nullptr;
    HighscoresList *_scoresList, *_playersList = nullptr;
    AdditionalTab  *_statsTab, *_histoTab = nullptr;
};

class HighscoresDialog : public KPageDialog
{
 Q_OBJECT
 public:
    HighscoresDialog(int rank, QWidget *parent);

 private slots:
    void tabChanged(int i) { _tab = i; }
    void highscorePageChanged(KPageWidgetItem *newpage, KPageWidgetItem *before);

 private:
    int _rank, _tab;
    QWidget *_current = nullptr;
    QList<KPageWidgetItem*> _pages;
};

//-----------------------------------------------------------------------------
class LastMultipleScoresList : public ScoresList
{
    Q_OBJECT
public:
    LastMultipleScoresList(const QVector<Score> &, QWidget *parent);

private:
    void addLineItem(const ItemArray &, uint index, QTreeWidgetItem *line) override;
    QString itemText(const ItemContainer &, uint row) const override;

private:
    const QVector<Score> &_scores;
};

class TotalMultipleScoresList : public ScoresList
{
    Q_OBJECT
public:
    TotalMultipleScoresList(const QVector<Score> &, QWidget *parent);

private:
    void addLineItem(const ItemArray &, uint index, QTreeWidgetItem *line) override;
    QString itemText(const ItemContainer &, uint row) const override;

private:
    const QVector<Score> &_scores;
};

//-----------------------------------------------------------------------------

} // namespace

#endif
