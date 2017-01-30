///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 27-APR-99
//


#ifndef __PHYSICSTYPE_H
#define __PHYSICSTYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Runge-Kutta order 4 integrator
//
template <class T>
T RungeKutta4(const T &x0, F32 h)
{
  const F32 ONE3 = 1.0F / 3.0F;
  const F32 ONE6 = 1.0F / 6.0F;

  T k1 = (x0)             * (h);
  T k2 = (x0 + k1 * 0.5F) * (h * 0.5F);
  T k3 = (x0 + k2 * 0.5F) * (h * 0.5F);
  T k4 = (x0 + k3)        * (h);

  return ((k1 * ONE6) + (k2 * ONE3) + (k3 * ONE3) + (k4 * ONE6));
}


///////////////////////////////////////////////////////////////////////////////
//
// First order Euler integrator
//
template <class T>
T IntegrateEuler(const T &x0, F32 h)
{
  return (x0 * h);
}


///////////////////////////////////////////////////////////////////////////////
//
// Rigid body object type definition
//
class PhysicsType
{
public:

  enum BodyType
  {
    BOX    = 0x1CCAFDCC, // "Box"
    SPHERE = 0x83B41B3B, // "Sphere"
  };

  // Mass (kg)
  F32 mass;
  F32 massInv;

  // Grip value
  F32 grip;

  // Coefficient of drag (%)
  F32 dragCoefficient;

  // Coefficient of restitution (%)
  F32 elasticity;

  // Surface area (m^3)
  F32 surface;
  F32 surfaceInv;

  // Moment of inertia (kg m/s^2)
  F32 momentOfInertia;

  // Centre of mass
  Vector centreOfMass;

  // Rigid body type
  U32 bodyType;

  // Points of contact
  Vector *contacts;

public:

  // Constructor
  PhysicsType();
  
  // Destructor
  ~PhysicsType();

  // Configure 
  void Setup(FScope *fScope);

  // Postload
  //void PostLoad(MeshEnt &mesh);
};


///////////////////////////////////////////////////////////////////////////////
//
// Rigid body object instance
//
class PhysicsObj
{
public:

  // Type definition
  PhysicsType *type;

  // Position
  Quaternion attitude;
  Vector position;

  // Linear velocity of cm in m/s
  Vector velocity;

  // Angular velocity in rad/s
  Vector omega;

  // Angular momentum
  Vector alpha;

  // Summation of external forces
  Vector force;

  // Summation of external torques
  Vector torque;

  // Magnitude of linear velocity in m/s
  F32 speed;

  // Distance from ground
  F32 height;

public:

  // Constructor 
  PhysicsObj(PhysicsType *type);

  // Setup position
  void Setup(const Matrix &m);

  // Basic simulation
  void BasicSimulation(F32 step, F32 stepInv);
};

#endif
