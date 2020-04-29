/*
 * Copyright (c) 1999, 2000, 2001 Greg Haerr <greg@censoft.com>
 * Portions Copyright (c) 1991 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Device-independent mid level drawing and color routines.
 *
 * These routines do the necessary range checking, clipping, and cursor
 * overwriting checks, and then call the lower level device dependent
 * routines to actually do the drawing.  The lower level routines are
 * only called when it is known that all the pixels to be drawn are
 * within the device area and are visible.
 */
/*#define NDEBUG*/
#include <stdio.h>
#include <stdlib.h>
#include "device.h"

#define assert(str) 									\
	if (!(str)) {									\
		printf("assertion failed in  file  %s, function  %s(), line  %d\n", 	\
				__FILE__, __FUNCTION__, __LINE__);			\
		abort();								\
	}

extern MWPIXELVAL gr_foreground;      /* current foreground color */
extern MWPIXELVAL gr_background;      /* current background color */
extern MWBOOL 	  gr_usebg;    	      /* TRUE if background drawn in pixmaps */
extern int 	  gr_mode; 	      /* drawing mode */
extern MWPALENTRY gr_palette[256];    /* current palette*/
extern int	  gr_firstuserpalentry;/* first user-changable palette entry*/
extern int 	  gr_nextpalentry;    /* next available palette entry*/

void drawpoint(PSD psd,MWCOORD x, MWCOORD y);
void drawrow(PSD psd,MWCOORD x1,MWCOORD x2,MWCOORD y);
static void drawcol(PSD psd,MWCOORD x,MWCOORD y1,MWCOORD y2);

/*
 * Set the drawing mode for future calls.
 */
int
GdSetMode(int mode)
{
	int	oldmode = gr_mode;

	gr_mode = mode;
	return oldmode;
}

/*
 * Set whether or not the background is used for drawing pixmaps and text.
 */
MWBOOL
GdSetUseBackground(MWBOOL flag)
{
	MWBOOL	oldusebg = gr_usebg;

	gr_usebg = flag;
	return oldusebg;
}

/*
 * Set the foreground color for drawing.
 */
MWPIXELVAL
GdSetForeground(MWPIXELVAL fg)
{
	MWPIXELVAL	oldfg = gr_foreground;
	
	gr_foreground = fg;
	return oldfg;
}

/*
 * Set the background color for bitmap and text backgrounds.
 */
MWPIXELVAL
GdSetBackground(MWPIXELVAL bg)
{
	MWPIXELVAL	oldbg = gr_background;

	gr_background = bg;
	return oldbg;
}

/*
 * Draw a point using the current clipping region and foreground color.
 */
void
GdPoint(PSD psd, MWCOORD x, MWCOORD y)
{
	if (GdClipPoint(psd, x, y)) {
		psd->DrawPixel(psd, x, y, gr_foreground);
// by yuanlii@29bbs.net, 1/25/2005 17:09:22
//		GdFixCursor(psd);
// by yuanlii@29bbs.net
	}
}

/*
 * Draw an arbitrary line using the current clipping region and foreground color
 * If bDrawLastPoint is FALSE, draw up to but not including point x2, y2.
 *
 * This routine is the only routine that adjusts coordinates for supporting
 * two different types of upper levels, those that draw the last point
 * in a line, and those that draw up to the last point.  All other local
 * routines draw the last point.  This gives this routine a bit more overhead,
 * but keeps overall complexity down.
 */
void
GdLine(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2,
	MWBOOL bDrawLastPoint)
{
  int xdelta;			/* width of rectangle around line */
  int ydelta;			/* height of rectangle around line */
  int xinc;			/* increment for moving x coordinate */
  int yinc;			/* increment for moving y coordinate */
  int rem;			/* current remainder */
  MWCOORD temp;

  /* See if the line is horizontal or vertical. If so, then call
   * special routines.
   */
  if (y1 == y2) {
	/*
	 * Adjust coordinates if not drawing last point.  Tricky.
	 */
	if(!bDrawLastPoint) {
		if (x1 > x2) {
			temp = x1;
			x1 = x2 + 1;
			x2 = temp;
		} else
			--x2;
	}

	/* call faster line drawing routine*/
	drawrow(psd, x1, x2, y1);
// by yuanlii@29bbs.net, 1/25/2005 17:33:25
//	GdFixCursor(psd);
// by yuanlii@29bbs.net
	return;
  }
  if (x1 == x2) {
	/*
	 * Adjust coordinates if not drawing last point.  Tricky.
	 */
	if(!bDrawLastPoint) {
		if (y1 > y2) {
			temp = y1;
			y1 = y2 + 1;
			y2 = temp;
		} else
			--y2;
	}

	/* call faster line drawing routine*/
	drawcol(psd, x1, y1, y2);
// by yuanlii@29bbs.net, 1/25/2005 17:33:42
//	GdFixCursor(psd);
// by yuanlii@29bbs.net
	return;
  }

  /* See if the line is either totally visible or totally invisible. If
   * so, then the line drawing is easy.
   */
  switch (GdClipArea(psd, x1, y1, x2, y2)) {
      case CLIP_VISIBLE:
	/*
	 * For size considerations, there's no low-level bresenham
	 * line draw, so we've got to draw all non-vertical
	 * and non-horizontal lines with per-point
	 * clipping for the time being
	psd->Line(psd, x1, y1, x2, y2, gr_foreground);
	GdFixCursor(psd);
	return;
	 */
	break;
      case CLIP_INVISIBLE:
	return;
  }

  /* The line may be partially obscured. Do the draw line algorithm
   * checking each point against the clipping regions.
   */
  xdelta = x2 - x1;
  ydelta = y2 - y1;
  if (xdelta < 0) xdelta = -xdelta;
  if (ydelta < 0) ydelta = -ydelta;
  xinc = (x2 > x1) ? 1 : -1;
  yinc = (y2 > y1) ? 1 : -1;
  if (GdClipPoint(psd, x1, y1))
	  psd->DrawPixel(psd, x1, y1, gr_foreground);
  if (xdelta >= ydelta) {
	rem = xdelta / 2;
	for(;;) {
		if(!bDrawLastPoint && x1 == x2)
			break;
		x1 += xinc;
		rem += ydelta;
		if (rem >= xdelta) {
			rem -= xdelta;
			y1 += yinc;
		}
		if (GdClipPoint(psd, x1, y1))
			psd->DrawPixel(psd, x1, y1, gr_foreground);
		if(bDrawLastPoint && x1 == x2)
			break;
	}
  } else {
	rem = ydelta / 2;
	for(;;) {
		if(!bDrawLastPoint && y1 == y2)
			break;
		y1 += yinc;
		rem += xdelta;
		if (rem >= ydelta) {
			rem -= ydelta;
			x1 += xinc;
		}
		if (GdClipPoint(psd, x1, y1))
			psd->DrawPixel(psd, x1, y1, gr_foreground);
		if(bDrawLastPoint && y1 == y2)
			break;
	}
  }
// by yuanlii@29bbs.net, 1/25/2005 17:33:59
// GdFixCursor(psd);
// by yuanlii@29bbs.net
}

