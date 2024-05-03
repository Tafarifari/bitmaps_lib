/*
 *  ppm.c
 *
 */
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

const int RGB_SIZE = 3;

#define SIZEOF_MAGIC	3
#define SIZEOF_DIGITS	4

unsigned char *
read_ppm3(const char * filename, int * width, int * height)
{
	FILE * fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "read_ppm3: could not open file '%s'\n", filename);
		return NULL;
	}

	uint64_t 	data_buffer_size = 0;
	uint64_t 	count;
	int32_t 	colors = 0;
	uint8_t *	data = NULL;
	uint8_t	* 	width_ptr = NULL; 
	uint8_t	*	height_ptr = NULL;
	uint8_t		magic[SIZEOF_MAGIC],
		 		digits[SIZEOF_DIGITS],
		 		width_and_height[128], 
		 		comment_line[512];

	// read PM3 header
	if(fread(magic, 1, SIZEOF_MAGIC, fp) != SIZEOF_MAGIC)						goto READ_ERROR; 
	magic[2] = '\0';	
	
	// read comment line
	if(fgets((char *) comment_line, sizeof(comment_line), fp) == NULL)			goto READ_ERROR;
	comment_line[strlen((const char*) comment_line)-1] = '\0';
	
	// read width and height
	if(fgets((char*)width_and_height, sizeof(width_and_height), fp) == NULL)	goto READ_ERROR;
	width_ptr 	= (uint8_t*) strtok((char*) width_and_height, " ");
	height_ptr 	= (uint8_t*) strtok(NULL, " ");
	*width 		= atoi((const char*) width_ptr);
	*height 	= atoi((const char*) height_ptr);

	// read color depth
	if(fread(digits, 1, SIZEOF_DIGITS, fp) != SIZEOF_DIGITS)					goto READ_ERROR;
	colors 		= atoi((const char*) digits);

	// check header
	if(magic[0] != 'P' || magic[1] != '3' || colors != 255) {
		fprintf(stderr, "read_ppm3: invalid PPM3 format (file %s)\n", filename);
		goto ERROR_EXIT;
	}

	// allocate memory for RGB data 
	data_buffer_size = (*width) * (*height) * RGB_SIZE;
	if((data = (unsigned char*) malloc(data_buffer_size)) == NULL) {
		fprintf(stderr, "read_ppm3: out of memory (file %s)\n", filename);
		goto ERROR_EXIT;
	}

	// read RGB data
	count = 0;
	while(!feof(fp) && count < data_buffer_size)
	{
		for(uint32_t i=0; i<sizeof(digits); ++i)
		{
			digits[i] = fgetc(fp);
			if(digits[i] == 0x0a || digits[i] == ' ') // digits separated by space or new line
			{ 
				digits[i] = 0;
				break;
			}
		}
		data[count] = (unsigned char) atoi((const char*) digits);
		++count;
	}
	
	fclose(fp);
	return data;

READ_ERROR:
	fprintf(stderr, "read_ppm3: read error (file %s)\n", filename);
ERROR_EXIT:
	fclose(fp);
	return NULL;
}

//
//

