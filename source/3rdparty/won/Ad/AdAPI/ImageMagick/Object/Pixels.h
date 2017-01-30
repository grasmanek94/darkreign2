// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999
//
// Representation of a pixel region.
//
// This class is completely EXPERIMENTAL in nature and is expected to
// be completely re-defined in the near future.
//

#if !defined(Pixels_header)
#define Pixels_header

#include "Include.h"
#include "Color.h"
#include "Image.h"

// Class representing a pixel color. The Color class contains a
// pointer to PixelPacket where it stores its RGBA information.
// Incrementing and decrementing is done on the PixelPacket pointer
// contained in Color.

namespace Magick
{
  
  template <class ColorClass>
  class Pixels : public ColorClass
  {
  public:
    Pixels( Image &image )
      : ColorClass (),
	_image(image),
	_base(0),
	_last(0)
      {
      }
    
    //////////////////////////////////////////////////////////////////////
    //
    // Pixel Access Methods
    //
    //////////////////////////////////////////////////////////////////////
    
    // Transfers pixels from the image to the pixel cache as defined
    // by the specified region. Modified pixels may be subsequently
    // transferred back to the image via syncPixels.
    
    void getPixels ( int x_, int y_,
		     unsigned int columns_, unsigned int rows_ )
      {
	_base = MagickLib::GetPixelCache( _image.image(), x_, y_,
					  columns_, rows_ );
	_last = _base + columns_ * rows_;

	// Set pixel in Color
	pixel( _base, _image.colorSpace() == CMYKColorspace ? ColorClass::CYMKPixel :
	       ( _image.matte() ? ColorClass::RGBAPixel : RGBPixel ) );
      }
    
    // Transfers the image cache pixels to the image.
    void syncPixels ( void )
      {
	MagickLib::SyncPixelCache( _image.image() );
      }
    
    // Allocates a pixel cache region to store image pixels as defined
    // by the region rectangle.  This area is subsequently transferred
    // from the pixel cache to the image via syncPixels.
    void setPixels ( int x_, int y_,
		     unsigned int columns_, unsigned int rows_ )
      {
	_base = MagickLib::SetPixelCache( _image.image(), x_, y_,
					  columns_, rows_ );
	_last = _base + columns_ * rows_;

	// Set pixel in Color
	pixel( _base, _image.colorSpace() == CMYKColorspace ? ColorClass::CYMKPixel :
	       ( _image.matte() ? ColorClass::RGBAPixel : RGBPixel ) );
      }
    
    // Transfers one or more pixel components from a buffer or file
    // into the image pixel cache of an image.
    // Used to support image decoders.
    void readPixel ( QuantumTypes quantum_,
		     unsigned char *source_ )
      {
	MagickLib::ReadPixelCache( _image.image(), quantum_, source_ );
      }
    
    // Transfers one or more pixel components from the image pixel
    // cache to a buffer or file.
    // Used to support image encoders.
    void writePixel ( QuantumTypes quantum_,
		      unsigned char *destination_ )
      {
	MagickLib::WritePixelCache( _image.image(), quantum_, destination_ );
      }
    
    //////////////////////////////////////////////////////////////////////
    //
    // Operators
    //
    //////////////////////////////////////////////////////////////////////
    
//     Pixels& operator * () const
//       {
// 	return *this;
//       }
    
//     Pixels* operator -> () const
//       {
// 	return this;
//       }

    // Increment to next pixel
    Pixels& operator ++ ()
      {
	if ( _pixel != _last )
	  ++_pixel;
	return *this;
      }
    
//     Pixels operator ++ (int)
//       {
// 	Pixels tmp = *this;
// 	++*this;
// 	return tmp;
//       }

    // Decrement to preceding pixel
    Pixels& operator -- ()
      {
	if ( _pixel != _base )
	  --_pixel;
	return *this;
      }

    // Compare equality
    bool operator == ( const Pixels& iter_ )
      {
	return (_pixel == iter_._pixel);
      }

    // Compare inequality
    bool operator != ( const Pixels& iter_ )
      {
	return (_pixel != iter_._pixel);
      }

  private:
      
    Image                       _image;
    MagickLib::PixelPacket*     _base;
    MagickLib::PixelPacket*     _last;
  }; // class Pixels

  // Typedefs to represent specific pixel types
  typedef Pixels <ColorGray> PixelsGray;
  typedef Pixels <ColorHSL>  PixelsHSL;
  typedef Pixels <ColorMono> PixelsMono;
  typedef Pixels <ColorRGB>  PixelsRGB;
  typedef Pixels <ColorYUV>  PixelsYUV;

} // Magick namespace

#endif // Pixels_header
