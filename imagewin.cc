/**
 **	Imagewin.cc - A window to blit images into.
 **
 **	Written: 8/13/98 - JSF
 **/

/*
Copyright (C) 1998 Jeffrey S. Freedman

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA  02111-1307, USA.
*/

#include "imagewin.h"
#include <string.h>
#include <iostream.h>

/*
 *	Create buffer.
 */

Image_buffer_base::Image_buffer_base
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Depth (bits/pixel).
	) : width(w), height(h), depth(dpth),
	    clipx(0), clipy(0),
	    clipw(w), cliph(h), bits(0), line_width(w)
	{
	switch (depth)			// What depth?
		{
	case 8: 
		pixel_size = 1; break;
	case 15:
	case 16:
		pixel_size = 2; break;
	case 32:
		pixel_size = 4; break;
		}
	}


/*
 *	Copy an area of the image within itself.
 */

void Image_buffer8::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	char *to = bits + yto*line_width + destx;
	char *from = bits + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Fill with a given 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix
	)
	{
	char *pixels = bits;
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with an 8-bit value.
 */

void Image_buffer8::fill8
	(
	unsigned char pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned char *pixels = (unsigned char *) bits + 
						desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer8::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer8::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	char *to = bits + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = chr;
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create a default palette for 8-bit graphics on a 16-bit display.
 */

void Image_buffer16::create_default_palette
	(
	)
	{
	delete palette;			// Delete old.
	palette = new unsigned short[256];
	for (int i = 0; i < 256; i++)
		palette[i] = rgb(4*((i >> 5) & 0x7), 
			4*((i >> 2) & 0x7), 9*(i & 0x3));
	}

/*
 *	Fill with a given 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix
	)
	{
	unsigned short *pixels = get_pixels();
	int cnt = line_width*height;
	for (int i = 0; i < cnt; i++)
		*pixels++ = pix;
	}

/*
 *	Fill a rectangle with a 16-bit value.
 */

void Image_buffer16::fill16
	(
	unsigned short pix,
	int srcw, int srch,
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *pixels = get_pixels() + desty*line_width + destx;
	int to_next = line_width - srcw;// # pixels to next line.
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*pixels++ = pix;
		pixels += to_next;	// Get to start of next line.
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy16
	(
	unsigned short *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned short *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = *from++;
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy an area of the image within itself.
 */

void Image_buffer16::copy
	(
	int srcx, int srcy,		// Where to start.
	int srcw, int srch,		// Dimensions to copy.
	int destx, int desty		// Where to copy to.
	)
	{
	int ynext, yfrom, yto;		// Figure y stuff.
	if (srcy >= desty)		// Moving up?
		{
		ynext = line_width;
		yfrom = srcy;
		yto = desty;
		}
	else				// Moving down.
		{
		ynext = -line_width;
		yfrom = srcy + srch - 1;
		yto = desty + srch - 1;
		}
	unsigned short *to = get_pixels() + yto*line_width + destx;
	unsigned short *from = get_pixels() + yfrom*line_width + srcx;
					// Go through lines.
	while (srch--)
		{
		memmove((char *) to, (char *) from, srcw * 2);
		to += ynext;
		from += ynext;
		}
	}

/*
 *	Convert rgb value.
 */

inline unsigned char Get_color16
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned int c = (((unsigned int) val)*brightness*32)/
							(100*(maxval + 1));
	return (c < 32 ? c : 31);
	}

/*
 *	Set palette.
 */

void Image_buffer16::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
					// Get the colors.
	for (int i = 0; i < 3*256; i += 3)
		{
		unsigned char r = Get_color16(rgbs[i], maxval, brightness);
		unsigned char g = Get_color16(rgbs[i + 1], maxval, brightness);
		unsigned char b = Get_color16(rgbs[i + 2], maxval, brightness);
		set_palette_color(i/3, r, g, b);
		}
	}

/*
 *	Copy another rectangle into this one.
 */

void Image_buffer16::copy8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--)
			*to++ = palette[*from++];
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Copy another rectangle into this one, with 0 being the transparent
 *	color.
 */

void Image_buffer16::copy_transparent8
	(
	unsigned char *src_pixels,	// Source rectangle pixels.
	int srcw, int srch,		// Dimensions of source.
	int destx, int desty
	)
	{
	int srcx = 0, srcy = 0;
	int src_width = srcw;		// Save full source width.
					// Constrain to window's space.
	if (!clip(srcx, srcy, srcw, srch, destx, desty))
		return;
	unsigned short *to = get_pixels() + desty*line_width + destx;
	unsigned char *from = src_pixels + srcy*src_width + srcx;
	int to_next = line_width - srcw;// # pixels to next line.
	int from_next = src_width - srcw;
	while (srch--)			// Do each line.
		{
		for (int cnt = srcw; cnt; cnt--, to++)
			{
			register int chr = *from++;
			if (chr)
				*to = palette[chr];
			}
		to += to_next;
		from += from_next;
		}
	}

/*
 *	Create top-level image buffer.
 */

Image_buffer::Image_buffer
	(
	unsigned int w,			// Desired width, height.
	unsigned int h,
	int dpth			// Color depth (bits/pixel).
	)
	{
	switch (dpth)			// What depth?
		{
	case 8: 
		ibuf = new Image_buffer8(w, h);
		break;
	case 15:
	case 16:
		ibuf = new Image_buffer16(w, h, dpth);
		break;
	case 32:
	default:
		ibuf = 0;		// Later.
		}
	}

/*
 *	Get best depth.  Returns bits/pixel.
 */
static int Get_best_depth
	(
	)
	{
					// Get video info.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
					// The "best" format:
	return (vinfo->vfmt->BitsPerPixel);
	}

/*
 *	Create window.
 */

Image_window::Image_window
	(
	unsigned int w,			// Desired width, height.
	unsigned int h
	) : Image_buffer(w, h, Get_best_depth()),
	    surface(0)
	{
	create_surface(w, h);
	}

/*
 *	Destroy window.
 */

Image_window::~Image_window
	(
	)
	{
	SDL_FreeSurface(surface);
	}

/*
 *	Create the surface.
 */

void Image_window::create_surface
	(
	unsigned int w, 
	unsigned int h
	)
	{
	ibuf->width = w;
	ibuf->height = h;
	surface = SDL_SetVideoMode(w, h, ibuf->depth, 
					SDL_SWSURFACE |  SDL_HWPALETTE);
	if (!surface)
		{
		cout << "Couldn't set video mode (" << w << ", " << h <<
			") at " << ibuf->depth << " bpp depth: " <<
			SDL_GetError() << '\n';
		exit(-1);
		}
	ibuf->bits = (char *) surface->pixels;
					// Update line size in words.
	ibuf->line_width = surface->pitch/ibuf->pixel_size;
	}

/*
 *	Free the surface.
 */

void Image_window::free_surface
	(
	)
	{
	if (!surface)
		return;
	SDL_FreeSurface(surface);
	ibuf->bits = 0;
	surface = 0;
	}

/*
 *	Window was resized.
 */

void Image_window::resized
	(
	unsigned int neww, 
	unsigned int newh
	)
	{
	if (surface)
		{
		if (neww == ibuf->width && newh == ibuf->height)
			return;		// Nothing changed.
		free_surface();		// Delete old image.
		}
	create_surface(neww, newh);	// Create new one.
	}

/*
 *	Repaint window.
 */

void Image_window::show
	(
	)
	{
	if (!ready())
		return;
	SDL_UpdateRect(surface, 0, 0, ibuf->width, ibuf->height);
	}

/*
 *	Convert rgb value.
 */

inline unsigned short Get_color8
	(
	unsigned char val,
	int maxval,
	int brightness			// 100=normal.
	)
	{
	unsigned long c = (((unsigned long) val)*brightness*65535L)/
							(100*(maxval + 1));
	return (c <= 65535L ? (unsigned short) c : 65535);
	}

/*
 *	Set palette.
 */

void Image_window::set_palette
	(
	unsigned char *rgbs,		// 256 3-byte entries.
	int maxval,			// Highest val. for each color.
	int brightness			// Brightness control (100 = normal).
	)
	{
	if (ibuf->depth != 8)		// Need to handle in software?
		{
		Image_buffer::set_palette(rgbs, maxval, brightness);
		return;
		}
	SDL_Color colors[256];
					// Get the colors.
	for (int i = 0; i < 256; i++)
		{
		colors[i].r = Get_color8(rgbs[3*i], maxval, brightness);
		colors[i].g = Get_color8(rgbs[3*i + 1], maxval,
							brightness);
		colors[i].b = Get_color8(rgbs[3*i + 2], maxval,
							brightness);
		}
	SDL_SetColors(surface, colors, 0, 256);
	}