/* Draw a point in the foreground color, applying clipping if necessary*/
void
drawpoint(PSD psd, MWCOORD x, MWCOORD y)
{
	if (GdClipPoint(psd, x, y))
		psd->DrawPixel(psd, x, y, gr_foreground);
}

/* Draw a horizontal line from x1 to and including x2 in the
 * foreground color, applying clipping if necessary.
 */
void
drawrow(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y)
{
  MWCOORD temp;

  /* reverse endpoints if necessary*/
  if (x1 > x2) {
	temp = x1;
	x1 = x2;
	x2 = temp;
  }

  /* clip to physical device*/
  if (x1 < 0)
	  x1 = 0;
  if (x2 >= psd->xvirtres)
	  x2 = psd->xvirtres - 1;

  /* check cursor intersect once for whole line*/
// by yuanlii@29bbs.net, 1/25/2005 17:34:14
// GdCheckCursor(psd, x1, y, x2, y);
// by yuanlii@29bbs.net

  while (x1 <= x2) {
	if (GdClipPoint(psd, x1, y)) {
		temp = MWMIN(clipmaxx, x2);
		psd->DrawHorzLine(psd, x1, temp, y, gr_foreground);
	} else
		temp = MWMIN(clipmaxx, x2);
	x1 = temp + 1;
  }
}

/* Draw a vertical line from y1 to and including y2 in the
 * foreground color, applying clipping if necessary.
 */
static void
drawcol(PSD psd, MWCOORD x,MWCOORD y1,MWCOORD y2)
{
  MWCOORD temp;

  /* reverse endpoints if necessary*/
  if (y1 > y2) {
	temp = y1;
	y1 = y2;
	y2 = temp;
  }

  /* clip to physical device*/
  if (y1 < 0)
	  y1 = 0;
  if (y2 >= psd->yvirtres)
	  y2 = psd->yvirtres - 1;

  /* check cursor intersect once for whole line*/
// by yuanlii@29bbs.net, 1/25/2005 17:34:26
// GdCheckCursor(psd, x, y1, x, y2);
// by yuanlii@29bbs.net

  while (y1 <= y2) {
	if (GdClipPoint(psd, x, y1)) {
		temp = MWMIN(clipmaxy, y2);
		psd->DrawVertLine(psd, x, y1, temp, gr_foreground);
	} else
		temp = MWMIN(clipmaxy, y2);
	y1 = temp + 1;
  }
}

/* Draw a rectangle in the foreground color, applying clipping if necessary.
 * This is careful to not draw points multiple times in case the rectangle
 * is being drawn using XOR.
 */
void
GdRect(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height)
{
  MWCOORD maxx;
  MWCOORD maxy;

  if (width <= 0 || height <= 0)
	  return;
  maxx = x + width - 1;
  maxy = y + height - 1;
  drawrow(psd, x, maxx, y);
  if (height > 1)
	  drawrow(psd, x, maxx, maxy);
  if (height < 3)
	  return;
  y++;
  maxy--;
  drawcol(psd, x, y, maxy);
  if (width > 1)
	  drawcol(psd, maxx, y, maxy);
// by yuanlii@29bbs.net, 1/25/2005 18:03:39
// GdFixCursor(psd);
// by yuanlii@29bbs.net
}

/* Draw a filled in rectangle in the foreground color, applying
 * clipping if necessary.
 */
