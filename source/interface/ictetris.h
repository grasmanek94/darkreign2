/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999 Matthew Versluys
//
// Tetris Control
//
// 15-JUL-1999
//


#ifndef __ICTETRIS_H
#define __ICTETRIS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Button messages
//
namespace ICTetrisMsg
{
  const U32 Left          = 0xB4BA5EF6; // "Tetris::Message::Left"
  const U32 Right         = 0x7D0CE221; // "Tetris::Message::Right"
  const U32 Drop          = 0x1E84BD4B; // "Tetris::Message::Drop"
  const U32 Clockwise     = 0x34B29F4E; // "Tetris::Message::Clockwise"
  const U32 AntiClockwise = 0xD3BC30D0; // "Tetris::Message::AntiClockwise"

  const U32 Reset         = 0xFF8EEF65; // "Tetris::Message::Reset"
}

namespace ICTetrisNotify
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTetris - A Tetris Control
//
class ICTetris : public IControl
{
  PROMOTE_LINK(ICTetris, IControl, 0x4C3EF701) // "ICTetris"

private:

  struct Rotation;
  struct Piece;

  friend Rotation;
  friend Piece;

  // The pit
  U32 pitMask[25];
  Color pit[20][10];

  // Block texture
  TextureInfo blockTexture; 

  // The pieces
  List<Piece> pieces;

  // Current piece
  Piece *current;

  // Next piece
  Piece *next;

  // Location of the current piece
  Point<S32> currentLocation;

  // Pit offset
  Point<S32> pitOffset;

  // Next offset
  Point<S32> nextOffset;

  // Bonuses
  U32 bonus[4];

  // Current rotation
  Rotation *rotation;

  // Score
  IFaceVar *score;

  // Lines
  IFaceVar *lines;

  // Drop speed
  IFaceVar *speed;

  // Lines which are being collected
  S32 collect[4];

  // Current game state
  enum
  {
    START,
    DROP,
    COLLECT_MARK,
    COLLECT_DROP,
    BONUS
  } state;

  // General purpose timer
  U32 timer;

protected:

  // Draw this control into the bitmap
  void DrawSelf(PaintInfo &pi);

public:

  ICTetris(IControl *parent);
  ~ICTetris();

  // Activate the control
  Bool Activate();
  Bool Deactivate();

  // Configure this control with an FScope
  void Setup(FScope *fScope);

  // Event handling
  U32 HandleEvent(Event &e);

  // Process
  void Poll();

private:

  // Reset a game
  void Reset();

  // Get the next piece
  void NextPiece();

  // Test a rotation in the pit
  Bool TestRotation(const Rotation &rotation, const Point<S32> &location);

  // Split a piece in the pit
  void SplitPiece(const Piece &piece, const Rotation &rotation, const Point<S32> &location);

  // Test a row of the pit
  Bool TestRow(S32 row);

  // Remove a row of the pit
  void RemoveRow(S32 row);

  // Empty the pit
  void EmptyPit();

  // Clear everything
  void Clear();

};

#endif
