/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#define _USE_MATH_DEFINES 
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"

int GzPushMatrix(GzRender *render, GzMatrix	matrix, bool normIdentity);
short	ctoi(float color);
void cal_coef(float v1[3], float v2[3], float coaf[3]);
void normalize(float v[3]);
void interpolate(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float* res);

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	GzMatrix matrix =
	{
		1,	0,	0,	0,
		0,	cos(degree*M_PI / 180.0),	-sin(degree*M_PI / 180.0),	0,
		0,	sin(degree*M_PI / 180.0),	cos(degree*M_PI / 180.0),	0,
		0,	0,	0,	1
	};
	memcpy(mat, matrix, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	GzMatrix matrix =
	{
		cos(degree*M_PI / 180.0),	0,	sin(degree*M_PI / 180.0),	0,
		0,	1,	0,	0,
		-sin(degree*M_PI / 180.0),	0,	cos(degree*M_PI / 180.0),	0,
		0,	0,	0,	1
	};
	memcpy(mat, matrix, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	GzMatrix matrix =
	{
		cos(degree*M_PI / 180.0),	-sin(degree*M_PI / 180.0),	0,	0,
		sin(degree*M_PI / 180.0),	cos(degree*M_PI / 180.0),	0,	0,
		0,	0,	1,	0,
		0,	0,	0,	1
	};
	memcpy(mat, matrix, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	GzMatrix matrix =
	{
		1,	0,	0,	translate[X],
		0,	1,	0,	translate[Y],
		0,	0,	1,	translate[Z],
		0,	0,	0,	1
	};
	memcpy(mat, matrix, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)

{
// Create scaling matrix
// Pass back the matrix using mat value
	GzMatrix matrix =
	{
		scale[X],	0,	0,	0,
		0,	scale[Y],	0,	0,
		0,	0,	scale[Z],	0,
		0,	0,	0,	1
	};
	memcpy(mat, matrix, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- setup Xsp and anything only done once 
- save the pointer to display 
- init default camera 
*/ 
	//new render and check null
	GzRender *p = NULL;
	p = new GzRender;
	if (p == NULL) {
		return GZ_FAILURE;
	}

	//set everything
	p->display = display;
	p->matlevel = -1;
	p->numlights = 0;
	GzMatrix xsp =
	{
		display->xres / 2.0,	0.0,	0.0,	display->xres / 2.0,
		0.0,	-display->yres / 2.0,	0.0,	display->yres / 2.0,
		0.0,	0.0,	float(MAXINT),	0.0,
		0.0,	0.0,	0.0,	1.0
	};
	memcpy(p->Xsp, xsp, sizeof(GzMatrix));

	//default camera
	p->camera.position[X] = DEFAULT_IM_X;
	p->camera.position[Y] = DEFAULT_IM_Y;
	p->camera.position[Z] = DEFAULT_IM_Z;

	p->camera.lookat[X] = 0.0;
	p->camera.lookat[Y] = 0.0;
	p->camera.lookat[Z] = 0.0;

	p->camera.worldup[X] = 0.0;
	p->camera.worldup[Y] = 1.0;
	p->camera.worldup[Z] = 0.0;

	p->camera.FOV = DEFAULT_FOV;
	
	p->Xoffset = 0;
	p->Yoffset = 0;

	*render = p;

	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	if (render != NULL) {
		delete render;
	}

	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- setup for start of each frame - init frame buffer color,alpha,z
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms when needed 
*/ 
	//check input
	if (render == NULL || render->display == NULL)
		return GZ_FAILURE;
	//call initdisplay and check return value
	if (GzInitDisplay(render->display) == GZ_FAILURE)
		return GZ_FAILURE;
	//compute Xiw
	float x[3], y[3], z[3];
	float sum, norm;
	//compute z axis
	for (int i = 0; i < 3; i++) {
		z[i] = render->camera.lookat[i] - render->camera.position[i];
	}
	sum = 0;
	for (float f : z) {
		sum += f*f;
	}
	norm = sqrt(sum);
	for (int i = 0; i < 3; i++) {
		z[i] /= norm;
	}

	//compute y axis
	for (int i = 0; i < 3; i++) {
		y[i] = render->camera.worldup[i] - (render->camera.worldup[X]*z[X]+render->camera.worldup[Y]*z[Y]+render->camera.worldup[Z]*z[Z])*z[i];
	}
	sum = 0;
	for (float f : y) {
		sum += f*f;
	}
	norm = sqrt(sum);
	for (int i = 0; i < 3; i++) {
		y[i] /= norm;
	}

	//compute x axis
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = y[2] * z[0] - y[0] * z[2];
	x[2] = y[0] * z[1] - y[1] * z[0];

	sum = 0;
	for (float f : x) {
		sum += f*f;
	}
	norm = sqrt(sum);
	for (int i = 0; i < 3; i++) {
		x[i] /= norm;
	}

	//set Xiw
	GzMatrix xiw = 
	{
		x[X],	x[Y],	x[Z],	-(x[X] * render->camera.position[X] + x[Y] * render->camera.position[Y] + x[Z] * render->camera.position[Z]),
		y[X],	y[Y],	y[Z],	-(y[X] * render->camera.position[X] + y[Y] * render->camera.position[Y] + y[Z] * render->camera.position[Z]),
		z[X],	z[Y],	z[Z],	-(z[X] * render->camera.position[X] + z[Y] * render->camera.position[Y] + z[Z] * render->camera.position[Z]),
		0.0,	0.0,	0.0,	1.0
	};

	memcpy(render->camera.Xiw, xiw, sizeof(GzMatrix));

	//compute Xpi
	float d_1 = tan(render->camera.FOV*M_PI /180.0 / 2.0);
	GzMatrix xpi =
	{
		1.0,	0.0,	0.0,	0.0,
		0.0,	1.0,	0.0,	0.0,
		0.0,	0.0,	d_1,	0.0,
		0.0,	0.0,	d_1,	1.0
	};
	memcpy(render->camera.Xpi, xpi, sizeof(GzMatrix));

	//push Xsw
	int status = GZ_SUCCESS;
	status |= GzPushMatrix(render, render->Xsp, true);
	status |= GzPushMatrix(render, render->camera.Xpi, true);
	status |= GzPushMatrix(render, render->camera.Xiw);

	if (status == GZ_FAILURE)
		return GZ_FAILURE;

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	//check null and copy camera
	if (render == NULL || camera == NULL)
		return GZ_FAILURE;
	memcpy(&(render->camera), camera, sizeof(GzCamera));

	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
	/*
	- push a matrix onto the Ximage stack
	- check for stack overflow
	*/
	//remove translation in norm matrix
	GzMatrix matrixNorm;
	memcpy(matrixNorm, matrix, sizeof(GzMatrix));
	for (int r = 0; r < 3; r++) {
		matrixNorm[r][3] = 0.0;
	}
	//if it's the first matrix
	if (render->matlevel == -1) {
		memcpy(render->Ximage[render->matlevel + 1], matrix, sizeof(GzMatrix));
		memcpy(render->Xnorm[render->matlevel + 1], matrixNorm, sizeof(GzMatrix));
	}
	else {
		//check overflow
		if (render->matlevel + 1 >= MATLEVELS)
			return GZ_FAILURE;

		//compute multiplication in Ximage
		GzMatrix temp =
		{
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp[i][j] += render->Ximage[render->matlevel][i][k] * matrix[k][j];
				}
			}
		}

		memcpy(render->Ximage[render->matlevel + 1], temp, sizeof(GzMatrix));

		//compute multiplication in Xnorm
		GzMatrix temp2 =
		{
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp2[i][j] += render->Xnorm[render->matlevel][i][k] * matrixNorm[k][j];
				}
			}
		}

		memcpy(render->Xnorm[render->matlevel + 1], temp2, sizeof(GzMatrix));
	}
	//increment top pointer
	render->matlevel++;

	return GZ_SUCCESS;
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix, bool normIdentity)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	GzMatrix matrixNorm =
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	if (!normIdentity) {
		memcpy(matrixNorm, matrix, sizeof(GzMatrix));
	}
	//if it's the first matrix
	if (render->matlevel == -1) {
		memcpy(render->Ximage[render->matlevel + 1], matrix, sizeof(GzMatrix));
		memcpy(render->Xnorm[render->matlevel + 1], matrixNorm, sizeof(GzMatrix));
	}
	else {
		//check overflow
		if (render->matlevel + 1 >= MATLEVELS)
			return GZ_FAILURE;

		//compute multiplication in Ximage
		GzMatrix temp =
		{
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp[i][j] += render->Ximage[render->matlevel][i][k] * matrix[k][j];
				}
			}
		}

		memcpy(render->Ximage[render->matlevel + 1], temp, sizeof(GzMatrix));

		//compute multiplication in Xnorm
		GzMatrix temp2 =
		{
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp2[i][j] += render->Xnorm[render->matlevel][i][k] * matrixNorm[k][j];
				}
			}
		}

		memcpy(render->Xnorm[render->matlevel + 1], temp2, sizeof(GzMatrix));
	}
	//increment top pointer
	render->matlevel++;

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	//check underflow
	if (render->matlevel < 0)
		return GZ_FAILURE;

	//decrement top
	render->matlevel--;

	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	//check input
	if (render == NULL || numAttributes < 0 || (numAttributes > 0 && (nameList == NULL || valueList == NULL)))
		return GZ_FAILURE;
	//set attributes
	for (int i = 0; i < numAttributes; i++) {
		if (nameList[i] == GZ_RGB_COLOR) {
			memcpy(render->flatcolor, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_DIRECTIONAL_LIGHT) {
			if (render->numlights == MAX_LIGHTS) {
				return GZ_FAILURE;
			}
			render->lights[render->numlights] = (*(GzLight *)valueList[i]);
			render->numlights++;
		}
		else if (nameList[i] == GZ_AMBIENT_LIGHT) {
			render->ambientlight = (*(GzLight *)valueList[i]);
		}
		else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT) {
			memcpy(render->Kd, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_AMBIENT_COEFFICIENT) {
			memcpy(render->Ka, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_SPECULAR_COEFFICIENT) {
			memcpy(render->Ks, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_INTERPOLATE) {
			render->interp_mode = *(int *)valueList[i];
		}
		else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT) {
			render->spec = *(float *)valueList[i];
		}
		else if (nameList[i] == GZ_TEXTURE_MAP) {
			render->tex_fun = (GzTexture)valueList[i];
		}
		else if (nameList[i] == GZ_AASHIFTX) {
			render->Xoffset = *(float *)valueList[i];
		}
		else if (nameList[i] == GZ_AASHIFTY) {
			render->Yoffset = *(float *)valueList[i];
		}
		else if (nameList[i] == GZ_AAWEIGHT) {
			render->weight = *(float *)valueList[i];
		}
	}

	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts using matrix on top of stack 
- Clip - just discard any triangle with any vert(s) behind view plane 
       - optional: test for triangles with all three verts off-screen (trivial frustum cull)
- invoke triangle rasterizer  
*/ 
	//check inputs
	if (render == NULL || render->display == NULL || numParts < 0 || (numParts > 0 && (nameList == NULL || valueList == NULL)))
		return GZ_FAILURE;
	//put Triangle into display
	GzCoord *w3vert;
	GzCoord *w3norm;
	GzTextureIndex *p3uv;
	for (int i = 0; i < numParts; i++) {
		if (nameList[i] == GZ_POSITION) {
			w3vert = (GzCoord*)valueList[i];
		}
		else if (nameList[i] == GZ_NORMAL) {
			w3norm = (GzCoord*)valueList[i];
		}
		else if (nameList[i] == GZ_TEXTURE_INDEX) {
			p3uv = (GzTextureIndex*)valueList[i];
		}
	}


	//convert coords and norms of 3 vertices from model to image
	GzCoord vnorm[3];
	for (int pi = 0; pi < 3; pi++) {
		//compute norms of vertices in image
		float wnorm[4];
		memcpy(wnorm, w3norm[pi], sizeof(GzCoord));
		wnorm[3] = 1.0;
		float inorm[4] = { 0.0, 0.0, 0.0, 0.0 };
		for (int r1 = 0; r1 < 4; r1++) {
			for (int c1 = 0; c1 < 4; c1++) {
				inorm[r1] += render->Xnorm[render->matlevel][r1][c1] * wnorm[c1];
			}
		}
		vnorm[pi][X] = inorm[0] / inorm[3];
		vnorm[pi][Y] = inorm[1] / inorm[3];
		vnorm[pi][Z] = inorm[2] / inorm[3];
		normalize(vnorm[pi]);
	}

	//if Gouraud, compute color on each vertices
	GzColor verColor[3];
	if (render->interp_mode == GZ_COLOR || render->interp_mode == GZ_FLAT) {
		for (int pi = 0; pi < 3; pi++) {
			GzCoord e = {0, 0, -1};
			//compute specular light and diffuse light
			GzColor specular = {0,0,0};
			GzColor diffuse = { 0,0,0 };
			for (int l = 0; l < render->numlights; l++) {
				float ne = vnorm[pi][X] * e[X] + vnorm[pi][Y] * e[Y] + vnorm[pi][Z] * e[Z];
				float nl = vnorm[pi][X] * render->lights[l].direction[X] + vnorm[pi][Y] * render->lights[l].direction[Y] + vnorm[pi][Z] * render->lights[l].direction[Z];
				//if NE and NL has different sign, skip
				if (ne * nl < 0) {
					continue;
				}
				//if both are negative, flip norm
				else if (ne < 0 && nl < 0) {
					vnorm[pi][X] *= -1;
					vnorm[pi][Y] *= -1;
					vnorm[pi][Z] *= -1;
					nl *= -1;
				}
				
				GzCoord r = { 2 * nl*vnorm[pi][X] - render->lights[l].direction[X], 2 * nl*vnorm[pi][Y] - render->lights[l].direction[Y] , 2 * nl*vnorm[pi][Z] - render->lights[l].direction[Z] };
				normalize(r);
				float re = r[X] * e[X] + r[Y] * e[Y] + r[Z] * e[Z];
				if (re < 0) {
					re = 0.0;
				} 
				else if (re > 1) {
					re = 1.0;
				}
				float res = pow(re, render->spec);
				specular[RED] += render->lights[l].color[RED] * res;
				specular[GREEN] += render->lights[l].color[GREEN] * res;
				specular[BLUE] += render->lights[l].color[BLUE] * res;

				diffuse[RED] += render->lights[l].color[RED] * nl;
				diffuse[GREEN] += render->lights[l].color[GREEN] * nl;
				diffuse[BLUE] += render->lights[l].color[BLUE] * nl;
			}
			//ambient light
			GzColor ambient = { 0,0,0 };
			ambient[RED] = render->ambientlight.color[RED];
			ambient[GREEN] = render->ambientlight.color[GREEN];
			ambient[BLUE] = render->ambientlight.color[BLUE];

			//final, if we have texture function, only need to add them together
			if (render->tex_fun != NULL) {
				verColor[pi][RED] = specular[RED] + diffuse[RED] + ambient[RED];
				verColor[pi][GREEN] = specular[GREEN] + diffuse[GREEN] + ambient[GREEN];
				verColor[pi][BLUE] = specular[BLUE] + diffuse[BLUE] + ambient[BLUE];
			}
			else {
				verColor[pi][RED] = render->Ks[RED] * specular[RED] + render->Kd[RED] * diffuse[RED] + render->Ka[RED] * ambient[RED];
				verColor[pi][GREEN] = render->Ks[GREEN] * specular[GREEN] + render->Kd[GREEN] * diffuse[GREEN] + render->Ka[GREEN] * ambient[GREEN];
				verColor[pi][BLUE] = render->Ks[BLUE] * specular[BLUE] + render->Kd[BLUE] * diffuse[BLUE] + render->Ka[BLUE] * ambient[BLUE];
			}
		}
	}


	//Xform
	GzCoord vert[3];
	for (int pi = 0; pi < 3; pi++) {
		float wvert[4];
		memcpy(wvert, w3vert[pi], sizeof(GzCoord));
		wvert[3] = 1.0;
		float svert[4] = { 0.0, 0.0, 0.0, 0.0 };
		for (int r1 = 0; r1 < 4; r1++) {
			for (int c1 = 0; c1 < 4; c1++) {
				svert[r1] += render->Ximage[render->matlevel][r1][c1] * wvert[c1];
			}
		}
		vert[pi][X] = svert[0] / svert[3] - render->Xoffset;
		vert[pi][Y] = svert[1] / svert[3] - render->Yoffset;
		vert[pi][Z] = svert[2] / svert[3];
	}

	//compute U,V on vertices in perspective
	GzTextureIndex puv[3];
	for (int pi = 0; pi < 3; pi++) {
		float Vz = vert[pi][Z] / (INT_MAX - vert[pi][Z]);
		puv[pi][U] = p3uv[pi][U] / (Vz + 1);
		puv[pi][V] = p3uv[pi][V] / (Vz + 1);
	}

			
	//ignore negative z
	if (vert[0][Z] < 0 || vert[1][Z] < 0 || vert[2][Z] < 0)
		return GZ_SUCCESS;

	//rasterizer(LEE)
	float coef[3][3];
	int seq[3] = { 0, 1, 2 };
	int cw[3];
	//sort verts from top to bottom and save the sorting in seq
	for (int j = 1; j < 3; j++) {
		for (int k = 0; k < 3 - j; k++) {
			if (vert[seq[k]][Y] > vert[seq[k + 1]][Y]) {
				int temp = seq[k];
				seq[k] = seq[k + 1];
				seq[k + 1] = temp;
			}
		}
	}
	//if the top two vertices have same y, then compare theier x and save the cw sorting in cw
	if (vert[seq[0]][Y] == vert[seq[1]][Y]) {
		if (vert[seq[0]][X] < vert[seq[1]][X]) {
			cw[0] = seq[0];
			cw[1] = seq[1];
			cw[2] = seq[2];
		}
		else {
			cw[0] = seq[1];
			cw[1] = seq[0];
			cw[2] = seq[2];
		}
	}
	//else, calculate the formula of line on vert0 and vert2, then calculate the x of the point on the line with the same y of vert1, and save the cw sorting
	else {
		float coef02[3];
		cal_coef(vert[seq[0]], vert[seq[2]], coef02);
		float x1 = vert[seq[1]][X];
		float y1 = vert[seq[1]][Y];
		float tx = (-coef02[1] * y1 - coef02[2]) / coef02[0];
		if (x1 > tx) {
			cw[0] = seq[0];
			cw[1] = seq[1];
			cw[2] = seq[2];
		}
		else {
			cw[0] = seq[0];
			cw[1] = seq[2];
			cw[2] = seq[1];
		}
	}
	//calculate formulas of three lines
	float coef01[3], coef12[3], coef20[3];
	cal_coef(vert[cw[0]], vert[cw[1]], coef01);
	cal_coef(vert[cw[1]], vert[cw[2]], coef12);
	cal_coef(vert[cw[2]], vert[cw[0]], coef20);
	//calculate the formula of the plane
	float ABCD[4];
	interpolate(vert[0][X], vert[0][Y], vert[0][Z], vert[1][X], vert[1][Y], vert[1][Z], vert[2][X], vert[2][Y], vert[2][Z], ABCD);

	float A = ABCD[0];
	float B = ABCD[1];
	float C = ABCD[2];
	float D = ABCD[3];

	//coefficient of uv
	float uvcoef[2][4];
	interpolate(vert[0][X], vert[0][Y], puv[0][U], vert[1][X], vert[1][Y], puv[1][U], vert[2][X], vert[2][Y], puv[2][U], uvcoef[U]);
	interpolate(vert[0][X], vert[0][Y], puv[0][V], vert[1][X], vert[1][Y], puv[1][V], vert[2][X], vert[2][Y], puv[2][V], uvcoef[V]);
	

	//if Gouraud, compute four coeffients
	float colcoef[3][4];
	if (render->interp_mode == GZ_COLOR) {
		interpolate(vert[0][X], vert[0][Y], verColor[0][RED], vert[1][X], vert[1][Y], verColor[1][RED], vert[2][X], vert[2][Y], verColor[2][RED], colcoef[RED]);
		interpolate(vert[0][X], vert[0][Y], verColor[0][GREEN], vert[1][X], vert[1][Y], verColor[1][GREEN], vert[2][X], vert[2][Y], verColor[2][GREEN], colcoef[GREEN]);
		interpolate(vert[0][X], vert[0][Y], verColor[0][BLUE], vert[1][X], vert[1][Y], verColor[1][BLUE], vert[2][X], vert[2][Y], verColor[2][BLUE], colcoef[BLUE]);
	}

	//set all pixels in the bbox
	for (int x = 0; x < render->display->xres; x++)
		for (int y = 0; y < render->display->yres; y++) {
			float f01 = coef01[0] * x + coef01[1] * y + coef01[2];
			float f12 = coef12[0] * x + coef12[1] * y + coef12[2];
			float f20 = coef20[0] * x + coef20[1] * y + coef20[2];

			//if the pixel is in the triangle or on the top horizonal line or on the left line or on the bottom line when this line is at left
			if ((f01 > 0 && f12 > 0 && f20 > 0) || (vert[cw[0]][Y] == vert[cw[1]][Y] && f01 == 0 && f12 > 0 && f20 > 0) || (f20 == 0 && f01 > 0 && f12 > 0) || (vert[cw[1]][Y] > vert[cw[2]][Y] && f12 == 0 && f01 > 0 && f20 > 0)) {
				float z = (-A*x - B*y - D) / C;
				int oz;
				if (GzGetDisplay(render->display, x, y, NULL, NULL, NULL, NULL, &oz) == GZ_FAILURE)
					return GZ_FAILURE;
				if (z >= 0 && z <= oz) {
					//uv interpolate
					GzTextureIndex UV;
					UV[U] = (-uvcoef[U][0] * x - uvcoef[U][1] * y - uvcoef[U][3]) / uvcoef[U][2];
					UV[V] = (-uvcoef[V][0] * x - uvcoef[V][1] * y - uvcoef[V][3]) / uvcoef[V][2];

					float Vz = z / (INT_MAX - z);
					GzTextureIndex uv;
					uv[U] = UV[U] * (Vz + 1);
					uv[V] = UV[V] * (Vz + 1);

					GzColor texcol;
					if (render->tex_fun != NULL) {
						render->tex_fun(uv[U], uv[V], texcol);
					}


					//if gouraud
					GzColor color;
					if (render->interp_mode == GZ_COLOR) {
						for (int c = 0; c < 3; c++) {
							color[c] = (-colcoef[c][0] * x - colcoef[c][1] * y - colcoef[c][3]) / colcoef[c][2];
							if (render->tex_fun != NULL) {
								color[c] *= texcol[c];
							}
						}
					}
					//if phong
					else if (render->interp_mode == GZ_NORMALS) {
						float norcoef[3][4];
						interpolate(vert[0][X], vert[0][Y], vnorm[0][X], vert[1][X], vert[1][Y], vnorm[1][X], vert[2][X], vert[2][Y], vnorm[2][X], norcoef[X]);
						interpolate(vert[0][X], vert[0][Y], vnorm[0][Y], vert[1][X], vert[1][Y], vnorm[1][Y], vert[2][X], vert[2][Y], vnorm[2][Y], norcoef[Y]);
						interpolate(vert[0][X], vert[0][Y], vnorm[0][Z], vert[1][X], vert[1][Y], vnorm[1][Z], vert[2][X], vert[2][Y], vnorm[2][Z], norcoef[Z]);
						GzCoord norm = { (-norcoef[X][0] * x - norcoef[X][1] * y - norcoef[X][3]) / norcoef[X][2], (-norcoef[Y][0] * x - norcoef[Y][1] * y - norcoef[Y][3]) / norcoef[Y][2], (-norcoef[Z][0] * x - norcoef[Z][1] * y - norcoef[Z][3]) / norcoef[Z][2] };
						normalize(norm);
						GzCoord e = { 0,0,-1 };
						//compute specular light and diffuse light
						GzColor specular = { 0,0,0 };
						GzColor diffuse = { 0,0,0 };
						for (int l = 0; l < render->numlights; l++) {
							float ne = norm[X] * e[X] + norm[Y] * e[Y] + norm[Z] * e[Z];
							float nl = norm[X] * render->lights[l].direction[X] + norm[Y] * render->lights[l].direction[Y] + norm[Z] * render->lights[l].direction[Z];
							//if NE and NL has different sign, skip
							if (ne * nl < 0) {
								continue;
							}
							//if both are negative, flip norm
							else if (ne < 0 && nl < 0) {
								norm[X] *= -1;
								norm[Y] *= -1;
								norm[Z] *= -1;
								nl *= -1;
							}

							GzCoord r = { 2 * nl*norm[X] - render->lights[l].direction[X], 2 * nl*norm[Y] - render->lights[l].direction[Y] , 2 * nl*norm[Z] - render->lights[l].direction[Z] };
							normalize(r);
							float re = r[X] * e[X] + r[Y] * e[Y] + r[Z] * e[Z];
							if (re < 0) {
								re = 0.0;
							}
							else if (re > 1) {
								re = 1.0;
							}
							float res = pow(re, render->spec);
							specular[RED] += render->lights[l].color[RED] * res;
							specular[GREEN] += render->lights[l].color[GREEN] * res;
							specular[BLUE] += render->lights[l].color[BLUE] * res;

							diffuse[RED] += render->lights[l].color[RED] * nl;
							diffuse[GREEN] += render->lights[l].color[GREEN] * nl;
							diffuse[BLUE] += render->lights[l].color[BLUE] * nl;
						}
						//ambient light
						GzColor ambient = { 0,0,0 };
						ambient[RED] = render->ambientlight.color[RED];
						ambient[GREEN] = render->ambientlight.color[GREEN];
						ambient[BLUE] = render->ambientlight.color[BLUE];

						//final
						if (render->tex_fun != NULL) {
							color[RED] = render->Ks[RED] * specular[RED] + texcol[RED] * diffuse[RED] + texcol[RED] * ambient[RED];
							color[GREEN] = render->Ks[GREEN] * specular[GREEN] + texcol[GREEN] * diffuse[GREEN] + texcol[GREEN] * ambient[GREEN];
							color[BLUE] = render->Ks[BLUE] * specular[BLUE] + texcol[BLUE] * diffuse[BLUE] + texcol[BLUE] * ambient[BLUE];
						}
						else {
							color[RED] = render->Ks[RED] * specular[RED] + render->Kd[RED] * diffuse[RED] + render->Ka[RED] * ambient[RED];
							color[GREEN] = render->Ks[GREEN] * specular[GREEN] + render->Kd[GREEN] * diffuse[GREEN] + render->Ka[GREEN] * ambient[GREEN];
							color[BLUE] = render->Ks[BLUE] * specular[BLUE] + render->Kd[BLUE] * diffuse[BLUE] + render->Ka[BLUE] * ambient[BLUE];
						}
					}
					else {
						memcpy(color, verColor[0], sizeof(GzColor));
					}

					for (int c = 0; c < 3; c++) {
						if (color[c] < 0) {
							color[c] = 0;
						}
						else if (color[c] > 1) {
							color[c] = 1;
						}
					}


					if (GzPutDisplay(render->display, x, y, ctoi(color[RED]), ctoi(color[GREEN]), ctoi(color[BLUE]), 1, z) == GZ_FAILURE)
						return GZ_FAILURE;
				}
			}
		}


	return GZ_SUCCESS;
}

//a function to calculate coaf of line formula
void cal_coef(float v1[3], float v2[3], float coaf[3]) {
	float dx = v1[X] - v2[X];
	float dy = v1[Y] - v2[Y];

	coaf[0] = dy;
	coaf[1] = -dx;
	coaf[2] = dx*v1[Y] - dy*v1[X];
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

void normalize(float v[3]) {
	float sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += v[i]*v[i];
	}
	v[0] /= sqrt(sum);
	v[1] /= sqrt(sum);
	v[2] /= sqrt(sum);
}

void interpolate(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float* res) {
	float dx1 = x1 - x2;
	float dy1 = y1 - y2;
	float dz1 = z1 - z2;
	float dx2 = x2 - x3;
	float dy2 = y2 - y3;
	float dz2 = z2 - z3;

	float A = dy1*dz2 - dy2*dz1;
	float B = dz1*dx2 - dz2*dx1;
	float C = dx1*dy2 - dx2*dy1;
	float D = -A*x3 - B*y3 - C*z3;

	res[0] = A;
	res[1] = B;
	res[2] = C;
	res[3] = D;
}