void
GdFillRect(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD width, MWCOORD height)
{
  MWCOORD x2 = x1+width-1;
  MWCOORD y2 = y1+height-1;

  if (width <= 0 || height <= 0)
	  return;

  /* See if the rectangle is either totally visible or totally
   * invisible. If so, then the rectangle drawing is easy.
   */
  switch (GdClipArea(psd, x1, y1, x2, y2)) {
      case CLIP_VISIBLE:
	psd->FillRect(psd, x1, y1, x2, y2, gr_foreground);
// by yuanlii@29bbs.net, 1/25/2005 18:03:57
//	GdFixCursor(psd);
// by yuanlii@29bbs.net
	return;

      case CLIP_INVISIBLE:
	return;
  }

  /* The rectangle may be partially obstructed. So do it line by line. */
  while (y1 <= y2)
	  drawrow(psd, x1, x2, y1++);
// by yuanlii@29bbs.net, 1/25/2005 18:04:08
// GdFixCursor(psd);
// by yuanlii@29bbs.net
}

/*
 * Draw a rectangular area using the current clipping region and the
 * specified bit map.  This differs from rectangle drawing in that the
 * rectangle is drawn using the foreground color and possibly the background
 * color as determined by the bit map.  Each row of bits is aligned to the
 * next bitmap word boundary (so there is padding at the end of the row).
 * The background bit values are only written if the gr_usebg flag
 * is set.
 */
void
GdBitmap(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	MWIMAGEBITS *imagebits)
{
  MWCOORD minx;
  MWCOORD maxx;
  MWPIXELVAL savecolor;		/* saved foreground color */
  MWIMAGEBITS bitvalue = 0;	/* bitmap word value */
  int bitcount;			/* number of bits left in bitmap word */

  switch (GdClipArea(psd, x, y, x + width - 1, y + height - 1)) {
      case CLIP_VISIBLE:
	/*
	 * For size considerations, there's no low-level bitmap
	 * draw so we've got to draw everything with per-point
	 * clipping for the time being.
	if (gr_usebg)
		psd->FillRect(psd, x, y, x + width - 1, y + height - 1,
			gr_background);
	psd->DrawBitmap(psd, x, y, width, height, imagebits, gr_foreground);
	return;
	*/
	break;

      case CLIP_INVISIBLE:
	return;
  }

  /* The rectangle is partially visible, so must do clipping. First
   * fill a rectangle in the background color if necessary.
   */
  if (gr_usebg) {
	savecolor = gr_foreground;
	gr_foreground = gr_background;
	/* note: change to fillrect*/
	GdFillRect(psd, x, y, width, height);
	gr_foreground = savecolor;
  }
  minx = x;
  maxx = x + width - 1;
  bitcount = 0;
  while (height > 0) {
	if (bitcount <= 0) {
		bitcount = MWIMAGE_BITSPERIMAGE;
		bitvalue = *imagebits++;
	}
	if (MWIMAGE_TESTBIT(bitvalue) && GdClipPoint(psd, x, y))
		psd->DrawPixel(psd, x, y, gr_foreground);
	bitvalue = MWIMAGE_SHIFTBIT(bitvalue);
	bitcount--;
	if (x++ == maxx) {
		x = minx;
		y++;
		height--;
		bitcount = 0;
	}
  }
// by yuanlii@29bbs.net, 1/29/2005 17:08:14
// GdFixCursor(psd);
// by yuanlii@29bbs.net
}








///////////////////////////////////////////////////////////////////////
//
//	Sepatate the functions as the file order -- by yuanlii
//
///////////////////////////////////////////////////////////////////////
/**
 * Return true if color is in palette
 *
 * @param cr Color to look for.
 * @param palette Palette to look in.
 * @param palsize Size of the palette.
 * @return TRUE iff the color is in palette.
 */
MWBOOL
GdColorInPalette(MWCOLORVAL cr,MWPALENTRY *palette,int palsize)
{
	int	i;

	for(i=0; i<palsize; ++i)
		if(GETPALENTRY(palette, i) == cr)
			return TRUE;
	return FALSE;
}

/*
 * Create a MWPIXELVAL conversion table between the passed palette
 * and the in-use palette.  The system palette is loaded/merged according
 * to fLoadType.
 */
void
GdMakePaletteConversionTable(PSD psd,MWPALENTRY *palette,int palsize,
	MWPIXELVAL *convtable,int fLoadType)
{
	int		i;
	MWCOLORVAL	cr;
	int		newsize, nextentry;
	MWPALENTRY	newpal[256];

	/*
	 * Check for load palette completely, or add colors
	 * from passed palette to system palette until full.
	 */
#if 1
	if(psd->pixtype == MWPF_PALETTE) {
	    switch(fLoadType) {
	    case LOADPALETTE:
		/* Load palette from beginning with image's palette.
		 * First palette entries are Microwindows colors
		 * and not changed.
		 */
		GdSetPalette(psd, gr_firstuserpalentry, palsize, palette);
		break;

	    case MERGEPALETTE:
		/* get system palette*/
		for(i=0; i<(int)psd->ncolors; ++i)
			newpal[i] = gr_palette[i];

		/* merge passed palette into system palette*/
		newsize = 0;
		nextentry = gr_nextpalentry;

		/* if color missing and there's room, add it*/
		for(i=0; i<palsize && nextentry < (int)psd->ncolors; ++i) {
			cr = GETPALENTRY(palette, i);
			if(!GdColorInPalette(cr, newpal, nextentry)) {
				newpal[nextentry++] = palette[i];
				++newsize;
			}
		}

		/* set the new palette if any color was added*/
		if(newsize) {
			GdSetPalette(psd, gr_nextpalentry, newsize,
				&newpal[gr_nextpalentry]);
			gr_nextpalentry += newsize;
		}
		break;
	    }
	}

	/*
	 * Build conversion table from inuse system palette and
	 * passed palette.  This will load RGB values directly
	 * if running truecolor, otherwise it will find the
	 * nearest color from the inuse palette.
	 * FIXME: tag the conversion table to the bitmap image
	 */
	for(i=0; i<palsize; ++i) {
		cr = GETPALENTRY(palette, i);
		convtable[i] = GdFindColor(cr);
	}
#else
	for(i=0; i<palsize; ++i) {
		convtable[i] = i;
	}
#endif
}

