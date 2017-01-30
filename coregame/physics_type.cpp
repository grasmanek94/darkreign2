///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 27-APR-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "physics_type.h"
#include "physicsctrl.h"


#define Integrate(x,h) IntegrateEuler(x,h)
//#define Integrate(x,h) RungeKutta4(x,h)


///////////////////////////////////////////////////////////////////////////////
//
// Rigid body object type definition
//


//
// Constructor
//
PhysicsType::PhysicsType() 
: contacts(NULL)
{
}


//  
// Destructor
//
PhysicsType::~PhysicsType()
{
  if (contacts)
  {
    delete[] contacts;
    contacts = NULL;
  }
}


//
// PhysicsType::Setup
//
// Configure a physics type
//
void PhysicsType::Setup(FScope *fScope)
{
  // Mass in g (convert to kg)
  mass = StdLoad::TypeF32(fScope, "Mass", 1E6f) * 1E-3f;
  massInv = (mass > F32_EPSILON) ? (1.0F / mass) : 0;

  // Elasticity
  elasticity = StdLoad::TypeF32(fScope, "Elasticity", 0.05F, Range<F32>(0.0F, 1.0F));

  // Grip
  grip = StdLoad::TypeF32(fScope, "Grip", 3.0F, Range<F32>(0.1F, 100.0F));

  // Drag coefficient
  dragCoefficient = StdLoad::TypeF32(fScope, "DragCoefficient", 0.1f, Range<F32>(0.0F, 1.0F));

  // Body type
  bodyType = StdLoad::TypeStringCrc(fScope, "BodyType", 0x1CCAFDCC); // "Box"
}


//
// PhysicsType::PostLoad
//
// Post load the type
//
/*
void PhysicsType::PostLoad(MeshEnt &mesh)
{
}
*/


/*
//
// Setup contacts for corners of a box
//
void PhysicsType::SetupContactsBox(MeshEnt &mesh)
{
  ASSERT(!contacts)

  const Sphere &s = mesh.WorldBounds();

  contacts = new Vector[8];

  contacts[0].Set(-s.Width, -s.Breadth, -s.Height);
  contacts[1].Set( s.Width, -s.Breadth, -s.Height);
  contacts[2].Set( s.Width,  s.Breadth, -s.Height);
  contacts[3].Set(-s.Width,  s.Breadth, -s.Height);
  contacts[4].Set(-s.Width, -s.Breadth,  s.Height);
  contacts[5].Set( s.Width, -s.Breadth,  s.Height);
  contacts[6].Set( s.Width,  s.Breadth,  s.Height);
  contacts[7].Set(-s.Width,  s.Breadth,  s.Height);
}
*/


//
// Constructor 
//
PhysicsObj::PhysicsObj(PhysicsType *type) 
: type(type) 
{
  velocity.ClearData();
  omega.ClearData();
  force.ClearData();
  torque.ClearData();
}


//
// Setup position
//
void PhysicsObj::Setup(const Matrix &m)
{
  position = m.posit;
  attitude.Set(m);
}


//
// Basic physics simulation
//
void PhysicsObj::BasicSimulation(F32 step, F32)
{
  F32 weight = type->mass * PhysicsCtrl::GetGravity();

  // Gravity
  force.y -= weight;

  // Ground friction?

  // Air friction
  force  -= velocity * (type->dragCoefficient * type->mass);
  torque -= omega * (type->dragCoefficient * type->mass);

  // Integrate position
  position += Integrate(velocity, step);

  // Integrate velocity
  velocity += Integrate(force * type->massInv, step);

  // Damping
  //velocity -= velocity * (0.02F * cd);

  // Integrate angular momentum
  omega += Integrate(torque * type->massInv, step);

  // Integrate orientation
  if (omega.Magnitude2() > PhysicsConst::EPSILON)
  {
    Quaternion dq;
    dq.s = 0.0F;
    dq.v = omega;

    attitude += Integrate(dq * attitude * 0.5F, step);
  }

  //CollisionCtrl::AddItem(this, mapObj);

  // Clear forces
  force.ClearData();
  torque.ClearData();
}


//
// Check for ground collisions
//
/*
void PhysicsObj::TestGroundCollision()
{
  // Penetrating contacts

  Vector dir = targetPos - m.posit;

  if (TerrainData::Intersect(dir )


}
*/
