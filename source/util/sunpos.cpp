///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sun Position
//

#include "mathtypes.h"
#include "sunpos.h"


//
// the epoch upon which these astronomical calculations are based is
// 1990 january 0.0, 631065600 seconds since the beginning of the
// "unix epoch" (00:00:00 GMT, Jan. 1, 1970)
//
// given a number of seconds since the start of the unix epoch,
// DaysSinceEpoch() computes the number of days since the start of the
// astronomical epoch (1990 january 0.0)
//
#define EpochStart           (631065600)
#define DaysSinceEpoch(secs) (((secs)-EpochStart)*(1.0/(24*3600)))


//
// assuming the apparent orbit of the sun about the earth is circular,
// the rate at which the orbit progresses is given by RadsPerDay --
// PI2 radians per orbit divided by 365.242191 days per year:
//
#define RadsPerDay (PI2/365.242191)


//
// details of sun's apparent orbit at epoch 1990.0 (after
// duffett-smith, table 6, section 46)
//
// Epsilon      (ecliptic longitude at epoch 1990.0) 279.403303 degrees
// OmegaBar     (ecliptic longitude of perigee)      282.768422 degrees
// Eccentricity (eccentricity of orbit)                0.016713
//
#define Epsilon      (279.403303*(PI2/360))
#define OmegaBar     (282.768422*(PI2/360))
#define Eccentricity (0.016713)


//
// MeanObliquity gives the mean obliquity of the earth's axis at epoch
// 1990.0 (computed as 23.440592 degrees according to the method given
// in duffett-smith, section 27)
//
#define MeanObliquity (23.440592*(PI2/360))


///////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//
static F64 SolveKeplersEquation(F64);
static F64 SunEclipticLongitude(time_t);
static void EclipticToEquatorial(F64, F64, F64 &, F64 &);
static F64 JulianDate(int, int, int);
static F64 GST(time_t);


//
// Given a particular time (expressed in seconds since the unix
// epoch), compute position on the earth (lat, lon) such that sun is
// directly overhead.
//
void SunPosition(F64 &lat, F64 &lon)
{
  time_t ssue = time(NULL);
  F64 lambda;
  F64 alpha;

  lambda = SunEclipticLongitude(ssue);
  EclipticToEquatorial(lambda, 0.0, alpha, lat);

  lon = alpha - (PI2 / 24) * GST(ssue);
  if (lon < -PI)
  {
    do lon += PI2;
    while (lon < -PI);
  }
  else if (lon > PI)
  {
    do lon -= PI2;
    while (lon < -PI);
  }
}


//
// SolveKeplersEquation
//
// solve Kepler's equation via Newton's method
// (after duffett-smith, section 47)
//
static F64 SolveKeplersEquation(F64 m)
{
  F64 e;
  F64 delta;

  e = m;
  for (;;)
  {
    delta = e - Eccentricity * sin(e) - m;
    if (fabs(delta) <= 1e-10) 
    {
      break;
    }
    e -= delta / (1 - Eccentricity * cos(e));
  }

  return (e);
}


//
// SunEclipticLongitude
//
// compute ecliptic longitude of sun (in radians)
// (after duffett-smith, section 47)
//
static F64 SunEclipticLongitude(time_t ssue)
{
  F64 d, n;
  F64 sun, e;
  F64 v;

  d = DaysSinceEpoch(ssue);

  n = RadsPerDay * d;
  n = fmod(n, PI2);
  if (n < 0)
  {
    n += PI2;
  }

  sun = n + Epsilon - OmegaBar;
  if (sun < 0)
  {
    sun += PI2;
  }

  e = SolveKeplersEquation(sun);
  v = 2.0 * atan(sqrt((1 + Eccentricity) / (1 - Eccentricity)) * tan(e / 2.0));

  return (v + OmegaBar);
}


//
// EclipticToEquatorial
//
// convert from ecliptic to equatorial coordinates
// (after duffett-smith, section 27)
//
static void EclipticToEquatorial(F64 lambda, F64 beta, F64 &alpha, F64 &delta)
{
  F64 sin_e, cos_e;

  sin_e = sin(MeanObliquity);
  cos_e = cos(MeanObliquity);
  alpha = atan2(sin(lambda) * cos_e - tan(beta) * sin_e, cos(lambda));
  delta = asin(sin(beta) * cos_e + cos(beta) * sin_e * sin(lambda));

//  if (view_rot == 0.0 && proj_type == ProjTypeOrthographic) 
//  {
//    view_rot = -asin(sin(beta) * cos_e + cos(beta) * sin_e * cos(lambda)) * RAD2DEG;
//  }
}


//
// JulianDate
//
// Computing julian dates (assuming gregorian calendar, thus this is
// only valid for dates of 1582 oct 15 or later)
// (after duffett-smith, section 4)
//
static F64 JulianDate(int year, int month, int day)
{
  int a, b, c, d;
  F64 jd;

  // lazy test to ensure gregorian calendar
  ASSERT(year >= 1583)

  if ((month == 1) || (month == 2))
  {
    year -= 1;
    month += 12;
  }

  a = year / 100;
  b = 2 - a + (a / 4);
  c = int(365.25 * year);
  d = int(30.6001 * (month + 1));
  jd = b + c + d + day + 1720994.5;

  return (jd);
}


//
// GST
//
// Compute greenwich mean sidereal time (GST) corresponding to a given
// number of seconds since the unix epoch (after duffett-smith, section 12)
//
static F64 GST(time_t ssue)
{
  F64 jd;
  F64 t, t0;
  F64 ut;
  struct tm *tm;

  tm = gmtime(&ssue);

  jd = JulianDate(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
  t = (jd - 2451545) / 36525;

  t0 = ((t + 2.5862e-5) * t + 2400.051336) * t + 6.697374558;

  t0 = fmod(t0, 24.0);
  if (t0 < 0)
  {
    t0 += 24;
  }

  ut = tm->tm_hour + (tm->tm_min + tm->tm_sec / 60.0) / 60.0;

  t0 += ut * 1.002737909;
  t0 = fmod(t0, 24.0);
  if (t0 < 0)
  {
    t0 += 24;
  }

  return (t0);
}


