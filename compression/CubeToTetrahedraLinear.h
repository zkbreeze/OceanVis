
#ifndef KVS__CUBE_TO_TETRAHEDRA_LINEAR_H_INCLUDE
#define KVS__CUBE_TO_TETRAHEDRA_LINEAR_H_INCLUDE

#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/FilterBase>
#include <kvs/ClassName>
#include <kvs/Module>
#include"ConvertToSpline.h"

namespace kvs
{

class CubeToTetrahedraLinear : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    // Class name.
    kvsClassName( kvs::CubeToTetrahedraLinear );

    // Module information.
    kvsModuleCategory( Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );
    
    //block_size
    size_t block_size;

public:

    CubeToTetrahedraLinear( const kvs::StructuredVolumeObject* volume, size_t i );

public:

    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
};

} // end of namespace kvs

#endif // KVS__CUBE_TO_TETRAHEDRA_H_INCLUDE
