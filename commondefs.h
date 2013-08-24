/*******************************************************************
 *
 * Copyright 2006 Dmitry Suzdalev <dimsuz@gmail.com>
 * Copyright 2013 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This file is part of the KDE project "KReversi"
 *
 * KReversi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KReversi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KReversi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 ********************************************************************/
#ifndef COMMONDEFS_H
#define COMMONDEFS_H

// noColor = empty
enum ChipColor {White = 0, Black = 1, NoColor = 2};

struct KReversiPos {
    KReversiPos(int r = -1, int c = -1)
        : row(r), col(c) { }

    int row;
    int col;

    bool isValid() const {
        return (row >= 0 && col >= 0 && row < 8 && col < 8);
    }
};

struct KReversiMove: public KReversiPos {
    KReversiMove(ChipColor col = NoColor, int r = -1, int c = -1)
        : KReversiPos(r, c), color(col) { }

    KReversiMove(ChipColor col, const KReversiPos &pos)
        : KReversiPos(pos), color(col) { }

    ChipColor color;

    bool isValid() const {
        return (color != NoColor
                && row >= 0 && col >= 0
                && row < 8 && col < 8);
    }
};

static ChipColor opponentColorFor(ChipColor color)
{
    if (color == NoColor)
        return NoColor;
    else
        return (color == White ? Black : White);
}

typedef QList<KReversiMove> MoveList;

#endif
