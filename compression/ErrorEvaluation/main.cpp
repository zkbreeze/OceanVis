#include <kvs/TetrahedralCell>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/UnstructuredVolumeImporter>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/glew/RayCastingRenderer>
#include <kvs/glut/Screen>
#include <kvs/glut/Application>
#include "BlockLoader.h"

double valid_opacity = 30.0/256.0;

void CalcBoundingBox( kvs::TetrahedralCell<float>* cell, kvs::Vector3f& min_c, kvs::Vector3f& max_c )
{
    kvs::Vector3f c[4];
    for ( int i = 0; i < 4; ++i )
    {
        c[i] = cell->vertices()[i];
    }

    min_c = c[0];
    max_c = c[0];
    for ( int i = 1; i < 4; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            min_c[j] = kvs::Math::Min( min_c[j], c[i][j] );
            max_c[j] = kvs::Math::Max( max_c[j], c[i][j] );
        }
    }
}

int main( int argc, char** argv )
{
    //kvs::glut::Application app( argc, argv );
    kvs::StructuredVolumeObject* ori_volume = new kvs::StructuredVolumeImporter( argv[1] );
    size_t nx = ori_volume->resolution().x();
    size_t ny = ori_volume->resolution().y();
    size_t nz = ori_volume->resolution().z();
    
    kvs::UnstructuredVolumeObject* eva_volume = new kvs::BlockLoader( argv[2] );
    float max_value = (float)ori_volume->maxValue();
    float min_value = (float)ori_volume->minValue();
    std::cout << "max valume of the compressed volume:" << max_value << std::endl;
    std::cout << "min valume of the compressed volume:" << min_value << std::endl;
    
    float* ori_values = new float[nx * ny * nz];
    ori_values = (float*)ori_volume->values().pointer();
    
    size_t NCells = eva_volume->ncells();
    kvs::TetrahedralCell<float>* cell = new kvs::TetrahedralCell<float>( eva_volume );
    
    kvs::ValueArray<float> eva_values;
    eva_values.allocate( nx * ny * nz );
    
    for ( size_t count = 0; count < NCells ; count++ )
    {
        cell->bindCell( count );
    
        kvs::Vector3f minf, maxf;
        kvs::Vector3i mini, maxi;
        CalcBoundingBox( cell, minf, maxf );
        
        for ( int a = 0; a < 3; ++a )
        {
            mini[a] = kvs::Math::Floor( minf[a] ); 
            maxi[a] = kvs::Math::Ceil( maxf[a] ); 
        }
        
        if ( mini.x() < 0 || maxi.x() > nx ) std::cout << "x out" << std::endl;
        if ( mini.y() < 0 || maxi.y() > ny ) std::cout << "y out" << std::endl;
        if ( mini.z() < 0 || maxi.z() > nz ) std::cout << "z out" << std::endl;
        
        for ( int k = mini.z(); k <= maxi.z(); k++ )
            for ( int j = mini.y(); j <= maxi.y(); j ++ )
                for ( int i = mini.x(); i <= maxi.x(); i ++ )
                {
                    cell->setGlobalPoint( kvs::Vector3f( i, j, k ) );
                    kvs::Vector3f local = cell->localPoint();
                    if ( !(local.x() < 0 || local.y() < 0 || local.z() < 0 || 1 - local.x() + local.y() + local.z() < 0 ) )
                    {
                        int index = i + j * nx + k * nx * ny;
                        eva_values[ index ] = cell->scalar();
                    }
                }
    }
    
//    //check the interpolation division volume
//    kvs::glut::Screen screen( &app );
//    kvs::Vector3ui resolution( nx, ny, nz );
//    kvs::AnyValueArray interpolation_values( eva_values );
//    kvs::StructuredVolumeObject* interpolation_volume = new kvs::StructuredVolumeObject(
//                                                                                        resolution,
//                                                                                        1,
//                                                                                        interpolation_values
//                                                                                        );
//    interpolation_volume->updateMinMaxCoords();
//    interpolation_volume->updateMinMaxValues();
//
//    kvs::glew::RayCastingRenderer* renderer = new kvs::glew::RayCastingRenderer();
//    screen.registerObject( interpolation_volume, renderer );
//    screen.show();    
//    return( app.run());
    
    //Calculate the error
    float error = 0;
    float error_e = 0; //efficient error
    float average_error = 0;
    float average_error_e = 0;
    size_t count_e = 0;
    
    double valid_value = min_value + ( ( max_value - min_value) * valid_opacity );
    std::cout << "valid value: " << valid_value << std::endl;
    for ( long long i = 0; i < nx * ny * nz; i ++ )
    {
        // efficient error, not needed for the ocean data
        if(ori_values[i] >= valid_value)
        {
            error_e += fabs(eva_values[i] - ori_values[i]) / ( max_value - min_value ) ;
            count_e ++;
        }    
        error += fabs( (eva_values[i] - ori_values[i]) / max_value ) ;
    }
//    average_error = error/(nx * ny * nz);
    average_error_e = error_e / count_e;
    average_error = error / ( nx * ny * nz );
    
    std::cout << "whole error: " << error << std::endl;
    std::cout << "whole error_e: " << error_e << std::endl;
    std::cout << "The average error is: " << average_error << std::endl;
    std::cout << "The average error_e is:" << average_error_e << std::endl;
    std::cout << "RMSE: " << std::sqrt( average_error ) << std::endl;
}