/*
 * Draw a color bitmap image in 1, 4, 8, 24 or 32 bits per pixel.  The
 * Microwindows color image format is DWORD padded bytes, with
 * the upper bits corresponding to the left side (identical to 
 * the MS Windows format).  This format is currently different
 * than the MWIMAGEBITS format, which uses word-padded bits
 * for monochrome display only, where the upper bits in the word
 * correspond with the left side.
 */
void
GdDrawImage(PSD psd, MWCOORD x, MWCOORD y, PMWIMAGEHDR pimage)
{
  MWCOORD minx;
  MWCOORD maxx;
  MWUCHAR bitvalue = 0;
  int bitcount;
  MWUCHAR *imagebits;
  MWCOORD	height, width;
  MWPIXELVAL pixel;
  MWPIXELVAL *pixelline;
  int clip;
  int extra, linesize;
  int	rgborder;
  MWCOLORVAL cr;
  MWCOORD yoff;
  unsigned long transcolor;
  MWPIXELVAL convtable[256];

  height = pimage->height;
  width = pimage->width;

  /* determine if entire image is clipped out, save clipresult for later*/
  clip = GdClipArea(psd, x, y, x + width - 1, y + height - 1);
  if(clip == CLIP_INVISIBLE)
	return;

  if (NULL == (pixelline = (MWPIXELVAL *)malloc(width * sizeof(MWPIXELVAL)))) {
	  return;
  }

  transcolor = pimage->transcolor;

  /*
   * Merge the images's palette and build a palette index conversion table.
   */
  if (pimage->bpp <= 8) {
	if(!pimage->palette) {
		/* for jpeg's without a palette*/
		for(yoff=0; yoff<pimage->palsize; ++yoff)
			convtable[yoff] = yoff;
	}
	else
	       	GdMakePaletteConversionTable(psd, pimage->palette,
			pimage->palsize, convtable, MERGEPALETTE);

	/* The following is no longer used.  One reason is that it required */
	/* the transparent color to be unique, which was unnessecary        */

	/* convert transcolor to converted palette index for speed*/
	/* if (transcolor != -1L)
	   transcolor = (unsigned long) convtable[transcolor];  */
  }

  minx = x;
  maxx = x + width - 1;
  imagebits = pimage->imagebits;

  /* check for bottom-up image*/
  if(pimage->compression & MWIMAGE_UPSIDEDOWN) {
	y += height - 1;
	yoff = -1;
  } else
	yoff = 1;

#define PIX2BYTES(n)	(((n)+7)/8)
  /* imagebits are dword aligned*/
  switch(pimage->bpp) {
  default:
  case 8:
	linesize = width;
	break;
  case 32:
	linesize = width*4;
	break;
  case 24:
	linesize = width*3;
	break;
  case 4:
	linesize = PIX2BYTES(width<<2);
	break;
  case 1:
	linesize = PIX2BYTES(width);
	break;
  }
  extra = pimage->pitch - linesize;

  /* 24bpp RGB rather than BGR byte order?*/
  rgborder = pimage->compression & MWIMAGE_RGB; 

  bitcount = 0;
  switch(pimage->bpp) {
	  case 24:
	  case 32:
		  while(height > 0) {
			  unsigned long trans = 0;

			  if (bitcount <= 0) {
				  bitcount = sizeof(MWUCHAR) * 8;
				  bitvalue = *imagebits++;
			  }

			  cr = rgborder? MWRGB(bitvalue, imagebits[0], imagebits[1]):
				  MWRGB(imagebits[1], imagebits[0], bitvalue);

			  /* Include the upper bits for transcolor stuff */
			  if (imagebits[2])	/* FIXME: 24bpp error*/
				  trans = cr | 0x01000000L;

			  if (pimage->bpp == 32)
				  imagebits += 3;
			  else imagebits += 2;
			  bitcount = 0;

			  /* handle transparent color*/
			  if (transcolor == trans)
				  goto next24;

			  switch(psd->pixtype) {
				  case MWPF_PALETTE:
				  default:
					  //			pixel = GdFindColor(cr);
					  pixelline[x - minx] = GdFindColor(cr);
					  break;
				  case MWPF_TRUECOLOR0888:
				  case MWPF_TRUECOLOR888:
					  pixel = COLOR2PIXEL888(cr);
					  break;
				  case MWPF_TRUECOLOR565:
					  pixel = COLOR2PIXEL565(cr);
					  break;
				  case MWPF_TRUECOLOR555:
					  pixel = COLOR2PIXEL555(cr);
					  break;
				  case MWPF_TRUECOLOR332:
					  pixel = COLOR2PIXEL332(cr);
					  break;
			  }
next24:
			  if(x++ == maxx) {
				  psd->DrawLine(psd, minx, y, pixelline, width);
				  x = minx;
				  y += yoff;
				  height--;
				  bitcount = 0;
				  imagebits += extra;
			  }
		  }
		  break;
	  default:
	  case 8:
		  while(height > 0) {
			  bitvalue = *imagebits++;
			  if (bitvalue == transcolor)
				  goto next8;

			  //	  pixel = convtable[bitvalue];
			  pixelline[x - minx] = convtable[bitvalue];
next8:
			  if(x++ == maxx) {
				  if (y < psd->yres) {
					  psd->DrawLine(psd, minx, y, pixelline, width);
				  }
				  x = minx;
				  y += yoff;
				  height--;
				  imagebits += extra;
			  }
		  }
		  break;
	  case 4:
		  while(height > 0) {
			  if (bitcount <= 0) {
				  bitcount = sizeof(MWUCHAR) * 8;
				  bitvalue = *imagebits++;
			  }
			  if (((bitvalue & 0xf0) >> 4) == transcolor) {
				  bitvalue <<= 4;
				  bitcount -= 4;
				  goto next4;
			  }

			  //	  pixel = convtable[(bitvalue & 0xf0) >> 4];
			  pixelline[x - minx] = convtable[(bitvalue & 0xf0) >> 4];
			  bitvalue <<= 4;
			  bitcount -= 4;
next4:
			  if(x++ == maxx) {
				  psd->DrawLine(psd, minx, y, pixelline, width);
				  x = minx;
				  y += yoff;
				  height--;
				  bitcount = 0;
				  imagebits += extra;
			  }
		  }
		  break;
	  case 1:
		  while(height > 0) {
			  if (bitcount <= 0) {
				  bitcount = sizeof(MWUCHAR) * 8;
				  bitvalue = *imagebits++;
			  }
			  --bitcount;
			  if (((bitvalue & 0x80) ? 1 : 0) == transcolor) {
				  bitvalue <<= 1;
				  goto next1;
			  }

			  //	  pixel = convtable[(bitvalue & 0x80)? 1: 0];
			  pixelline[x - minx] = convtable[(bitvalue & 0x80)? 1: 0];
			  bitvalue <<= 1;
next1:
			  if(x++ == maxx) {
				  psd->DrawLine(psd, minx, y, pixelline, width);
				  x = minx;
				  y += yoff;
				  height--;
				  bitcount = 0;
				  imagebits += extra;
			  }
		  }
		  break;
  }

	/* if((unsigned long)pixel != transcolor &&*/
//	if (clip == CLIP_VISIBLE || GdClipPoint(psd, x, y))
//	    psd->DrawPixel(psd, x, y, pixel);
#if 0
	/* fix: use clipmaxx to clip quicker*/
	else if(clip != CLIP_VISIBLE && !clipresult && x > clipmaxx) {
		x = maxx;
	}
#endif
  free(pixelline);
// by yuanlii@29bbs.net, 1/31/2005 13:47:26
// GdFixCursor(psd);
// by yuanlii@29bbs.net

}

