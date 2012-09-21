#include <iostream>
#include <fstream>
#include <kvs/StructuredVolumeObject>
#include "LinearInterpolator.cpp"

using namespace std;

kvs::StructuredVolumeObject* Interpolator( kvs::StructuredVolumeObject* object, float min, float max )
{
    size_t nx_ori = object->resolution().x();
    size_t ny_ori = object->resolution().y();
    size_t nz_ori = object->resolution().z();
    size_t nx = nx_ori;
    size_t ny = ny_ori;
    size_t nz = 200;
    
    float* pvalues = (float*)object->values().pointer();
    // value processing
    unsigned int n = object->nnodes();
    for ( size_t i = 0; i < n ; i++ )
    {
        if ( pvalues[i] < min ) pvalues[i] = min;
        if ( pvalues[i] > max ) pvalues[i] = max;
    }
    
    // read the depth from outside
    ifstream infile( "depth.txt" );    
    float* index = new float[78];
    float* depth = new float[78];
    float* dz = new float[78];
    for ( size_t i = 0; i < 78; i ++ )
    {
        infile >> index[i];
        infile >> depth[i];
        infile >> dz[i];
    }

    std::cout << nz_ori << std::endl;
    
    // interpolate the value linearly
    // build the interpolator
    LinearInterpolator interp[ nx * ny ];
    for ( size_t k = 11; k < nz_ori; k ++ )
        for ( size_t j = 0; j < ny_ori; j ++ )
            for ( size_t i = 0; i < nx_ori; i ++ )
            {
                size_t index = i + j * nx + k * nx * ny;
                interp[i + j * nx_ori].addDataPoint( depth[nz_ori - 1 - k], pvalues[index] );
            }
    // interpolate the value
    kvs::AnyValueArray values;
    float* buf = static_cast<float*>( values.allocate<float>( nx * ny * nz ) );
    for ( size_t k = 0; k < nz; k ++ )
        for ( size_t j = 0; j < ny; j ++ )
            for ( size_t i = 0; i < nx; i ++ )
            {
                size_t index = i + j * nx + k * nx * ny;
                float depth_new = 997.0 - k * 5.0;
                buf[index] = interp[i + j * nx].interpolate( depth_new );
            }
    
    kvs::Vector3ui resolution( nx, ny, nz );
    kvs::VolumeObjectBase::GridType grid_type = kvs::VolumeObjectBase::Uniform;
    kvs::StructuredVolumeObject* t_object = new kvs::StructuredVolumeObject();
    t_object->setGridType( grid_type);
    t_object->setVeclen( 1 );
    t_object->setResolution( resolution );
    t_object->setValues( values );
    
    t_object->updateMinMaxCoords();
    t_object->updateMinMaxValues();
    
    return ( t_object );
}
