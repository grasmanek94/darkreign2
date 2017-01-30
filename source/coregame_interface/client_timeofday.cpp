///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Time of day display
//
// 05-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"
#include "client_timeofday.h"
#include "environment_light.h"
#include "iface_util.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  //
  // Constructor
  //
  TimeOfDay::TimeOfDay(IControl *parent) 
  : IControl(parent)
  {
  }


  //
  // Destructor
  //
  TimeOfDay::~TimeOfDay()
  {
  }


  //
  // DrawSelf
  //
  void TimeOfDay::DrawSelf(PaintInfo &pi)
  {
    if (texture && texture->texture)
    {
      // Radius of texture 
      const F32 SCALE = 0.5F;

      const struct { F32 u, v; } UV[4] = 
      {
        { -SCALE, SCALE },
        {  SCALE, SCALE },
        {  SCALE, 0.0F },
        { -SCALE, 0.0F },
      };

      // Middle of day is 90deg rotation, Middle of night is 270deg rotation
      F32 angle = Environment::Light::Azimuth() * PI + PIBY2;

      if (!Environment::Light::IsSunUp())
      {
        angle += PI;
      }

      // Rotate anticlockwise
      //angle = -angle;

      // Calculate UV coordinates, assumes square texture taking up entire image
      F32 cosAngle = F32(cos(angle));
      F32 sinAngle = F32(sin(angle));

      // Scale the alpha down
      Color color = pi.colors->bg[ColorIndex()];

      if (pi.alphaScale < 1.0F)
      {
        color.a = U8(Utils::FtoL(F32(color.a) * pi.alphaScale));
      }

		  // vertices of the rectangle
      U16 offset;
      VertexTL *point = IFace::GetVerts(4, texture->texture, texture->filter, RS_TEXCLAMP, offset);
      IFace::SetIndex(Vid::rectIndices, 6, offset);
        
		  // top left corner
		  point[0].vv.x = (F32)pi.client.p0.x;
		  point[0].vv.y = (F32)pi.client.p0.y;
		  point[0].vv.z = 0.0F;
		  point[0].rhw = 1.0F;
		  point[0].diffuse = color;
		  point[0].specular = 0xFF000000;

			point[0].u = 0.5F + UV[0].u * cosAngle - UV[0].v * sinAngle;
			point[0].v = 0.5F + UV[0].u * sinAngle + UV[0].v * cosAngle;;

		  // top right corner
		  point[1].vv.x = (F32)pi.client.p1.x;
		  point[1].vv.y = (F32)pi.client.p0.y;
		  point[1].vv.z = 0.0F;
		  point[1].rhw = 1.0F;
		  point[1].diffuse = color;
		  point[1].specular = 0xFF000000;

			point[1].u = 0.5F + UV[1].u * cosAngle - UV[1].v * sinAngle;
			point[1].v = 0.5F + UV[1].u * sinAngle + UV[1].v * cosAngle;;

		  // bottom right corner
		  point[2].vv.x = (F32)pi.client.p1.x;
		  point[2].vv.y = (F32)pi.client.p1.y;
		  point[2].vv.z = 0.0F;
		  point[2].rhw = 1.0F;
		  point[2].diffuse = color;
		  point[2].specular = 0xFF000000;

			point[2].u = 0.5F + UV[2].u * cosAngle - UV[2].v * sinAngle;
			point[2].v = 0.5F + UV[2].u * sinAngle + UV[2].v * cosAngle;;

		  // bottom left corner
		  point[3].vv.x = (F32)pi.client.p0.x;
		  point[3].vv.y = (F32)pi.client.p1.y;
		  point[3].vv.z = 0.0F;
		  point[3].rhw = 1.0F;
		  point[3].diffuse = color;
		  point[3].specular = 0xFF000000;

			point[3].u = 0.5F + UV[3].u * cosAngle - UV[3].v * sinAngle;
			point[3].v = 0.5F + UV[3].u * sinAngle + UV[3].v * cosAngle;;

    /*
    // Are we after sunrise and before sunset ?
    if (currentTime > sunriseStartTime && currentTime < sunsetStartTime)
    {
      tempText = "Day";
      tempVal = (sunsetStartTime - currentTime) * 24.0f * 60.0f;
    }
    else
    {
      tempText = "Night";
      tempVal = sunriseStartTime - currentTime;
      if (tempVal < 0.0f)
      {
        tempVal = tempVal + 1.0f;
      }
      tempVal = tempVal * 24.0f * 60.0f;
    }
    */
    }
  }
}
