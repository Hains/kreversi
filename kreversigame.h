/*******************************************************************
 *
 * Copyright 2006 Dmitry Suzdalev <dimsuz@gmail.com>
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

// Qt
#include <QObject>
#include <QStack>

// KReversi
#include "commondefs.h"
#include "reversi.h"

class Engine;

using namespace Reversi;

/**
 *  KReversiGame incapsulates all of the game logic.
 *  Whenever the board state changes it emits corresponding signals.
 *  The idea is also to abstract from any graphic representation of the game process
 *
 *  KReversiGame is supposed to be driven by someone from outside.
 *  I.e. it receives commands and emits events when it's internal state changes
 *  due to this commands dispatching.
 *  The main commands are:
 *  startNextTurn() and  makePlayerMove()
 *
 *  See KReversiView for example of working with KReversiGame
 */
class KReversiGame : public QObject
{
    Q_OBJECT
public:
    KReversiGame();
    ~KReversiGame();
    /**
     *  Starts next player turn.
     *  If game isn't over yet, then this function does the following:
     *  - if it is computer turn and computer can move, it'll make that move.
     *  - if it is computer turn and computer can't move it'll emit "computerCantMove"
     *  signal and exit
     *  - if it is player turn and player can move then this function
     *  will do nothing - you can call makePlayerMove(row,col) to make player move (but see last item)
     *  - if it is player turn and player can't move it'll make a computer move
     *  - in demo mode this function will make computer play player moves,
     *  so you don't need to call makePlayerMove.
     *
     *  If game is over it'll emit gameOver()
     *
     *  If it's still unclear how to use it please see KReversiView for working example.
     *  In short: KReversiView calls startNextTurn() at the end of each turn and makePlayerMove()
     *  in onPlayerMove()
     *
     *  @param demoMode if @c true then computer will decide for player turn
     */
    void startNextTurn(bool demoMode);
    /**
     *  This will make the player move at row, col.
     *  If that is possible of course
     *  If demoMode is true, the computer will decide on what move to take.
     *  row and col values do not matter in that case.
     */
    void makePlayerMove(int row, int col, bool demoMode);
    /**
     *  This function will make computer decide where he
     *  wants to put his chip... and he'll put it there!
     */
    void makeComputerMove();
    /**
     *  Undoes all the computer moves and one player move
     *  (so after calling this function it will be player turn)
     *  @return number of undone moves
     */
    int undo();
    /**
     *  Sets the computer skill level. From 1 to 7
     */
    void setComputerSkill(int skill);
    /**
     * @return whether the game is currently computing turn
     */
    bool isThinking() const;
    /**
     *  @return whether the game is already over
     */
    bool isGameOver() const; // perhaps this doesn't need to be public
    /**
     *  @return whether any player move is at all possible
     */
    bool isAnyPlayerMovePossible() const;// perhaps this doesn't need to be public
    /**
     *  @return whether any computer move is at all possible
     */
    bool isAnyComputerMovePossible() const;// perhaps this doesn't need to be public
    /**
     *  @return a color of the current player
     */
    //Color currentPlayer() const { return m_curPlayer; }
    Color currentPlayer() const { return m_position.toMove(); }

    /**
     *  @return score (number of chips) of the player
     */
    int playerScore(Color player) const;
    /**
     *  @return color of the chip at position [row, col]
     */
    Color chipColorAt(int row, int col) const;
    /**
     *  @return if undo is possible
     */
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    /**
     *  @return a hint to current player
     */
    KReversiPos getHint() const;
    /**
     *  @return last move made
     */
    KReversiPos getLastMove() const;
    /**
     *  @return true, if it's computer's turn now
     */
    //bool isComputersTurn() const { return m_curPlayer == m_computerColor; }
    bool isComputersTurn() const { return m_position.toMove() == m_computerColor; }
    /**
     *  @return a list of chips which were changed during last move.
     *  First of them will be the move itself, and the rest - chips which
     *  were turned by that move
     */
    PosList changedChips() const;
    /**
     *  @return a list of possible moves for current player
     */
    PosList possibleMoves() const;

signals:
    void gameOver();
    void boardChanged();
    void moveFinished();
    void computerCantMove();
    void playerCantMove();

private:
    enum Direction { Up, Down, Right, Left, UpLeft, UpRight, DownLeft, DownRight };
    /**
     * This function will tell you if the move is possible.
     * That's why it was given such a name ;)
     */
    bool isMovePossible(const KReversiPos& move) const;
    /**
     *  Searches for "chunk" in direction dir for move.
     *  As my English-skills are somewhat limited, let me introduce
     *  new terminology ;).
     *  I'll define a "chunk" of chips for color "C" as follows:
     *  (let "O" be the color of the opponent for color "C")
     *  CO[O]C <-- this is a chunk
     *  where [O] is one or more opponent's pieces
     */
    bool hasChunk( Direction dir, const KReversiPos& move) const;
    /**
     *  Performs move, i.e. marks all the chips that player wins with
     *  this move with current player color
     */
    void makeMove(const KReversiPos& move, PosList &changedChips);
    /**
     *  Sets the type of chip at (row,col)
     */
    void setColor(Color type, int row, int col);

    // The actual game (this is the part that will be switched to the new code).
    /**
     *  The board itself
     */
    Color m_cells[8][8];
    /**
     *  Score of each player
     */
    int m_score[2];
    /**
     *  Color of the current player
     */
    Color m_curPlayer;

    // Values from the UI
    /**
     *  The color of the human played chips
     */
    Color m_playerColor;
    /**
     *  The color of the computer played chips
     */
    Color m_computerColor;

    /**
     *  Our AI
     */
    Engine *m_engine;

    /**
     *  This is an undo stack.
     *  It contains a lists of chips changed with each turn.
     *  @see m_changedChips
     */
    QStack<PosList> m_undoStack;

    // ----------------------------------------------------------------
    // New code

    /**
     * The current position of the game.
     */
    Position  m_position;

    QStack<Move> m_newUndoStack;

    // Checks if the board in m_cells is equal to the board in m_position
    void checkBoard();
};

#endif
