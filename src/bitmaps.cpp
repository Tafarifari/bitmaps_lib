#include "bitmaps.hpp"
#include "ppm.hpp"



/*	---------------------------------------------------------------
 *
 *						LOAD AND SAVE SP4 
 *
 *	--------------------------------------------------------------- */


/*
 *	read_sp4
 *	has to receive VALID file pointer set at the beginning of the SP4 file
 *	and unallocated, NULL char pointer
 *	returns 0 on success, -1 on failure
 *	DOESN'T close the file pointer!
 */
static int read_sp4(FILE * fp,
				    char ** data, 
					uint16_t * width,
					uint16_t * height,
					uint8_t	* screen_time,
					uint8_t * frames_num)
{
	uint32_t 	raw_data_length;
	char 		marker[__MARKER_LEN];

	*data = NULL;

	if(fread(marker, 1, __MARKER_LEN, fp) != __MARKER_LEN) 		goto FREAD_ERROR;
	if(marker[0] != 'S' && marker[1] != '4') 
	{
		fprintf(stderr, "read_sp4: wrong format marker: \"%s\"\n", marker);
		return -1;
	}

	if(fread(width, 2, 1, fp) != 1) 							goto FREAD_ERROR;
	if(fread(height, 2, 1, fp) != 1)							goto FREAD_ERROR;
	if(fread(frames_num, 1, 1, fp) != 1)						goto FREAD_ERROR;
	if(fread(screen_time, 1, *frames_num, fp) != *frames_num)	goto FREAD_ERROR;

	raw_data_length = (*width) * (*height) * (*frames_num) * RGBA_PIXEL_SIZE;
	
	*data = (char *) malloc(raw_data_length);
	if(*data == NULL) {
		fprintf(stderr, "read_sp4: failed to allocate memory for data\n");
		return -1;
	} 
	
	if(fread(*data, 1, raw_data_length, fp) != raw_data_length)	goto FREAD_ERROR;

	//return 0;
	return raw_data_length;

FREAD_ERROR:
	fprintf(stderr, "read_sp4: fread error, data may be corrupt\n");
	if(*data != NULL) {
		free(*data);
		*data = NULL;
	}
	return -1;
}


//
//		SP4 - RGBA_BITMAP
//

//
//	LOAD_SP4_RGBA_BITM
//	loads first frame of sp4 file
//	returns 0 in SUCCESS, -1 on FAILURE
//
int load_sp4_rgba_bitm(const char * filename, RGBA_bitmap * bitmap)
{
	FILE *		fp;

	char * 		data = NULL;
	uint16_t	width = 0,
				height = 0;
	uint8_t		screen_time[UINT8_MAX] = { 0 };
	uint8_t 	frames_num = 0;

	if((fp = fopen(filename,"rb")) == NULL) 
	{
		fprintf(stderr, "load_sp4_rgba_bitm: error opening file \"%s\"\n", filename);
		return -1;
	}

	if(read_sp4(fp, &data, &width, &height, screen_time, &frames_num) == -1)
	{
		fclose(fp);
		fprintf(stderr, "load_sp4_rgba_bitm: error reading file \"%s\"\n", filename);
		return -1;	
	}

	fclose(fp);

	if(bitmap->exists()) bitmap->erase();
	if(bitmap->create(width, height) == -1) {
		fprintf(stderr, "load_sp4_rgba_bitm: failed to create bitmap\n");
		free(data);
		return -1;	
	}
	memcpy(bitmap->data(), data, bitmap->raw_data_length_);
	free(data);
	bitmap->meaningful_alpha(true);

	return 0;
}


//
//	SAVE_SP4_RGBA_BITM
//	saves RGBA_Bitmap as 1-frame SP4
//	returns 0 in SUCCESS, -1 on FAILURE
//
int save_sp4_rgba_bitm(const char *filename, RGBA_bitmap * bitmap)
{
	if(!bitmap->exists()) return -1;

	FILE *fp;
	if((fp = fopen(filename,"wb")) == NULL) {
			fprintf(stderr, "save_sp4_rgba_bitm: failed to create file \"%s\"\n", filename);
			return -1;
	}

	uint8_t frames_num = 1;
	uint8_t screen_time = 0;

	/* size_t fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream); */
	if(fwrite(__SP4_MARKER, 1, 2, fp) != 2)			goto FWRITE_ERROR; // marker
	if(fwrite(&(bitmap->width_), 2, 1, fp) != 1)	goto FWRITE_ERROR; // width
	if(fwrite(&(bitmap->height_), 2, 1, fp) != 1)	goto FWRITE_ERROR; // height
	if(fwrite(&frames_num, 1, 1, fp) != 1)			goto FWRITE_ERROR; // number of frames = 1
	if(fwrite(&screen_time, 1, 1, fp) != 1)			goto FWRITE_ERROR; // screen time table (1 byte, value = 0)
	if(fwrite(bitmap->data_, 1, bitmap->raw_data_length_, fp) != bitmap->raw_data_length_) goto FWRITE_ERROR;
	
	fclose(fp);
	return 0;

FWRITE_ERROR:
	fclose(fp);
	fprintf(stderr, "save_sp4_rgba_bitm: fwrite error at file \"%s\", some data may be corrupt\n", filename);	
	return -1;
}

//
//		RGB_BITMAP
//

//
//	LOAD_SP4_RGB_BITM
//	loads first frame of sp4 file, transparency lost
//	expects unallocated RGBA_Bitmap
//	returns 0 in SUCCESS, -1 on FAILURE
//
int load_sp4_rgb_bitm(const char *filename, RGB_bitmap * bitmap)
{
	FILE *		fp;

	char * 		rgba_data = NULL;
	char * 		rgb_data = NULL;

	uint16_t	width = 0,
				height = 0;
	uint8_t		screen_time[UINT8_MAX] = { 0 };
	uint8_t 	frames_num = 0;

	if((fp = fopen(filename,"rb")) == NULL) 
	{
		fprintf(stderr, "load_sp4_rgb_bitm: error opening file \"%s\"\n", (char *) filename);
		return -1;
	}

	if(read_sp4(fp, &rgba_data, &width, &height, screen_time, &frames_num) == -1)
	{
		fclose(fp);
		if(rgba_data != NULL) free(rgba_data);
		fprintf(stderr, "load_sp4_bitm: error reading file \"%s\"\n", (char *) filename);
		return -1;	
	}
	fclose(fp);

	uint32_t rgba_data_length = width * height * RGBA_PIXEL_SIZE;
	uint32_t rgb_data_length = width * height * RGB_PIXEL_SIZE;

	if((rgb_data = (char *) malloc(rgb_data_length)) == NULL)
	{
		fprintf(stderr, "load_sp4_rgb_bitm: failed to allocate rgb memory for file \"%s\"\n", filename);
		free(rgba_data);
		return -1;		
	}

	uint32_t rgba_offset = 0;
	uint32_t rgb_offset = 0;

	while(rgba_offset < rgba_data_length) {
		memcpy(&rgb_data[rgb_offset], &rgba_data[rgba_offset], RGB_PIXEL_SIZE);
		rgba_offset += RGBA_PIXEL_SIZE;
		rgb_offset += RGB_PIXEL_SIZE;
	}

	free(rgba_data);

	if(bitmap->exists()) bitmap->erase();

	bitmap->data_ = rgb_data;
	bitmap->width_ = width;
	bitmap->height_ = height;
	bitmap->raw_data_length_ = rgb_data_length;

	return 0;
}

