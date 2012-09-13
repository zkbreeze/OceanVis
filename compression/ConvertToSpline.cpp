#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#include "ConvertToSpline.h"

#define OneSix3 1.0/216.0
#define OneSix2 2.0/108.0
#define OneSix1 4.0/54.0
#define OneSix0 8.0/27.0


#define CVC_DBL_EPSILON 1.0e-9f


using namespace std;

// New conversion code by Xuguo
/*-----------------------------------------------------------------------------*/
void ConvertToInterpolationCoefficients_1D(float *s, int DataLength, float EPSILON) 
//		float	*s,		/* input samples --> output coefficients */
//		int	DataLength,	/* number of samples or coefficients     */
//		float	EPSILON		/* admissible relative error             */

{
int   i, n, ni, ni1, K;
float sum, z1, w1, w2;

n = DataLength + 1;
z1 = sqrt(3.0) - 2.0;
K = log(EPSILON)/log(fabs(z1));
//printf("K = %i\n", K);

// compute initial value s(0)
sum = 0.0;
w2 = pow(z1, 2*n);
if (n < K) {
   for (i = 1; i < n; i++){
      w1 = pow(z1, i);
      sum = sum + s[i-1]*(w1 - w2/w1);
   }
} else {
   for (i = 1; i < n; i++){
      sum = (sum + s[n- i-1])*z1;
   }
}
sum = -sum/(1.0 - w2);


// compute c^{+}
n = DataLength;
s[0]  = s[0] + z1*sum;
for (i = 1; i < n; i++) {
   s[i]  = s[i] + z1*s[i-1];
   //printf("cp[%i] = %e, %f \n", i, cp[i], z1);
}

// compute c^- 
s[n-1] = -z1*s[n-1];
for (i = 1; i < n; i++) {
   ni = n - i; 
   ni1 = ni - 1;
   s[ni1]  = z1*(s[ni] - s[ni1]);
}

for (i = 0; i < n; i++) {
   s[i]  = 6.0*s[i];
}

}




/*-----------------------------------------------------------------------------*/
void ConvertToInterpolationCoefficients_2D(float *s, int nx, int ny, float EPSILON) 
//		float	*s,     /* input samples --> output coefficients               */
//		int	nx,	/* number of samples or coefficients in x direction    */
//		int	ny,	/* number of samples or coefficients in y direction    */
//		float	EPSILON	/* admissible relative error                           */
{
float *d, z1;
int    i, l;

d = (float *) malloc (nx*sizeof (float)); 

z1 = sqrt(3.0) - 2.0;
// x-direction interpolation
for (l = 0; l < ny; l++) {
   for (i = 0; i < nx; i++) {
      d[i] = s[i*ny + l];
   }
   ConvertToInterpolationCoefficients_1D(d, nx,CVC_DBL_EPSILON);
   //ConvertToInterpolationCoefficients_Qu(d, nx,&z1, 1, CVC_DBL_EPSILON);


   for (i = 0; i < nx; i++) {
      s[i*ny + l] = d[i];
   }
}

// y-direction interpolation
for (i = 0; i < nx; i++) {
   ConvertToInterpolationCoefficients_1D(s+i*ny, ny,CVC_DBL_EPSILON);
   //ConvertToInterpolationCoefficients_Qu(s+i*ny, ny, &z1, 1, CVC_DBL_EPSILON);

}

free(d);
}


/*-----------------------------------------------------------------------------*/
void ConvertToInterpolationCoefficients_3D(float *s, int nx, int ny, 
                                                     int nz,   float EPSILON)
//              float   *s,    /* input samples --> output coefficients               */
//              int     nx,    /* number of samples or coefficients in x direction    */
//              int     ny,    /* number of samples or coefficients in y direction    */
//              int     nz,    /* number of samples or coefficients in z direction    */
//              float   EPSILON/* admissible relative error                           */
{
float *d, z1;
int    u, v, w, k, kk;

d = (float *) malloc (nx*sizeof (float));

k = ny*nz;
z1 = sqrt(3.0) - 2.0;

// x-direction interpolation
for (v = 0; v < ny; v++) {
   for (w = 0; w < nz; w++) {
      kk = v*nz + w;
      for (u = 0; u < nx; u++) {
         d[u] = s[u*k + kk];
      }
      ConvertToInterpolationCoefficients_1D(d, nx,CVC_DBL_EPSILON);
      //ConvertToInterpolationCoefficients_Qu(d, nx,&z1, 1, CVC_DBL_EPSILON);


      for (u = 0; u < nx; u++) {
         s[u*k + kk] = d[u];
      }
   }
}

for (u = 0; u < nx; u++) {
   ConvertToInterpolationCoefficients_2D(s+u*k, ny, nz, CVC_DBL_EPSILON);
}

free(d);
}





