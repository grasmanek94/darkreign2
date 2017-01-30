/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  M   M   AAA    GGGG  IIIII   CCCC  K   K                   %
%                  MM MM  A   A  G        I    C      K  K                    %
%                  M M M  AAAAA  G GGG    I    C      KKK                     %
%                  M   M  A   A  G   G    I    C      K  K                    %
%                  M   M  A   A   GGGG  IIIII   CCCC  K   K                   %
%                                                                             %
%                                                                             %
%               Methods to Read or List ImageMagick Image formats             %
%                                                                             %
%                                                                             %
%                            Software Design                                  %
%                            Bob Friesenhahn                                  %
%                              John Cristy                                    %
%                             November 1998                                   %
%                                                                             %
%                                                                             %
%  Copyright (C) 2000 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

/*
  Global declarations.
*/
static MagickInfo
  *magick_info = (MagickInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M a g i c k I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DestroyMagickInfo deallocates memory associated MagickInfo list.
%
%  The format of the DestroyMagickInfo method is:
%
%      void DestroyMagickInfo()
%
*/
Export void DestroyMagickInfo()
{
  MagickInfo
    *entry;

  register MagickInfo
    *p;

  for (p=GetMagickInfo((char *) NULL); p != (MagickInfo *) NULL; )
  {
    entry=p;
    p=p->next;
    if (entry->tag != (char *) NULL)
      FreeMemory(entry->tag);
    if (entry->description != (char *) NULL)
      FreeMemory(entry->description);
    FreeMemory(entry);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M a g i c k L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetMagickInfo returns a pointer MagickInfo structure that matches
%  the specified tag.  If tag is NULL, the head of the image format list is
%  returned.
%
%  The format of the GetMagickInfo method is:
%
%      MagickInfo *GetMagickInfo(const char *tag)
%
%  A description of each parameter follows:
%
%    o magick_info: Method GetMagickInfo returns a pointer MagickInfo
%      structure that matches the specified tag.
%
%    o tag: a character string that represents the image format we are
%      looking for.
%
%
*/
Export MagickInfo *GetMagickInfo(const char *tag)
{
  register MagickInfo
    *p;

  if (magick_info == (MagickInfo *) NULL)
    {
      MagickInfo
        *entry;
/*
      entry=SetMagickInfo("8BIM");
      entry->decoder=Read8BIMImage;
      entry->encoder=Write8BIMImage;
      entry->magick=Is8BIM;
      entry->adjoin=False;
      entry->description=AllocateString("Photoshop resource format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("AVS");
      entry->decoder=ReadAVSImage;
      entry->encoder=WriteAVSImage;
      entry->description=AllocateString("AVS X image");
      RegisterMagickInfo(entry);
#if defined(HasJBIG)
      entry=SetMagickInfo("BIE");
      entry->decoder=ReadJBIGImage;
      entry->encoder=WriteJBIGImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Joint Bi-level Image experts Group interchange format");
      RegisterMagickInfo(entry);
#endif
*/
      entry=SetMagickInfo("BMP");
      entry->decoder=ReadBMPImage;
      entry->encoder=WriteBMPImage;
      entry->magick=IsBMP;
      entry->description=AllocateString("Microsoft Windows bitmap image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("BMP24");
      entry->decoder=ReadBMPImage;
      entry->encoder=WriteBMPImage;
      entry->description=
        AllocateString("Microsoft Windows 24-bit bitmap image");
      RegisterMagickInfo(entry);
/*
      entry=SetMagickInfo("CMYK");
      entry->decoder=ReadCMYKImage;
      entry->encoder=WriteCMYKImage;
      entry->adjoin=False;
      entry->raw=True;
      entry->description=
        AllocateString("Raw cyan, magenta, yellow, and black bytes");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("DCM");
      entry->decoder=ReadDCMImage;
      entry->magick=IsDCM;
      entry->adjoin=False;
      entry->description=
        AllocateString("Digital Imaging and Communications in Medicine image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("DCX");
      entry->decoder=ReadPCXImage;
      entry->encoder=WritePCXImage;
      entry->magick=IsDCX;
      entry->description=
        AllocateString("ZSoft IBM PC multi-page Paintbrush");
      RegisterMagickInfo(entry);
*/
      entry=SetMagickInfo("DIB");
      entry->decoder=ReadBMPImage;
      entry->encoder=WriteBMPImage;
      entry->description=
        AllocateString("Microsoft Windows bitmap image");
      RegisterMagickInfo(entry);
/*
      entry=SetMagickInfo("EPDF");
      entry->decoder=ReadPDFImage;
      entry->encoder=WritePDFImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Encapsulated Portable Document Format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPI");
      entry->decoder=ReadPSImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Adobe Encapsulated PostScript Interchange format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPS");
      entry->decoder=ReadPSImage;
      entry->encoder=WritePSImage;
      entry->adjoin=False;
      entry->description=AllocateString("Adobe Encapsulated PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPS2");
      entry->decoder=ReadPSImage;
      entry->encoder=WritePS2Image;
      entry->adjoin=False;
      entry->description=
        AllocateString("Adobe Level II Encapsulated PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPSF");
      entry->decoder=ReadPSImage;
      entry->encoder=WritePSImage;
      entry->adjoin=False;
      entry->description=AllocateString("Adobe Encapsulated PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPSI");
      entry->decoder=ReadPSImage;
      entry->encoder=WritePSImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Adobe Encapsulated PostScript Interchange format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("EPT");
      entry->decoder=ReadPSImage;
      entry->encoder=WriteEPTImage;
      entry->magick=IsEPT;
      entry->adjoin=False;
      entry->description=
        AllocateString("Adobe Encapsulated PostScript with TIFF preview");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("FAX");
      entry->decoder=ReadFAXImage;
      entry->encoder=WriteFAXImage;
      entry->magick=IsFAX;
      entry->description=AllocateString("Group 3 FAX");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("FITS");
      entry->decoder=ReadFITSImage;
      entry->encoder=WriteFITSImage;
      entry->magick=IsFITS;
      entry->adjoin=False;
      entry->description=
        AllocateString("Flexible Image Transport System");
      RegisterMagickInfo(entry);
#if defined(HasFPX)
      entry=SetMagickInfo("FPX");
      entry->decoder=ReadFPXImage;
      entry->encoder=WriteFPXImage;
      entry->adjoin=False;
      entry->description=AllocateString("FlashPix Format");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("G3");
      entry->decoder=ReadFAXImage;
      entry->adjoin=False;
      entry->description=AllocateString("Group 3 FAX");
      RegisterMagickInfo(entry);
*/
      entry=SetMagickInfo("GIF");
      entry->decoder=ReadGIFImage;
      entry->encoder=WriteGIFImage;
      entry->magick=IsGIF;
      entry->description=
        AllocateString("CompuServe graphics interchange format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("GIF87");
      entry->decoder=ReadGIFImage;
      entry->encoder=WriteGIFImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("CompuServe graphics interchange format (version 87a)");
      RegisterMagickInfo(entry);
/*
      entry=SetMagickInfo("GRADATION");
      entry->decoder=ReadGRADATIONImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Gradual passing from one shade to another");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("GRANITE");
      entry->decoder=ReadLOGOImage;
      entry->encoder=WriteLOGOImage;
      entry->adjoin=False;
      entry->description=AllocateString("Granite texture");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("GRAY");
      entry->decoder=ReadGRAYImage;
      entry->encoder=WriteGRAYImage;
      entry->raw=True;
      entry->description=AllocateString("Raw gray bytes");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("H");
      entry->decoder=ReadLOGOImage;
      entry->encoder=WriteLOGOImage;
      entry->adjoin=False;
      entry->description=AllocateString("Internal format");
      RegisterMagickInfo(entry);
#if defined(HasHDF)
      entry=SetMagickInfo("HDF");
      entry->decoder=ReadHDFImage;
      entry->encoder=WriteHDFImage;
      entry->magick=IsHDF;
      entry->blob_support=False;
      entry->description=AllocateString("Hierarchical Data Format");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("HISTOGRAM");
      entry->decoder=ReadHISTOGRAMImage;
      entry->encoder=WriteHISTOGRAMImage;
      entry->adjoin=False;
      entry->description=AllocateString("Histogram of the image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("HTM");
      entry->encoder=WriteHTMLImage;
      entry->magick=IsHTML;
      entry->adjoin=False;
      entry->description=
        AllocateString("Hypertext Markup Language and a client-side image map");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("HTML");
      entry->encoder=WriteHTMLImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Hypertext Markup Language and a client-side image map");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("ICB");
      entry->decoder=ReadTGAImage;
      entry->encoder=WriteTGAImage;
      entry->description=AllocateString("Truevision Targa image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("ICC");
      entry->decoder=ReadICCImage;
      entry->encoder=WriteICCImage;
      entry->adjoin=False;
      entry->description=AllocateString("ICC Color Profile");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("ICO");
      entry->decoder=ReadICONImage;
      entry->adjoin=False;
      entry->description=AllocateString("Microsoft icon");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("IMPLICIT");
      entry->description=AllocateString("Internal format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("IPTC");
      entry->decoder=ReadIPTCImage;
      entry->encoder=WriteIPTCImage;
      entry->magick=IsIPTC;
      entry->adjoin=False;
      entry->description=AllocateString("IPTC Newsphoto");
      RegisterMagickInfo(entry);
#if defined(HasJBIG)
      entry=SetMagickInfo("JBG");
      entry->decoder=ReadJBIGImage;
      entry->encoder=WriteJBIGImage;
      entry->description=
        AllocateString("Joint Bi-level Image experts Group interchange format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("JBIG");
      entry->decoder=ReadJBIGImage;
      entry->encoder=WriteJBIGImage;
      entry->description=
        AllocateString("Joint Bi-level Image experts Group interchange format");
      RegisterMagickInfo(entry);
#endif
#if defined(HasJPEG)
      entry=SetMagickInfo("JPG");
      entry->decoder=ReadJPEGImage;
      entry->encoder=WriteJPEGImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Joint Photographic Experts Group JFIF format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("JPEG");
      entry->decoder=ReadJPEGImage;
      entry->encoder=WriteJPEGImage;
      entry->magick=IsJPEG;
      entry->adjoin=False;
      entry->description=
        AllocateString("Joint Photographic Experts Group JFIF format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("JPEG24");
      entry->decoder=ReadJPEGImage;
      entry->encoder=WriteJPEGImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Joint Photographic Experts Group JFIF format");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("LABEL");
      entry->decoder=ReadLABELImage;
      entry->adjoin=False;
      entry->description=AllocateString("Text image format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("LOGO");
      entry->decoder=ReadLOGOImage;
      entry->encoder=WriteLOGOImage;
      entry->adjoin=False;
      entry->description=AllocateString("ImageMagick Logo");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("MAP");
      entry->decoder=ReadMAPImage;
      entry->encoder=WriteMAPImage;
      entry->adjoin=False;
      entry->raw=True;
      entry->description=
        AllocateString("Colormap intensities and indices");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("MATTE");
      entry->decoder=ReadMIFFImage;
      entry->encoder=WriteMATTEImage;
      entry->raw=True;
      entry->description=AllocateString("Matte format");
      RegisterMagickInfo(entry);
*/
      entry=SetMagickInfo("MIFF");
      entry->decoder=ReadMIFFImage;
      entry->encoder=WriteMIFFImage;
      entry->magick=IsMIFF;
      entry->description=AllocateString("Magick image format");
      RegisterMagickInfo(entry);
/*
#if defined(HasPNG)
      entry=SetMagickInfo("MNG");
      entry->decoder=ReadPNGImage;
      entry->encoder=WritePNGImage;
      entry->magick=IsMNG;
      entry->description=AllocateString("Multiple-image Network Graphics");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("MONO");
      entry->decoder=ReadMONOImage;
      entry->encoder=WriteMONOImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Bi-level bitmap in least-significant-byte first order");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("MTV");
      entry->decoder=ReadMTVImage;
      entry->encoder=WriteMTVImage;
      entry->description=AllocateString("MTV Raytracing image format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("NETSCAPE");
      entry->decoder=ReadLOGOImage;
      entry->encoder=WriteLOGOImage;
      entry->adjoin=False;
      entry->description=AllocateString("Netscape 216 color cube");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("NULL");
      entry->decoder=ReadNULLImage;
      entry->adjoin=False;
      entry->description=AllocateString("NULL image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("P7");
      entry->decoder=ReadPNMImage;
      entry->encoder=WritePNMImage;
      entry->description=AllocateString("Xv thumbnail format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PBM");
      entry->decoder=ReadPNMImage;
      entry->encoder=WritePNMImage;
      entry->description=
        AllocateString("Portable bitmap format (black and white)");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PCD");
      entry->decoder=ReadPCDImage;
      entry->encoder=WritePCDImage;
      entry->magick=IsPCD;
      entry->adjoin=False;
      entry->description=AllocateString("Photo CD");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PCDS");
      entry->decoder=ReadPCDImage;
      entry->encoder=WritePCDImage;
      entry->adjoin=False;
      entry->description=AllocateString("Photo CD");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PCL");
      entry->decoder=ReadPCLImage;
      entry->encoder=WritePCLImage;
      entry->magick=IsPCL;
      entry->adjoin=False;
      entry->description=AllocateString("Page Control Language");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PCT");
      entry->encoder=WritePICTImage;
      entry->adjoin=False;
      entry->description=AllocateString("Apple Macintosh QuickDraw/PICT");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PCX");
      entry->decoder=ReadPCXImage;
      entry->encoder=WritePCXImage;
      entry->magick=IsPCX;
      entry->adjoin=False;
      entry->description=AllocateString("ZSoft IBM PC Paintbrush");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PDF");
      entry->decoder=ReadPDFImage;
      entry->encoder=WritePDFImage;
      entry->magick=IsPDF;
      entry->description=AllocateString("Portable Document Format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PIC");
      entry->decoder=ReadPICTImage;
      entry->encoder=WritePICTImage;
      entry->adjoin=False;
      entry->description=AllocateString("Apple Macintosh QuickDraw/PICT");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PICT");
      entry->decoder=ReadPICTImage;
      entry->encoder=WritePICTImage;
      entry->adjoin=False;
      entry->description=AllocateString("Apple Macintosh QuickDraw/PICT");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PICT24");
      entry->decoder=ReadPICTImage;
      entry->encoder=WritePICTImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("24-bit Apple Macintosh QuickDraw/PICT");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PIX");
      entry->decoder=ReadPIXImage;
      entry->description=
        AllocateString("Alias/Wavefront RLE image format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PLASMA");
      entry->decoder=ReadPLASMAImage;
      entry->adjoin=False;
      entry->description=AllocateString("Plasma fractal image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PGM");
      entry->decoder=ReadPNMImage;
      entry->encoder=WritePNMImage;
      entry->description=
        AllocateString("Portable graymap format (gray scale)");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PM");
      entry->decoder=ReadXPMImage;
      entry->encoder=WriteXPMImage;
      entry->adjoin=False;
      entry->description=AllocateString("X Windows system pixmap (color)");
      RegisterMagickInfo(entry);
#if defined(HasPNG)
      entry=SetMagickInfo("PNG");
      entry->decoder=ReadPNGImage;
      entry->encoder=WritePNGImage;
      entry->magick=IsPNG;
      entry->adjoin=False;
      entry->description=AllocateString("Portable Network Graphics");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("PNM");
      entry->decoder=ReadPNMImage;
      entry->encoder=WritePNMImage;
      entry->magick=IsPNM;
      entry->description=AllocateString("Portable anymap");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PPM");
      entry->decoder=ReadPNMImage;
      entry->encoder=WritePNMImage;
      entry->description=AllocateString("Portable pixmap format (color)");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PREVIEW");
      entry->encoder=WritePREVIEWImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Show a preview an image enhancement, effect, or f/x");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PS");
      entry->decoder=ReadPSImage;
      entry->encoder=WritePSImage;
      entry->magick=IsPS;
      entry->description=AllocateString("Adobe PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PS2");
      entry->encoder=WritePS2Image;
      entry->description=AllocateString("Adobe Level II PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PS3");
      entry->encoder=WritePS3Image;
      entry->description=AllocateString("Adobe Level III PostScript");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("PSD");
      entry->decoder=ReadPSDImage;
      entry->encoder=WritePSDImage;
      entry->magick=IsPSD;
      entry->adjoin=False;
      entry->description=AllocateString("Adobe Photoshop bitmap");
      RegisterMagickInfo(entry);
#if defined(HasTIFF)
      entry=SetMagickInfo("PTIF");
      entry->decoder=ReadTIFFImage;
      entry->encoder=WriteTIFFImage;
      entry->blob_support=False;
      entry->description=AllocateString("Pyramid encoded TIFF");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("PWP");
      entry->decoder=ReadPWPImage;
      entry->magick=IsPWP;
      entry->description=AllocateString("Seattle Film Works");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("RAS");
      entry->decoder=ReadSUNImage;
      entry->encoder=WriteSUNImage;
      entry->magick=IsSUN;
      entry->description=AllocateString("SUN Rasterfile");
      RegisterMagickInfo(entry);
*/
      entry=SetMagickInfo("RGB");
      entry->decoder=ReadRGBImage;
      entry->encoder=WriteRGBImage;
      entry->raw=True;
      entry->description=AllocateString("Raw red, green, and blue bytes");
      RegisterMagickInfo(entry);
/*
      entry=SetMagickInfo("RGBA");
      entry->decoder=ReadRGBImage;
      entry->encoder=WriteRGBImage;
      entry->raw=True;
      entry->description=
        AllocateString("Raw red, green, blue, and matte bytes");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("RLA");
      entry->decoder=ReadRLAImage;
      entry->adjoin=False;
      entry->description=AllocateString("Alias/Wavefront image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("RLE");
      entry->decoder=ReadRLEImage;
      entry->magick=IsRLE;
      entry->adjoin=False;
      entry->description=AllocateString("Utah Run length encoded image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("SCT");
      entry->decoder=ReadSCTImage;
      entry->magick=IsSCT;
      entry->adjoin=False;
      entry->description=AllocateString("Scitex HandShake");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("SFW");
      entry->decoder=ReadSFWImage;
      entry->magick=IsSFW;
      entry->adjoin=False;
      entry->description=AllocateString("Seattle Film Works");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("SGI");
      entry->decoder=ReadSGIImage;
      entry->encoder=WriteSGIImage;
      entry->magick=IsSGI;
      entry->description=AllocateString("Irix RGB image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("SHTML");
      entry->encoder=WriteHTMLImage;
      entry->adjoin=False;
      entry->description=
        AllocateString("Hypertext Markup Language and a client-side image map");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("STEGANO");
      entry->decoder=ReadSTEGANOImage;
      entry->description=AllocateString("Steganographic image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("SUN");
      entry->decoder=ReadSUNImage;
      entry->encoder=WriteSUNImage;
      entry->description=AllocateString("SUN Rasterfile");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("TEXT");
      entry->decoder=ReadTXTImage;
      entry->encoder=WriteTXTImage;
      entry->raw=True;
      entry->description=AllocateString("Raw text");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("TGA");
      entry->decoder=ReadTGAImage;
      entry->encoder=WriteTGAImage;
      entry->description=AllocateString("Truevision Targa image");
      RegisterMagickInfo(entry);
#if defined(HasTIFF)
      entry=SetMagickInfo("TIF");
      entry->decoder=ReadTIFFImage;
      entry->encoder=WriteTIFFImage;
      entry->blob_support=False;
      entry->description=AllocateString("Tagged Image File Format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("TIFF");
      entry->decoder=ReadTIFFImage;
      entry->encoder=WriteTIFFImage;
      entry->magick=IsTIFF;
      entry->blob_support=False;
      entry->description=AllocateString("Tagged Image File Format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("TIFF24");
      entry->decoder=ReadTIFFImage;
      entry->encoder=WriteTIFFImage;
      entry->blob_support=False;
      entry->description=AllocateString("24-bit Tagged Image File Format");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("TILE");
      entry->decoder=ReadTILEImage;
      entry->raw=True;
      entry->description=AllocateString("Tile image with a texture");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("TIM");
      entry->decoder=ReadTIMImage;
      entry->description=AllocateString("PSX TIM");
      RegisterMagickInfo(entry);
#if defined(HasTTF)
      entry=SetMagickInfo("TTF");
      entry->decoder=ReadTTFImage;
      entry->adjoin=False;
      entry->description=AllocateString("TrueType font");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("TXT");
      entry->decoder=ReadTXTImage;
      entry->encoder=WriteTXTImage;
      entry->description=AllocateString("Raw text");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("UIL");
      entry->decoder=ReadUILImage;
      entry->encoder=WriteUILImage;
      entry->adjoin=False;
      entry->description=AllocateString("X-Motif UIL table");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("UYVY");
      entry->decoder=ReadUYVYImage;
      entry->encoder=WriteUYVYImage;
      entry->adjoin=False;
      entry->raw=True;
      entry->description=AllocateString("16bit/pixel interleaved YUV");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("VDA");
      entry->decoder=ReadTGAImage;
      entry->encoder=WriteTGAImage;
      entry->description=AllocateString("Truevision Targa image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("VICAR");
      entry->decoder=ReadVICARImage;
      entry->encoder=WriteVICARImage;
      entry->magick=IsVICAR;
      entry->adjoin=False;
      entry->description=AllocateString("VICAR rasterfile format");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("VID");
      entry->decoder=ReadVIDImage;
      entry->encoder=WriteMIFFImage;
      entry->description=AllocateString("Visual Image Directory");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("VIFF");
      entry->decoder=ReadVIFFImage;
      entry->encoder=WriteVIFFImage;
      entry->magick=IsVIFF;
      entry->description=AllocateString("Khoros Visualization image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("VST");
      entry->decoder=ReadTGAImage;
      entry->encoder=WriteTGAImage;
      entry->description=AllocateString("Truevision Targa image");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("WBMP");
      entry->decoder=ReadWBMPImage;
      entry->encoder=WriteWBMPImage;
      entry->adjoin=False;
      entry->description=AllocateString("Wireless Bitmap (level 0) image");
      RegisterMagickInfo(entry);
#if defined(HasX11)
      entry=SetMagickInfo("X");
      entry->decoder=ReadXImage;
      entry->encoder=WriteXImage;
      entry->adjoin=False;
      entry->description=AllocateString("X Image");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("XBM");
      entry->decoder=ReadXBMImage;
      entry->encoder=WriteXBMImage;
      entry->magick=IsXBM;
      entry->adjoin=False;
      entry->description=
        AllocateString("X Windows system bitmap (black and white)");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("XC");
      entry->decoder=ReadXCImage;
      entry->adjoin=False;
      entry->raw=True;
      entry->description=
        AllocateString("Constant image of X server color");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("XPM");
      entry->decoder=ReadXPMImage;
      entry->encoder=WriteXPMImage;
      entry->magick=IsXPM;
      entry->adjoin=False;
      entry->description=AllocateString("X Windows system pixmap (color)");
      RegisterMagickInfo(entry);
      entry=SetMagickInfo("XV");
      entry->decoder=ReadVIFFImage;
      entry->encoder=WriteVIFFImage;
      entry->description=AllocateString("Khoros Visualization image");
      RegisterMagickInfo(entry);
#if defined(HasX11)
      entry=SetMagickInfo("XWD");
      entry->decoder=ReadXWDImage;
      entry->encoder=WriteXWDImage;
      entry->magick=IsXWD;
      entry->adjoin=False;
      entry->description=
        AllocateString("X Windows system window dump (color)");
      RegisterMagickInfo(entry);
#endif
      entry=SetMagickInfo("YUV");
      entry->decoder=ReadYUVImage;
      entry->encoder=WriteYUVImage;
      entry->adjoin=False;
      entry->raw=True;
      entry->description=AllocateString("CCIR 601 4:1:1");
      RegisterMagickInfo(entry);
*/
    }
  if (tag == (char *) NULL)
    return(magick_info);
  for (p=magick_info; p != (MagickInfo *) NULL; p=p->next)
    if (Latin1Compare(p->tag,tag) == 0)
      return(p);
  return((MagickInfo *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M a g i c k I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ListMagickInfo lists the image formats to a file.
%
%  The format of the ListMagickInfo method is:
%
%      void ListMagickInfo(FILE *file)
%
%  A description of each parameter follows.
%
%    o file: A pointer to a FILE structure.
%
%
*/
Export void ListMagickInfo(FILE *file)
{
  register MagickInfo
    *p;

  if (file == (FILE *) NULL)
    file=stdout;
  (void) fprintf(file,"\nHere is a list of image formats recognized by "
    "ImageMagick.  Mode 'rw+'\nmeans ImageMagick can read, write, and "
    "save more than one image of a\nsequence to the same blob or file.\n\n");
  (void) fprintf(file,"    Format  Mode  Description\n");
  (void) fprintf(file,"--------------------------------------------------------"
    "-----------------\n");
  for (p=GetMagickInfo((char *) NULL); p != (MagickInfo *) NULL; p=p->next)
    (void) fprintf(file,"%10s%c  %c%c%c  %s\n",p->tag ? p->tag : "",
      p->blob_support ? '*' : ' ',p->decoder ? 'r' : '-',p->encoder ? 'w' : '-',
      p->encoder && p->adjoin ? '+' : '-',p->description ? p->description : "");
  (void) fprintf(file,"\n* native blob support\n\n");
  (void) fflush(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M a g i c k I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterMagickInfo adds attributes for a particular image format to
%  the list of supported formats.  The attributes include the image format tag,
%  a method to read and/or write the format, whether the format supports
%  the saving of more than one frame to the same file or blob, whether the
%  format supports native in-memory I/O, and a brief description of the format.
%
%  The format of the RegisterMagickInfo method is:
%
%      MagickInfo *RegisterMagickInfo(MagickInfo *entry)
%
%  A description of each parameter follows:
%
%    o magick_info: Method RegisterMagickInfo returns a pointer MagickInfo
%      structure that contains the specified tag info.
%
%    o entry: 
%
*/
Export MagickInfo *RegisterMagickInfo(MagickInfo *entry)
{
  register MagickInfo
    *p;

  /*
    Add tag info to the image format list.
  */
  p=(MagickInfo*) NULL;
  if (magick_info != (MagickInfo *) NULL)
    {
      for (p=magick_info; p->next != (MagickInfo *) NULL; p=p->next)
      {
        if (Latin1Compare(p->tag,entry->tag) >= 0)
          {
            if (Latin1Compare(p->tag,entry->tag) == 0)
              {
                p=p->previous;
                UnregisterMagickInfo(entry->tag);
              }
            break;
          }
      }
    }
  if (magick_info == (MagickInfo *) NULL)
    {
      magick_info=entry;
      return(entry);
    }
  entry->previous=p;
  entry->next=p->next;
  p->next=entry;
  return(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t M a g i c k I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SetMagickInfo allocates a MagickInfo structure and initializes the
%  members to default values.
%
%  The format of the SetMagickInfo method is:
%
%      MagickInfo *SetMagickInfo(const char *tag)
%
%  A description of each parameter follows:
%
%    o magick_info: Method SetMagickInfo returns the allocated and initialized
%      MagickInfo structure.
%
%    o tag: a character string that represents the image format associated
%      with the MagickInfo structure.
%
%
*/
Export MagickInfo *SetMagickInfo(const char *tag)
{
  MagickInfo
    *entry;

  entry=(MagickInfo *) AllocateMemory(sizeof(MagickInfo));
  if (entry == (MagickInfo *) NULL)
    MagickError(ResourceLimitError,"Unable to allocate image",
      "Memory allocation failed");
  entry->tag=AllocateString(tag);
  entry->decoder=(Image *(*)(const ImageInfo *)) NULL;
  entry->encoder=(unsigned int (*)(const ImageInfo *,Image *)) NULL;
  entry->magick=
    (unsigned int (*)(const unsigned char *,const unsigned int)) NULL;
  entry->adjoin=True;
  entry->blob_support=True;
  entry->raw=False;
  entry->description=(char *) NULL;
  entry->data=(void *) NULL;
  entry->previous=(MagickInfo *) NULL;
  entry->next=(MagickInfo *) NULL;
  return(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M a g i c k I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterMagickInfo removes a tag from the magick info list.  It
%  returns False if the tag does not exist in the list otherwise True.
%
%  The format of the UnregisterMagickInfo method is:
%
%      unsigned int UnregisterMagickInfo(const char *tag)
%
%  A description of each parameter follows:
%
%    o status: Method UnregisterMagickInfo returns False if the tag does not
%      exist in the list otherwise True.
%
%    o tag: a character string that represents the image format we are
%      looking for.
%
*/
Export unsigned int UnregisterMagickInfo(const char *tag)
{
  register MagickInfo
    *p;

  for (p=GetMagickInfo((char *) NULL); p != (MagickInfo *) NULL; p=p->next)
  {
    if (Latin1Compare(p->tag,tag) == 0)
      {
        if (p->tag != (char *) NULL)
          FreeMemory(p->tag);
        if (p->description != (char *) NULL)
          FreeMemory(p->description);
        if (p->previous != (MagickInfo *) NULL)
          p->previous->next=p->next;
        else
          {
            magick_info=p->next;
            if (p->next != (MagickInfo*) NULL)
              p->next->previous=(MagickInfo *) NULL;
          }
        if (p->next != (MagickInfo*) NULL)
          p->next->previous=p->previous;
        FreeMemory(p);
        return(True);
    }
  }
  return(False);
}