//
//	SAVE_SP4_RGB_BITM
//	saves RGB_Bitmap as 1-frame SP4, ALPHA set to 100
//	returns 0 in SUCCESS, -1 on FAILURE
//
int save_sp4_rgb_bitm(const char * filename, RGB_bitmap * bitmap)
{
	if(!bitmap->exists()) return -1;

	RGBA_bitmap temp;
	if(rgb_to_rgba(&temp, bitmap) == -1) return -1;

	int result = save_sp4_rgba_bitm(filename, &temp);
	temp.erase();
	return result;
}


//
//		RGBA_SPRITE
//

//
//	SAVE_SP4_SPRITE
//	returns 0 on SUCCESS, -1 on FAILURE
//
int save_sp4_sprite(const char *filename, RGBA_sprite * spr)
{
	if(!spr->exists()) return -1;
	
	FILE *fp;
	if((fp = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "save_sp4_sprite: error opening file \"%s\"\n", filename);
		return -1;
	}

	fwrite(__SP4_MARKER, 1, 2, fp);						// marker
	fwrite(&(spr->width_), 2, 1, fp);					// width
	fwrite(&(spr->height_), 2, 1, fp);					// height
	fwrite(&(spr->frames_num_), 1, 1, fp); 				// number of frames
	fwrite(spr->screen_time, 1, spr->frames_num_, fp);	// screen time table (1 byte per frame)
	for(int i=0; i<spr->frames_num_; ++i) {
		fwrite((const void *) spr->frames[i], 1, spr->frame_data_length, fp);
	}
	fclose(fp);

	return 0;
}


//
//	LOAD_SP4_SPRITE
//	expects fully unallocated RGBA_Sprite
//	returns 0 on SUCCESS, -1 on FAILURE
//
int load_sp4_sprite(const char * filename, RGBA_sprite * spr)
{
	FILE * fp;

	if ((fp = fopen(filename,"rb")) == NULL) {
		fprintf(stderr, "load_sp4_sprite: error opening file \"%s\"\n", filename); 
		return -1;
	}
	
	char *		data = NULL;
	uint16_t	width = 0,
				height = 0;
	uint8_t		screen_time[UINT8_MAX] = { 0 };
	uint8_t 	frames_num = 0;

	if(read_sp4(fp, &data, &width, &height, screen_time, &frames_num) == -1)
	{
		fclose(fp);
		fprintf(stderr, "load_sp4_rgba_bitm: error reading file \"%s\"\n", filename);
		return -1;	
	}
	fclose(fp);
	
	if(spr->exists()) spr->erase();
	if(spr->create(frames_num, width, height) == -1) 
	{
		free(data);
		fprintf(stderr, "load_sp4_sprite: failed to create sprite\n");
		return -1;
	}

	memcpy(spr->screen_time, screen_time, frames_num);

	uint32_t frame_offset = 0;
	for(int i=0; i<frames_num; ++i) {
		memcpy(spr->frames[i], &data[frame_offset], spr->frame_data_length);
		frame_offset += spr->frame_data_length;
	}
	free(data);

	spr->default_screen_times_ = false;
	for(int i=0; i<frames_num; ++i) 
		if(spr->screen_time[i] == 0) {
			spr->default_screen_times_ = true;
			break;
		}

	return 0;
}


/*	---------------------------------------------------------------
 *
 *						LOAD AND SAVE PPM
 *
 *	--------------------------------------------------------------- */




int save_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap)
{
	if(!bitmap->exists()) return -1;

	if(save_ppm3(filename, (unsigned char *) bitmap->data_, bitmap->width_, bitmap->height_) == -1)
	{
		fprintf(stderr, "save_ppm_rgb_bitm: error writing file %s\n", filename);
		return -1;
	}
	return 0;
}


int load_ppm_rgb_bitm(const char *filename, RGB_bitmap * bitmap)
{
	if(bitmap->exists()) bitmap->erase();

	bitmap->data_ = (char *) read_ppm3(filename, (int *) &(bitmap->width_), (int *) &(bitmap->height_));
	if(bitmap->data_ == NULL) {
		fprintf(stderr, "load_ppm_rgb_bitm: error reading file %s\n", filename);
		bitmap->erase();
		return -1;
	}
	bitmap->raw_data_length_ = bitmap->width_ * bitmap->height_ * RGB_PIXEL_SIZE;
	return 0;
}


int save_ppm_rgba_bitm(const char *filename, RGBA_bitmap * bitmap)
{
	if(!bitmap->exists()) return -1;

	RGB_bitmap rgb_bitm;
	if(rgba_to_rgb(&rgb_bitm, bitmap) == -1) {
		fprintf(stderr, "save_ppm_rgba_bitm: error converting to rgb bitmap%s\n", filename);
		return -1;
	}

	int result = save_ppm_rgb_bitm(filename, &rgb_bitm);
	rgb_bitm.erase();
	return result;	
}


int load_ppm_rgba_bitm(const char *filename, RGBA_bitmap * bitmap)
{
	if(bitmap->exists()) bitmap->erase();

	RGB_bitmap rgb_bitm;
	if(load_ppm_rgb_bitm(filename, &rgb_bitm) == -1) {
		return -1;
	}

	if(rgb_to_rgba(bitmap, &rgb_bitm) == -1) {
		fprintf(stderr, "load_ppm_rgba_bitm: error converting to rgb bitmap%s\n", filename);
		return -1;
	}
	rgb_bitm.erase();
	return 0;
}


/*	---------------------------------------------------------------
 *
 *							PLOTTING
 *
 *	--------------------------------------------------------------- */

/*
 *	ALPHA management while plotting (if dst is RGBA):
 *
 *	PLOT_FLAG__DST_ALPHA_0x64 (default)
 *		for every plotted pixel alpha set to 0x64;
 *		set: plot_bitmap__dst_alpha_0x64()
 *
 *	PLOT_FLAG__USE_SRC_ALPHA
 *		for every plotted pixel alpha copied from src pixel (if RGBA)
 *		or override_alpha value used (as 1-100 uint) if src pixel is RGB 
 *		and override_alpha != -1; otherwise dst alpha is preserved;
 *		set: plot_bitmap__use_src_alpha()
 *
 *	PLOT_FLAG__USE_DST_ALPHA
 *		dst alpha is preserverd for all pixels;
 *		set: plot_bitmap__use_dst_alpha()
 */