/*-----------------------------------------------------------------------------*/
float TakeACoefficient_Fast(float *c, int nx, int ny, int nz, int u, int v, int w)
{
    return(c[(u*ny + v)*nz + w]);
}


/*-----------------------------------------------------------------------------*/
float TakeACoefficient_Slow(float *c, int nx, int ny, int nz, int u, int v, int w)
{
float result;

result = 0.0;
if (( u  >= 0  && u < nx) &&
    ( v  >= 0  && v < ny) &&
    ( w  >= 0  && w < nz) ) {

    result = c[(u*ny + v)*nz + w];
}

return(result);
}




/*-----------------------------------------------------------------------------*/
float   EvaluateCubicSplineAtGridPoint(float *c, int nx, int ny, int nz, int u, int v, int w)
//              float   *c,    /* the spline  coefficients                            */
//              int     nx,    /* number of samples or coefficients in x direction    */
//              int     ny,    /* number of samples or coefficients in y direction    */
//              int     nz,    /* number of samples or coefficients in z direction    */
{
float result;

if (( u  > 0  && u < nx-1) &&
    ( v  > 0  && v < ny-1) &&
    ( w  > 0  && w < nz-1) ) {

    result = OneSix3*(TakeACoefficient_Fast(c, nx, ny, nz, u-1,v-1,w-1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u-1,v-1,w+1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u-1,v+1,w+1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u-1,v+1,w-1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u+1,v-1,w-1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u+1,v-1,w+1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u+1,v+1,w+1) +
                      TakeACoefficient_Fast(c, nx, ny, nz, u+1,v+1,w-1)); 

   result = OneSix2*(TakeACoefficient_Fast(c, nx, ny, nz, u,v-1,w-1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v-1,w+1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v+1,w-1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v+1,w+1) 
                     +
                     TakeACoefficient_Fast(c, nx, ny, nz, u-1,v,w-1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u-1,v,w+1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u+1,v,w-1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u+1,v,w+1) 
                     +
                     TakeACoefficient_Fast(c, nx, ny, nz, u-1,v-1,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u-1,v+1,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u+1,v-1,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u+1,v+1,w)) + result; 


   result = OneSix1*(TakeACoefficient_Fast(c, nx, ny, nz, u,v,w-1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v,w+1) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v-1,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u,v+1,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u-1,v,w) +
                     TakeACoefficient_Fast(c, nx, ny, nz, u+1,v,w)) + result;

   result = OneSix0*TakeACoefficient_Fast(c, nx, ny, nz, u,v,w) + result;
   return(result);
} 

    result = OneSix3*(TakeACoefficient_Slow(c, nx, ny, nz, u-1,v-1,w-1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u-1,v-1,w+1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u-1,v+1,w+1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u-1,v+1,w-1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u+1,v-1,w-1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u+1,v-1,w+1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u+1,v+1,w+1) +
                      TakeACoefficient_Slow(c, nx, ny, nz, u+1,v+1,w-1)); 

   result = OneSix2*(TakeACoefficient_Slow(c, nx, ny, nz, u,v-1,w-1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v-1,w+1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v+1,w-1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v+1,w+1) 
                     +
                     TakeACoefficient_Slow(c, nx, ny, nz, u-1,v,w-1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u-1,v,w+1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u+1,v,w-1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u+1,v,w+1) 
                     +
                     TakeACoefficient_Slow(c, nx, ny, nz, u-1,v-1,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u-1,v+1,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u+1,v-1,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u+1,v+1,w)) + result; 


   result = OneSix1*(TakeACoefficient_Slow(c, nx, ny, nz, u,v,w-1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v,w+1) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v-1,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u,v+1,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u-1,v,w) +
                     TakeACoefficient_Slow(c, nx, ny, nz, u+1,v,w)) + result;

   result = OneSix0*TakeACoefficient_Slow(c, nx, ny, nz, u,v,w) + result;
   return(result);

}

void EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint(float* c, int nx, int ny, int nz, int u, int v, int w, float* p, float* partials,float* minExtent, float* maxExtent,  float* dxyz)
// float *c, /* the spline coefficients */
// int nx, /* number of samples or coefficients in x direction */
// int ny, /* number of samples or coefficients in y direction */
// int nz, /* number of samples or coefficients in z direction */
// int	u, v, w,	/* the indices of the lower left corner of the bounding box */
// float p[3],	/* the given point, be careful if any transformation is done	*/
// float *partial /* partial derivatives */
//		The minExtent[3] and maxExtent[3] in HLevelSet.h are the bounding box of the molecule; dx, dy, dz are the grid length.
{
	float dx = dxyz[2];
	float dy = dxyz[1];
	float dz = dxyz[0];
	float c64[64], indx, indy, indz;
	float TensorF_p[64], TensorFx_p[64], TensorFy_p[64], TensorFz_p[64];
	int i;
	Take_64_Coefficients(c, nx, ny, nz, u, v, w, c64);
	ComputeTensorForGivenPoint(nx, ny, nz, u, v, w, p, TensorF_p, TensorFx_p, TensorFy_p, TensorFz_p,minExtent, maxExtent,dxyz);
	for(i = 0; i < 4; i++)
	{
		partials[i] = 0.0;
	}
	for(i = 0; i < 64; i++)
	{
	//	cout<<"c64[] "<<i<<", " <<c64[i]<<", "<< TensorF_p[i]<<endl;
		partials[0] = partials[0] + c64[i]*TensorF_p[i];
		partials[1] = partials[1] + c64[i]*TensorFx_p[i];
		partials[2] = partials[2] + c64[i]*TensorFy_p[i];
		partials[3] = partials[3] + c64[i]*TensorFz_p[i];
	}
	indx = 1.0/dx;
	indy = 1.0/dy;
	indz = 1.0/dz;
	partials[1] = partials[1] * indx;
	partials[2] = partials[2] * indy;
	partials[3] = partials[3] * indz;

//	cout<<"partial[0,1,2,3]:" << partials[0] <<" " << partials[1]<<" "<<partials[2]<<" " <<partials[3]<<endl;
}

void ComputeTensorForGivenPoint(int nx, int ny, int nz, int u, int v, int w, float* p, float* TensorF_p, float* TensorFx_p, float* TensorFy_p, float* TensorFz_p,float* minExtent, float* maxExtent, float* dxyz)

