/*
 *  ppm.h
 *
 *  changes:
 *    13.10.23  added read_ppm6() / binary RGB
 *	  29.01.24	added save_ppm6()
 */
#ifndef __PPM_H
	#define __PPM_H

	#define PPM_LIB_VERSION "1.03 / 29.01.24" 

	unsigned char * read_ppm3(const char* filename, int* width, int* height); 			/* width and height saved in pointers, returns NULL on error and pointer to data on success */
	unsigned char * read_ppm6(const char* filename, int* width, int* height); 			/* width and height saved in pointers, returns NULL on error and pointer to data on success */
	
	int save_ppm3(const char *filename, unsigned char *data, int width, int height); 	/* returns -1 on error, 0 on success */
	int save_ppm6(const char *filename, unsigned char *data, int width, int height);	/* returns -1 on error, 0 on success */

#endif