///////////////////////////////////////////////////////////////////////
//
//	GdReadArea here	-- by yuanlii
//
///////////////////////////////////////////////////////////////////////

/* Draw a rectangle of color values, clipping if necessary.
 * If a color matches the background color,
 * then that pixel is only drawn if the gr_usebg flag is set.
 *
 * The pixels are packed according to pixtype:
 *
 * pixtype		array of
 * MWPF_RGB		MWCOLORVAL (unsigned long)
 * MWPF_PIXELVAL	MWPIXELVAL (compile-time dependent)
 * MWPF_PALETTE		unsigned char
 * MWPF_TRUECOLOR0888	unsigned long
 * MWPF_TRUECOLOR888	packed struct {char r,char g,char b} (24 bits)
 * MWPF_TRUECOLOR565	unsigned short
 * MWPF_TRUECOLOR555	unsigned short
 * MWPF_TRUECOLOR332	unsigned char
 *
 * NOTE: Currently, no translation is performed if the pixtype
 * is not MWPF_RGB.  Pixtype is only then used to determine the 
 * packed size of the pixel data, and is then stored unmodified
 * in a MWPIXELVAL and passed to the screen driver.  Virtually,
 * this means there's only three reasonable options for client
 * programs: (1) pass all data as RGB MWCOLORVALs, (2) pass
 * data as unpacked 32-bit MWPIXELVALs in the format the current
 * screen driver is running, or (3) pass data as packed values
 * in the format the screen driver is running.  Options 2 and 3
 * are identical except for the packing structure.
 */
