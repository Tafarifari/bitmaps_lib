/*	----------------------------------------------------------------
 *  	RGBA_bitmap
 *	---------------------------------------------------------------- */
#ifndef __CLASS_RGBA_BITMAP_HPP
	#define __CLASS_RGBA_BITMAP_HPP

	#include <cstdio>
	#include <cstdlib>
	#include <cstdint>
	#include <cstring>

	#include "bitmaps.hpp"

class RGBA_bitmap
{
	friend int load_sp4_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);
	friend int save_sp4_rgba_bitm(const char *filename, RGBA_bitmap * bitmap);
	friend int move_bitmap_data(RGBA_bitmap *dst, RGBA_bitmap *src);

public:
	enum LoadFileFormat { FORMAT_SP4, FORMAT_PPM };

private:
	char * 		data_;
	uint16_t	width_,
				height_;
	uint32_t	raw_data_length_;
	bool 		flag_meaningful_alpha;

public:

	RGBA_bitmap(void) : 
		data_(nullptr), width_(0), height_(0), raw_data_length_(0), flag_meaningful_alpha(false) {}

	RGBA_bitmap(const int w, const int h) : flag_meaningful_alpha(true)
	{
		create(w, h);
	}
	
	~RGBA_bitmap(void) { erase(); }

	//

	bool has_data(void)				{ return (data_ != nullptr ? true : false); }
	bool exists(void)				{ return (data_ != nullptr ? true : false); }
	bool empty(void)				{ return (data_ == nullptr ? true : false); }

	int width(void) 		 		{ return width_; }
	int height(void) 		 		{ return height_; }

	uint8_t pixel_size(void)		{ return RGBA_PIXEL_SIZE; }

	uint32_t raw_data_length(void)	{ return raw_data_length_; }

	void meaningful_alpha(bool v) 	{ flag_meaningful_alpha = v; }
	bool meaningful_alpha(void)		{ return flag_meaningful_alpha; }

	//

	int create(const int w, const int h);
	int load(const char * filename, LoadFileFormat format = FORMAT_SP4);
	int save(const char * filename, LoadFileFormat format = FORMAT_SP4);
	void erase(void);

	char * data(void) 				{ return data_; };

	RGBA get_pixel(const int w, const int h);
	RGBA * get_pixel_ptr(const int w, const int h);

	int put_pixel(const int w, const int h, RGBA pixel);

	int fill(RGBA color);

};

#endif