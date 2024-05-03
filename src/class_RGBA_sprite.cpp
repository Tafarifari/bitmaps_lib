/*	-----------------------------------------------------------
 *		RGBA_sprite
 *	-----------------------------------------------------------*/

#include "class_RGBA_sprite.hpp"

int 
RGBA_sprite::create(uint8_t fr, const uint16_t w, const uint16_t h)
{
	uint32_t frame_data_length = w * h * RGBA_PIXEL_SIZE;
	uint32_t offset;

	if((screen_time = (uint8_t *) malloc(fr)) == nullptr) {
		fprintf(stderr, "RGBA_sprite::create: failed to allocate memory for screen times\n");
		goto ERROR_EXIT;
	}

	if((frames = (uint8_t **) malloc(fr * sizeof(uint8_t *))) == nullptr) {
		fprintf(stderr, "RGBA_sprite::create: failed to allocate memory for frames index\n");
		goto ERROR_EXIT;
	}

	if((frames_data = (uint8_t*) malloc(fr * frame_data_length * sizeof(uint8_t))) == nullptr) {
		fprintf(stderr, "RGBA_sprite::create: failed to allocate memory for frames data\n");
		goto ERROR_EXIT;
	}
	
	offset = 0;
	for(uint i = 0; i < fr; offset += frame_data_length, ++i) 
	{
		frames[i] = &frames_data[offset];
	}

	this->frames_num_ = fr;
	this->current_frame_ = 0;
	this->x_ = 0;
	this->y_ = 0;
	this->width_ = w;
	this->height_ = h; 
	this->pixel_size_ = RGBA_PIXEL_SIZE;
	this->frame_data_length = frame_data_length;
	this->default_screen_times_ = true;
	return 0;

ERROR_EXIT:
	if(screen_time) free(screen_time);
	if(frames) free(frames);
	if(frames_data) free(frames_data);
	memset(this, 0, sizeof(RGBA_sprite));
	return -1;
}


void RGBA_sprite::erase(void)
{
	if(screen_time) free(screen_time);
	if(frames) free(frames);
	if(frames_data) free(frames_data);
	memset(this, 0, sizeof(RGBA_sprite));
}


int 
RGBA_sprite::fill_current(RGBA color)
{
	if(!frames) return -1;

	uint8_t * 	data = frames[current_frame_];
	uint32_t 	offset = 0;
	for(; offset < frame_data_length; offset += RGBA_PIXEL_SIZE) {
		memcpy(&data[offset], &color, RGBA_PIXEL_SIZE);
	}
	return 0;
}

int 
RGBA_sprite::fill_all(RGBA color)
{
	if(!frames) return -1;
	
	uint8_t * 	data;
	uint32_t 	offset;

	for(uint fr = 0; fr < frames_num_; ++fr)
	{
		data = frames[fr];
		for(offset = 0; offset < frame_data_length; offset += RGBA_PIXEL_SIZE) {
			memcpy(&data[offset], &color, RGBA_PIXEL_SIZE);
		}
	}
	return 0;
}


int 
RGBA_sprite::push_frame(void)
{
	if(++current_frame_ == frames_num_) current_frame_ = 0;
	return current_frame_;
}


RGBA
RGBA_sprite::get_pixel(uint16_t x, uint16_t y)
{
	if(!frames) return { 0, 0, 0, 0 };
	uint8_t * data = frames[current_frame_];
	if(!data) return { 0, 0, 0, 0 };
	
	RGBA pixel;
	uint32_t offset = (y * width_ + x) * RGBA_PIXEL_SIZE;
	memcpy(&pixel, &data[offset], RGBA_PIXEL_SIZE);
	return pixel;
}


RGBA * 
RGBA_sprite::get_pixel_ptr(uint16_t x, uint16_t y)
{
	if(!frames) return nullptr;
	uint8_t * data = frames[current_frame_];
	if(!data) return nullptr;
	
	uint32_t offset = (y * width_ + x) * RGBA_PIXEL_SIZE;
	return (RGBA *) &data[offset];
}

int
RGBA_sprite::put_pixel(uint16_t x, uint16_t y, RGBA pixel)
{
	if(!frames) return -1;
	uint8_t * data = frames[current_frame_];
	if(!data) return -1;

	uint32_t offset = (y * width_ + x) * RGBA_PIXEL_SIZE;
	memcpy(&data[offset], &pixel, RGBA_PIXEL_SIZE);
	return 0;
}
