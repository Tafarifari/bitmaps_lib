/*
 *	bitmaps.h
 *	RGB, RGBA handling routines
 *
 */
#ifndef __BITMAPS_HPP
	#define __BITMAPS_HPP

	#include <cstdio>
	#include <cstdlib>
	#include <cstring>

	//#include "geometry.hpp"

	#include "struct_RGB.hpp"
	#include "struct_RGBA.hpp"

	#include "class_RGB_bitmap.hpp"
	#include "class_RGBA_bitmap.hpp"
	#include "class_RGBA_sprite.hpp"
	
	#define __SP4_MARKER    "S4"
	#define __MARKER_LEN    2       // in bytes
	
	/* 		LOAD/SAVE
	 *		sp4																*/

	int save_sp4_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);
	int load_sp4_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);

	int save_sp4_rgb_bitm(const char *filename, RGB_bitmap * bitmap);					/* no transparency conversion, all alpha set to 100 */
	int load_sp4_rgb_bitm(const char *filename, RGB_bitmap * bitmap);

	int save_sp4_sprite(const char *filename, RGBA_sprite * spr);
	int load_sp4_sprite(const char *filename, RGBA_sprite * spr);

	/* 		LOAD/SAVE
	 *		ppm3																*/
	
	int save_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap);
	int load_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap);	// bugged? / tested 11.03, ok

	int save_ppm_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);
	int load_ppm_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);	// bugged ???
	// TODO int load_ppm_rgba_transp(const char *filename, RGBA_bitmap * bitmap, RGB transp);	/* for every pixel == transp alpha = 0 */

	/*		ALPHA PRESERVATION
	 *		valid for all plot_bitmap and plot_sprite routines if dst is RGBA
	 *		default value DST_ALPHA_0x64									*/

	void plot_bitmap__dst_alpha_0x64(void);												/* for every plotted pixel alpha is set to 0x64 */
	void plot_bitmap__use_dst_alpha(void);												/* dst alpha is preserverd for all pixels; */
	void plot_bitmap__use_src_alpha(void);												/* for every plotted pixel alpha is copied from src pixel (if RGBA)
																						   or override_alpha value used (as 1-100 uint) if src pixel is RGB 
																						   and override_alpha != -1; otherwise dst alpha is preserved; */
	/*		PLOT SPRITE
	 * 		plot with clipping and alpha for all visible pixels 			*/

	int plot_sprite(RGB_bitmap *dst, RGBA_sprite *src, float alpha = 1.0); 				/* sprite on rgb, clipped, fixed alpha for all visible pixels */
	int plot_sprite(RGBA_bitmap *dst, RGBA_sprite *src, float alpha = 1.0);				/* sprite on rgb, clipped, fixed alpha for all visible pixels */
	int plot_sprite(RGBA_sprite *dst, RGBA_sprite *src, float alpha = 1.0); 			/* sprite on sprite, clipped, fixed alpha for all visible pixels */

	/*		PLOT BITMAP														*/

	int plot_bitmap(RGBA_bitmap *dst, RGBA_bitmap *src, int x, int y); 					/* rgba on rgba, clipped, meaningful alpha */
	int plot_bitmap(RGBA_bitmap *dst, RGB_bitmap *src, int x, int y, float alpha = 1.0);/* rgb on rgb, clipped, fixed alpha */

	int plot_bitmap(RGB_bitmap *dst, RGBA_bitmap *src, int x, int y);					/* rgba on rgb, clipped, meaningful alpha */
	int plot_bitmap(RGB_bitmap *dst, RGB_bitmap *src, int x, int y, float alpha = 1.0);	/* rgb on rgb, clipped, fixed alpha */

	int plot_bitmap(RGBA_sprite *dst, RGBA_bitmap *src, int x, int y);					/* rgba on sprite, clipped, meaningful alpha */
	int plot_bitmap(RGBA_sprite *dst, RGB_bitmap *src, int x, int y, float alpha = 1.0);/* rgb on sprite, clipped, fixed alpha */

	/*		DESTRUCTIVE FADE TO BLACK										*/

	int fade_bitmap(RGB_bitmap *dst, uint8_t alpha);
	int fade_bitmap(RGBA_bitmap *dst, uint8_t alpha);									/* fades only pixels with alpha != 0, sets alpha to 0x64 */

	/* 		QUICK COPY
	 *		effectively quick way of plotting
	 * 			- no alpha/transparency checking, no clipping
	 *			- both bitmaps must be initialised							*/

	int quick_copy(RGB_bitmap *dst, RGB_bitmap *src, int dst_x, int dst_y, int src_x, int src_y, int width, int height); 
	int quick_copy(RGBA_bitmap *dst, RGBA_bitmap *src, int dst_x, int dst_y, int src_x, int src_y, int width, int height);

	/* 		MOVE DATA
	 *		moves data from src to dst without copying, leaves stc empty	*/

	int move_bitmap_data(RGB_bitmap *dst, RGB_bitmap *src);
	int move_bitmap_data(RGBA_bitmap *dst, RGBA_bitmap *src);

	/* 		COPY
	 * 		no alpha/transparency checking, no clipping						*/

	int copy_bitmap(RGB_bitmap *out, RGB_bitmap *in);
	int copy_bitmap(RGBA_bitmap *out, RGBA_bitmap *in);

	/*		SCALE															*/

	int scale_bitmap(RGB_bitmap *out, RGB_bitmap *in, float scale);
	int scale_bitmap(RGBA_bitmap *out, RGBA_bitmap *in, float scale);

/*	move all draw functionality to separate library so that Bitmaps won't depend on geometry.cpp
 	//		DRAW										
	int draw_line(RGB_bitmap *dst, uint x1, uint y1, uint x2, uint y2, RGB color, LineAlgorithm alg = DDA);
	// TODO	int draw_line(RGBA_bitmap *dst, uint x1, uint y1, uint x2, uint y2, RGBA color);

	// TODO	int draw_rect(RGB_bitmap *dst, uint x1, uint y1, uint x2, uint y2, RGB color);
	// TODO	int draw_rect(RGBA_bitmap *dst, uint x1, uint y1, uint x2, uint y2, RGBA color);  */

	/*		CONVERT															*/

	// TODO	int sprite_to_rgba(RGBA_bitmap *dst, RGBA_sprite *src, int frame = 0);
	// TODO	int sprite_to_rgb(RGB_bitmap *dst, RGBA_sprite *src, int frame = 0);
	
	int rgb_to_rgba(RGBA_bitmap *dst, RGB_bitmap *src, uint8_t alpha = 100, bool transp = false, RGB transp_color = { 0, 0xff, 0});
	int rgba_to_rgb(RGB_bitmap *dst, RGBA_bitmap *src);

	// TODO	int rgba_to_sprite(RGBA_sprite *dst, RGBA_bitmap **src_list, int frames_num);
	// TODO	int rgb_to_sprite(RGBA_sprite *dst, RGB_bitmap **src_list, int frames_num, RGB transp = { 0, 0xff, 0});

#endif