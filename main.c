#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "dataBMP.h"

void * safe_malloc(long size) // exits if problem occurs
{
	void * ptr = malloc(size);
	if(ptr == NULL)
	{
		fprintf(stderr, "Error alocating memory.\n");
		exit(2);
	}
	return ptr;
}

void load_string(char * ptr)
{
	printf("Enter name: ");
	scanf("%s", ptr);
}

FILE * load_file(char * name, char * mode) // safely loads file
{
	FILE * fptr = fopen(name, mode);
	if(fptr == NULL){
		fprintf(stderr, "File not found: '%s'\n", name);
		exit(1);
	}
	return fptr;
}


// loads bmp file as header struct. returns image and full header data
unsigned char * load_bmp(char * name, BMPHeader_t * BMPHeader, unsigned char ** headerFull)
{
	FILE * fp = load_file(name, "rb");
	
	fread(BMPHeader, sizeof(BMPHeader_t), 1, fp);
	
	//check if file is BMP (I had to look up the code)
	//if not BMP this will often be unitialized, but I don't know how to circumvent this
	if(BMPHeader->type != 0x4D42){
		fclose(fp);
		fprintf(stderr, "File format is not BMP\n");
		exit(2);
	}
	
	fseek(fp, BMPHeader->offset, SEEK_SET);
	
	unsigned char * BMPImage = (unsigned char *)safe_malloc(BMPHeader->image_size_bytes);
	
	fread(BMPImage, BMPHeader->image_size_bytes, 1, fp);
	
	// save entire header
	*headerFull = safe_malloc(BMPHeader->offset);//TODO -1??
	fseek(fp, 0, SEEK_SET);
	fread(*headerFull, BMPHeader->offset, 1, fp);
	
	
	fclose(fp);
	return BMPImage;
}

// take header and content and place into file. No header contents were changed.
void save_bmp(char * name, BMPHeader_t BMPHeader, unsigned char * headerFull, unsigned char * BMPData)
{
	FILE * fpw = load_file(name, "wb");
	
	fwrite(headerFull,BMPHeader.offset,1,fpw);
	fwrite(BMPData,BMPHeader.image_size_bytes,1,fpw);
	fclose(fpw);
}

// switch lines. Has to do a correction for even-sized images
unsigned char * switcheroo(unsigned char * BMPData, int height, int width)  // assumes 1 byte per pixel
{
	int size = height * width;
	unsigned char * newBMPData = safe_malloc(size);
	
	for(int i = 0; i < size; i++)
	{
		int iHeight = i/width; // 0-indexed
		int iWidth = i%width;
		int resHeight = height - 1 - iHeight; // inverted heigth
		int resWidth = iWidth; // inverted width is the same
		if(iHeight % 2 == 1) // every second line (0-indexed)
		{
			newBMPData[i] = BMPData[i];
		}
		else{
			newBMPData[i] = BMPData[resHeight*width + resWidth]; // this only works for odd-heigth images
			newBMPData[i] = BMPData[(resHeight-(height%2==0))*width + resWidth];
		}
	}
	
	free(BMPData);
	return newBMPData;
}

int main(int argc, char * argv[])
{
	char * input = malloc(80*sizeof(char));
	if(argc > 1){
		input = argv[1];
	}
	else{
		printf("Enter input file name: ");
		scanf("%s", input);
	}
	
	char * output = malloc(80*sizeof(char));
	if(argc > 2){
		output = argv[2];
	}
	else{
		printf("Enter output file name: ");
		scanf("%s", output);
	}
	
	// declare header here so it can still be used
	BMPHeader_t BMPHeader;
	unsigned char * headerFull = NULL;
	unsigned char * BMPData = load_bmp(input, &BMPHeader, &headerFull);
	
	BMPData = switcheroo(BMPData, (int)BMPHeader.height_px, (int)BMPHeader.width_px);
	
	save_bmp(output, BMPHeader, headerFull, BMPData);
	
	if(argc <= 1){free(input);}
	if(argc <= 2){free(output);}
	free(headerFull);
	free(BMPData);
	return 0;
}
	
