/*******************************************************************
 *
 * Copyright 2006 Dmitry Suzdalev <dimsuz@gmail.com>
 * Copyright 2013 Denis Kuplaykov <dener.kup@gmail.com>
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
#ifndef KREVERSI_GAME_H
#define KREVERSI_GAME_H

#include <QObject>
#include <QStack>
#include <QTimer>

#include <Engine.h>
class Engine;
#include <commondefs.h>
#include <kreversiplayer.h>
class KReversiPlayer;

/**
 *  KReversiGame incapsulates all of the game logic.
 *  Whenever the board state changes it emits corresponding signals.
 *  The idea is also to abstract from any graphic representation of the game
 *  process
 *
 *  KReversiGame receives signals from KReversiPlayer classes.
 *  I.e. it receives commands and emits events when it's internal state changes
 *  due to this commands dispatching.
 *
 *  See KReversiView, KReversiPlayer, KReversiHumanPlayer
 *  and KReversiComputerPlayer for example of working with KReversiGame
 *
 *  @see KReversiView, KReversiPlayer, KReversiHumanPlayer
 */
class KReversiGame : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs game with two specified players.
     */
    KReversiGame(KReversiPlayer *blackPlayer, KReversiPlayer *whitePlayer);
    ~KReversiGame();
    /**
     *  @return if undo is possible
     */
    bool canUndo() const;
    /**
     *  Undoes all the opponent of current player moves and one his move
     *  (so after calling this function it will be still current player turn)
     *  @return number of undone moves
     */
    int undo();
    /**
     *  @return score (number of chips) of the @p player
     */
    int playerScore(ChipColor player) const;
    /**
     *  @return color of the chip at position @p pos
     */
    ChipColor chipColorAt(KReversiPos pos) const;
    /**
     *  @return a hint to current player
     */
    KReversiMove getHint() const;
    /**
     *  @return last move made
     */
    KReversiMove getLastMove() const;
    /**
     *  @return a list of chips which were changed during last move.
     *  First of them will be the move itself, and the rest - chips which
     *  were turned by that move
     */
    MoveList changedChips() const {
        return m_changedChips;
    }
    /**
     *  @return a list of possible moves for current player
     */
    MoveList possibleMoves() const;
    /**
     *  @return whether the game is already over
     */
    bool isGameOver() const;
    /**
     *  @return whether any player move is at all possible
     */
    bool isAnyPlayerMovePossible(ChipColor player) const;
    /**
     *  @return a color of the current player
     */
    ChipColor currentPlayer() const {
        return m_curPlayer;
    }
    /**
     *  Sets animation times from players to @p delay milliseconds
     */
    void setDelay(int delay);

    /**
     *  @return History of moves as MoveList
     */
    MoveList getHistory() const;

    /**
     *  @return Is hint allowed for current player
     */
    bool isHintAllowed() const;
private slots:
    /**
     *  Starts next player's turn.
     */
    void startNextTurn();
    /**
     *  Slot used to handle moves from black player
     *  @param move Move of black player
     */
    void blackPlayerMove(KReversiMove move);
    /**
     *  Slot used to handle moves from white player
     *  @param move Move of white player
     */
    void whitePlayerMove(KReversiMove move);
    /**
     *  Slot to handle end of animations with m_delayTimer
     */
    void onDelayTimer();
    /**
     *  Slot to handle ready-status of black player
     */
    void blackReady();
    /**
     *  Slot to handle ready-status of white player
     */
    void whiteReady();

signals:
    void gameOver();
    void boardChanged();
    void moveFinished();
    void whitePlayerCantMove();
    void blackPlayerCantMove();
    void whitePlayerTurn();
    void blackPlayerTurn();
private:
    // predefined direction arrays for easy implementation
    static const int DIRECTIONS_COUNT = 8;
    static const int DX[];
    static const int DY[];
    /**
     *  Used to make player think about his move again after unpossible move
     */
    void kickCurrentPlayer();
    /**
     *  This will make the player @p move
     *  If that is possible, of course
     */
    void makeMove(const KReversiMove &move);
    /**
     * This function will tell you if the move is possible.
     */
    bool isMovePossible(const KReversiMove &move) const;
    /**
     *  Searches for "chunk" in direction @p dirNum for @p move.
     *  As my English-skills are somewhat limited, let me introduce
     *  new terminology ;).
     *  I'll define a "chunk" of chips for color "C" as follows:
     *  (let "O" be the color of the opponent for color "C")
     *  CO[O]C <-- this is a chunk
     *  where [O] is one or more opponent's pieces
     */
    bool hasChunk(int dirNum, const KReversiMove &move) const;
    /**
     *  Performs @p move, i.e. marks all the chips that player wins with
     *  this move with current player color
     */
    void turnChips(const KReversiMove &move);
    /**
     *  Sets the type of chip according to @p move
     */
    void setChipColor(const KReversiMove &move);
    /**
     *  Delay time
     */
    int m_delay;
    /**
     *  Status flags used to know when both players are ready
     */
    bool m_isReady[2];
    /**
     *  Last player who has made a move. Cannot be NoColor after the first move
     */
    ChipColor m_lastPlayer;
    /**
     *  The board itself
     */
    ChipColor m_cells[8][8];
    /**
     *  Score of each player
     */
    int m_score[2];
    /**
     *  AI to give hints
     */
    Engine *m_engine;
    /**
     *  Color of the current player.
     *  @c NoColor if it is interchange for animations
     */
    ChipColor m_curPlayer;
    // Well I'm not brief at all :). That's because I think that my
    // English is not well shaped sometimes, so I try to describe things
    // so that me and others can understand. Even simple things.
    // Specially when I think that my description sucks :)
    /**
     *  This list holds chips that were changed/added during last move
     *  First of them will be the chip added to the board by the player
     *  during last move. The rest of them - chips that were turned by that
     *  move.
     */
    MoveList m_changedChips;
    /**
     *  This is an undo stack.
     *  It contains a lists of chips changed with each turn.
     *  @see m_changedChips
     */
    QStack<MoveList> m_undoStack;
    /**
     *  Used to handle end of player's animations or other stuff
     */
    QTimer m_delayTimer;

    /**
     *  Actual players, who play the game
     */
    KReversiPlayer *m_player[2];
};
#endif
