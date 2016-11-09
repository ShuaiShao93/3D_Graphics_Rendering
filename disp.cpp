/*   CS580 HW1 display functions to be completed   */

#include   "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* HW1.1 create a framebuffer for MS Windows display:
 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
 -- pass back pointer 
 */

	//bounds and pointer check
	if (width < 0 || width > MAXXRES || height < 0 || height > MAXYRES || framebuffer == NULL) {
		return GZ_FAILURE;
	}

	//allocate memory
	char* p = NULL;
	int len = 3 * width*height;
	p = (char*)malloc(len);
	if (p == NULL) {
		return GZ_FAILURE;
	}
	
	*framebuffer = p;
	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, int xRes, int yRes)
{
/* HW1.2 create a display:
  -- allocate memory for indicated resolution
  -- pass back pointer to GzDisplay object in display
*/
	//bounds and pointer check
	if (xRes < 0 || xRes > MAXXRES || yRes < 0 || yRes > MAXYRES || display == NULL) {
		return GZ_FAILURE;
	}

	//create object and set resolutions
	GzDisplay* p = NULL;
	p = new GzDisplay;
	if (p == NULL) {
		return GZ_FAILURE;
	}
	p->xres = xRes;
	p->yres = yRes;

	*display = p;
	
	//allocate memory for pixel buffer
	GzPixel* bp = NULL;
	bp = new GzPixel[xRes*yRes];
	if (bp == NULL) {
		return GZ_FAILURE;
	}
	(*display)->fbuf = bp;
	
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* HW1.3 clean up, free memory */
	//free buffer and display
	if (display == NULL)
		return GZ_FAILURE;

	delete display;
	display = NULL;
	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes)
{
/* HW1.4 pass back values for a display */
	//check parameters
	if (display == NULL || xRes == NULL || yRes == NULL)
		return GZ_FAILURE;
	*xRes = display->xres;
	*yRes = display->yres;

	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* HW1.5 set everything to some default values - start a new frame */
	//check parameter
	if (display == NULL)
		return GZ_FAILURE;
	//initialize background
	for (int x = 0; x < display->xres; x++) 
		for (int y = 0; y < display->yres; y++) {
			display->fbuf[ARRAY(x, y)].alpha = 1;
			display->fbuf[ARRAY(x, y)].blue = 42<<4;
			display->fbuf[ARRAY(x, y)].green = 65<<4;
			display->fbuf[ARRAY(x, y)].red = 74<<4;
			display->fbuf[ARRAY(x, y)].z = MAXINT;
		}
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* HW1.6 write pixel values into the display */
	//set pixels in bounds
	if (i >= 0 && i < display->xres && j >= 0 && j < display->yres) {
		//check rgb value
		display->fbuf[ARRAY(i, j)].red = r > 4095 ? 4095 : (r < 0 ? 0 : r);
		display->fbuf[ARRAY(i, j)].green = g > 4095 ? 4095 : (g < 0 ? 0 : g);
		display->fbuf[ARRAY(i, j)].blue = b > 4095 ? 4095 : (b < 0 ? 0 : b);
		display->fbuf[ARRAY(i, j)].alpha = a;
		display->fbuf[ARRAY(i, j)].z = z;
	}

	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
/* HW1.7 pass back a pixel value to the display */
	if (r != NULL)
		*r = display->fbuf[ARRAY(i, j)].red;
	if (g != NULL)
		*g = display->fbuf[ARRAY(i, j)].green;
	if (b != NULL)
		*b = display->fbuf[ARRAY(i, j)].blue;
	if (a != NULL)
		*a = display->fbuf[ARRAY(i, j)].alpha;
	if (z != NULL)
		*z = display->fbuf[ARRAY(i, j)].z;

	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

/* HW1.8 write pixels to ppm file -- "P6 %d %d 255\r" */
	//check parameters
	if (outfile == NULL || display == NULL) {
		return GZ_FAILURE;
	}
	//print the header
	fprintf(outfile, "P6 %d %d 255\n", display->xres, display->yres);
	//write the content
	for (int i = 0; i < display->xres*display->yres; i++) {
		char rr = (char)(display->fbuf[i].red >> 4);
		char gg = (char)(display->fbuf[i].green >> 4);
		char bb = (char)(display->fbuf[i].blue >> 4);
		fwrite(&rr, sizeof(char), 1, outfile);
		fwrite(&gg, sizeof(char), 1, outfile);
		fwrite(&bb, sizeof(char), 1, outfile);
	}

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

/* HW1.9 write pixels to framebuffer: 
	- put the pixels into the frame buffer
	- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red 
	- NOT red, green, and blue !!!
*/
	//parameter check
	if (framebuffer == NULL || display == NULL)
		return GZ_FAILURE;
	//set framebuffer
	for (int i = 0; i < display->xres * display->yres; i++) {
		framebuffer[3 * i] = (char)(display->fbuf[i].blue >> 4);
		framebuffer[3 * i+1] = (char)(display->fbuf[i].green >> 4);
		framebuffer[3 * i+2] = (char)(display->fbuf[i].red >> 4);
	}

	return GZ_SUCCESS;
}