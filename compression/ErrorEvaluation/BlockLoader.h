
#ifndef KVS__BLOCKLOADER_H_INCLUDE
#define KVS__BLOCKLOADER_H_INCLUDE

#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/FilterBase>
#include <kvs/ClassName>
#include <kvs/Module>

namespace kvs
{

class BlockLoader : public kvs::FilterBase, public kvs::UnstructuredVolumeObject
{
    // Class name.
    kvsClassName( kvs::BlockLoader );

    // Module information.
    kvsModuleCategory( Filter );
    kvsModuleBaseClass( kvs::FilterBase );
    kvsModuleSuperClass( kvs::UnstructuredVolumeObject );
    
    //block_size
    size_t block_size;
    size_t nx;
    size_t ny;
    size_t nz;
    float* ori_values;

public:

    BlockLoader( std::string filename );

public:

    kvs::ObjectBase* exec( const kvs::ObjectBase* object );
};

} // end of namespace kvs

#endif // KVS__CUBE_TO_TETRAHEDRA_H_INCLUDE
