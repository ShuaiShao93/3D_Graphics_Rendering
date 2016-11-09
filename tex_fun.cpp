/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image=NULL;
int xs, ys;
int reset = 1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
/* determine texture cell corner values and perform bilinear interpolation */
/* set color to interpolated GzColor value and return */
  if (u < 0) {
	  u = 0;
  }
  else if (u > 1) {
	  u = 1;
  }

  if (v < 0) {
	  v = 0;
  }
  else if (v > 1) {
	  v = 1;
  }

  float nu = u*(xs - 1);
  float nv = v*(ys - 1);
  int Ax = floor(nu);
  int Ay = floor(nv);
  int Bx = ceil(nu);
  int By = floor(nv);
  int Cx = ceil(nu);
  int Cy = ceil(nv);
  int Dx = floor(nu);
  int Dy = ceil(nv);
  float s = nu - floor(nu);
  float t = nv - floor(nv);

  GzColor A, B, C, D;
  memcpy(A, image[Ax+Ay*xs], sizeof(GzColor));
  memcpy(B, image[Bx+By*xs], sizeof(GzColor));
  memcpy(C, image[Cx+Cy*xs], sizeof(GzColor));
  memcpy(D, image[Dx+Dy*xs], sizeof(GzColor));
  
  for (int c = 0; c < 3; c++) {
	  color[c] = s * t * C[c] + (1 - s) * t * D[c] + s * (1 - t) * B[c] + (1 - s) * (1 - t) * A[c];
  }

  return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
	if (u < 0) {
		u = 0;
	}
	else if (u > 1) {
		u = 1;
	}

	if (v < 0) {
		v = 0;
	}
	else if (v > 1) {
		v = 1;
	}

	int t = int((u + v) / 0.1);
	if (t % 2 == 0) {
		color[RED] = 1;
		color[GREEN] = 0;
		color[BLUE] = 0;
	}
	else {
		color[RED] = 0;
		color[GREEN] = 0;
		color[BLUE] = 1;
	}

	return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}