// int nx, /* number of samples or coefficients in x direction */
// int ny, /* number of samples or coefficients in y direction */
// int nz, /* number of samples or coefficients in z direction */
{
	float f1[4], f2[4], f3[4];
	float f1x[4], f2x[4], f3x[4];
	float f1xx[4], f2xx[4], f3xx[4];
	int i, j, k, l;
	float x, y, z;
	float dx = dxyz[2];
	float dy = dxyz[1];
	float dz = dxyz[0];
	l = 0;
	if((u > 0 && u < nx-2) &&
			(v > 0 && v < ny-2) &&
			(w > 0 && w < nz-2))
	{
		x = p[0] - (minExtent[0] + (u-1)*dx);
		x = x/dx;
		f1[0] = (2.0 - x)*(2.0 - x)*(2.0 - x)/6.0;
		f1x[0] = -0.5*(2.0 - x)*(2.0 - x);
		x = p[0] - (minExtent[0] + u*dx);
		x = x/dx;
		f1[1] = 2.0/3.0 - x*x + 0.5*x*x*x;
		f1x[1] = -2.0*x + 1.5*x*x;
		x = minExtent[0] + (u+1)*dx - p[0];
		x = x/dx;
		f1[2] = 2.0/3.0 - x*x + 0.5*x*x*x;
		f1x[2] = 2.0*x - 1.5*x*x;
		x = minExtent[0] + (u+2)*dx - p[0];
		x = x/dx;
		f1[3] = (2.0 - x)*(2.0 - x)*(2.0 - x)/6.0;
		f1x[3] = 0.5*(2.0 - x)*(2.0 - x);
		y = p[1] - (minExtent[1] + (v-1)*dy);
		y = y/dy;
		f2[0] = (2.0 - y)*(2.0 - y)*(2.0 - y)/6.0;
		f2x[0] = -0.5*(2.0 - y)*(2.0 - y);
		y = p[1] - (minExtent[1] + v*dy);
		y = y/dy;
		f2[1] = 2.0/3.0 - y*y + 0.5*y*y*y;
		f2x[1] = -2.0*y + 1.5*y*y;
		y = minExtent[1] + (v+1)*dy - p[1];
		y = y/dy;
		f2[2] = 2.0/3.0 - y*y + 0.5*y*y*y;
		f2x[2] = 2.0*y - 1.5*y*y;
		y = minExtent[1] + (v+2)*dy - p[1];
		y = y/dy;
		f2[3] = (2.0 - y)*(2.0 - y)*(2.0 - y)/6.0;
		f2x[3] = 0.5*(2.0 - y)*(2.0 - y);
		z = p[2] - (minExtent[2] + (w-1)*dz);
		z = z/dz;
		f3[0] = (2.0 - z)*(2.0 - z)*(2.0 - z)/6.0;
		f3x[0] = -0.5*(2.0 - z)*(2.0 - z);
		z = p[2] - (minExtent[2] + w*dz);
		z = z/dz;
		f3[1] = 2.0/3.0 - z*z + 0.5*z*z*z;
		f3x[1] = -2.0*z + 1.5*z*z;
		z = minExtent[2] + (w+1)*dz - p[2];
		z = z/dz;
		f3[2] = 2.0/3.0 - z*z + 0.5*z*z*z;
		f3x[2] = 2.0*z - 1.5*z*z;
		z = minExtent[2] + (w+2)*dz - p[2];
		z = z/dz;
		f3[3] = (2.0 - z)*(2.0 - z)*(2.0 - z)/6.0;
		f3x[3] = 0.5*(2.0 - z)*(2.0 - z);
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				for(k = 0; k < 4; k++)
				{
					TensorF_p[l] = f1[i] * f2[j] * f3[k];
					TensorFx_p[l] = f1x[i] * f2[j] * f3[k];
					TensorFy_p[l] = f1[i] * f2x[j] * f3[k];
					TensorFz_p[l] = f1[i] * f2[j] * f3x[k];
					l = l + 1;
				}
			}
		}
		return;
	}
	x = minExtent[0] + (u-1)*dx;
	if(x >= minExtent[0] && x <= maxExtent[0])
	{
		x = p[0] - x;
		x = x/dx;
		f1[0] = (2.0 - x)*(2.0 - x)*(2.0 - x)/6.0;
		f1x[0] = -0.5*(2.0 - x)*(2.0 - x);
	}
	else
	{
		f1[0] = 0.0;
		f1x[0] = 0.0;
	}
	x = minExtent[0] + u*dx;
	if(x >= minExtent[0] && x <= maxExtent[0])
	{
		x = p[0] - x;
		x = x/dx;
		f1[1] = 2.0/3.0 - x*x + 0.5*x*x*x;
		f1x[1] = -2.0*x + 1.5*x*x;
	}
	else
	{
		f1[1] = 0.0;
		f1x[1] = 0.0;
	}
	x = minExtent[0] + (u+1)*dx;
	if(x >= minExtent[0] && x <= maxExtent[0])
	{
		x = x - p[0];
		x = x/dx;
		f1[2] = 2.0/3.0 - x*x + 0.5*x*x*x;
		f1x[2] = 2.0*x - 1.5*x*x;
	}
	else
	{
		f1[2] = 0.0;
		f1x[2] = 0.0;
	}
	x = minExtent[0] + (u+2)*dx;
	if(x >= minExtent[0] && x <= maxExtent[0])
	{
		x = x - p[0];
		x = x/dx;
		f1[3] = (2.0 - x)*(2.0 - x)*(2.0 - x)/6.0;
		f1x[3] = 0.5*(2.0 - x)*(2.0 - x);
	}
	else
	{
		f1[3] = 0.0;
		f1x[3] = 0.0;
	}
	// y
	y = minExtent[1] + (v-1)*dy;
	if(y >= minExtent[1] && y <= maxExtent[1])
	{
		y = p[1] - y;
		y = y/dy;
		f2[0] = (2.0 - y)*(2.0 - y)*(2.0 - y)/6.0;
		f2x[0] = -0.5*(2.0 - y)*(2.0 - y);
	}
	else
	{
		f2[0] = 0.0;
		f2x[0] = 0.0;
	}
	y = minExtent[1] + v*dy;
	if(y >= minExtent[1] && y <= maxExtent[1])
	{
		y = p[1] - y;
		y = y/dy;
		f2[1] = 2.0/3.0 - y*y + 0.5*y*y*y;
		f2x[1] = -2.0*y + 1.5*y*y;
	}
	else
	{
		f2[1] = 0.0;
		f2x[1] = 0.0;
	}
	y = minExtent[1] + (v+1)*dy;
	if(y >= minExtent[1] && y <= maxExtent[1])
	{
		y = y - p[1];
		y = y/dy;
		f2[2] = 2.0/3.0 - y*y + 0.5*y*y*y;
		f2x[2] = 2.0*y - 1.5*y*y;
	}
	else
	{
		f2[2] = 0.0;
		f2x[2] = 0.0;
	}
	y = minExtent[1] + (v+2)*dy;
	if(y >= minExtent[1] && y <= maxExtent[1])
	{
		y = y - p[1];
		y = y/dy;
		f2[3] = (2.0 - y)*(2.0 - y)*(2.0 - y)/6.0;
		f2x[3] = 0.5*(2.0 - y)*(2.0 - y);
	}
	else
	{
		f2[3] = 0.0;
		f2x[3] = 0.0;
	}
	// z
	z = minExtent[2] + (w-1)*dz;
	if(z >= minExtent[2] && z <= maxExtent[2])
	{
		z = p[2] - z;
		z = z/dz;
		f3[0] = (2.0 - z)*(2.0 - z)*(2.0 - z)/6.0;
		f3x[0] = -0.5*(2.0 - z)*(2.0 - z);
	}
	else
	{
		f3[0] = 0.0;
		f3x[0] = 0.0;
	}
	z = minExtent[2] + w*dz;
	if(z >= minExtent[2] && z <= maxExtent[2])
	{
		z = p[2] - z;
		z = z/dz;
		f3[1] = 2.0/3.0 - z*z + 0.5*z*z*z;
		f3x[1] = -2.0*z + 1.5*z*z;
	}
	else
	{
		f3[1] = 0.0;
		f3x[1] = 0.0;
	}
	z = minExtent[2] + (w+1)*dz;
	if(z >= minExtent[2] && z <= maxExtent[2])
	{
		z = z - p[2];
		z = z/dz;
		f3[2] = 2.0/3.0 - z*z + 0.5*z*z*z;
		f3x[2] = 2.0*z - 1.5*z*z;
	}
	else
	{
		f3[2] = 0.0;
		f3x[2] = 0.0;
	}
	z = minExtent[2] + (w+2)*dz;
	if(z >= minExtent[2] && z <= maxExtent[2])
	{
		z = z - p[2];
		z = z/dz;
		f3[3] = (2.0 - z)*(2.0 - z)*(2.0 - z)/6.0;
		f3x[3] = 0.5*(2.0 - z)*(2.0 - z);
	}
	else
	{
		f3[3] = 0.0;
		f3x[3] = 0.0;
	}
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			for(k = 0; k < 4; k++)
			{
				TensorF_p[l] = f1[i] * f2[j] * f3[k];
				TensorFx_p[l] = f1x[i] * f2[j] * f3[k];
				TensorFy_p[l] = f1[i] * f2x[j] * f3[k];
				TensorFz_p[l] = f1[i] * f2[j] * f3x[k];
				l = l + 1;
			}
		}
	}
	return;
}

void Take_64_Coefficients(float* c, int nx, int ny, int nz, int u, int v, int w, float* c64)
// float *c, /* the spline coefficients */
// int nx, /* number of samples or coefficients in x direction */
// int ny, /* number of samples or coefficients in y direction */
// int nz, /* number of samples or coefficients in z direction */
// float *c64 /* 64 coefficients */
{
	int i, j, k, l;
	if((u > 0 && u < nx-2) &&
			(v > 0 && v < ny-2) &&
			(w > 0 && w < nz-2))
	{
		l = 0;
		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 4; j++)
			{
				for(k = 0; k < 4; k++)
				{
					c64[l] = TakeACoefficient_Fast(c, nx, ny, nz, u-1+i,v-1+j,w-1+k);
					l = l + 1;
				}
			}
		}
		return;
	}
	l = 0;
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			for(k = 0; k < 4; k++)
			{
				c64[l] = TakeACoefficient_Slow(c, nx, ny, nz, u-1+i,v-1+j,w-1+k);
				l = l + 1;
			}
		}
	}
}


