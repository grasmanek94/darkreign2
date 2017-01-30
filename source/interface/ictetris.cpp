/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999 Matthew Versluys
//
// Tetris Control
//
// 15-JUL-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ictetris.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "stdload.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Struct ICTetris::Rotation
//
struct ICTetris::Rotation
{
  // Mask
  U32 mask[4];

  // Texture
  TextureInfo texture;

  // List node
  NList<Rotation>::Node node;

  // Constructor
  Rotation(FScope *fScope);

};


///////////////////////////////////////////////////////////////////////////////
//
// Struct ICTetris::Piece
//
struct ICTetris::Piece
{
  // List of rotations
  NList<Rotation> rotations;

  // Piece color
  Color color;

  // Constructor and Destructor
  Piece(FScope *fScope);
  ~Piece();

};


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTetris - Standard button control
//


//
// ICTetris::ICTetris
//
ICTetris::ICTetris(IControl *parent)
: IControl(parent),
  current(NULL),
  next(NULL),
  rotation(NULL),
  state(DROP),
  timer(0)
{
  // Set the poll interval to 20x per second
  SetPollInterval(50);

  // Set default bonuses
	bonus[0] = 50;
	bonus[1] = 150;
	bonus[2] = 450;
	bonus[3] = 900;

  score = new IFaceVar(this, CreateInteger("Score", 0));
  lines = new IFaceVar(this, CreateInteger("Lines", 0));
  speed = new IFaceVar(this, CreateInteger("Speed", 0));
}


//
// ICTetris::~ICTetris
//
ICTetris::~ICTetris()
{
  // Clean up all of the pieces
  pieces.DisposeAll();

  // Delete vars
  delete score;
  delete lines;
  delete speed;
}


//
// ICTetris::DrawSelf
//
// Draw the button control
//
void ICTetris::DrawSelf(PaintInfo &pi)
{
  // Fill the background
  DrawCtrlBackground(pi, GetTexture());

  // Draw the frame
  DrawCtrlFrame(pi);

  // Draw the pit
  S32 x, y;

  for (y = 0; y < 20; y++)
  {
    for (x = 0; x < 10; x++)
    {
      if (pitMask[y + 2] & 0x00100000 >> x)
      {
        ClipRect c;
        c.p0 = pi.client.p0 + pitOffset + Point<S32>(x * 16, (y + 2) * 16);
        c.p1 = c.p0 + Point<S32>(16, 16);

        // Render the block texture at this location 
        // using the appropriate pit color
        IFace::RenderRectangle(c, pit[y][x], &blockTexture);
      }
    }
  }

  switch (state)
  {
    case DROP:
      ClipRect c;

      // Draw the current piece at the current location
      c.p0 = pi.client.p0 + pitOffset + Point<S32>((currentLocation.x - 11) * 16, currentLocation.y * 16);
      c.p1 = c.p0 + Point<S32>(64, 64);
      IFace::RenderRectangle(c, current->color, &rotation->texture);

      // Draw the next piece at the next location
      c.p0 = pi.client.p0 + nextOffset;
      c.p1 = c.p0 + Point<S32>(64, 64);
      IFace::RenderRectangle(c, next->color, &next->rotations.GetHead()->texture);
      break;
  }

}


//
// ICTetris::Activate
//
Bool ICTetris::Activate()
{
  if (IControl::Activate())
  {
    ActivateVar(score);
    ActivateVar(lines);
    ActivateVar(speed);

    // Clear everything
    Clear();

    return (TRUE);
  }

  return (FALSE);
}


//
// ICTetris::Deactivate
//
Bool ICTetris::Deactivate()
{
  if (IControl::Deactivate())
  {
    score->Deactivate();
    lines->Deactivate();
    speed->Deactivate();

    return (TRUE);
  }

  return (FALSE);
}