/*
#define PLOT_FLAG__USE_SRC_ALPHA	0x0001
#define PLOT_FLAG__USE_DST_ALPHA	0x0002
#define PLOT_FLAG__DST_ALPHA_0x64	0x0004

static uint32_t	plotting_flags = PLOT_FLAG__DST_ALPHA_0x64;

void plot_bitmap__use_src_alpha(void)
{
	plotting_flags &= ~(PLOT_FLAG__USE_DST_ALPHA | PLOT_FLAG__DST_ALPHA_0x64);
	plotting_flags |= PLOT_FLAG__USE_SRC_ALPHA;
}

void plot_bitmap__use_dst_alpha(void)
{
	plotting_flags &= ~(PLOT_FLAG__USE_SRC_ALPHA | PLOT_FLAG__DST_ALPHA_0x64);
	plotting_flags |= PLOT_FLAG__USE_DST_ALPHA;
}

void plot_bitmap__dst_alpha_0x64(void)
{
	plotting_flags &= ~(PLOT_FLAG__USE_SRC_ALPHA | PLOT_FLAG__USE_DST_ALPHA);
	plotting_flags |= PLOT_FLAG__DST_ALPHA_0x64;
}*/


#define RGB_PIXEL_SIZE 		3
#define RGBA_PIXEL_SIZE 	4

#define RED					0
#define GREEN				1
#define BLUE				2
#define ALPHA				3		

/*	
 *	GENERIC PLOT BITMAP
 *	plot whole src bitmap into dst with clipping
 *	 	alpha application
 *	 	Resulting_R = (Alpha * Top_R) + ((1 - Alpha) * Bottom_R)
 *	 	Resulting_G = (Alpha * Top_G) + ((1 - Alpha) * Bottom_G)
 *	 	Resulting_B = (Alpha * Top_B) + ((1 - Alpha) * Bottom_B)		
 *  																*/
static int plot_bitmap_w_memcpy(uint8_t * 	dst,
								uint8_t * 	src,
					   			int16_t 	x,
					   			int16_t 	y,				/* top-left x, y within dst */
					   			uint8_t 	dst_step,
					   			uint8_t 	src_step,		/* size of 1 pixel (RGB = 3, RGBA = 4 bytes) */
					   			uint16_t 	dst_width,
					   			uint16_t 	dst_height,	
					   			uint16_t 	src_width,
					   			uint16_t 	src_height,
					   			float 		override_alpha)
{
	// safety check done by wrapper routines
	
	// values after clipping
	int16_t		dst_eff_x 	= x,
				src_eff_x	= 0,
				dst_eff_y 	= y,
				src_eff_y 	= 0,
				src_eff_w 	= src_width,
				src_eff_h 	= src_height;

	// calculate clipping
	// src starts off left edege
	if(x < 0) 
	{
		if(x + src_width <= 0) {
			fprintf(stderr, "plot_bitmap: source off destination leftwards\n"); 
			return -1;
		} 
		else {
			dst_eff_x = 0;
			src_eff_w = src_width - abs(x);
			src_eff_x = src_width - src_eff_w;
		}
	} 
	// src ends off right edege
	else if(x + src_width > dst_width) 
	{
		if(x >= dst_width) {
			fprintf(stderr, "plot_bitmap: source off destination rightwards\n"); 
			return -1;
		} 
		else {
			dst_eff_x = x;
			src_eff_w = dst_width - x;
			src_eff_x = 0;
		}
	}
	// src starts above top edge
	if(y < 0) 
	{
		if(y + src_height <= 0) {
			fprintf(stderr, "plot_bitmap: source off destination upwards\n"); 
			return -1;
		} 
		else {
			dst_eff_y = 0;
			src_eff_h = src_height - abs(y);
			src_eff_y = src_height - src_eff_h;
		}
	}
	// src ends below bottom edge
	else if(y + src_height > dst_height)
	{
		if(y >= dst_height) {
			fprintf(stderr, "plot_bitmap: source off destination downwards\n"); 
			return -1;
		} 
		else {
			dst_eff_y = y;
			src_eff_h = dst_height - y;
			src_eff_y = 0;
		}
	}

	uint8_t *	src_pixel;
	uint8_t *	old_pixel;
	uint8_t 	dst_pixel[] = { 0x00, 0x00, 0x00, 0x64 };

	uint32_t 	base_src_offset = src_step * (src_eff_x + (src_eff_y * src_width));
	uint32_t 	base_dst_offset = dst_step * (dst_eff_x + (dst_eff_y * dst_width));

	uint32_t 	src_byte_row = src_width * src_step;
	uint32_t 	dst_byte_row = dst_width * dst_step;

	uint32_t 	src_offset = base_src_offset;
	uint32_t 	dst_offset = base_dst_offset;

	float 		alpha = 1.0;

	/*uint8_t		int_override_alpha;
	if(override_alpha != -1) int_override_alpha = (uint8_t) (override_alpha * 100);*/

	for(int i = 0; i < src_eff_h; ++i, src_offset = (base_src_offset += src_byte_row), dst_offset = (base_dst_offset += dst_byte_row))
		for(int j = 0; j < src_eff_w; ++j, src_offset += src_step, dst_offset += dst_step)
		{
			src_pixel = &src[src_offset];	

			if(src_step == RGBA_PIXEL_SIZE) 
			{
				if(src_pixel[ALPHA] == 0) continue; 	
				if(src_pixel[ALPHA] > 99) alpha = 1.0;
				else 					  alpha = (float) src_pixel[ALPHA] * 0.01f;
			}			

			if(override_alpha != -1.0) alpha = override_alpha;

			if(alpha != 1.0)
			{
				old_pixel = &dst[dst_offset];			

				dst_pixel[RED] 	 = (uint8_t) (src_pixel[RED] * alpha) + (old_pixel[RED] * (1 - alpha));
				dst_pixel[GREEN] = (uint8_t) (src_pixel[GREEN] * alpha) + (old_pixel[GREEN] * (1 - alpha));
				dst_pixel[BLUE]  = (uint8_t) (src_pixel[BLUE] * alpha) + (old_pixel[BLUE] * (1 - alpha));
			}
			else {
				dst_pixel[RED] 	 = src_pixel[RED];
				dst_pixel[GREEN] = src_pixel[GREEN];
				dst_pixel[BLUE]  = src_pixel[BLUE];
			}
			/*
			if(plotting_flags & PLOT_FLAG__USE_SRC_ALPHA) {
				if(src_step == RGBA_PIXEL_SIZE) dst_pixel[ALPHA] = (uint8_t) src_pixel[ALPHA];
				else if(override_alpha != -1.0)	dst_pixel[ALPHA] = int_override_alpha;
			} */
			/*
			if(plotting_flags & PLOT_FLAG__USE_DST_ALPHA) memcpy(&dst[dst_offset], dst_pixel, RGB_PIXEL_SIZE);
			else */										  
			memcpy(&dst[dst_offset], dst_pixel, dst_step);
		}
	return 0;
}


/*
 *	test routine writing directly to memory, no memcpy
 */