void
GdArea(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height, void *pixels,
	int pixtype)
{
	unsigned char *PIXELS = pixels;	/* for ANSI compilers, can't use void*/
	long cellstodo;			/* remaining number of cells */
	long count;			/* number of cells of same color */
	long cc;			/* current cell count */
	long rows;			/* number of complete rows */
	MWCOORD minx;			/* minimum x value */
	MWCOORD maxx;			/* maximum x value */
	MWPIXELVAL savecolor;		/* saved foreground color */
	MWBOOL dodraw;			/* TRUE if draw these points */
	MWCOLORVAL rgbcolor = 0L;
	int pixsize;
	unsigned char r, g, b;

	minx = x;
	maxx = x + width - 1;

	/* Set up area clipping, and just return if nothing is visible */
	if ( GdClipArea(psd, minx, y, maxx, y + height - 1) == CLIP_INVISIBLE )
		return;


	/* Calculate size of packed pixels*/
	switch(pixtype) {
	case MWPF_RGB:
		pixsize = sizeof(MWCOLORVAL);
		break;
	case MWPF_PIXELVAL:
		pixsize = sizeof(MWPIXELVAL);
		break;
	case MWPF_PALETTE:
	case MWPF_TRUECOLOR332:
		pixsize = sizeof(unsigned char);
		break;
	case MWPF_TRUECOLOR0888:
		pixsize = sizeof(unsigned long);
		break;
	case MWPF_TRUECOLOR888:
		pixsize = 3;
		break;
	case MWPF_TRUECOLOR565:
	case MWPF_TRUECOLOR555:
		pixsize = sizeof(unsigned short);
		break;
	default:
		return;
	}

  savecolor = gr_foreground;
  cellstodo = (long)width * height;
  while (cellstodo > 0) {
	/* read the pixel value from the pixtype*/
	switch(pixtype) {
	case MWPF_RGB:
		rgbcolor = *(MWCOLORVAL *)PIXELS;
		PIXELS += sizeof(MWCOLORVAL);
		gr_foreground = GdFindColor(rgbcolor);
		break;
	case MWPF_PIXELVAL:
		gr_foreground = *(MWPIXELVAL *)PIXELS;
		PIXELS += sizeof(MWPIXELVAL);
		break;
	case MWPF_PALETTE:
	case MWPF_TRUECOLOR332:
		gr_foreground = *PIXELS++;
		break;
	case MWPF_TRUECOLOR0888:
		gr_foreground = *(unsigned long *)PIXELS;
		PIXELS += sizeof(unsigned long);
		break;
	case MWPF_TRUECOLOR888:
		r = *PIXELS++;
		g = *PIXELS++;
		b = *PIXELS++;
		gr_foreground = (MWPIXELVAL)MWRGB(r, g, b);
		break;
	case MWPF_TRUECOLOR565:
	case MWPF_TRUECOLOR555:
		gr_foreground = *(unsigned short *)PIXELS;
		PIXELS += sizeof(unsigned short);
		break;
	}
	dodraw = (gr_usebg || (gr_foreground != gr_background));
	count = 1;
	--cellstodo;

	/* See how many of the adjacent remaining points have the
	 * same color as the next point.
	 *
	 * NOTE: Yes, with the addition of the pixel unpacking,
	 * it's almost slower to look ahead than to just draw
	 * the pixel...  FIXME
	 */
	while (cellstodo > 0) {
		switch(pixtype) {
		case MWPF_RGB:
			if(rgbcolor != *(MWCOLORVAL *)PIXELS)
				goto breakwhile;
			PIXELS += sizeof(MWCOLORVAL);
			break;
		case MWPF_PIXELVAL:
			if(gr_foreground != *(MWPIXELVAL *)PIXELS)
				goto breakwhile;
			PIXELS += sizeof(MWPIXELVAL);
			break;
		case MWPF_PALETTE:
		case MWPF_TRUECOLOR332:
			if(gr_foreground != *(unsigned char *)PIXELS)
				goto breakwhile;
			++PIXELS;
			break;
		case MWPF_TRUECOLOR0888:
			if(gr_foreground != *(unsigned long *)PIXELS)
				goto breakwhile;
			PIXELS += sizeof(unsigned long);
			break;
		case MWPF_TRUECOLOR888:
			r = *(unsigned char *)PIXELS;
			g = *(unsigned char *)(PIXELS + 1);
			b = *(unsigned char *)(PIXELS + 2);
			if(gr_foreground != (MWPIXELVAL)MWRGB(r, g, b))
				goto breakwhile;
			PIXELS += 3;
			break;
		case MWPF_TRUECOLOR565:
		case MWPF_TRUECOLOR555:
			if(gr_foreground != *(unsigned short *)PIXELS)
				goto breakwhile;
			PIXELS += sizeof(unsigned short);
			break;
		}
		++count;
		--cellstodo;
	}
breakwhile:

	/* If there is only one point with this color, then draw it
	 * by itself.
	 */
	if (count == 1) {
		if (dodraw)
			drawpoint(psd, x, y);
		if (++x > maxx) {
			x = minx;
			y++;
		}
		continue;
	}

	/* There are multiple points with the same color. If we are
	 * not at the start of a row of the rectangle, then draw this
	 * first row specially.
	 */
	if (x != minx) {
		cc = count;
		if (x + cc - 1 > maxx)
			cc = maxx - x + 1;
		if (dodraw)
			drawrow(psd, x, x + cc - 1, y);
		count -= cc;
		x += cc;
		if (x > maxx) {
			x = minx;
			y++;
		}
	}

	/* Now the x value is at the beginning of a row if there are
	 * any points left to be drawn.  Draw all the complete rows
	 * with one call.
	 */
	rows = count / width;
	if (rows > 0) {
		if (dodraw) {
			/* note: change to fillrect, (parm types changed)*/
			/*GdFillRect(psd, x, y, maxx, y + rows - 1);*/
			GdFillRect(psd, x, y, maxx - x + 1, rows);
		}
		count %= width;
		y += rows;
	}

	/* If there is a final partial row of pixels left to be
	 * drawn, then do that.
	 */
	if (count > 0) {
		if (dodraw)
			drawrow(psd, x, x + count - 1, y);
		x += count;
	}
  }
  gr_foreground = savecolor;
// by yuanlii@29bbs.net, 1/29/2005 17:11:52
// GdFixCursor(psd);
// by yuanlii@29bbs.net
}