//
// ICTetris::Setup
//
// Configure this control with an FScope
//
void ICTetris::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0xE29F5FFC: // "BlockTexture"
      IFace::FScopeToTextureInfo(fScope, blockTexture);
      break;

    case 0xEAB21734: // "PitOffset"
      StdLoad::TypePoint<S32>(fScope, pitOffset);
      break;

    case 0xD35E79B0: // "NextOffset"
      StdLoad::TypePoint<S32>(fScope, nextOffset);
      break;

    case 0xE677A8AB: // "AddPiece"
      pieces.Append(new Piece(fScope));
      break;

    case 0xBD9E1D75: // "Bonus"
      bonus[0] = StdLoad::TypeU32(fScope);
      bonus[1] = StdLoad::TypeU32(fScope);
      bonus[2] = StdLoad::TypeU32(fScope);
      bonus[3] = StdLoad::TypeU32(fScope);
      break;

    default:
    {
      // Pass it to the previous level in the hierarchy
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICTetris::HandleEvent
//
// Process input events
//
U32 ICTetris::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICTetrisMsg::Left:
            // Attempt to move left
            if (!TestRotation(*rotation, currentLocation + Point<S32>(-1, 0)))
            {
              currentLocation.x--;
            }
            break;

          case ICTetrisMsg::Right:
            // Attempt to move right
            if (!TestRotation(*rotation, currentLocation + Point<S32>(1, 0)))
            {
              currentLocation.x++;
            }
            break;

          case ICTetrisMsg::Drop:
            timer = 0;
            break;

          case ICTetrisMsg::Clockwise:
          {
            // Attempt to rotate clockwise
            NList<Rotation>::Node *node = &current->rotations.GetNode(*rotation);
            if (node->GetNext())
            {
              node = node->GetNext();
            }
            else
            {
              node = current->rotations.GetHeadNode();
            }
            Rotation *nextRotation = node->GetData();

            // Does the piece fit using this rotation ?
            if (!TestRotation(*nextRotation, currentLocation))
            {
              rotation = nextRotation;
            }
            else
            // Does the piece fit one to the right
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(1, 0)))
            {
              rotation = nextRotation;
              currentLocation.x++;
            }
            else
            // Does the piece fit one to the left
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(-1, 0)))
            {
              rotation = nextRotation;
              currentLocation.x--;
            }
            else
            // Does the piece fit two to the right
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(2, 0)))
            {
              rotation = nextRotation;
              currentLocation.x += 2;
            }
            else
            // Does the piece fit two to the left
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(-2, 0)))
            {
              rotation = nextRotation;
              currentLocation.x -= 2;
            }
            break;
          }

          case ICTetrisMsg::AntiClockwise:
          {
            // Attempt to rotate anitclockwise
            NList<Rotation>::Node *node = &current->rotations.GetNode(*rotation);
            if (node->GetPrev())
            {
              node = node->GetPrev();
            }
            else
            {
              node = current->rotations.GetTailNode();
            }
            Rotation *nextRotation = node->GetData();

            // Does the piece fit using this rotation ?
            if (!TestRotation(*nextRotation, currentLocation))
            {
              rotation = nextRotation;
            }
            else
            // Does the piece fit one to the right
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(1, 0)))
            {
              rotation = nextRotation;
              currentLocation.x++;
            }
            else
            // Does the piece fit one to the left
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(-1, 0)))
            {
              rotation = nextRotation;
              currentLocation.x--;
            }
            else
            // Does the piece fit two to the right
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(2, 0)))
            {
              rotation = nextRotation;
              currentLocation.x += 2;
            }
            else
            // Does the piece fit two to the left
            if (!TestRotation(*nextRotation, currentLocation + Point<S32>(-2, 0)))
            {
              rotation = nextRotation;
              currentLocation.x -= 2;
            }
            break;
          }

          case ICTetrisMsg::Reset:
          {
            Clear();
            break;
          }
        }
        break;
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}