unsigned char *
read_ppm6(const char* filename, int* width, int* height)
{
	FILE * fp = fopen(filename, "rb");
	if(fp == NULL) {
		fprintf(stderr, "read_ppm6 ERROR: couldn't open file %s\n", filename);
		return NULL;	
	}

	//	P6 file structure:
	//	------------------
	//	'P6' 0x0a 
	//  ( if '#') [COMMENT] 0x0a
	//  [WIDTH] 0x20 [HEIGHT] 0x0a
	//  [DEPTH = 255] 0x0a
	// 	--- here ends ASCII part ---
	//	DATA...

	const char 	MARKER[] 		= { 'P', '6', '\0' };
	const int 	BufferLenght 	= 32;
	const int	MaxLen 			= BufferLenght - 1;

	char 		read_buffer[BufferLenght];

	// format specifier for fscanf()
    char format_spec[10];
    snprintf(format_spec, sizeof(format_spec) - 1, "%%%ds", MaxLen);


	//	---------------------------------------------------------------
	// 		READ AND CHECK MARKER
	//

	if(fscanf(fp, format_spec, read_buffer) == EOF) {
		fprintf(stderr, "read_ppm6: unexpected end of file in %s\n", filename);
		fclose(fp);
		return NULL;
	}

	if(strcmp(read_buffer, MARKER) != 0) {
		fprintf(stderr, "read_ppm6: incorrenct file format marker: %2s\n", read_buffer);
		fclose(fp);
		return NULL;
	}

	//	---------------------------------------------------------------
	// 		IGNORE COMMENTS
	//
		
	char read_byte = 0;
	int flag_comment_section = 1;
	
	// step over new line
	fseek(fp, 1, SEEK_CUR);

	while(flag_comment_section)
	{
		size_t ret = fread(&read_byte, 1, 1, fp);
		if(ret != 1) {
			if(feof(fp)) fprintf(stderr, "read_ppm6: unexpected end of file %s\n", filename);
			else 		 fprintf(stderr, "read_ppm6: error while reading file %s\n", filename);
			fclose(fp);
			return NULL;
		}

		// ignore the line of comment
		if(read_byte == '#') {
			while(read_byte != '\n')
			{
				ret = fread(&read_byte, 1, 1, fp);
				if(ret != 1) {
					if(feof(fp)) fprintf(stderr, "read_ppm6: unexpected end of file %s\n", filename);
					else 		 fprintf(stderr, "read_ppm6: error while reading file %s\n", filename);
					fclose(fp);
					return NULL;
				}
			}
		} 
		// end of comments, move fp back 1 byte and exit while loop
		else {
			fseek(fp, -1, SEEK_CUR);
			flag_comment_section = 0;
		}
	}

	//	---------------------------------------------------------------
	// 		READ WIDTH, HEIGHT AND COLOR DEPTH
	//
	
	char width_ascii[BufferLenght];
	char height_ascii[BufferLenght];
	char depht_ascii[BufferLenght];

	size_t ret = fscanf(fp, format_spec, width_ascii);
	if(ret == 0 || feof(fp)) {
		fprintf(stderr, "read_ppm6: unexpected end of file %s\n", filename);
		fclose(fp);
		return NULL;
	}
	
	ret = fscanf(fp, format_spec, height_ascii);
	if(ret == 0 || feof(fp)) {
		fprintf(stderr, "read_ppm6: unexpected end of file %s\n", filename);
		fclose(fp);
		return NULL;
	}
	
	ret = fscanf(fp, format_spec, depht_ascii);
	if(ret == 0 || feof(fp)) {
		fprintf(stderr, "read_ppm6: unexpected end of file %s\n", filename);
		fclose(fp);
		return NULL;
	}
	
	int width_ 	= atoi(width_ascii);
	int height_ = atoi(height_ascii);
	int depth 	= atoi(depht_ascii);	

	if(depth != 255) {
		fprintf(stderr, "read_ppm6 WARNING: color depth of %s = %d (expected 255)\n", filename, depth);
	}


	//	---------------------------------------------------------------
	// 		READ DATA
	//
	unsigned long data_size = width_ * height_ * RGB_SIZE;

	unsigned char * data = (unsigned char *) malloc(data_size);
	if(data == NULL) {
		fprintf(stderr, "read_ppm6 ERROR: couldn't allocate %ld bytes of memory for %s\n", data_size, filename);
		fclose(fp);
		return NULL;
	}

	// step over new line
	fseek(fp, 1, SEEK_CUR);

	// read bitmap data
	if(fread(data, 1, data_size, fp) != data_size) {
		fprintf(stderr, "read_ppm6 ERROR: couldn't read bitmap data from %s\n", filename);
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	*width = width_;
	*height = height_;
	return data;
}
  

//	------------------------------------------------------------------
//		SAVE_PPM3
//
int save_ppm3(const char *filename, unsigned char *data, int width, int height)
{
	long data_buffer_size = width * height * RGB_SIZE;

	FILE* fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "save_ppm3 ERROR: could not open file '%s'\n", filename);
		return -1;
	}

	fprintf(fp, "P3\n");
	fprintf(fp, "# Created with save_ppm3\n");
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "255\n");

	for(long i=0; i<data_buffer_size; ++i) {
		fprintf(fp, "%d ", (int) data[i]);
	}
	
	fclose(fp);
	return 0;
}


//	------------------------------------------------------------------
//		SAVE_PPM6
//
int save_ppm6(const char *filename, unsigned char *data, int width, int height)
{
	if(data == NULL) return -1;

	//	P6 file structure:
	//	------------------
	//	'P6' 0x0a 
	//  ( if '#') [COMMENT] 0x0a
	//  [WIDTH] 0x20 [HEIGHT] 0x0a
	//  [DEPTH = 255] 0x0a
	// 	--- here ends ASCII part ---
	//	DATA...
	
	long data_buffer_size = width * height * RGB_SIZE;

	FILE* fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "save_ppm6 ERROR: could not open file '%s'\n", filename);
		return -1;
	}

	fprintf(fp, "P6\n");
	fprintf(fp, "# Created with save_ppm6\n");
	fprintf(fp, "%d %d\n", width, height);
	fprintf(fp, "255\n");

	fwrite(data, data_buffer_size, 1, fp);	// write data bytes as one stream 
	
	fclose(fp);
	return 0;
}