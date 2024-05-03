/*	----------------------------------------------------------------
 *  	RGB_bitmap
 *	---------------------------------------------------------------- */
#ifndef __CLASS_RGB_BITMAP_HPP
	#define __CLASS_RGB_BITMAP_HPP

	#include <cstdio>
	#include <cstdlib>
	#include <cstdint>
	#include <cstring>

	//#include "bitmaps.hpp"
	#include "struct_RGB.hpp"

class RGB_bitmap
{
	friend int load_sp4_rgb_bitm(const char *filename, RGB_bitmap * bitmap);
	friend int save_sp4_rgb_bitm(const char *filename, RGB_bitmap * bitmap);
	friend int save_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap);
	friend int load_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap);
	friend int move_bitmap_data(RGB_bitmap *dst, RGB_bitmap *src);
	
public:
	enum LoadFileFormat { FORMAT_SP4, FORMAT_PPM };

private:
	char * 		data_;
	uint16_t	width_,
				height_;
	uint32_t	raw_data_length_;

public:
	
	RGB_bitmap(void) : 																		// empty unallocated bitmap
		data_(nullptr), width_(0), height_(0), raw_data_length_(0) {}	

	RGB_bitmap(const int w, const int h) : 													// allocate empty bitmap
		data_(nullptr), width_(0), height_(0), raw_data_length_(0) 
	{
		create(w, h);
	}

	~RGB_bitmap(void) { erase(); }

	//

	bool has_data(void)				{ return (data_ != nullptr ? true : false); }
	bool exists(void)				{ return (data_ != nullptr ? true : false); }
	bool empty(void)				{ return (data_ == nullptr ? true : false); }

	int width(void) 		 		{ return width_; }
	int height(void) 		 		{ return height_; }

	uint32_t raw_data_length(void)	{ return raw_data_length_; }

	uint8_t pixel_size(void)		{ return RGB_PIXEL_SIZE; }

	//

	int create(const int w, const int h);
	int load(const char * filename, LoadFileFormat format = FORMAT_SP4);
	int save(const char * filename, LoadFileFormat format = FORMAT_SP4);
	void erase(void);

	char * data(void) 				{ return data_; };

	RGB get_pixel(const int w, const int h);
	RGB * get_pixel_ptr(const int w, const int h);

	int put_pixel(const int w, const int h, RGB pixel);

	int fill(RGB color);

};

#endif