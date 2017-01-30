///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Rain         for studio_weather
//
// 30-OCT-1999
//

namespace Environment
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Environment::Rain::
  //
  namespace Rain
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Environment::Rain::Splat
    //
    namespace Splat
    {
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat:: forward references
      //
      struct Type;
      struct Object;

      // Rain::Splat:: data members
      //
      //
      extern BinTree<Type>   typeList;
      extern Type *   	     type;       // current Splat scope data for CmdHandler()        ;

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Type
      //
      // one for each different kind of splat
      //
      struct Type
      {
        ///////////////////////////////////////////////////////////////////////////////
        //
        // Rain::Splat::Type:: data members
        //
        // type data
        //
        enum TYPE
        {
          NONE = 0,
	        GROUND,
	        WATER,
          END
        };

        GameIdent     name;

	      TYPE	        type;  	        // Are we ground, water or windshield?

        U32           count;					// number of splats in this set
		    Color		      color;
        GameIdent     texture;
        U32           blend;

		    Bool		      centered;				// Is it centered at the position?
  	    F32           offset;         // in front of the camera
		    F32           minWidth;
        F32           maxWidth;   		// Splat will grow using these limits
		    F32		        minHeight;
        F32           maxHeight;
		    Vector		    velocity;				// Direction of the splat motion (slide on windshield)

		    F32		        lifeSpan;				// Average time to keep alive (or dead)
		    F32		        minDist;		    // Don't draw if too close or too far

        // live data
        //
        Bitmap *      tex;          
        Object *      object;				    // Offset into a buffer
		    Vector		    right, up;				// Orientation of object

        ///////////////////////////////////////////////////////////////////////////////
        //
        // Rain::Splat::Type:: function members
        //
  	    void Process( F32 dt);
        void Render();

        void LoadInfo( FScope *fScope);
	      void SaveInfo( FScope *fScope);

        void PostLoad();

        Type( const char *s, TYPE _type, Type * copyType = NULL);

        void SetupDialog();
        void Setup();

        Bool CmdHandler(U32 pathCrc);

	    }; // Rain::Splat::Type

    } // Environment::Rain::Splat


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain:: forward references
    //
    struct Type;
    struct Object;

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain:: data members
    //
    // type data
    // 
    extern BinTree<Type>      typeList;           // available Rain::Type 's
    extern Type *             type;               // current Rain::Type:: scope

    extern VarString          groundSplatVar;
    extern VarString          waterSplatVar;
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type
    //
    // one for each kind of rain (water, snow, fireballs?)
    //
    struct Type
    { 
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Type:: data members
      //
      // type data
      //
      GameIdent     name;

	    F32           blockSize;
	    F32           blockHeight;
	    U32           grids;					      
	    U32           count;						
	    F32           scale;						
	    F32           width;						
	    F32           height;
	    F32           speed;
	    Color         color;
	    U32           blend;
	    F32           minDist;						  
	    Vector	      direction;						
      F32           chanceOfStrike;
      GameIdent     sound;

	    GameIdent     texture;
      F32           animRate;

      U32           hasSplats : 1;

      GameIdent     groundSplatName;
      GameIdent     waterSplatName;

      // live data
      //
      Splat::Type * groundSplat;
      Splat::Type * waterSplat;
      Bitmap *      tex;          

      F32           dirVal;
      F32           rotVal;

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Type:: function members
      //
      void Render();

      void LoadInfo( FScope *fScope);
	    void SaveInfo( FScope *fScope);

      void PostLoad();

      Type( const char *s, Type * copyType = NULL);

      void Setup();

      Bool CmdHandler(U32 pathCrc);

    }; // Rain::Type

  } // Environment::Rain

} // Environment::

