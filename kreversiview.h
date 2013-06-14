/*
    Copyright 2013 Denis Kuplyakov <dener.kup@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KREVERSI_VIEW_H
#define KREVERSI_VIEW_H

#include <KgDeclarativeView>
#include "kreversigame.h"

#include <KGameRenderer>
#include <KStandardDirs>
#include <KgThemeProvider>

class KReversiView : public KgDeclarativeView
{
    Q_OBJECT
public:
    KReversiView(KReversiGame* game, QWidget *parent = 0);
    void setGame(KReversiGame* game);
private slots:
    void onGameUpdate();
    void onPlayerMove(int row, int col);
private:
    KReversiGame* m_game;
    QObject *m_qml_root;
    KgThemeProvider *m_provider;
};
#endif
