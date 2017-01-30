
#include "magick.h"
#include "defines.h"

Export Image *AddNoiseImage(Image *image,const NoiseType noise_type)
{
	return NULL;
}

Export void AnnotateImage(Image *image,const AnnotateInfo *annotate_info)
{}

Export Image *BlurImage(Image *image,const double factor)
{
	return image;
}

Export Image *BorderImage(Image *image,const RectangleInfo *border_info)
{
	return image;
}

Export Image *ChopImage(Image *image,const RectangleInfo *chop_info)
{
	return image;
}

Export AnnotateInfo *CloneAnnotateInfo(const ImageInfo *image_info, const AnnotateInfo *annotate_info)
{
	return NULL;
}

Export Image *CoalesceImages(Image *images)
{
	return images;
}

Export void ColorFloodfillImage(Image *image,const PixelPacket *target, Image *tile,const int x_offset,const int y_offset,const PaintMethod method)
{}

Export Image *ColorizeImage(Image *image,const char *opacity, const char *pen_color)
{
	return image;
}

Export void ContrastImage(Image *image,const unsigned int sharpen)
{}

Export Image *CropImage(Image *image,const RectangleInfo *crop_info)
{
	return image;
}

Export Image *DespeckleImage(Image *image)
{
	return image;
}

Export void DestroyAnnotateInfo(AnnotateInfo *annotate_info)
{}

Export void DrawImage(Image *image,const AnnotateInfo *annotate_info)
{}

Export Image *EdgeImage(Image *image,const double factor)
{
	return image;
}

Export Image *EmbossImage(Image *image)
{
	return image;
}

Export Image *EnhanceImage(Image *image)
{
	return image;
}

Export void EqualizeImage(Image *image)
{}

Export Image *FlipImage(Image *image)
{
	return image;
}

Export Image *FlopImage(Image *image)
{
	return image;
}

Export Image *FrameImage(Image *image,const FrameInfo *frame_info)
{
	return image;
}

Export void GammaImage(Image *image,const char *gamma)
{}

Export Image *ImplodeImage(Image *image,const double factor)
{
	return image;
}

Export Image *MagnifyImage(Image *image)
{
	return image;
}

Export void MatteFloodfillImage(Image *image,const PixelPacket *target, const unsigned int matte,const int x_offset,const int y_offset, const PaintMethod method)
{}

Export Image *MedianFilterImage(Image *image,const unsigned int radius)
{
	return image;
}

Export Image *MinifyImage(Image *image)
{
	return image;
}

Export void ModulateImage(Image *image,const char *modulate)
{}

Export void NegateImage(Image *image,const unsigned int grayscale)
{}

Export void NormalizeImage(Image *image)
{}

Export Image *OilPaintImage(Image *image,const unsigned int radius)
{
	return image;
}

Export void OpaqueImage(Image *image,const char *opaque_color, const char *pen_color)
{}

Export void ProgressMonitor(const char *text,const off_t quantum,
  const off_t span)
{}

Export void RaiseImage(Image *image,const RectangleInfo *raise_info, const int raised)
{}

Export Image *ReduceNoiseImage(Image *image)
{
	return image;
}

Export Image *RollImage(Image *image,const int x_offset,const int y_offset)
{
	return image;
}

Export Image *RotateImage(Image *image,const double degrees)
{
	return image;
}

Export Image *SampleImage(Image *image,const unsigned int columns, const unsigned int rows)
{
	return image;
}

Export Image *ScaleImage(Image *image,const unsigned int columns, const unsigned int rows)
{
	return image;
}

Export unsigned int SegmentImage(Image *image,const ColorspaceType colorspace, const unsigned int verbose,const double cluster_threshold, const double smoothing_threshold)
{
	return 0;
}

Export MonitorHandler SetMonitorHandler(MonitorHandler handler)
{
	return handler;
}

Export Image *ShadeImage(Image *image,const unsigned int color_shading, double azimuth,double elevation)
{
	return image;
}

Export Image *SharpenImage(Image *image,const double factor)
{
	return image;
}

Export Image *ShearImage(Image *image,const double x_shear,const double y_shear)
{
	return image;
}

Export void SolarizeImage(Image *image,const double factor)
{}

Export Image *SpreadImage(Image *image,const unsigned int amount)
{
	return image;
}

Export Image *SteganoImage(Image *image,Image *watermark)
{
	return image;
}

Export Image *StereoImage(Image *left_image,Image *right_image)
{
	return left_image;
}

Export Image *SwirlImage(Image *image,double degrees)
{
	return image;
}

Export void ThresholdImage(Image *image,const double threshold)
{}

Export void TransformImage(Image **image,const char *crop_geometry, const char *image_geometry)
{}

Export void TransparentImage(Image *image,const char *color)
{}

Export Image *WaveImage(Image *image,const double amplitude,
  const double wavelength)
{
	return image;
}

Export Image *ZoomImage(Image *image,const unsigned int columns, const unsigned int rows)
{
	return image;
}