static int plot_bitmap(				uint8_t * 	dst,
									uint8_t * 	src,
						   			int16_t 	x,
						   			int16_t 	y,						/* top-left x, y within dst */
						   			uint8_t 	dst_step,
						   			uint8_t 	src_step,				/* size of 1 pixel (RGB = 3, RGBA = 4 bytes) */
						   			uint16_t	dst_width,
						   			uint16_t 	dst_height,	
						   			uint16_t 	src_width,
						   			uint16_t 	src_height,
						   			float 		override_alpha = -1)	/* use given alpha value instead src_pixel[ALPHA]; 
						   												   doesn't change RGBA pixels with alpha = 0 */
{
	/* alpha can't be negative, -1 = off
	 * other safety check done by wrapper routines */
	if(override_alpha != -1.0 && override_alpha <= 0) override_alpha = -1;
	
	// values after clipping
	int16_t		dst_eff_x 	= x,
				src_eff_x	= 0,
				dst_eff_y 	= y,
				src_eff_y 	= 0,
				src_eff_w 	= src_width,
				src_eff_h 	= src_height;

	// calculate clipping
	// src starts off left edege
	if(x < 0) 
	{
		if(x + src_width <= 0) {
			fprintf(stderr, "plot_bitmap: source off destination leftwards\n"); 
			return -1;
		} 
		else {
			dst_eff_x = 0;
			src_eff_w = src_width - abs(x);
			src_eff_x = src_width - src_eff_w;
		}
	} 
	// src ends off right edege
	else if(x + src_width > dst_width) 
	{
		if(x >= dst_width) {
			fprintf(stderr, "plot_bitmap: source off destination rightwards\n"); 
			return -1;
		} 
		else {
			dst_eff_x = x;
			src_eff_w = dst_width - x;
			src_eff_x = 0;
		}
	}
	// src starts above top edge
	if(y < 0) 
	{
		if(y + src_height <= 0) {
			fprintf(stderr, "plot_bitmap: source off destination upwards\n"); 
			return -1;
		} 
		else {
			dst_eff_y = 0;
			src_eff_h = src_height - abs(y);
			src_eff_y = src_height - src_eff_h;
		}
	}
	// src ends below bottom edge
	else if(y + src_height > dst_height)
	{
		if(y >= dst_height) {
			fprintf(stderr, "plot_bitmap: source off destination downwards\n"); 
			return -1;
		} 
		else {
			dst_eff_y = y;
			src_eff_h = dst_height - y;
			src_eff_y = 0;
		}
	}

	uint8_t *	src_pixel;
	uint8_t *	dst_pixel;

	uint32_t 	base_src_offset = src_step * (src_eff_x + (src_eff_y * src_width));
	uint32_t 	base_dst_offset = dst_step * (dst_eff_x + (dst_eff_y * dst_width));

	uint32_t 	src_byte_row = src_width * src_step;
	uint32_t 	dst_byte_row = dst_width * dst_step;

	uint32_t 	src_offset = base_src_offset;
	uint32_t 	dst_offset = base_dst_offset;

	float 		alpha = 1.0;

	if(override_alpha != -1.0) alpha = override_alpha; 

	const float alpha_reference_table[128] = {
		0.000000, 
		0.010000, 0.020000, 0.030000, 0.040000, 0.050000, 0.060000, 0.070000, 0.080000, 
		0.090000, 0.100000, 0.110000, 0.120000, 0.130000, 0.140000, 0.150000, 0.160000, 
		0.170000, 0.180000, 0.190000, 0.200000, 0.210000, 0.220000, 0.230000, 0.240000, 
		0.250000, 0.260000, 0.270000, 0.280000, 0.290000, 0.300000, 0.310000, 0.320000, 
		0.330000, 0.340000, 0.350000, 0.360000, 0.370000, 0.380000, 0.390000, 0.400000, 
		0.410000, 0.420000, 0.430000, 0.440000, 0.450000, 0.460000, 0.470000, 0.480000, 
		0.490000, 0.500000, 0.510000, 0.520000, 0.530000, 0.540000, 0.550000, 0.560000, 
		0.570000, 0.580000, 0.590000, 0.600000, 0.610000, 0.620000, 0.630000, 0.640000, 
		0.650000, 0.660000, 0.670000, 0.680000, 0.690000, 0.700000, 0.710000, 0.720000, 
		0.730000, 0.740000, 0.750000, 0.760000, 0.770000, 0.780000, 0.790000, 0.800000, 
		0.810000, 0.820000, 0.830000, 0.840000, 0.850000, 0.860000, 0.870000, 0.880000, 
		0.890000, 0.900000, 0.910000, 0.920000, 0.930000, 0.940000, 0.950000, 0.960000, 
		0.970000, 0.980000, 0.990000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
		1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
		1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 
		1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000
	};

	for(int i = 0;
		i < src_eff_h;
		++i, src_offset = (base_src_offset += src_byte_row), dst_offset = (base_dst_offset += dst_byte_row))
	{
		for(int j = 0;
			j < src_eff_w;
			++j, src_offset += src_step, dst_offset += dst_step)
		{
			src_pixel = &src[src_offset];	
			dst_pixel = &dst[dst_offset];

			if(src_step == RGBA_PIXEL_SIZE) 
			{
				if(src_pixel[ALPHA] == 0) 		continue; 				
				if(override_alpha != -1.0) 		alpha = override_alpha;
				else							alpha = alpha_reference_table[src_pixel[ALPHA] & 0x7F]; // chop off most significant bit
			}			

			if(alpha == 1.0)
			{
				dst_pixel[RED] 	 = src_pixel[RED];
				dst_pixel[GREEN] = src_pixel[GREEN];
				dst_pixel[BLUE]  = src_pixel[BLUE];
			}
			else {
				dst_pixel[RED] 	 = (uint8_t) (src_pixel[RED] * alpha) 	+ (dst_pixel[RED] * (1 - alpha));
				dst_pixel[GREEN] = (uint8_t) (src_pixel[GREEN] * alpha) + (dst_pixel[GREEN] * (1 - alpha));
				dst_pixel[BLUE]  = (uint8_t) (src_pixel[BLUE] * alpha) 	+ (dst_pixel[BLUE] * (1 - alpha));
			}	
			if(dst_step == RGBA_PIXEL_SIZE) dst_pixel[ALPHA] = 0x64;
		}
	}
	// end of for loops
	return 0;
}


/*	---------------------------------------------------------------
 *
 *							PLOT SPRITE
 *
 *	--------------------------------------------------------------- */

/*
static int plot_bitmap(uint8_t * dst, 		uint8_t * src,
					   int16_t x, 			int16_t y,
					   uint8_t dst_step,	uint8_t src_step,
					   uint16_t dst_width, uint16_t dst_height,	
					   uint16_t src_width, uint16_t src_height,
					   float override_alpha)*/

//	PLOT SPRITE ON RGB
//	uses current_frame
//	clipping, fixed alpha
//
int plot_sprite(RGB_bitmap *dst, RGBA_sprite *src, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_sprite: source uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_sprite: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			printf("plot_sprite: alpha=%f <= 0, nothing to plot\n", alpha);
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0)	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->current_frame_data(),
					   src->x(), src->y(),
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}