/* Copy source rectangle of pixels to destination rectangle quickly*/
void
GdBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD width, MWCOORD height,
	PSD srcpsd, MWCOORD srcx, MWCOORD srcy, long rop)
{
	int rx1, rx2, ry1, ry2;
	int px1, px2, py1, py2;
	int pw, ph;
	int count;
#if DYNAMICREGIONS
	MWRECT *	prc;
	extern MWCLIPREGION *clipregion;
#else
	MWCLIPRECT *	prc;
	extern MWCLIPRECT cliprects[];
	extern int clipcount;
#endif

	/*FIXME: compare bpp's and convert if necessary*/
	assert(dstpsd->planes == srcpsd->planes);
	assert(dstpsd->bpp == srcpsd->bpp);

	/* temporary assert() until rotation blits completed*/
	assert(dstpsd->portrait == srcpsd->portrait);
	
	/* clip blit rectangle to source screen/bitmap size*/
	/* we must do this because there isn't any source clipping setup*/
	if(srcx < 0) {
		width += srcx;
		dstx -= srcx;
		srcx = 0;
	}
	if(srcy < 0) {
		height += srcy;
		dsty -= srcy;
		srcy = 0;
	}
	if(srcx+width > srcpsd->xvirtres)
		width = srcpsd->xvirtres - srcx;
	if(srcy+height > srcpsd->yvirtres)
		height = srcpsd->yvirtres - srcy;

	switch(GdClipArea(dstpsd, dstx, dsty, dstx+width-1, dsty+height-1)) {
	case CLIP_VISIBLE:
		/* check cursor in src region*/
// by yuanlii@29bbs.net, 2/ 1/2005 18:03:07
//		GdCheckCursor(dstpsd, srcx, srcy, srcx+width-1, srcy+height-1);
// by yuanlii@29bbs.net
		dstpsd->Blit(dstpsd, dstx, dsty, width, height,
			srcpsd, srcx, srcy, rop);
// by yuanlii@29bbs.net, 2/ 1/2005 18:03:26
//		GdFixCursor(dstpsd);
// by yuanlii@29bbs.net
		return;

	case CLIP_INVISIBLE:
		return;
	}

	/* Partly clipped, we'll blit using destination clip
	 * rectangles, and offset the blit accordingly.
	 * Since the destination is already clipped, we
	 * only need to clip the source here.
	 */
#if DYNAMICREGIONS
	prc = clipregion->rects;
	count = clipregion->numRects;
#else
	prc = cliprects;
	count = clipcount;
#endif
	while(--count >= 0) {
#if DYNAMICREGIONS
		rx1 = prc->left;
		ry1 = prc->top;
		rx2 = prc->right;
		ry2 = prc->bottom;
#else
		rx1 = prc->x;
		ry1 = prc->y;
		rx2 = prc->x + prc->width;
		ry2 = prc->y + prc->height;
#endif
		/* Check:  does this rect intersect the one we want to draw? */
		px1 = dstx;
		py1 = dsty;
		px2 = dstx + width;
		py2 = dsty + height;
		if (px1 < rx1) px1 = rx1;
		if (py1 < ry1) py1 = ry1;
		if (px2 > rx2) px2 = rx2;
		if (py2 > ry2) py2 = ry2;

		pw = px2 - px1;
		ph = py2 - py1;
		if(pw > 0 && ph > 0) {
			/* check cursor in dest and src regions*/
// by yuanlii@29bbs.net, 2/ 1/2005 18:04:14
//			GdCheckCursor(dstpsd, px1, py1, px2-1, py2-1);
//			GdCheckCursor(dstpsd, srcx, srcy,
//				srcx+width, srcy+height);
// by yuanlii@29bbs.net
			dstpsd->Blit(dstpsd, px1, py1, pw, ph, srcpsd,
				srcx + (px1-dstx), srcy + (py1-dsty), rop);
		}
		++prc;
	}
// by yuanlii@29bbs.net, 2/ 1/2005 18:04:30
//	GdFixCursor(dstpsd);
// by yuanlii@29bbs.net
}

