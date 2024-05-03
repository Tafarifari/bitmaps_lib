/*	--------------------------------------------------------------
 * 		RGB_bitmap
 *	-------------------------------------------------------------- */
#include <cstdint>

#include "bitmaps.hpp"


int RGB_bitmap::create(const int w, const int h)
{
	uint32_t rgb_pixel_length = w * h * RGB_PIXEL_SIZE;

	if(exists()) erase();

	data_ = (char *) malloc(rgb_pixel_length);
	if(data_ == nullptr) {
		fprintf(stderr, "RGB_bitmap::create: could not allocate memory\n");
		return -1;
	}
	memset(data_, 0, rgb_pixel_length); // fill array with zeros so the alocated memory is fully 'owned' by the process

	width_ = w;
	height_ = h;
	raw_data_length_ = rgb_pixel_length;
	return 0;
}


int RGB_bitmap::load(const char * filename, LoadFileFormat format)
{
	if(exists()) erase();

	switch(format) 
	{
	case FORMAT_SP4: 
		if(load_sp4_rgb_bitm(filename, this) == -1) {
			fprintf(stderr, "RGB_bitmap::load: could not allocate memory\n");
			return -1;
		}
		break;	
	case FORMAT_PPM:
		if(load_ppm_rgb_bitm(filename, this) == -1) {
			fprintf(stderr, "RGB_bitmap::load: could not allocate memory\n");
			return -1;
		}
		break;	
	}
	return 0;
}


int RGB_bitmap::save(const char * filename, LoadFileFormat format)
{
	if(!exists()) return -1;

	switch(format) 
	{
	case FORMAT_SP4: 
		if(save_sp4_rgb_bitm(filename, this) == -1) {
			fprintf(stderr, "RGB_bitmap::save: could not allocate memory\n");
			return -1;
		}
		break;	
	case FORMAT_PPM:
		if(save_ppm_rgb_bitm(filename, this) == -1) {
			fprintf(stderr, "RGB_bitmap::save: could not allocate memory\n");
			return -1;
		}
		break;	
	}
	return 0;
}


void RGB_bitmap::erase(void)
{ 
	if(data_ != nullptr) 
	{
		free(data_);
		data_ = nullptr;
	}
	width_ = height_ = raw_data_length_ = 0;
}


RGB RGB_bitmap::get_pixel(const int x, const int y)
{
	if(x < 0 || y < 0 || y >= height_ || x >= width_) {
		fprintf(stderr, "RGB_bitmap::get_pixel: pixel out of range\n");
		return { 0, 0, 0 };
	}
	RGB pixel;
	uint32_t offset = (y * width_ + x) * RGB_PIXEL_SIZE;
	memcpy(&pixel, &data_[offset], RGB_PIXEL_SIZE);
	return pixel;
}


RGB * RGB_bitmap::get_pixel_ptr(const int x, const int y) 
{ 
	uint32_t offset = (y * width_ + x) * RGB_PIXEL_SIZE;
	return (RGB *) &data_[offset]; 
}


int RGB_bitmap::put_pixel(const int x, const int y, RGB pixel)
{
	if(data_ == nullptr) {
		fprintf(stderr, "RGB_bitmap::put_pixel: empty bitmap\n");
		return -1;
	} else if(x >= width_) {
		fprintf(stderr, "RGB_bitmap::put_pixel: width out of range (%d>=%d)\n", x, width_);
		return -1;
	} else if(y >= height_) {
		fprintf(stderr, "RGB_bitmap::put_pixel: height out of range (%d>=%d)\n", y, height_);
		return -1;
	}
	uint32_t offset = (y * width_ + x) * RGB_PIXEL_SIZE;
	memcpy(&data_[offset], &pixel, RGB_PIXEL_SIZE);
	return 0;
}


int RGB_bitmap::fill(RGB color)
{
	if(!exists()) return -1;

	for(uint32_t offset = 0; offset < raw_data_length_; offset += RGB_PIXEL_SIZE) {
		memcpy(&data_[offset], &color, RGB_PIXEL_SIZE);
	}
	return 0;
}