//	PLOT SPRITE ON RGBA
//	uses current_frame
//	clipping, fixed alpha
//
int plot_sprite(RGBA_bitmap *dst, RGBA_sprite *src, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_sprite: source uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_sprite: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			printf("plot_sprite: alpha=%f <= 0, nothing to plot\n", alpha);
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0)	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->current_frame_data(),
					   src->x(), src->y(),
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}


//	PLOT SPRITE ON SPRITE
//	uses current_frame
//	clipping, fixed alpha
//
int plot_sprite(RGBA_sprite *dst, RGBA_sprite *src, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_sprite: source uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_sprite: destination uninitialised\n");
			error_escape = true;
		}
		if(src == dst) {
			fprintf(stderr, "plot_bitmap: can't plot onto itself\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			printf("plot_sprite: alpha=%f <= 0, nothing to plot\n", alpha);
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0)	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->current_frame_data(), (uint8_t*) src->current_frame_data(),
					   src->x(), src->y(),
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}

/*	---------------------------------------------------------------
 *
 *							PLOT BITMAP
 *
 *	--------------------------------------------------------------- */


//		PLOT RGBA on RGBA
//		meaningful alpha - alpha values: 0-100 (0x00-0x64), values >100 truncated to 100
// 		preserves dst alpha   
//         
int plot_bitmap(RGBA_bitmap *dst, RGBA_bitmap *src, int x, int y)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source data uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(src == dst) {
			fprintf(stderr, "plot_bitmap: can't plot onto itself\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   -1.0);
}


//	------------------------------------------------------------------------	
//		PLOT RGB on RGBA
//		fixed alpha
// 		preserves dst alpha   
//         
int plot_bitmap(RGBA_bitmap *dst, RGB_bitmap *src, int x, int y, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source data uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			fprintf(stderr, "plot_bitmap: alpha = 0, nothing to plot\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0)	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}


//	------------------------------------------------------------------------	
//		PLOT RGBA on RGB
//		meaningful alpha, alpha values: 0-100 (0x00-0x64), values >100 truncated to 100              
//
int plot_bitmap(RGB_bitmap *dst, RGBA_bitmap *src, int x, int y)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source data uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   -1.0);
}


//	------------------------------------------------------------------------	
//		PLOT RGB ON RGB
//		single alpha channel (0-1) for all pixels
//		
//
int plot_bitmap(RGB_bitmap *dst, RGB_bitmap *src, int x, int y, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source uninitialised\n");
			error_escape = true;
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(src == dst) {
			fprintf(stderr, "plot_bitmap: can't plot onto itself\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			fprintf(stderr, "plot_bitmap: alpha = 0, nothing to plot\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0) 	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}


//	------------------------------------------------------------------------	
//		PLOT RGBA BITMAP ON SPRITE
//		uses sprite's  current_frame
//		uses bitmap's meaningful alpha, alpha values: 0-100 (0x00-0x64), values >100 truncated to 100              
// 		sprite's alpha remains unchanged
//
int plot_bitmap(RGBA_sprite *dst, RGBA_bitmap *src, int x, int y)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	return plot_bitmap((uint8_t*) dst->current_frame_data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   -1.0);
}



//	------------------------------------------------------------------------	
//		PLOT RGB BITMAP ON SPRITE
//		uses sprite's  current_frame
//		uses single alpha for all pixels, alpha values: 0-1.0, values >1.0 truncated to 1.0              
// 		sprite's alpha remains unchanged
//
int plot_bitmap(RGBA_sprite *dst, RGB_bitmap *src, int x, int y, float alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "plot_bitmap: source uninitialised\n");
			error_escape = true;			
		}
		if(!dst->exists()) {
			fprintf(stderr, "plot_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha <= 0) {
			fprintf(stderr, "plot_bitmap: alpha = 0, nothing to plot\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}
	
	if(alpha > 1.0) alpha = 1.0;
	if(alpha < 0)	alpha = -1.0;

	return plot_bitmap((uint8_t*) dst->current_frame_data(), (uint8_t*) src->data(),
					   x, y,
					   dst->pixel_size(), src->pixel_size(),
					   dst->width(), dst->height(),	
					   src->width(), src->height(),
					   alpha);
}


/*	---------------------------------------------------------------
 *
 *							  QUICK COPY
 *
 *	--------------------------------------------------------------- */



int quick_copy(RGB_bitmap *dst, RGB_bitmap *src, int dst_x, int dst_y, int src_x, int src_y, int width, int height)
{
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "quick_copy: source not initialised\n");
	 		error_escape = true;
		}
		if(!dst->exists()) {
			fprintf(stderr, "quick_copy: destination not initialised\n");
	 		error_escape = true;
		}
		if(src == dst) {
			fprintf(stderr, "quick_copy: can't copy to itself\n");
	 		error_escape = true;
		}
		if(src->width() - src_x < width) {
			fprintf(stderr, "quick_copy: block too wide for source\n");
	 		error_escape = true;
		}
		if(src->height() - src_y < height) {
			fprintf(stderr, "quick_copy: block too high for source\n");
	 		error_escape = true;
		}	
		if(dst->width() - dst_x < width) {
			fprintf(stderr, "quick_copy: block too wide for destination\n");
	 		error_escape = true;
		}
		if(dst->height() - dst_y < height) {
			fprintf(stderr, "quick_copy: block too high for destination\n");
	 		error_escape = true;
		}
		if(error_escape) return -1;
	}

	uint32_t dst_offset = (dst_y * dst->width() * RGB_PIXEL_SIZE) + (dst_x * RGB_PIXEL_SIZE);
	uint32_t src_offset = (src_y * src->width() * RGB_PIXEL_SIZE) + (src_x * RGB_PIXEL_SIZE);

	uint32_t copy_width_bytes = width * RGB_PIXEL_SIZE;

	char * src_data = src->data();
	char * dst_data = dst->data();

	for(int i=0; i<height; ++i) 
	{
		memcpy(&dst_data[dst_offset], &src_data[src_offset], copy_width_bytes);
		dst_offset += (dst->width() * RGB_PIXEL_SIZE);
		src_offset += (src->width() * RGB_PIXEL_SIZE);
	}

	return 0;
}



