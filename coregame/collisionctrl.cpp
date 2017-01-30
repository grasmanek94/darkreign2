///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 09-SEP-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "collisionctrl.h"
#include "worldctrl.h"
#include "terrain.h"
#include "physicsctrl.h"
#include "gameobjctrl.h"
#include "varsys.h"
#include "projectileobj.h"
#include "unitobj.h"
#include "promote.h"
#include "perfstats.h"
#include "meshent.h"
#include "gametime.h"
#include "ray.h"
#include "team.h"
#include "sync.h"


// Collision test happens only with target object
#define QUICK_COLLISIONS


///////////////////////////////////////////////////////////////////////////////
//
// Namespace CollisionCtrl
//
namespace CollisionCtrl
{
  // Is the system initialised
  static Bool sysInit = FALSE;

  // List of objects
  static MapObjList objList;

  // Counters
  #ifdef DEVELOPMENT
    Clock::CycleWatch objTime;
    Clock::CycleWatch rayTestTime;
    Clock::CycleWatch terrainTime;

    #define START(x) x.Start()
    #define STOP(x) x.Stop()
  #else
    #define START(x)
    #define STOP(x)
  #endif


  //
  // Initialise the collision system
  //
  void Init()
  {
    ASSERT(!sysInit);
    sysInit = TRUE;
  }


  //
  // Shutdown the collision system
  //
  void Done()
  {
    ASSERT(sysInit);
    sysInit = FALSE;
  }

