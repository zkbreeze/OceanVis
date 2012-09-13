#include <cmath>
#define CVC_DBL_EPSILON 1.0e-9f

float   EvaluateCubicSplineAtGridPoint(float *, int, int, int, int, int, int);
void    ConvertToInterpolationCoefficients_1D(float *, int, float);
void    ConvertToInterpolationCoefficients_2D(float *, int, int, float);
void    ConvertToInterpolationCoefficients_3D(float *, int, int, int, float);

float   TakeACoefficient_Fast(float *c, int nx, int ny, int nz, int u, int v, int w);
float   TakeACoefficient_Slow(float *c, int nx, int ny, int nz, int u, int v, int w);



void EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint(float* c, int nx, int ny, int nz, int u, int v, int w, float* p, float* partials, float* minExtent, float* maxExtent, float*);
void ComputeTensorForGivenPoint(int nx, int ny, int nz, int u, int v, int w, float* p, float* TensorF_p, float* TensorFx_p, float* TensorFy_p, float* TensorFz_p, float*, float*,float*);

void Take_64_Coefficients(float* c, int nx, int ny, int nz, int u, int v, int w, float* c64);


