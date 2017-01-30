///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// viderror.cpp
//
// 06-APR-1998
//

#include "vid.h"
//-----------------------------------------------------------------------------

HRESULT dxError;

char *GetErrorString( HRESULT error)
{
  switch (error) {
	default:
		return "Unrecognized error value.";
	case DD_OK:
		return( "No Error.");

	case ERROR_INVALID_FUNCTION:
		return( "ERROR_INVALID_FUNCTION."); 
	case ERROR_NOT_ENOUGH_MEMORY:
		return( "ERROR_NOT_ENOUGH_MEMORY.");
	case ERROR_DEV_NOT_EXIST:
		return( "ERROR_DEV_NOT_EXIST.");
	case ERROR_INVALID_PARAMETER:
		return( "ERROR_INVALID_PARAMETER.");
	case ERROR_INSUFFICIENT_BUFFER:
		return( "ERROR_INSUFFICIENT_BUFFER.");
	case ERROR_MORE_DATA:
		return( "ERROR_MORE_DATA.");
	case ERROR_IO_PENDING:
		return( "ERROR_IO_PENDING.");

	case DDERR_ALREADYINITIALIZED:
		return( "The object has already been initialized.");
	case DDERR_BLTFASTCANTCLIP:
		return( "A DirectDrawClipper object is attached to a source surface that has passed into a call to the IDirectDrawSurface3::BltFast method.");
	case DDERR_CANNOTATTACHSURFACE:
		return( "A surface cannot be attached to another requested surface.");
	case DDERR_CANNOTDETACHSURFACE:
		return( "A surface cannot be detached from another requested surface.");
	case DDERR_CANTCREATEDC:
		return( "Windows cannot create any more device contexts (DCs).");
	case DDERR_CANTDUPLICATE:
		return( "Primary and 3-D surfaces, or surfaces that are implicitly created, cannot be duplicated.");
	case DDERR_CANTLOCKSURFACE:
		return( "Access to this surface is refused because an attempt was made to lock the primary surface without DCI support.");
	case DDERR_CANTPAGELOCK:
		return( "An attempt to page lock a surface failed. Page lock will not work on a display-memory surface or an emulated primary surface.");
	case DDERR_CANTPAGEUNLOCK:
		return( "An attempt to page unlock a surface failed. Page unlock will not work on a display-memory surface or an emulated primary surface.");
	case DDERR_CLIPPERISUSINGHWND:
		return( "An attempt was made to set a clip list for a DirectDrawClipper object that is already monitoring a window handle.");
	case DDERR_COLORKEYNOTSET:
		return( "No source color key is specified for this operation.");
	case DDERR_CURRENTLYNOTAVAIL:
		return( "No support is currently available.");
	case DDERR_DCALREADYCREATED:
		return( "A device context (DC) has already been returned for this surface. Only one DC can be retrieved for each surface.");
	case DDERR_DEVICEDOESNTOWNSURFACE:
		return( "Surfaces created by one direct draw device cannot be used directly by another direct draw device.");
	case DDERR_DIRECTDRAWALREADYCREATED:
		return( "A DirectDraw object representing this driver has already been created for this process.");
	case DDERR_EXCEPTION:
		return( "An exception was encountered while performing the requested operation.");
	case DDERR_EXCLUSIVEMODEALREADYSET:
		return( "An attempt was made to set the cooperative level when it was already set to exclusive.");
	case DDERR_GENERIC:
		return( "There is an undefined error condition.");
	case DDERR_HEIGHTALIGN:
		return( "The height of the provided rectangle is not a multiple of the required alignment.");
	case DDERR_HWNDALREADYSET:
		return( "The DirectDraw cooperative level window handle has already been set. It cannot be reset while the process has surfaces or palettes created.");
	case DDERR_HWNDSUBCLASSED:
		return( "DirectDraw is prevented from restoring state because the DirectDraw cooperative level window handle has been subclassed.");
	case DDERR_IMPLICITLYCREATED:
		return( "The surface cannot be restored because it is an implicitly created surface.");
	case DDERR_INCOMPATIBLEPRIMARY:
		return( "The primary surface creation request does not match with the existing primary surface.");
	case DDERR_INVALIDCAPS:
		return( "One or more of the capability bits passed to the callback function are incorrect.");
	case DDERR_INVALIDCLIPLIST:
		return( "DirectDraw does not support the provided clip list.");
	case DDERR_INVALIDDIRECTDRAWGUID:
		return( "The globally unique identifier (GUID) passed to the DirectDrawCreate function is not a valid DirectDraw driver identifier.");
	case DDERR_INVALIDMODE:
		return( "DirectDraw does not support the requested mode.");
	case DDERR_INVALIDOBJECT:
		return( "DirectDraw received a pointer that was an invalid DirectDraw object.");
	case DDERR_INVALIDPARAMS:
		return( "One or more of the parameters passed to the method are incorrect.");
	case DDERR_INVALIDPIXELFORMAT:
		return( "The pixel format was invalid as specified.");
	case DDERR_INVALIDPOSITION:
		return( "The position of the overlay on the destination is no longer legal.");
	case DDERR_INVALIDRECT:
		return( "The provided rectangle was invalid.");
	case DDERR_INVALIDSURFACETYPE:
		return( "The requested operation could not be performed because the surface was of the wrong type.");
	case DDERR_LOCKEDSURFACES:
		return( "One or more surfaces are locked, causing the failure of the requested operation.");
	case DDERR_MOREDATA:
		return( "There is more data available than the specified buffer size can hold.");
	case DDERR_NO3D:
		return( "No 3-D hardware or emulation is present.");
	case DDERR_NOALPHAHW:
		return( "No alpha acceleration hardware is present or available, causing the failure of the requested operation.");
	case DDERR_NOBLTHW:
		return( "No blitter hardware is present.");
	case DDERR_NOCLIPLIST:
		return( "No clip list is available.");
	case DDERR_NOCLIPPERATTACHED:
		return( "No DirectDrawClipper object is attached to the surface object.");
	case DDERR_NOCOLORCONVHW:
		return( "The operation cannot be carried out because no color-conversion hardware is present or available.");
	case DDERR_NOCOLORKEY:
		return( "The surface does not currently have a color key.");
	case DDERR_NOCOLORKEYHW:
		return( "The operation cannot be carried out because there is no hardware support for the destination color key.");
	case DDERR_NOCOOPERATIVELEVELSET:
		return( "A create function is called without the IDirectDraw2::SetCooperativeLevel method being called.");
	case DDERR_NODC:
		return( "No DC has ever been created for this surface.");
	case DDERR_NODDROPSHW:
		return( "No DirectDraw raster operation (ROP) hardware is available.");
	case DDERR_NODIRECTDRAWHW:
		return( "Hardware-only DirectDraw object creation is not possible; the driver does not support any hardware.");
	case DDERR_NODIRECTDRAWSUPPORT:
		return( "DirectDraw support is not possible with the current display driver.");
	case DDERR_NOEMULATION:
		return( "Software emulation is not available.");
	case DDERR_NOEXCLUSIVEMODE:
		return( "The operation requires the application to have exclusive mode, but the application does not have exclusive mode.");
	case DDERR_NOFLIPHW:
		return( "Flipping visible surfaces is not supported.");
	case DDERR_NOGDI:
		return( "No GDI is present.");
	case DDERR_NOHWND:
		return( "Clipper notification requires a window handle, or no window handle has been previously set as the cooperative level window handle.");
	case DDERR_NOMIPMAPHW:
		return( "The operation cannot be carried out because no mipmap texture mapping hardware is present or available.");
	case DDERR_NOMIRRORHW:
		return( "The operation cannot be carried out because no mirroring hardware is present or available.");
	case DDERR_NONONLOCALVIDMEM:
		return( "An attempt was made to allocate non-local video memory from a device that does not support non-local video memory.");
	case DDERR_NOOPTIMIZEHW:
		return( "The device does not support optimized surfaces.");
	case DDERR_NOOVERLAYDEST:
		return( "The IDirectDrawSurface3::GetOverlayPosition method is called on an overlay that the IDirectDrawSurface3::UpdateOverlay method has not been called on to establish a destination.");
	case DDERR_NOOVERLAYHW:
		return( "The operation cannot be carried out because no overlay hardware is present or available.");
	case DDERR_NOPALETTEATTACHED:
		return( "No palette object is attached to this surface.");
	case DDERR_NOPALETTEHW:
		return( "There is no hardware support for 16- or 256-color palettes.");
	case DDERR_NORASTEROPHW:
		return( "The operation cannot be carried out because no appropriate raster operation hardware is present or available.");
	case DDERR_NOROTATIONHW:
		return( "The operation cannot be carried out because no rotation hardware is present or available.");
	case DDERR_NOSTRETCHHW:
		return( "The operation cannot be carried out because there is no hardware support for stretching.");
	case DDERR_NOT4BITCOLOR:
		return( "The DirectDrawSurface object is not using a 4-bit color palette and the requested operation requires a 4-bit color palette.");
	case DDERR_NOT4BITCOLORINDEX:
		return( "The DirectDrawSurface object is not using a 4-bit color index palette and the requested operation requires a 4-bit color index palette.");
	case DDERR_NOT8BITCOLOR:
		return( "The DirectDrawSurface object is not using an 8-bit color palette and the requested operation requires an 8-bit color palette.");
	case DDERR_NOTAOVERLAYSURFACE:
		return( "An overlay component is called for a non-overlay surface.");
	case DDERR_NOTEXTUREHW:
		return( "The operation cannot be carried out because no texture-mapping hardware is present or available.");
	case DDERR_NOTFLIPPABLE:
		return( "An attempt has been made to flip a surface that cannot be flipped.");
	case DDERR_NOTFOUND:
		return( "The requested item was not found.");
	case DDERR_NOTINITIALIZED:
		return( "An attempt was made to call an interface method of a DirectDraw object created by CoCreateInstance before the object was initialized.");
	case DDERR_NOTLOADED:
		return( "The surface is an optimized surface, but it has not yet been allocated any memory.");
	case DDERR_NOTLOCKED:
		return( "An attempt is made to unlock a surface that was not locked.");
	case DDERR_NOTPAGELOCKED:
		return( "An attempt is made to page unlock a surface with no outstanding page locks.");
	case DDERR_NOTPALETTIZED:
		return( "The surface being used is not a palette-based surface.");
	case DDERR_NOVSYNCHW:
		return( "The operation cannot be carried out because there is no hardware support for vertical blank synchronized operations.");
	case DDERR_NOZBUFFERHW:
		return( "The operation to create a z-buffer in display memory or to perform a blit using a z-buffer cannot be carried out because there is no hardware support for z-buffers.");
	case DDERR_NOZOVERLAYHW:
		return( "The overlay surfaces cannot be z-layered based on the z-order because the hardware does not support z-ordering of overlays.");
	case DDERR_OUTOFCAPS:
		return( "The hardware needed for the requested operation has already been allocated.");
	case DDERR_OUTOFMEMORY:
		return( "DirectDraw does not have enough memory to perform the operation.");
	case DDERR_OUTOFVIDEOMEMORY:
		return( "DirectDraw does not have enough display memory to perform the operation.");
	case DDERR_OVERLAYCANTCLIP:
		return( "The hardware does not support clipped overlays.");
	case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
		return( "An attempt was made to have more than one color key active on an overlay.");
	case DDERR_OVERLAYNOTVISIBLE:
		return( "The IDirectDrawSurface3::GetOverlayPosition method is called on a hidden overlay.");
	case DDERR_PALETTEBUSY:
		return( "Access to this palette is refused because the palette is locked by another thread.");
	case DDERR_PRIMARYSURFACEALREADYEXISTS:
		return( "This process has already created a primary surface.");
	case DDERR_REGIONTOOSMALL:
		return( "The region passed to the IDirectDrawClipper::GetClipList method is too small.");
	case DDERR_SURFACEALREADYATTACHED:
		return( "An attempt was made to attach a surface to another surface to which it is already attached.");
	case DDERR_SURFACEALREADYDEPENDENT:
		return( "An attempt was made to make a surface a dependency of another surface to which it is already dependent.");
	case DDERR_SURFACEBUSY:
		return( "Access to the surface is refused because the surface is locked by another thread.");
	case DDERR_SURFACEISOBSCURED:
		return( "Access to the surface is refused because the surface is obscured.");
	case DDERR_SURFACELOST:
		return( "Access to the surface is refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have the IDirectDrawSurface3::Restore method called on it.");
	case DDERR_SURFACENOTATTACHED:
		return( "The requested surface is not attached.");
	case DDERR_TOOBIGHEIGHT:
		return( "The height requested by DirectDraw is too large.");
	case DDERR_TOOBIGSIZE:
		return( "The size requested by DirectDraw is too large. However, the individual height and width are OK.");
	case DDERR_TOOBIGWIDTH:
		return( "The width requested by DirectDraw is too large.");
	case DDERR_UNSUPPORTED:
		return( "The operation is not supported.");
	case DDERR_UNSUPPORTEDFORMAT:
		return( "The FourCC format requested is not supported by DirectDraw.");
	case DDERR_UNSUPPORTEDMASK:
		return( "The bitmask in the pixel format requested is not supported by DirectDraw.");
	case DDERR_UNSUPPORTEDMODE:
		return( "The display is currently in an unsupported mode.");
	case DDERR_VERTICALBLANKINPROGRESS:
		return( "A vertical blank is in progress.");
	case DDERR_VIDEONOTACTIVE:
		return( "The video port is not active.");
	case DDERR_WASSTILLDRAWING:
		return( "The previous blit operation that is transferring information to or from this surface is incomplete.");
	case DDERR_WRONGMODE:
		return( "This surface cannot be restored because it was created in a different mode.");
	case DDERR_XALIGN:
		return( "The provided rectangle was not horizontally aligned on a required boundary.");

	case DSERR_ALLOCATED:
		return( "The request failed because resources, such as a priority level, were already in use by another caller.");
	case DSERR_ALREADYINITIALIZED:
		return( "The object is already initialized.");
	case DSERR_BADFORMAT:
		return( "The specified wave format is not supported.");
	case DSERR_BUFFERLOST:
		return( "The buffer memory has been lost and must be restored.");
	case DSERR_CONTROLUNAVAIL:
		return( "The control (volume, pan, and so forth) requested by the caller is not available.");
	case DSERR_INVALIDCALL:
		return( "This function is not valid for the current state of this object.");
	case DSERR_NOAGGREGATION:
		return( "The object does not support aggregation.");
	case DSERR_NODRIVER:
		return( "No sound driver is available for use.");
	case DSERR_NOINTERFACE:
		return( "The requested COM interface is not available.");
	case DSERR_OTHERAPPHASPRIO:
		return( "Another application has a higher priority level, preventing this call from succeeding.");
	case DSERR_PRIOLEVELNEEDED:
		return( "The caller does not have the priority level required for the function to succeed.");
	case DSERR_UNINITIALIZED:
		return( "The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called.");

	case D3DERR_BADMAJORVERSION:
      return "D3DERR_BADMAJORVERSION";
  case D3DERR_BADMINORVERSION:
      return "D3DERR_BADMINORVERSION"; 
  case D3DERR_DEVICEAGGREGATED: 
      return "D3DERR_DEVICEAGGREGATED";
  case D3DERR_INITFAILED:
      return "D3DERR_INITFAILED";
  case D3DERR_INBEGIN:
      return "D3DERR_INBEGIN";
  case D3DERR_INVALID_DEVICE:
      return "D3DERR_INVALID_DEVICE";
  case D3DERR_INVALIDCURRENTVIEWPORT:
      return "D3DERR_INVALIDCURRENTVIEWPORT";
  case D3DERR_INVALIDPALETTE:
      return "D3DERR_INVALIDPALETTE";
  case D3DERR_INVALIDPRIMITIVETYPE:
      return "D3DERR_INVALIDPRIMITIVETYPE";
  case D3DERR_INVALIDRAMPTEXTURE:
      return "D3DERR_INVALIDRAMPTEXTURE";
  case D3DERR_INVALIDVERTEXTYPE:
      return "D3DERR_INVALIDVERTEXTYPE";

  case D3DERR_EXECUTE_LOCKED:
      return "D3DERR_EXECUTE_LOCKED";
  case D3DERR_EXECUTE_NOT_LOCKED:
      return "D3DERR_EXECUTE_NOT_LOCKED";
  case D3DERR_EXECUTE_CREATE_FAILED:
      return "D3DERR_EXECUTE_CREATE_FAILED";
  case D3DERR_EXECUTE_DESTROY_FAILED:
      return "D3DERR_EXECUTE_DESTROY_FAILED";
  case D3DERR_EXECUTE_LOCK_FAILED:
      return "D3DERR_EXECUTE_LOCK_FAILED";
  case D3DERR_EXECUTE_UNLOCK_FAILED:
      return "D3DERR_EXECUTE_UNLOCK_FAILED";
  case D3DERR_EXECUTE_FAILED:
      return "D3DERR_EXECUTE_FAILED";
  case D3DERR_EXECUTE_CLIPPED_FAILED:
      return "D3DERR_EXECUTE_CLIPPED_FAILED";

  case D3DERR_TEXTURE_NO_SUPPORT:
      return "D3DERR_TEXTURE_NO_SUPPORT";
  case D3DERR_TEXTURE_NOT_LOCKED:
      return "D3DERR_TEXTURE_NOT_LOCKED";
  case D3DERR_TEXTURE_LOCKED:
      return "D3DERR_TEXTURELOCKED";
  case D3DERR_TEXTURE_CREATE_FAILED:
      return "D3DERR_TEXTURE_CREATE_FAILED";
  case D3DERR_TEXTURE_DESTROY_FAILED:
      return "D3DERR_TEXTURE_DESTROY_FAILED";
  case D3DERR_TEXTURE_LOCK_FAILED:
      return "D3DERR_TEXTURE_LOCK_FAILED";
  case D3DERR_TEXTURE_UNLOCK_FAILED:
      return "D3DERR_TEXTURE_UNLOCK_FAILED";
  case D3DERR_TEXTURE_LOAD_FAILED: 
      return "D3DERR_TEXTURE_LOAD_FAILED";
  case D3DERR_TEXTURE_BADSIZE:
      return "D3DERR_TEXTURE_BADSIZE";
  case D3DERR_TEXTURE_GETSURF_FAILED:
      return "D3DERR_TEXTURE_GETSURF_FAILED";
  case D3DERR_TEXTURE_SWAP_FAILED:
      return "D3DERR_TEXTURE_SWAP_FAILED"; 

  case D3DERR_MATRIX_CREATE_FAILED:
      return "D3DERR_MATRIX_CREATE_FAILED";
  case D3DERR_MATRIX_DESTROY_FAILED:
      return "D3DERR_MATRIX_DESTROY_FAILED";
  case D3DERR_MATRIX_SETDATA_FAILED:
      return "D3DERR_MATRIX_SETDATA_FAILED";
  case D3DERR_MATRIX_GETDATA_FAILED:
      return "D3DERR_MATRIX_GETDATA_FAILED";

  case D3DERR_MATERIAL_CREATE_FAILED:
      return "D3DERR_MATERIAL_CREATE_FAILED";
  case D3DERR_MATERIAL_DESTROY_FAILED:
      return "D3DERR_MATERIAL_DESTROY_FAILED";
  case D3DERR_MATERIAL_SETDATA_FAILED:
      return "D3DERR_MATERIAL_SETDATA_FAILED";
  case D3DERR_MATERIAL_GETDATA_FAILED:
      return "D3DERR_MATERIAL_GETDATA_FAILED";

  case D3DERR_LIGHT_SET_FAILED:
      return "D3DERR_LIGHT_SET_FAILED";
  case D3DERR_LIGHTHASVIEWPORT:
      return "D3DERR_LIGHTHASVIEWPORT";
  case D3DERR_LIGHTNOTINTHISVIEWPORT:
      return "D3DERR_LIGHTNOTINTHISVIEWPORT";

  case D3DERR_NOCURRENTVIEWPORT:
      return "D3DERR_NOCURRENTVIEWPORT";
  case D3DERR_NOTINBEGIN: 
      return "D3DERR_NOTINBEGIN";
  case D3DERR_NOVIEWPORTS:
      return "D3DERR_NOVIEWPORTS";
  case D3DERR_SCENE_BEGIN_FAILED:
      return "D3DERR_SCENE_BEGIN_FAILED";
  case D3DERR_SCENE_END_FAILED:
      return "D3DERR_SCENE_END_FAILED";
  case D3DERR_SCENE_IN_SCENE:
      return "D3DERR_SCENE_IN_SCENE";
  case D3DERR_SCENE_NOT_IN_SCENE:
      return "D3DERR_SCENE_NOT_IN_SCENE";
  case D3DERR_SETVIEWPORTDATA_FAILED:
      return "D3DERR_SETVIEWPORTDATA_FAILED"; 
  case D3DERR_SURFACENOTINVIDMEM: 
      return "D3DERR_SURFACENOTINVIDMEM"; 
  case D3DERR_VIEWPORTDATANOTSET:
      return "D3DERR_VIEWPORTDATANOTSET";
  case D3DERR_VIEWPORTHASNODEVICE:
      return "D3DERR_VIEWPORTHASNODEVICE";
  case D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY: 
      return "D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY";
  case D3DERR_ZBUFF_NEEDS_VIDEOMEMORY: 
      return "D3DERR_ZBUFF_NEEDS_VIDEOMEMORY";

  //case DI_NOTATTACHED :
  //case DI_BUFFEROVERFLOW :

//  case DI_PROPNOEFFECT :
//    return "DI_NOTATTACHED, DI_BUFFEROVERFLOW, DI_PROPNOEFFECT : The device exists but is not currently attached, OR The device buffer overflowed; some input was lost, OR The change in device properties had no effect.";
  case DI_POLLEDDEVICE :
    return "DI_POLLEDDEVICE : The device is a polled device.  As a result, device buffering will not collect any data and event notifications will not be signalled until GetDeviceState is called.";
  case DIERR_OLDDIRECTINPUTVERSION :
    return "DIERR_OLDDIRECTINPUTVERSION : The application requires a newer version of DirectInput.";
  case DIERR_BETADIRECTINPUTVERSION :
    return "DIERR_BETADIRECTINPUTVERSION : The application was written for an unsupported prerelease version of DirectInput.";
  case DIERR_BADDRIVERVER :           
    return "DIERR_BADDRIVERVER : The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.";
  case DIERR_DEVICENOTREG :           
    return "DIERR_DEVICENOTREG : The device or device instance is not registered with DirectInput.";
  case DIERR_OBJECTNOTFOUND :         
    return "DIERR_OBJECTNOTFOUND : The requested object does not exist.";
//  case DIERR_INVALIDPARAM :           
//    return "DIERR_INVALIDPARAM : An invalid parameter was passed to the returning function, or the object was not in a state that admitted the function to be called.";
//  case DIERR_NOINTERFACE :            
//    return "DIERR_NOINTERFACE : The specified interface is not supported by the object";
//  case DIERR_GENERIC :                
//    return "DIERR_NOINTERFACE : An undetermined error occured inside the DInput subsystem";
//  case DIERR_OUTOFMEMORY :            
//    return "DIERR_OUTOFMEMORY : The DInput subsystem couldn't allocate sufficient memory to complete the caller's request.";
//  case DIERR_UNSUPPORTED :            
//    return "DIERR_OUTOFMEMORY : The function called is not supported at this time";
  case DIERR_NOTINITIALIZED :         
    return "DIERR_OUTOFMEMORY : This object has not been initialized";
  case DIERR_ALREADYINITIALIZED :     
    return "DIERR_ALREADYINITIALIZED : This object is already initialized";
//  case DIERR_NOAGGREGATION :          
//    return "DIERR_NOAGGREGATION : This object does not support aggregation";
  case DIERR_INPUTLOST :              
    return "DIERR_INPUTLOST : Access to the input device has been lost.  It must be re-acquired.";
  case DIERR_ACQUIRED :               
    return "DIERR_ACQUIRED : The operation cannot be performed while the device is acquired.";
  case DIERR_NOTACQUIRED :            
    return "DIERR_NOTACQUIRED : The operation cannot be performed unless the device is acquired.";
  //case DIERR_OTHERAPPHASPRIO :
  //case DIERR_READONLY :
  case DIERR_HANDLEEXISTS :           
    return "DIERR_OTHERAPPHASPRIO, DIERR_READONLY, DIERR_HANDLEEXISTS Another app has a higher priority level, preventing this call from succeeding, OR The device already has an event notification associated with it, OR The specified property cannot be changed.";
  case E_PENDING :                    
    return "E_PENDING : Data is not yet available.";
  }
}
//----------------------------------------------------------------------------