int quick_copy(RGBA_bitmap *dst, RGBA_bitmap *src, int dst_x, int dst_y, int src_x, int src_y, int width, int height)
{
	{
		bool error_escape = false;
		if(!src->exists()) {
			fprintf(stderr, "quick_copy: source not initialised\n");
	 		error_escape = true;
		}
		if(!dst->exists()) {
			fprintf(stderr, "quick_copy: destination not initialised\n");
	 		error_escape = true;
		}
		if(src == dst) {
			fprintf(stderr, "quick_copy: can't copy to itself\n");
	 		error_escape = true;
		}
		if(src->width() - src_x < width) {
			fprintf(stderr, "quick_copy: block too wide for source\n");
	 		error_escape = true;
		}
		if(src->height() - src_y < height) {
			fprintf(stderr, "quick_copy: block too high for source\n");
	 		error_escape = true;
		}	
		if(dst->width() - dst_x < width) {
			fprintf(stderr, "quick_copy: block too wide for destination\n");
	 		error_escape = true;
		}
		if(dst->height() - dst_y < height) {
			fprintf(stderr, "quick_copy: block too high for destination\n");
	 		error_escape = true;
		}
		if(error_escape) return -1;
	}


	uint32_t dst_offset = (dst_y * dst->width() * RGBA_PIXEL_SIZE) + (dst_x * RGBA_PIXEL_SIZE);
	uint32_t src_offset = (src_y * src->width() * RGBA_PIXEL_SIZE) + (src_x * RGBA_PIXEL_SIZE);
	uint32_t copy_width_bytes = width * RGBA_PIXEL_SIZE;

	char * src_data = src->data();
	char * dst_data = dst->data();

	for(int i=0; i<height; ++i) 
	{
		memcpy(&dst_data[dst_offset], &src_data[src_offset], copy_width_bytes);
		dst_offset += (dst->width() * RGBA_PIXEL_SIZE);
		src_offset += (src->width() * RGBA_PIXEL_SIZE);
	}

	return 0;
}

/*	---------------------------------------------------------------
 *
 *							  MOVE DATA
 *
 *	--------------------------------------------------------------- */


int move_bitmap_data(RGB_bitmap *dst, RGB_bitmap *src)
{
	if(src == dst) {
		fprintf(stderr, "move_bitmap_data: can't move to itself\n");
		return -1;
	}
	if(!src->exists()) {
		fprintf(stderr, "move_bitmap_data: source not initialised\n");
		return -1;
	}

	if(dst->exists()) dst->erase();

	dst->data_ = src->data_;
	dst->width_ = src->width_;
	dst->height_ = src->height_;
	dst->raw_data_length_ = src->raw_data_length_;
	
	src->data_ = nullptr;
	src->erase();

	return 0;
}


int move_bitmap_data(RGBA_bitmap *dst, RGBA_bitmap *src)
{
	if(src == dst) {
		fprintf(stderr, "move_bitmap_data: can't move to itself\n");
		return -1;
	}
	if(!src->exists()) {
		fprintf(stderr, "move_bitmap_data: source not initialised\n");
		return -1;
	}

	if(dst->exists()) dst->erase();

	dst->data_ = src->data_;
	dst->width_ = src->width_;
	dst->height_ = src->height_;
	dst->raw_data_length_ = src->raw_data_length_;
	dst->flag_meaningful_alpha = src->flag_meaningful_alpha;
	
	src->data_ = nullptr;
	src->erase();

	return 0;
}


/*	---------------------------------------------------------------
 *
 *								COPY
 *
 *	--------------------------------------------------------------- */

int copy_bitmap(RGB_bitmap *dst, RGB_bitmap *src)
{
	if(src == dst) {
		fprintf(stderr, "copy_bitmap: can't copy to itself\n");
		return -1;
	}
	if(!src->exists()) {
		fprintf(stderr, "copy_bitmap: source not initialised\n");
		return -1;
	}
	if(dst->exists()) dst->erase();

	if(dst->create(src->width(), src->height()) == -1) {
		fprintf(stderr, "copy_bitmap: failed to create new bitmap\n");
		return -1;
	}

	memcpy(dst->data(), src->data(), src->raw_data_length());
	return 0;
}


int copy_bitmap(RGBA_bitmap *dst, RGBA_bitmap *src)
{
	if(src == dst) {
		fprintf(stderr, "copy_bitmap: can't copy to itself\n");
		return -1;
	}
	if(!src->exists()) {
		fprintf(stderr, "copy_bitmap: source not initialised\n");
		return -1;
	}
	if(dst->exists()) dst->erase();

	if(dst->create(src->width(), src->height()) == -1) {
		fprintf(stderr, "copy_bitmap: failed to create new bitmap\n");
		return -1;
	}

	memcpy(dst->data(), src->data(), src->raw_data_length());
	return 0;
}


/*	---------------------------------------------------------------
 *
 *								SCALE
 *
 *	--------------------------------------------------------------- */


static int generic_scale_bitmap(uint8_t * out, uint8_t *in, float scale, uint8_t step,
								uint16_t out_width, uint16_t out_height,
								uint16_t in_width, uint16_t in_height )
{
	if(scale == 1) return -1;

	//
	//uint16_t int_scale = scale * 100;
	
	if(scale < 1) 
	{
		for(unsigned int y = 0; y < out_height; ++y)
			for(unsigned int x = 0; x < out_width; ++x) 
			{
				// Calculate corresponding position in the original bitmap
				int original_x = (int) (x / scale);
				int original_y = (int) (y / scale);
				//int original_x = (x * 100) / int_scale
				//int original_y = (y * 100) / int_scale;

				uint32_t in_offset = (original_x + (original_y * in_height)) * step;
				uint32_t out_offset = (x + (y * out_height)) * step;

				memcpy(&out[out_offset], &in[in_offset], step);
			}
		return 0;
	} 

	//
	// scale > 1
	//

	int orig_width = in_width;
	int orig_height = in_height;

	for(int y = 0; y < out_height; ++y) {
		for(int x = 0; x < out_width; ++x) 
		{
			// Calculate coordinates in the original bitmap
			float original_x = x / scale;
			float original_y = y / scale;

			// Get the four nearest pixels
			int x1 = (int) original_x;
			int y1 = (int) original_y;
			int x2 = ((x1 + 1) < (orig_width - 1) ? (x1 + 1) : (orig_width - 1));
			int y2 = ((y1 + 1) < (orig_height - 1) ? (y1 + 1) : (orig_height - 1));

			// Bilinear interpolation formula
			float dx = original_x - x1;
			float dy = original_y - y1;

			uint8_t * 	ptr00 = (uint8_t *) &in[(x1 +(y1 * in_width)) * step];
			uint8_t * 	ptr01 = (uint8_t *) &in[(x1 +(y2 * in_width)) * step];
			uint8_t * 	ptr10 = (uint8_t *) &in[(x2 +(y1 * in_width)) * step];
			uint8_t * 	ptr11 = (uint8_t *) &in[(x2 +(y2 * in_width)) * step];

			uint8_t    out_pixel[RGBA_PIXEL_SIZE];
			
			for(int i = 0; i < RGB_PIXEL_SIZE; ++i) // 3 channels (RGB)
			{ 
				out_pixel[i] = (uint8_t) (1 - dx) * (1 - dy) * 	ptr00[i] + 	// 00
										 dx * 		(1 - dy) * 	ptr10[i] +	// 10
										 (1 - dx) * dy * 		ptr01[i] +	// 01
										 dx * 		dy * 		ptr11[i];	// 11
			}

			if(step == RGBA_PIXEL_SIZE) out_pixel[3] = ptr00[3]; // use ALPHA of 00

			memcpy(&out[(x + (y * out_width)) * step], out_pixel, step);
		}
	}
	return 0;
}


