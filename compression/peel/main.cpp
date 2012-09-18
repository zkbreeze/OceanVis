//
//  main.cpp
//  
//
//  Created by Kun Zhao on 12/09/18.
//  Copyright (c) 2012 Kyoto University. All rights reserved.
//

#include <iostream>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/KVSMLObjectStructuredVolume>
#include <kvs/StructuredVolumeExporter>

void WriteKVSML( kvs::StructuredVolumeObject* object, std::string filename )
{
    kvs::KVSMLObjectStructuredVolume* kvsml = new kvs::StructuredVolumeExporter<kvs::KVSMLObjectStructuredVolume>( object );
    kvsml->setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
    kvsml->write( filename );
}


int main( int argc, char** argv )
{
    kvs::StructuredVolumeObject* object = new kvs::StructuredVolumeImporter( argv[1] );
    size_t nx = object->resolution().x();
    size_t ny = object->resolution().y();
    size_t nz = object->resolution().z();
    std::cout << nx << std::endl;
    std::cout << ny << std::endl;
    std::cout << nz << std::endl;
    
    float* pvalues = (float*)object->values().pointer();
    
    size_t nx_new = nx - 2;
    size_t ny_new = ny - 2;
    size_t nz_new = nz - 2;
    
    kvs::AnyValueArray values;
    float* buf = static_cast<float*>( values.allocate<float>( nx_new * ny_new * nz_new) );
    for ( size_t k = 0; k < nz_new; k ++ )
        for ( size_t j = 0; j < ny_new; j ++ )
            for ( size_t i = 0; i < nx_new; i ++ )
            {
                size_t index = i + j * nx_new + k * nx_new * ny_new;
                size_t index_ori = ( i + 1 ) + ( j + 1 ) * nx + ( k + 1 ) * nx * ny;
                buf[index] = pvalues[index_ori];
//                std::cout << index << std::endl;
//                std::cout << index_ori << std::endl;
            }
    
    kvs::Vector3ui resolution( nx_new, ny_new, nz_new );
    kvs::VolumeObjectBase::GridType grid_type = kvs::VolumeObjectBase::Uniform;
    kvs::StructuredVolumeObject* volume = new kvs::StructuredVolumeObject();
    volume->setGridType( grid_type);
    volume->setVeclen( 1 );
    volume->setResolution( resolution );
    volume->setValues( values );
    
    volume->updateMinMaxCoords();
    volume->updateMinMaxValues();
    
    WriteKVSML( volume, "peel0315s.kvsml");
    
}