/* experimental globals for ratio bug when src != 0*/
int g_row_inc, g_col_inc;
/* Stretch source rectangle of pixels to destination rectangle quickly*/
void
GdStretchBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
	MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy, MWCOORD srcw,
	MWCOORD srch, long rop)
{
	int count;
#if DYNAMICREGIONS
	MWRECT *	prc;
	extern MWCLIPREGION *clipregion;
#else
	MWCLIPRECT *	prc;
	extern MWCLIPRECT cliprects[];
	extern int clipcount;
#endif

g_row_inc = g_col_inc = 0;

	/* check for driver stretch blit implementation*/
	if (!dstpsd->StretchBlit)
		return;

	/*FIXME: compare bpp's and convert if necessary*/
	assert(dstpsd->planes == srcpsd->planes);
	assert(dstpsd->bpp == srcpsd->bpp);
	
	/* clip blit rectangle to source screen/bitmap size*/
	/* we must do this because there isn't any source clipping setup*/
	if(srcx < 0) {
		srcw += srcx;
		/*dstx -= srcx;*/
		srcx = 0;
	}
	if(srcy < 0) {
		srch += srcy;
		/*dsty -= srcy;*/
		srcy = 0;
	}
	if(srcx+srcw > srcpsd->xvirtres)
		srcw = srcpsd->xvirtres - srcx;
	if(srcy+srch > srcpsd->yvirtres)
		srch = srcpsd->yvirtres - srcy;

	/* temp dest clipping for partially visible case*/
	if(dstx+dstw > dstpsd->xvirtres)
		dstw = dstpsd->xvirtres - dstx;
	if(dsty+dsth > dstpsd->yvirtres)
		dsth = dstpsd->yvirtres - dsty;

	switch(GdClipArea(dstpsd, dstx, dsty, dstx+dstw-1, dsty+dsth-1)) {
	case CLIP_VISIBLE:
		/* check cursor in src region*/
// by yuanlii@29bbs.net, 2/ 1/2005 18:04:47
//		GdCheckCursor(dstpsd, srcx, srcy, srcx+srcw-1, srcy+srch-1);
// by yuanlii@29bbs.net
		dstpsd->StretchBlit(dstpsd, dstx, dsty, dstw, dsth,
			srcpsd, srcx, srcy, srcw, srch, rop);
// by yuanlii@29bbs.net, 2/ 1/2005 18:04:59
//		GdFixCursor(dstpsd);
// by yuanlii@29bbs.net
		return;

	case CLIP_INVISIBLE:
		return;
	}

	/* Partly clipped, we'll blit using destination clip
	 * rectangles, and offset the blit accordingly.
	 * Since the destination is already clipped, we
	 * only need to clip the source here.
	 */
#if DYNAMICREGIONS
	prc = clipregion->rects;
	count = clipregion->numRects;
#else
	prc = cliprects;
	count = clipcount;
#endif
	while(--count >= 0) {
		int rx1, rx2, ry1, ry2;
		int px1, px2, py1, py2;
		int pw, ph;
		int sx, sy, sw, sh;
#if DYNAMICREGIONS
		rx1 = prc->left;
		ry1 = prc->top;
		rx2 = prc->right;
		ry2 = prc->bottom;
#else
		rx1 = prc->x;
		ry1 = prc->y;
		rx2 = prc->x + prc->width;
		ry2 = prc->y + prc->height;
#endif
		/* Check:  does this rect intersect the one we want to draw? */
		px1 = dstx;
		py1 = dsty;
		px2 = dstx + dstw;
		py2 = dsty + dsth;
		if (px1 < rx1) px1 = rx1;
		if (py1 < ry1) py1 = ry1;
		if (px2 > rx2) px2 = rx2;
		if (py2 > ry2) py2 = ry2;

		pw = px2 - px1;
		ph = py2 - py1;
		if(pw > 0 && ph > 0) {
			/* calc proper src/dst offset for stretch rect*/
g_row_inc = (srch << 16) / dsth;
g_col_inc = (srcw << 16) / dstw;
			sw = pw * srcw / dstw;
			sh = ph * srch / dsth;

			if (sw > 0 && sh > 0) {
				sx = srcx + (px1-dstx) * srcw / dstw;
				sy = srcy + (py1-dsty) * srch / dsth;
/*printf("P %d,%d,%d,%d   %d,%d\n", sx, sy, sw, sh, g_row_inc, g_col_inc);*/

				/* check cursor in dest and src regions*/
// by yuanlii@29bbs.net, 2/ 1/2005 18:05:11
//				GdCheckCursor(dstpsd, px1, py1, px2-1, py2-1);
//				GdCheckCursor(dstpsd, srcx, srcy, srcx+srcw, srcy+srch);
// by yuanlii@29bbs.net
				dstpsd->StretchBlit(dstpsd, px1, py1, pw, ph, srcpsd,
					sx, sy, sw, sh, rop);
			}
		}
		++prc;
	}
// by yuanlii@29bbs.net, 2/ 1/2005 18:05:31
//	GdFixCursor(dstpsd);
// by yuanlii@29bbs.net
}

/*
 * Calculate size and linelen of memory gc.
 * If bpp or planes is 0, use passed psd's bpp/planes.
 * Note: linelen is calculated to be DWORD aligned for speed
 * for bpp <= 8.  Linelen is converted to bytelen for bpp > 8.
 */
int
GdCalcMemGCAlloc(PSD psd, unsigned int width, unsigned int height, int planes,
	int bpp, int *psize, int *plinelen)
{
	int	bytelen, linelen, tmp;

	if(!planes)
		planes = psd->planes;
	if(!bpp)
		bpp = psd->bpp;
	/* 
	 * swap width and height in left/right portrait modes,
	 * so imagesize is calculated properly
	 */
	if(psd->portrait & (MWPORTRAIT_LEFT|MWPORTRAIT_RIGHT)) {
		tmp = width;
		width = height;
		height = tmp;
	}

	/*
	 * use bpp and planes to create size and linelen.
	 * linelen is in bytes for bpp 1, 2, 4, 8, and pixels for bpp 16,24,32.
	 */
	if(planes == 1) {
		switch(bpp) {
		case 1:
			linelen = (width+7)/8;
			bytelen = linelen = (linelen+3) & ~3;
			break;
		case 2:
			linelen = (width+3)/4;
			bytelen = linelen = (linelen+3) & ~3;
			break;
		case 4:
			linelen = (width+1)/2;
			bytelen = linelen = (linelen+3) & ~3;
			break;
		case 8:
			bytelen = linelen = (width+3) & ~3;
			break;
		case 16:
			linelen = width;
			bytelen = width * 2;
			break;
		case 24:
			linelen = width;
			bytelen = width * 3;
			break;
		case 32:
			linelen = width;
			bytelen = width * 4;
			break;
		default:
			return 0;
		}
	} else if(planes == 4) {
		/* FIXME assumes VGA 4 planes 4bpp*/
		/* we use 4bpp linear for memdc format*/
		linelen = (width+1)/2;
		linelen = (linelen+3) & ~3;
		bytelen = linelen;
	} else {
		*psize = *plinelen = 0;
		return 0;
	}

	*plinelen = linelen;
	*psize = bytelen * height;
	return 1;
}
