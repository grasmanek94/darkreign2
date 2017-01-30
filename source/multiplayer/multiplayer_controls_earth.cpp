///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_earth.h"
#include "multiplayer_network.h"
#include "multiplayer_private.h"
#include "multiplayer_settings.h"
#include "sunpos.h"
#include "stdload.h"
#include "iface_util.h"
#include "vid_public.h"
#include "input.h"
#include "iface.h"
#include "font.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Earth
    //


    // Update every 60 seconds
    const UpdatePeriod = 60000;


    //
    // RenderBitmap
    //
    void RenderBitmap(Bitmap *bitmap, const Point<S32> &pos, Color color)
    {
      U16 vertOffset;
      VertexTL *point = IFace::GetVerts(4, bitmap, 0, 0, vertOffset);
      IFace::SetIndex(Vid::rectIndices, 6, vertOffset);

		  // top left corner
		  point[0].vv.x = F32(pos.x);
		  point[0].vv.y = F32(pos.y);
		  point[0].vv.z = 0.0f;
		  point[0].rhw = 1.0f;
		  point[0].diffuse = color;
		  point[0].specular = 0xFF000000;
			point[0].u = bitmap->UVShiftWidth();
			point[0].v = bitmap->UVShiftHeight();

		  // top right corner
		  point[1].vv.x = F32(pos.x + bitmap->Width());
		  point[1].vv.y = F32(pos.y);
		  point[1].vv.z = 0.0F;
		  point[1].rhw = 1.0F;
		  point[1].diffuse = color;
		  point[1].specular = 0xFF000000;
			point[1].u = 1.0f + bitmap->UVShiftWidth();
			point[1].v = bitmap->UVShiftHeight();

		  // bottom right corner
		  point[2].vv.x = F32(pos.x + bitmap->Width());
		  point[2].vv.y = F32(pos.y + bitmap->Height());
		  point[2].vv.z = 0.0F;
		  point[2].rhw = 1.0F;
		  point[2].diffuse = color;
		  point[2].specular = 0xFF000000;
			point[2].u = 1.0f + bitmap->UVShiftWidth();
			point[2].v = 1.0f + bitmap->UVShiftHeight();

      // bottom left corner
		  point[3].vv.x = F32(pos.x);
		  point[3].vv.y = F32(pos.y + bitmap->Height());
		  point[3].vv.z = 0.0F;
		  point[3].rhw = 1.0F;
		  point[3].diffuse = color;
		  point[3].specular = 0xFF000000;
			point[3].u = bitmap->UVShiftWidth();
			point[3].v = 1.0f + bitmap->UVShiftHeight();

    }


    //
    // Given the sun vector render night and day
    //
    void RenderEarth(const Vector &sun, Bitmap &dst, Bitmap &src, Earth::Precomp *column, Earth::Precomp *row)
    {
      // Given the sun vector
      src.InitPrimitives();
      dst.InitPrimitives();

      if (dst.Lock())
      {
        if (src.Lock())
        {
          const Pix * pixForm = src.PixelFormat();

          for (U32 z = 0; z < 256; z++)
          {
            Earth::Precomp *c = column;

            for (U32 x = 0; x < 256; x++)
            {
              // Calculate the lighting for this pixel
              F32 light = 
                sun.x * c->cosine * row->cosine +
                sun.y * row->sine +
                sun.z * c->sine * row->cosine;

              // Rescale the lighting

              // -1.0f             0.0f              1.0f
              // |------------------|-------------------|
              //              -0.05f 0.05f
              //                 |--|--|
              // Night            Dusk                Day

              if (light < -0.05f)
              {
                // Night
                U32 data = src.GetPixel(x, z);
                U8 r = (U8)(((((data & pixForm->rMask) >> pixForm->rShift) << (pixForm->rScaleInv)) * 64) >> 8);
                U8 g = (U8)(((((data & pixForm->gMask) >> pixForm->gShift) << (pixForm->gScaleInv)) * 64) >> 8);
                U8 b = (U8)(((((data & pixForm->bMask) >> pixForm->bShift) << (pixForm->bScaleInv)) * 64) >> 8);
                dst.PutPixel(x, z, dst.MakeRGBA(r, g, b, 255));
              }
              else if (light > 0.05f)
              {
                // Day
                dst.PutPixel(x, z, src.GetPixel(x, z));
              }
              else
              {
                // Dusk
                U32 intensity = U32((light + 0.05f) * 1920.f) + 64;
                U32 data = src.GetPixel(x, z);
                U8 r = (U8)(((((data & pixForm->rMask) >> pixForm->rShift) << (pixForm->rScaleInv)) * intensity) >> 8);
                U8 g = (U8)(((((data & pixForm->gMask) >> pixForm->gShift) << (pixForm->gScaleInv)) * intensity) >> 8);
                U8 b = (U8)(((((data & pixForm->bMask) >> pixForm->bShift) << (pixForm->bScaleInv)) * intensity) >> 8);
                dst.PutPixel(x, z, dst.MakeRGBA(r, g, b, 255));
              }

              // Move to the next column
              c++;
            }

            // Move to the next row
            row++;
          }
          src.UnLock();
        }
        dst.UnLock();
      }
    }


    //
    // Plot a location
    //
    void PlotLocation(const Point<S32> &local, F32 offset, Color color)
    {
      Color alpha(color.R(), color.G(), color.B(), 0.0f);

      // Get the angle
      F32 angle = F32(IFace::ElapsedMs() % 2000) * PI2 / 2000.0f + offset;

      for (int i = 0; i < 4; i++)
      {
        IFace::RenderLine
        (
          local,
          local + Point<S32>(S32(F32(cos(angle)) * 10.0f), S32(F32(sin(angle)) * 10.0f)),
          color,
          alpha
        );

        angle += PIBY2;
      }
    }


    //
    // Constructor
    //
    Earth::Earth(IControl *parent)
    : IControl(parent)
    {
      lastRedraw = 0;

      earth[0] = NULL;
      earth[1] = NULL;
      lit[0] = NULL;
      lit[1] = NULL;

      for (U32 i = 0; i < 256; i++)
      {
        F32 rowAngle = PIBY2 - ((F32(i) + 0.5f) * PIBY2 / 128.0f);
        rows[i].sine = F32(sin(rowAngle));
        rows[i].cosine = F32(cos(rowAngle));

        F32 columnAngle = ((F32(i) + 0.5f) * PI / 256.0f) - PI;
        columns[0][i].sine = F32(sin(columnAngle));
        columns[0][i].cosine = F32(cos(columnAngle));

        columnAngle = ((F32(i) + 0.5f) * PI / 256.0f);
        columns[1][i].sine = F32(sin(columnAngle));
        columns[1][i].cosine = F32(cos(columnAngle));
      }

      latitude = new IFaceVar(this, CreateFloat("Latitude", 0.0f));
      longitude = new IFaceVar(this, CreateFloat("Longitude", 0.0f));
    }


    //
    // Destructor
    //
    Earth::~Earth()
    {
      if (earth[0])
      {
        delete earth[0];
        delete earth[1];
        delete lit[0];
        delete lit[1];
      }

      delete latitude;
      delete longitude;
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Info
    //
    struct Info
    {
      NBinTree<Info, F32>::Node node;

      const Network::Player *network;
      const Player *player;
      const PlayerInfo *playerInfo;
      const Team *team;

      Info(const Network::Player *network, const Player *player, const PlayerInfo *playerInfo, const Team *team)
      : network(network),
        player(player),
        playerInfo(playerInfo),
        team(team)
      {
      }

    };


    //
    // Redraw self
    //
    void Earth::DrawSelf(PaintInfo &pi)
    {
      Update(FALSE);

      RenderBitmap(lit[0], pi.client.p0, Color(1.0f, 1.0f, 1.0f, 1.0f));
      RenderBitmap(lit[1], pi.client.p0 + Point<S32>(256, 0), Color(1.0f, 1.0f, 1.0f, 1.0f));

      // Draw everyone
      F32 offset = 0.0f;

      for (NBinTree<Network::Player>::Iterator np(&Network::GetPlayers()); *np; ++np)
      {
        const Player *player;
        const PlayerInfo *playerInfo;
        const Team *team;

        U32 playerId = (*np)->GetId();

        // Does this network player have Player and PlayerInfo and a Team ?
        if 
        (
          Data::Get(&player, playerId) &&
          Data::Get(&playerInfo, playerId) &&
          Data::Get(&team, playerInfo->teamId)
        )
        {
          // Draw the position of the local player
          Point<S32> local
          (
            S32(255.5f + (player->longitude * 256.0f / 180.0f)),
            S32(127.5f - (player->latitude * 128.0f / 90.0f))
          );
          PlotLocation(ClientToScreen(local), offset, GetTeamColor(team->color));
        }

        offset += 0.1f;
      }

      // Get the position of the mouse
      Point<S32> mouse = Input::MousePos();
      if (InClient(mouse))
      {
        mouse = ScreenToClient(mouse);

        // Convert the position of the mouse into longitude and latitude
        Point<F32> pos
        (
          (F32(mouse.x) - 255.5f) * 180.0f / 256.0f,
          (127.5f - F32(mouse.y)) * 90.0f / 128.0f
        );

        // Sort the locations of the players by their proximity to that location
        NBinTree<Info, F32> info(&Info::node);

        for (NBinTree<Network::Player>::Iterator np(&Network::GetPlayers()); *np; ++np)
        {
          const Player *player;
          const PlayerInfo *playerInfo;
          const Team *team;

          U32 playerId = (*np)->GetId();

          // Does this network player have Player and PlayerInfo and a Team ?
          if 
          (
            Data::Get(&player, playerId) &&
            Data::Get(&playerInfo, playerId) &&
            Data::Get(&team, playerInfo->teamId)
          )
          {
            F32 dx = player->longitude - pos.x;
            F32 dy = player->latitude - pos.y;
            info.Add(dx * dx + dy * dy, new Info((*np), player, playerInfo, team));
          }
        }

        // Get the first 8 or number who are close enough to the cursor (15 degrees)
        U32 count = 0;
        F32 distance = 15.0f * 15.0f;
        Point<F32> avg(0.0f, 0.0f);

        for (NBinTree<Info, F32>::Iterator i(&info); *i; ++i)
        {
          if (count < 8 && i.GetKey() < distance)
          {
            count++;
            avg += Point<F32>((*i)->player->longitude, (*i)->player->latitude);
          }
        }

        // No need to go any further is there's no one here
        if (count)
        {
          avg /= F32(count);

          Info *i = info.GetFirst();

          // Using the average point calculate the angle to the closest player
          F32 angle = F32(atan2(i->player->latitude - avg.y, i->player->longitude - avg.x));

          // Using the count work out the angle step
          F32 angleStep = PI2 / F32(count);

          // The 'count' first users should now get sorted by their angle
          if (count > 1)
          {
            U32 co = count;

            NBinTree<Info, F32> old(&Info::node);
            info.Transfer(old);

            // The first user is automatically included
            old.Unlink(i);
            info.Add(0.0f, i);
            
            // Now the next 'count - 1' get added in increasing angle to the tree
            co--;

            while (co)
            {
              U32 c = 0;
              i = NULL;
              F32 score = PI2;

              for (NBinTree<Info, F32>::Iterator o(&old); *o; ++o)
              {
                if (c++ < co)
                {
                  F32 a = F32(atan2((*o)->player->latitude - avg.y, (*o)->player->longitude - avg.x));

                  F32 s = a - angle;
                  if (s < 0.0f)
                  {
                    s += PI2;
                  }

                  if (s < score)
                  {
                    i = (*o);
                    score = s;
                  }
                }
              }

              ASSERT(i)

              // Add the winner
              old.Unlink(i);
              info.Add(score, i);

              co--;
            }

            // The remaining ones in old can be thrown away
            old.DisposeAll();
          }

          // Now draw them all
          for (NBinTree<Info, F32>::Iterator f(&info); *f; ++f)
          {
            if (count--)
            {
              // Point1 is the (angle * 50) + avg point
              Point<F32> p1 = avg + Point<F32>(F32(cos(angle)) * 50.0f, F32(sin(angle)) * 50.0f);

              // Point2 is the latitude and longitude of this player
              Point<F32> p2((*f)->player->longitude, (*f)->player->latitude);

              // Convert into client co-ordinates
              Point<S32> c1
              (
                S32(255.5f + (p1.x * 256.0f / 180.0f)),
                S32(127.5f - (p1.y * 128.0f / 90.0f))
              );
              Point<S32> c2
              (
                S32(255.5f + (p2.x * 256.0f / 180.0f)),
                S32(127.5f - (p2.y * 128.0f / 90.0f))
              );

              Point<S32> s = ClientToScreen(c1);

              Color color1 = GetTeamBgColor((*f)->team->color);
              color1.a = 255;

              Color color2 = Color(1.0f, 1.0f, 1.0f);

              // Draw the line
              IFace::RenderLine(s, ClientToScreen(c2), color1, color2, pi.alphaScale);

              // Draw the backing in the team color
              ClipRect c(s - Point<S32>(50, 8), s + Point<S32>(50, 8));

              IFace::RenderRectangle(c, color1);

              // Draw the name of the player
              const CH *ch = Utils::Ansi2Unicode((*f)->network->GetName());
              U32 len = Utils::Strlen(ch);
              pi.font->Draw(
                s.x - (pi.font->Width(ch, len) / 2),
                s.y - (pi.font->Height() / 2), 
                ch, 
                len,
                pi.colors->fg[ColorIndex()],
                NULL
              );

              angle += angleStep;
            }
            else
            {
              break;
            }
          }
        }

        info.DisposeAll();
      }
    }


    //
    // Setup
    //
    void Earth::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xDE16052E: // "Earth"
        {
          // Load bitmaps
          earth[0] = Bitmap::Manager::FindCreate(Bitmap::reduceNONE, StdLoad::TypeString(fScope));
          earth[1] = Bitmap::Manager::FindCreate(Bitmap::reduceNONE, StdLoad::TypeString(fScope));

          U32 size = earth[0]->Width();

          // Create buffers
          lit[0] = Bitmap::Manager::Create(Bitmap::reduceNONE, "earth0", 0, bitmapTEXTURE | bitmapWRITABLE);
          lit[0]->Create(size, size, FALSE);
          lit[1] = Bitmap::Manager::Create(Bitmap::reduceNONE, "earth1", 0, bitmapTEXTURE | bitmapWRITABLE);
          lit[1]->Create(size, size, FALSE);
          break;
        }

        default:
          IControl::Setup(fScope);
          break;
      }
    }


    //
    // Control activation
    //
    Bool Earth::Activate()
    {
      if (IControl::Activate())
      {
        ActivateVar(latitude);
        ActivateVar(longitude);

        Update(TRUE);
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Control deactivation
    //
    Bool Earth::Deactivate()
    {
      if (IControl::Deactivate())
      {
        latitude->Deactivate();
        longitude->Deactivate();

        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Update
    //
    void Earth::Update(Bool force)
    {
      U32 now = Clock::Time::Ms();
      if (force || (now - lastRedraw) > UpdatePeriod)
      {
        lastRedraw = now;

        F64 lat, lon;
        SunPosition(lat, lon);
        Vector sun(VectorDir(F32(lon), F32(PIBY2 - lat)));

        RenderEarth(sun, *lit[0], *earth[0], columns[0], rows);
        RenderEarth(sun, *lit[1], *earth[1], columns[1], rows);
      }
    }


    //
    // Earth::HandleEvent
    //
    U32 Earth::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case EarthMsg::Upload:
              {
                VarFloat longitude("$.longitude", e.iface.from);
                VarFloat latitude("$.latitude", e.iface.from);

                longitude = Settings::GetLongitude();
                latitude = Settings::GetLatitude();

                return (TRUE);
              }

              case EarthMsg::Download:
              {
                VarFloat longitude("$.longitude", e.iface.from);
                VarFloat latitude("$.latitude", e.iface.from);

                Settings::SetLongitude(Clamp<F32>(-180.0f, longitude, 180.0f));
                Settings::SetLatitude(Clamp<F32>(-90.0f, latitude, 90.0f));
                Settings::SaveToUser();

                UpdatePlayerInfo();

                return (TRUE);
              }
            }
          }
        }
      }
      else if (e.type == Input::EventID())
      {
        // Input events
        switch (e.subType)
        {
          case Input::MOUSEMOVE:
          {
            Point<S32> mouse(e.input.mouseX, e.input.mouseY);

            mouse = ScreenToClient(mouse);

            // Convert the position of the mouse into longitude and latitude
            Point<F32> pos
            (
              (F32(mouse.x) - 255.5f) * 180.0f / 256.0f,
              (127.5f - F32(mouse.y)) * 90.0f / 128.0f
            );

            longitude->SetFloatValue(pos.x);
            latitude->SetFloatValue(pos.y);

            return (TRUE);
            break;
          }
        }
      }

      return (IControl::HandleEvent(e));  
    }

  }

}