int scale_bitmap(RGB_bitmap *out, RGB_bitmap *in, float scale)
{
	if(in == out) {
		fprintf(stderr, "scale_bitmap: in and out bitmaps can't be one\n");
		return -1;
	}
	if(!in->exists()) {
		fprintf(stderr, "scale_bitmap: in bitmap uninitialised\n");
		return -1;
	}
	if(out->exists()) out->erase();
	
	if(scale == 1) return copy_bitmap(out, in);

	int out_width = (float) in->width() * scale;
	int out_height = (float) in->height() * scale;

	if(out->create(out_width, out_height) == -1) {
		fprintf(stderr, "scale_bitmap: failed to create out bitmap\n");
		return -1;		
	}

	return generic_scale_bitmap((uint8_t*) out->data(),
								(uint8_t*) in->data(), 
								scale,
								RGB_PIXEL_SIZE,
								out_width, out_height,
								(uint16_t) in->width(), (uint16_t) in->height());
}


int scale_bitmap(RGBA_bitmap *out, RGBA_bitmap *in, float scale)
{
	if(in == out) {
		fprintf(stderr, "scale_bitmap: in and out bitmaps can't be one\n");
		return -1;
	}
	if(!in->exists()) {
		fprintf(stderr, "scale_bitmap: in bitmap uninitialised\n");
		return -1;
	}
	if(out->exists()) out->erase();
	
	if(scale == 1) return copy_bitmap(out, in);

	int out_width = (float) in->width() * scale;
	int out_height = (float) in->height() * scale;

	if(out->create(out_width, out_height) == -1) {
		fprintf(stderr, "scale_bitmap: failed to create out bitmap\n");
		return -1;		
	}

	return generic_scale_bitmap((uint8_t*) out->data(),
								(uint8_t*) in->data(), 
								scale,
								RGBA_PIXEL_SIZE,
								out_width, out_height,
								(uint16_t) in->width(), (uint16_t) in->height());
}

/*int scale_bitmap(RGB_bitmap *out, RGB_bitmap *in, float scale)
{
	if(in == out) {
		fprintf(stderr, "scale_bitmap: in and out bitmaps can't be one\n");
		return -1;
	}
	if(!in->exists()) {
		fprintf(stderr, "scale_bitmap: in bitmap uninitialised\n");
		return -1;
	}
	if(out->exists()) out->erase();

	if(scale == 1) {
		return copy_bitmap(out, in);
	}

	int out_width = (float) in->width() * scale;
	int out_height = (float) in->height() * scale;

	if(out->create(out_width, out_height) == -1) {
		fprintf(stderr, "scale_bitmap: failed to create new bitmap\n");
		return -1;
	}

	if(scale < 1) 
	{
		for(int y = 0; y < out_height; ++y) {
			for (int x = 0; x < out_width; ++x) {
				// Calculate corresponding position in the original bitmap
				int original_x = (int) (x / scale);
				int original_y = (int) (y / scale);

				RGB * pixel = in->get_pixel_ptr(original_x, original_y);
				if(pixel == nullptr) {
					fprintf(stderr, "scale_bitmap: error reading pixel\n");
					return -1;
				}
				out->put_pixel(x, y, *pixel);
			}
		}
	} 
	else 
	{
		int orig_width = in->width();
		int orig_height = in->height();

		for(int y = 0; y < out_height; ++y) {
			for(int x = 0; x < out_width; ++x) 
			{
				// Calculate coordinates in the original bitmap
				float original_x = x / scale;
				float original_y = y / scale;

				// Get the four nearest pixels
				int x1 = (int) original_x;
				int y1 = (int) original_y;
				int x2 = ((x1 + 1) < (orig_width - 1) ? (x1 + 1) : (orig_width - 1));
				int y2 = ((y1 + 1) < (orig_height - 1) ? (y1 + 1) : (orig_height - 1));

				// Bilinear interpolation formula
				float dx = original_x - x1;
				float dy = original_y - y1;

				uint8_t * 	ptr00 = (uint8_t *) in->get_pixel_ptr(x1, y1);
				uint8_t * 	ptr01 = (uint8_t *) in->get_pixel_ptr(x1, y2);
				uint8_t * 	ptr10 = (uint8_t *) in->get_pixel_ptr(x2, y1);
				uint8_t * 	ptr11 = (uint8_t *) in->get_pixel_ptr(x2, y2);

				if(ptr00 == nullptr || ptr01 == nullptr || ptr10 == nullptr || ptr11 == nullptr)
					return -1;

				RGB out_pixel;
				uint8_t *  out_ptr = (uint8_t *) &out_pixel;

				for (int i = 0; i < RGB_PIXEL_SIZE; ++i) // 3 channels (RGB)
				{ 
					float interpolated = (1 - dx) * (1 - dy) * 	ptr00[i] + 	// 00
										 dx * 		(1 - dy) * 	ptr10[i] +	// 10
										 (1 - dx) * dy * 		ptr01[i] +	// 01
										 dx * 		dy * 		ptr11[i];	// 11

					uint8_t val = (uint8_t) interpolated; 
					out_ptr[i] = val;
				}

				out->put_pixel(x, y, out_pixel);
			}
		}
	}
	return 0;
}*/

/*	---------------------------------------------------------------
 *
 *								DRAW
 *
 *	--------------------------------------------------------------- */


/*int draw_line(RGB_bitmap *dst, uint x1, uint y1, uint x2, uint y2, RGB color, LineAlgorithm alg )
{
	if(!dst->exists()) {
		fprintf(stderr, "draw_line: bitmap doesn't exist\n");
		return -1;
	} 
	else if(x1 > (uint) dst->width() ||
		    x2 > (uint) dst->width() ||
			y1 > (uint) dst->height() ||
			y2 > (uint) dst->height()) 
	{
		fprintf(stderr, "draw_line : line (%d,%d)(%d,%d) excedes bitmap's width(%d) or height (%d)\n",
				x1, y1, x2, y2, dst->width(), dst->height());
		return -1;
	}

	std::vector<Point2D> line_pixels;
	Line2D line = { (int) x1, (int) y1, (int) x2, (int) y2 };
	line.line_algorithm = alg;
	line.calculate_line_points(line_pixels);

	for(int i=0; i<(int)line_pixels.size(); ++i) {
		dst->put_pixel(line_pixels[i].x, line_pixels[i].y, color);
	}
	return 0;
}*/

/*	---------------------------------------------------------------
 *
 *								CONVERT
 *
 *	--------------------------------------------------------------- */


