/*	----------------------------------------------------------------
 *  	RGBA_sprite
 *	---------------------------------------------------------------- */
#ifndef __CLASS_RGBA_SPRITE_HPP
	#define __CLASS_RGBA_SPRITE_HPP

	#include <cstdio>
	#include <cstdlib>
	#include <cstring>

	#include "struct_RGBA.hpp"


class RGBA_sprite;

// from bitmaps.hpp
int save_sp4_sprite(const char *filename, RGBA_sprite * spr);
int load_sp4_sprite(const char *filename, RGBA_sprite * spr);


class RGBA_sprite 
{
public:

	uint8_t **	frames;
	uint8_t	*	frames_data;
	uint8_t	*	screen_time;

	uint8_t		pixel_size_;	// curr. unused; for fut. GRAYSCALE/RGB/RGBA sprites
	uint8_t		frames_num_;
	uint8_t		current_frame_;

	int16_t 	x_,
				y_;
	uint16_t	width_,
				height_; 

	uint32_t 	frame_data_length;

	bool		default_screen_times_;


	RGBA_sprite(void) 					{ memset(this, 0, sizeof(RGBA_sprite)); }
	~RGBA_sprite(void) 					{ if(exists()) erase(); }

	void init(void)						{ memset(this, 0, sizeof(RGBA_sprite)); }

	//

	bool 	exists(void)				{ return (bool) frames; }
	bool 	empty(void)					{ return (frames == nullptr ? true : false); }

	int 	x(void)						{ return x_; }
	int 	y(void)						{ return y_; }
	int 	width(void) 				{ return width_; }
	int 	height(void)				{ return height_; }

	bool 	default_screen_times(void) 	{ return default_screen_times_; }

	uint8_t pixel_size(void)			{ return RGBA_PIXEL_SIZE; }

	//	

	int 	create(uint8_t fr, const uint16_t w, const uint16_t h);


	int 	save(const char *filename)	{ return save_sp4_sprite(filename, this); }
	int 	load(const char *filename)	{ if(exists()) erase(); return load_sp4_sprite(filename, this); }
	
	void 	erase(void);
	
	void 	x(int new_x)				{ x_ = new_x; }
	void 	y(int new_y)				{ y_ = new_y; }

	int 	fill_current(RGBA color);
	int 	fill_all(RGBA color);

	//

	int 	push_frame(void);

	uint8_t frames_num(void) 			{ return frames_num_; }
	uint8_t current_frame(void) 		{ return current_frame_; }
	uint8_t current_frame(uint8_t fr) 	{ return (current_frame_ = (fr < frames_num_ ? fr : frames_num_ - 1)); }
	uint8_t last_frame(void) 			{ return (frames_num_ > 0 ? frames_num_ - 1 : 0); }

	uint8_t get_time(uint8_t fr) 		{ if(!exists()) return 0; return (fr < frames_num_) ? screen_time[fr] : 0; }
	uint8_t get_time(void) 				{ if(!exists()) return 0; return screen_time[current_frame_]; }

	uint8_t * frame_data(uint8_t fr) 	{ if(!exists()) return nullptr; return (fr < frames_num_) ? frames[fr] : nullptr; }
	uint8_t * current_frame_data(void) 	{ if(!exists()) return nullptr; return (frames[current_frame_]); }

	RGBA 	get_pixel(uint16_t x, uint16_t y);
	RGBA * 	get_pixel_ptr(uint16_t x, uint16_t y);
	int 	put_pixel(uint16_t x, uint16_t y, RGBA pixel);

};

#endif