//
// ICTetris::Poll
//
// Process
//
void ICTetris::Poll()
{
  switch (state)
  {
    case START:
      // Thumb twiddlin
      break;

    case DROP:
      // Dropping a piece into the pit

      // Has the drop time expired ?
      if (timer)
      {
        timer--;
      }
      else
      {
        // Reset timer
        timer = speed->GetIntegerValue();

        // Can the current piece be dropped ?

        // Test a rotation in the pit
        if (TestRotation(*rotation, currentLocation + Point<S32>(0, 1)))
        {
          // If the current location is above the pit then its game over
          if (currentLocation.y < 2)
          {
            state = START;
          }
          else
          {
            // Split the piece at the current location
            SplitPiece(*current, *rotation, currentLocation);

            U32 numLines = 0;

            // Check to see if any bonus should be collected
            for (int i = 0; i < 4; i++)
            {
              if (TestRow(currentLocation.y + i))
              {
                numLines++;
                collect[i] = currentLocation.y + i;
                state = COLLECT_MARK;
                timer = speed->GetIntegerValue();
              }
              else
              {
                collect[i] = 0;
              }
            }

            U32 s;

            // Compute the drop score (based on fraction of the drop at high speed)
            U32 level = 11 - speed->GetIntegerValue();
				    s = level * (22 - currentLocation.y) + level * 2;
				    //s = ((s * drop_count[1]) / (drop_count[0] + drop_count[1])) + s;

            if (numLines)
            {
              // Add in any bonus
              s += bonus[numLines - 1];

              // Adjust lines
              lines->SetIntegerValue(lines->GetIntegerValue() + numLines);

              // Adjust speed
              speed->SetIntegerValue(10 - Min(10, lines->GetIntegerValue() / 10));
            }
            score->SetIntegerValue(score->GetIntegerValue() + s);

            // Get a new piece
            NextPiece();
          }
        }
        else
        {
          currentLocation.y++;
        }
      }

      break;

    case COLLECT_MARK:
    {
      // Change the color of the collect lines to be brighter and semi-translucent
      for (int i = 0; i < 4; i++)
      {
        if (collect[i])
        {
          for (S32 x = 0; x < 10; x++)
          {
            pit[collect[i] - 2][x].Lighten(192);
            //pit[collect[i] - 2][x].a = 192;
          }
        }
      }
      state = COLLECT_DROP;
      break;
    }

    case COLLECT_DROP:
    {
      for (int i = 0; i < 4; i++)
      {
        if (collect[i])
        {
          // Has this row faded out ?
          if (pit[collect[i] - 2][0].a <= 32)
          {
            // Remove that row
            RemoveRow(collect[i]);
            collect[i] = 0;
          }
          else
          {
            // Fade out the row
            for (S32 x = 0; x < 10; x++)
            {
              pit[collect[i] - 2][x].a -= 32;
            }
          }
          return;
        }
      }
      state = DROP;

      break;
    }

    case BONUS:
      // Collecting bonus from completed level
      break;
  }
}


//
// ICTetris::Reset
//
// Reset a game
//
void ICTetris::Reset()
{
  // Setup the next piece
  next = pieces[Random::nonSync.Integer(pieces.GetCount())];
}


//
// Get the next piece
//
void ICTetris::NextPiece()
{
  // Get the next piece
  current = next;

  ASSERT(current)

  // Setup the default rotation
  rotation = current->rotations.GetHead();

  // Get another piece for the next piece
  next = pieces[Random::nonSync.Integer(pieces.GetCount())];

  // Set the initial current position
  currentLocation.x = 14;
  currentLocation.y = 0;

}


//
// ICTetris::TestRotation
//
// Test a rotation in the pit
//
Bool ICTetris::TestRotation(const Rotation &rotation, const Point<S32> &location)
{
  U32 shift = 28 - location.x;

  return (
    pitMask[location.y    ] & (rotation.mask[0] << shift) ||
    pitMask[location.y + 1] & (rotation.mask[1] << shift) ||
    pitMask[location.y + 2] & (rotation.mask[2] << shift) ||
    pitMask[location.y + 3] & (rotation.mask[3] << shift));
}


//
// ICTetris::SplitPiece
//
// Split a piece in the pit
//
void ICTetris::SplitPiece(const Piece &piece, const Rotation &rotation, const Point<S32> &location)
{
  U32 shift = 28 - location.x;

  // Add the piece to the pit mask
  pitMask[location.y    ] |= rotation.mask[0] << shift;
  pitMask[location.y + 1] |= rotation.mask[1] << shift;
  pitMask[location.y + 2] |= rotation.mask[2] << shift;
  pitMask[location.y + 3] |= rotation.mask[3] << shift;

  // Add this piece's colors to the pit
  for (S32 y = 0; y < 4; y++)
  {
    for (S32 x = 0; x < 4; x++)
    {
      if (rotation.mask[y] & (0x8 >> x))
      {
        pit[y + location.y - 2][x + location.x - 11] = piece.color;
      }
    }
  }
}