//
//	RGB_BITMAP_TO_RGBA
//	returns 0 on SUCCESS, -1 on FAILURE
//
int rgb_to_rgba(RGBA_bitmap *dst, RGB_bitmap *src, uint8_t alpha, bool transp, RGB transp_color)
{
	if(dst->exists()) dst->erase();
	
	if(!src->exists()) {
		fprintf(stderr, "rgb_to_rgba: source uninitialised\n");
		return -1;			
	}

	if(dst->create(src->width(), src->height()) == -1) {
		fprintf(stderr, "rgb_to_rgba: failed to create rgba bitmap\n");
		return -1;			
	}

	if(alpha > 100) alpha = 100;

	RGB * 		pixel;
	char * 		rgba_data			= dst->data();
	char * 		rgb_data 			= src->data();
	uint32_t 	rgba_offset			= 0;
	uint32_t 	rgb_offset 			= 0;

	while(rgb_offset < src->raw_data_length())
	{
		memcpy(&rgba_data[rgba_offset], &rgb_data[rgb_offset], RGB_PIXEL_SIZE);
		
		// catch transparency
		pixel = (RGB *) &(rgb_data[rgb_offset]);
		if(transp 	== true &&
		   pixel->r == transp_color.r &&
		   pixel->g == transp_color.g &&
		   pixel->b == transp_color.b)
		{
			memset(&rgba_data[rgba_offset+RGB_PIXEL_SIZE], 0 , 1);		// alpha set to 0
		} else {
			memcpy(&rgba_data[rgba_offset+RGB_PIXEL_SIZE], &alpha, 1);
		}

		rgba_offset += RGBA_PIXEL_SIZE;
		rgb_offset += RGB_PIXEL_SIZE;
	}

	dst->meaningful_alpha(true);
	return 0;
}


//
//	RGBA_BITMAP_TO_RGB
//	all transparency lost
//	returns 0 on SUCCESS, -1 on FAILURE
//
int rgba_to_rgb(RGB_bitmap * dst, RGBA_bitmap * src)
{
	if(dst->exists()) dst->erase();

	if(!src->exists()) {
		fprintf(stderr, "rgba_to_rgb: source uninitialised\n");
		return -1;			
	}

	if(dst->create(src->width(), src->height()) == -1) {
		fprintf(stderr, "rgba_to_rgb: failed to create rgb bitmap\n");
		return -1;			
	}
	
	char *		rgb_data 			= dst->data();
	char * 		rgba_data 			= src->data();
	uint32_t	rgb_data_length 	= src->width() * src->height() * RGB_PIXEL_SIZE;

	uint32_t 	rgba_offset 		= 0;
	uint32_t 	rgb_offset 			= 0;

	while(rgb_offset < rgb_data_length) 
	{
		memcpy(&rgb_data[rgb_offset], &rgba_data[rgba_offset], RGB_PIXEL_SIZE);
		rgba_offset += RGBA_PIXEL_SIZE;
		rgb_offset += RGB_PIXEL_SIZE;
	}

	return 0;
}


/*	---------------------------------------------------------------
 *
 *								FADE
 *
 *	--------------------------------------------------------------- */

static int fade_bitmap(uint8_t * 	dst, 
					   uint16_t 	dst_width,
					   uint16_t 	dst_height,	
					   uint8_t 		dst_step,		/* size of 1 pixel (RGB = 3, RGBA = 4 bytes) */
					   uint16_t 	alpha)			/* 0 - 100 */
{
	uint8_t *	pixel;

	uint32_t 	base_dst_offset = 0;
	uint32_t 	dst_byte_row = dst_width * dst_step;
	uint32_t 	dst_offset = base_dst_offset;

	float 		f_alpha = 1.0;
	float 		conversion_table[100] = {
		0.000000, 
		0.010000, 0.020000, 0.030000, 0.040000, 0.050000, 
		0.060000, 0.070000, 0.080000, 0.090000, 0.100000, 
		0.110000, 0.120000, 0.130000, 0.140000, 0.150000, 
		0.160000, 0.170000, 0.180000, 0.190000, 0.200000, 
		0.210000, 0.220000, 0.230000, 0.240000, 0.250000, 
		0.260000, 0.270000, 0.280000, 0.290000, 0.300000, 
		0.310000, 0.320000, 0.330000, 0.340000, 0.350000, 
		0.360000, 0.370000, 0.380000, 0.390000, 0.400000, 
		0.410000, 0.420000, 0.430000, 0.440000, 0.450000, 
		0.460000, 0.470000, 0.480000, 0.490000, 0.500000, 
		0.510000, 0.520000, 0.530000, 0.540000, 0.550000, 
		0.560000, 0.570000, 0.580000, 0.590000, 0.600000, 
		0.610000, 0.620000, 0.630000, 0.640000, 0.650000, 
		0.660000, 0.670000, 0.680000, 0.690000, 0.700000, 
		0.710000, 0.720000, 0.730000, 0.740000, 0.750000, 
		0.760000, 0.770000, 0.780000, 0.790000, 0.800000, 
		0.810000, 0.820000, 0.830000, 0.840000, 0.850000, 
		0.860000, 0.870000, 0.880000, 0.890000, 0.900000, 
		0.910000, 0.920000, 0.930000, 0.940000, 0.950000, 
		0.960000, 0.970000, 0.980000, 0.990000, 
	};

	if(alpha == 0) return -1; 	
	if(alpha > 99) f_alpha = 1.0;
	else 		   f_alpha = conversion_table[alpha];

	/*if(alpha > 99) alpha = 1.0;
	else 					  alpha = (float) src_pixel[ALPHA] * 0.01f;*/

	for(int i = 0; i < dst_height; ++i, dst_offset = (base_dst_offset += dst_byte_row))
		for(int j = 0; j < dst_width; ++j, dst_offset += dst_step)
		{
			pixel = &dst[dst_offset];	

			if(dst_step == RGBA_PIXEL_SIZE) {
				if(pixel[ALPHA] == 0x00) 	continue;
				else 						pixel[ALPHA] = 0x64;
			}

			if(f_alpha != 1.0) {
				pixel[RED] 	 = (uint8_t) pixel[RED] * (1.0 - f_alpha);
				pixel[GREEN] = (uint8_t) pixel[GREEN] * (1.0 - f_alpha);
				pixel[BLUE]  = (uint8_t) pixel[BLUE] * (1.0 - f_alpha);
			} else {
				pixel[RED] 	 = 0x00;
				pixel[GREEN] = 0x00;
				pixel[BLUE]  = 0x00;
			}
		}
	return 0;
}

//
//	destructive
//
int fade_bitmap(RGB_bitmap *dst, uint8_t alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!dst->exists()) {
			fprintf(stderr, "fade_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha == 0) {
			fprintf(stderr, "fade_bitmap: alpha = 0, no fade applied\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	return fade_bitmap((uint8_t*) dst->data(), 
					   dst->width(),
					   dst->height(),
					   RGB_PIXEL_SIZE,
					   alpha);
}

//
//	sets all pixels' alpha to 0x64
//
int fade_bitmap(RGBA_bitmap *dst, uint8_t alpha)
{
	// safety check
	{
		bool error_escape = false;
		if(!dst->exists()) {
			fprintf(stderr, "fade_bitmap: destination uninitialised\n");
			error_escape = true;
		}
		if(alpha == 0) {
			fprintf(stderr, "fade_bitmap: alpha = 0, no fade applied\n");
			error_escape = true;
		}
		if(error_escape) return -1;
	}

	return fade_bitmap((uint8_t*) dst->data(), 
					   dst->width(),
					   dst->height(),
					   RGBA_PIXEL_SIZE,
					   alpha);
}