  //
  // Add the object to the collision list
  //
  void AddObject(MapObj *obj)
  {
    objList.Append(obj);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Resolver - Collision resolver
  //
  namespace Resolver
  {

    // Max number of collisions to process for an object
    static const U32 MAX_COLLISIONS = 16;

    // Collision structure
    struct CollidedStruct
    {
      // Distance from original position that collision occured
      F32 dist;

      // Position of collision
      Vector pos;

      // Object collided with (NULL for ground)
      MapObj *with;
    };


    // Collisions that have occured for this object
    static CollidedStruct collisions[MAX_COLLISIONS];

    // Number of collisions
    static U32 collisionCount;

    // Pointers to mapobjects involved in collision
    static MapObj *obj1;
    static MapObj *obj2;
    static ProjectileObj *proj1;

    // Velocity of each object
    static const Vector *veloc1;
    static const Vector *veloc2;


    //
    // qsort compare function for collisions
    //
    static int CDECL SortCollision(const void *e1, const void *e2)
    {
      CollidedStruct *c1 = (CollidedStruct *)e1;
      CollidedStruct *c2 = (CollidedStruct *)e2;

      if (c1->dist < c2->dist)
      {
        return -1;
      }
      else

      if (c1->dist> c2->dist)
      {
        return 1;
      }
      return 0;
    }


    //
    // Test for collisions with this object
    //
    static void Object()
    {
      ASSERT(obj1)

      collisionCount = 0;

      // Setup static data
      veloc1    = &obj1->GetVelocity();

      // Clear object 2
      obj2      = NULL;
      veloc2    = NULL;

      // Setup projectile
      if (obj1->MapType()->IsProjectile())
      {
        ASSERT((Promote::Object<ProjectileObjType, ProjectileObj>(obj1)))
        proj1 = (ProjectileObj *)obj1;
      }
      else
      {
        proj1 = NULL;
      }

      // Current position
      const Vector &worldPos = obj1->Position();

      // If object is not moving, don't do collision testing?
      if (veloc1->Magnitude2() > 1e-4F)
      {
        Vector targetPos = obj1->TargetPosition();
        Vector displacement = targetPos - worldPos;
        F32 dMag = displacement.Magnitude();
        F32 dMagInv = 1.0F / dMag;
        Vector dNorm = displacement * dMagInv;

        // Check for collisions with edge of map
        if (!obj1->CheckWorldPosition(targetPos))
        {
          CollidedStruct &cs = collisions[collisionCount++];

          // Using dMag will sort this collision after all others, as in theory
          // no object should be off the map, so edge of world collision should
          // be the last considered
          cs.dist = dMag;

          // CheckWorldPosition clamps targetPos to world boundary
          cs.pos  = targetPos;
          cs.with = NULL;
        }

        // Check for collisions with ground
        Vector hitPos;
        F32 dist = dMag;

        // Projectile's target
        const Target &target = proj1->GetTarget();

        START(terrainTime);

        // targetPos is guaranteed to be on the map from the previous CheckWworldPosition
        Bool testWater = (target.Valid() && target.GetType() == Target::LOCATION);
        Bool terrainTest = Ray::TerrainTest(worldPos, targetPos, dNorm, dist, hitPos, 0.0F, testWater);

        STOP(terrainTime);

        if (terrainTest)
        {
          CollidedStruct &cs = collisions[collisionCount++];

          cs.dist = dist;
          cs.pos  = hitPos;
          cs.with = NULL;
        }

#ifdef QUICK_COLLISIONS

        // Only check collision with target
        if (target.Valid() && target.GetType() == Target::OBJECT)
        {
          // Targetting an object, if its not our target and its an ally, ignore it
          MapObjPtr &ptr2 = target.GetObj();

          if (ptr2.Alive())
          {
            START(rayTestTime);

            Bool rayTest = Ray::Test(ptr2, worldPos, targetPos, dist, Ray::BOX);

            STOP(rayTestTime);

            if (rayTest)
            {
              CollidedStruct &cs = collisions[collisionCount++];

              cs.dist = F32(sqrt(dist));
              cs.pos  = worldPos + (displacement * (cs.dist * dMagInv));

              // Apply damage to the root object
              cs.with = ptr2;
            }
          }
        }

#else

        /*
        // Dont collide with firer or his allies
        UnitObj *firer = NULL;
        Team *firerTeam = NULL;
        
        if (proj1)
        {
          firer = proj1->GetSourceUnit();
          firerTeam = proj1->GetSourceTeam();
        }

        // Check for object collision
        MapCluster *cluster = obj1->currentCluster;
        ASSERT(obj1->currentCluster == WorldCtrl::MetresToCluster(worldPos.x, worldPos.z))

        // Check collisions with each other object in the cluster
        for (NList<MapObj>::Iterator objIt(&cluster->listObjs); *objIt; objIt++)
        {
          obj2 = *objIt;

          // Collide with root part
          MapObj *root2 = obj2;
          MapObj *p = obj2->GetParent();

          while (p)
          {
            root2 = p;
            p = p->GetParent();
          }

          // Check for collision with firer
          if (firer == root2)
          {
            continue;
          }

          // Or firer's allies
          //if (proj1->ProjectileType()->FriendlyFireCrap())

  #if 0
          if (firerTeam)
          {
            UnitObj *unit2 = Promote::Object<UnitObjType, UnitObj>(obj2);

            if (unit2)
            {
              const Target &target = proj1->GetTarget();

              if (target.GetType() == Target::OBJECT)
              {
                // Targetting an object, if its not our target and its an ally, ignore it
                MapObjPtr &ptr2 = target.GetObj();

                if (ptr2.Alive() && (ptr2 != obj2))
                {
                  if (Team::TestRelation(unit2, firerTeam, Relation::ALLY))
                  {
                    continue;
                  }
                }
              }
              else
              {
                // Targetting something else, try not to hit our allies
                if (Team::TestRelation(unit2, firerTeam, Relation::ALLY))
                {
                  continue;
                }
              }
            }
          }
  #else
          const Target &target = proj1->GetTarget();

          if (target.GetType() == Target::OBJECT)
          {
            MapObjPtr &ptr2 = target.GetObj();

            if (ptr2.Alive() && (ptr2 != obj2))
            {
              continue;
            }
          }
          else
          {
            continue;
          }
  #endif

          // Perform a ray test
          #ifdef DEVELOPMENT
            rayTestTime.Start();
          #endif

          Bool rayTest = Ray::Test(obj2, worldPos, targetPos, dist, Ray::BOX);

          #ifdef DEVELOPMENT
            rayTestTime.Stop();
          #endif

          if (rayTest)
          {
            CollidedStruct &cs = collisions[collisionCount++];

            cs.dist = F32(sqrt(dist));
            cs.pos  = worldPos + (displacement * (cs.dist * dMagInv));

            // Apply damage to the root object
            cs.with = root2;

            if (collisionCount == MAX_COLLISIONS)
            {
              LOG_ERR(("Too many collisions!"))
              break;
            }
          }
        }

  */

#endif

        // Sort collisions by their distance
        if (collisionCount)
        {
          if (collisionCount > 1)
          {
            qsort(collisions, collisionCount, sizeof(CollidedStruct), SortCollision);
          }

          // The first collision is the closest
          CollidedStruct &cs = collisions[0];

          // Move first object back to collided pos
          obj1->SetSimCurrent(cs.pos);

#if 0 // Impulse is ignored
          Vector *collideVeloc;

          if (cs.with)
          {
            collideVeloc = cs.with->GetVelocity();
          }
          else
          {
            collideVeloc = Matrix::I.posit;
          }
#endif

          // Inform the first object that it collided
          obj1->Collided(cs.with, NULL);

          // Inform the other object that it was hit
          if (cs.with)
          {
            cs.with->Collided(obj1, NULL);
          }
        }
      }
    }
  }


  //
  // Resolve collisions
  //
  void Resolve()
  {
    ASSERT(sysInit);

    MapObjList::Iterator i(&objList);
    MapObjListNode *node;

    while ((node = i++) != NULL)
    {
      if (node->Alive())
      {
        // Setup resolver static data
        Resolver::obj1 = *node;

        START(objTime);

        // Resolve this bullet
        Resolver::Object();

        STOP(objTime);
      }

      // Delete the node
      objList.Dispose(node);
    }

    ASSERT(objList.GetCount() == 0);

    #ifdef DEVELOPMENT
      MSWRITEV(13, ( 7, 0, "TerrTest  %s", terrainTime.Report()));
      MSWRITEV(13, ( 8, 0, "Obj-Obj   %s", objTime.Report()));
      MSWRITEV(13, ( 9, 0, "Ray test  %s", rayTestTime.Report()));
    #endif
  }


  //
  // Console Command handler
  //
  void CmdHandler(U32)
  {
  }
}