//
// ICTetris::TestRow
//
// Test a row of the pit
//
Bool ICTetris::TestRow(S32 row)
{
  return (row > 1 && row < 22 && pitMask[row] == 0xFFFFFFFF);
}


//
// ICTetris::RemoveRow
//
// Remove a row of the pit
//
void ICTetris::RemoveRow(S32 row)
{
  while (row > 2)
  {
    for (S32 x = 0; x < 10; x++)
    {
      pit[row - 2][x] = pit[row - 3][x];
    }
    pitMask[row] = pitMask[row - 1];

    row--;
  }

  for (S32 x = 0; x < 10; x++)
  {
    pit[0][x] = Color(0.0f, 0.0f, 0.0f);
  }
  pitMask[row] = pitMask[row - 1];
}


//
// ICTetris::EmptyPit
//
// Empty the pit
//
void ICTetris::EmptyPit()
{
  S32 x, y;

  for (y = 0; y < 22; y++)
  {
    pitMask[y] = 0xFFE007FF;
  }
  for (; y < 25; y++)
  {
    pitMask[y] = 0xFFFFFFFF;
  }

  for (y = 0; y < 20; y++)
  {
    for (x = 0; x < 10; x++)
    {
      pit[y][x] = Color(0.0f, 0.0f, 0.0f);
    }
  }
}


//
// Clear everything
//
void ICTetris::Clear()
{
  EmptyPit();
  Reset();
  NextPiece();

  score->SetIntegerValue(0);
  lines->SetIntegerValue(0);
  speed->SetIntegerValue(10);
  timer = speed->GetIntegerValue();
  state = DROP;
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTetris::Piece
//


//
// ICTetris::Piece::Piece
//
ICTetris::Piece::Piece(FScope *fScope)
: rotations(&Rotation::node)
{
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xDD6D8CBC: // "AddRotation"
        rotations.Append(new Rotation(sScope));
        break;

      case 0x0FF7F7A4: // "SetColor"
        StdLoad::TypeColor(sScope, color);

      default:
        break;
    }
  }
}


//
// ICTetris::Piece::~Piece
//
ICTetris::Piece::~Piece()
{
  // Clean up all of the rotations
  rotations.DisposeAll();
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTetris::Rotation
//

//
// ICTetris::Rotation::Rotation
//
ICTetris::Rotation::Rotation(FScope *fScope)
{
  FScope *sScope;

  // Load the mask
  sScope = fScope->GetFunction("Mask");
  mask[0] = sScope->NextArgInteger() == 1 ? 0x8 : 0x0;
  mask[0] |= sScope->NextArgInteger() == 1 ? 0x4 : 0x0;
  mask[0] |= sScope->NextArgInteger() == 1 ? 0x2 : 0x0;
  mask[0] |= sScope->NextArgInteger() == 1 ? 0x1 : 0x0;

  mask[1] = sScope->NextArgInteger() == 1 ? 0x8 : 0x0;
  mask[1] |= sScope->NextArgInteger() == 1 ? 0x4 : 0x0;
  mask[1] |= sScope->NextArgInteger() == 1 ? 0x2 : 0x0;
  mask[1] |= sScope->NextArgInteger() == 1 ? 0x1 : 0x0;

  mask[2] = sScope->NextArgInteger() == 1 ? 0x8 : 0x0;
  mask[2] |= sScope->NextArgInteger() == 1 ? 0x4 : 0x0;
  mask[2] |= sScope->NextArgInteger() == 1 ? 0x2 : 0x0;
  mask[2] |= sScope->NextArgInteger() == 1 ? 0x1 : 0x0;

  mask[3] = sScope->NextArgInteger() == 1 ? 0x8 : 0x0;
  mask[3] |= sScope->NextArgInteger() == 1 ? 0x4 : 0x0;
  mask[3] |= sScope->NextArgInteger() == 1 ? 0x2 : 0x0;
  mask[3] |= sScope->NextArgInteger() == 1 ? 0x1 : 0x0;

  // Load the texture
  IFace::FScopeToTextureInfo(fScope->GetFunction("Texture"), texture);
}
