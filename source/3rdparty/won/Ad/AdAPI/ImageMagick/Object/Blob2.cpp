// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999
//
// Implementation of Blob
//

#define MAGICK_IMPLEMENTATION

#include <cstring>
#include <new>

using namespace std;

#include "Thread.h"
#include "Include.h"
#include "Blob.h"

//
// Implementation of Magick::Blob
//

// Default constructor
Magick::Blob::Blob ( void )
{
  _blobRef = new Magick::BlobRef( 0, 0 );
}

// Construct with data
Magick::Blob::Blob ( const void* data_, size_t length_ )
{
  _blobRef = new Magick::BlobRef( data_, length_ );
}

// Copy constructor (reference counted)
Magick::Blob::Blob ( const Magick::Blob& blob_ )
  : _blobRef(blob_._blobRef)
{
  // Increase reference count
  Lock( _blobRef->_mutexLock );
  ++_blobRef->_refCount;
}

// Destructor (reference counted)
Magick::Blob::~Blob ()
{
  bool doDelete = false;
  {
    Lock( _blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }

  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
}

// Assignment operator (reference counted)
Magick::Blob Magick::Blob::operator= ( const Magick::Blob& blob_ )
{
  {
    Lock( blob_._blobRef->_mutexLock );
    ++blob_._blobRef->_refCount;
  }
  bool doDelete = false;
  {
    Lock( _blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }
  if ( doDelete )
    {
      delete _blobRef;
    }
  _blobRef = blob_._blobRef;

  return *this;
}

// Update object contents, making a copy of the supplied data.
// Any existing data in the object is deallocated.
void Magick::Blob::update ( const void* data_, size_t length_ )
{
  bool doDelete = false;
  {
    Lock( _blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }
  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }

  _blobRef = new Magick::BlobRef( data_, length_ );
}

// Update object contents, using supplied pointer directly (no copy)
// Any existing data in the object is deallocated.  The user must
// ensure that the pointer supplied is not deleted or otherwise
// modified after it has been supplied to this method.
void Magick::Blob::updateNoCopy ( void* data_, size_t length_ )
{
  bool doDelete = false;
  {
    Lock( _blobRef->_mutexLock );
    if ( --_blobRef->_refCount == 0 )
      doDelete = true;
  }
  if ( doDelete )
    {
      // Delete old blob reference with associated data
      delete _blobRef;
    }
  _blobRef = new Magick::BlobRef( 0, 0 );
  _blobRef->_data   = data_;
  _blobRef->_length = length_;
}

//
// Implementation of Magick::BlobRef
//

// Construct with data, making private copy of data
Magick::BlobRef::BlobRef ( const void* data_,
			   size_t length_ )
  : _data(0),
    _length(length_),
    _refCount(1)
{
  if ( data_ != 0 )
    {
      // Use ImageMagick's allocator to avoid possible problems from mixed
      // allocators
      _data = MagickLib::AllocateMemory( length_ );
      
      if ( _data == 0 )
	throw std::bad_alloc();
      
      memcpy( _data, data_, length_ );
    }
}

// Destructor (actually destroys data)
Magick::BlobRef::~BlobRef ( void )
{
  if ( _data )
    MagickLib::FreeMemory( _data );

  _data = 0;
  _length = 0;
  _refCount = 0;
}
