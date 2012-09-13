/*****************************************************************************/
/**
 *  @file   StructuredVolumeImporter.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/*****************************************************************************/
#include "StructuredVolumeImporter.h"
#include <kvs/Vector3>


namespace util
{

StructuredVolumeImporter::StructuredVolumeImporter(
    const std::string& filename,
    const size_t vindex,
    const size_t tindex,
    const bool zflip,
    const kvs::StructuredVolumeObject::GridType grid_type )
{
    kvs::GrADS* file = new kvs::GrADS( filename );
    this->import( file, vindex, tindex, zflip, grid_type );
    delete file;
}

StructuredVolumeImporter::StructuredVolumeImporter(
    const kvs::GrADS* file,
    const size_t vindex,
    const size_t tindex,
    const bool zflip,
    const kvs::StructuredVolumeObject::GridType grid_type )
{
    this->import( file, vindex, tindex, zflip, grid_type );
}

kvs::StructuredVolumeObject* StructuredVolumeImporter::exec( const kvs::FileFormatBase* file )
{
    const size_t vindex = 0;
    const size_t tindex = 0;
    const bool zflip = false;
    const kvs::StructuredVolumeObject::GridType grid_type = kvs::StructuredVolumeObject::Uniform;
    this->import( static_cast<const kvs::GrADS*>( file ), vindex, tindex, zflip, grid_type );
    return( this );
}

void StructuredVolumeImporter::import(
    const kvs::GrADS* file,
    const size_t vindex,
    const size_t tindex,
    const bool zflip,
    const kvs::StructuredVolumeObject::GridType grid_type )
{
    const size_t dimx = file->dataDescriptor().xdef().num;
    const size_t dimy = file->dataDescriptor().ydef().num;
    const size_t dimz = file->dataDescriptor().zdef().num;

    // Value array of the structured volume object.
    const size_t size = dimx * dimy * dimz;
    kvs::AnyValueArray values;
    float* pvalues = static_cast<float*>( values.allocate<float>( size * sizeof( float ) ) );

    // Copy (deep copy) scalar values to the value array from the GrADS datasets.
    float min_value = kvs::Value<float>::Max();
    float max_value = kvs::Value<float>::Min();
    const float ignore_value = file->dataDescriptor().undef().value;
    file->dataList().at( tindex ).load();
    {
        // src: head pointer to the scalar values specified by the sindex.
        const float* src = file->dataList().at(tindex).values().pointer() + size * vindex;
        for ( size_t i = 0; i < size; i++ )
        {
            pvalues[i] = src[i];

            // Calculate the min/max values in disregard for the undefined value.
            if ( !kvs::Math::Equal( src[i], ignore_value ) )
            {
                min_value = kvs::Math::Min( src[i], min_value );
                max_value = kvs::Math::Max( src[i], max_value );
            }
        }
    }
    file->dataList().at( tindex ).free();

    if ( zflip )
    {
        const size_t stride = dimx * dimy;
        const size_t end = dimz / 2;
        for ( size_t i = 0; i < end; i++ )
        {
            float* src = pvalues + ( i * stride );
            float* dst = pvalues + ( ( dimz - i - 1 ) * stride );
            for ( size_t j = 0; j < stride; j++ )
            {
                float tmp = *src;
                *src = *dst;
                *dst = tmp;
                src++;
                dst++;
            }
        }
    }

    setGridType( grid_type );
    setResolution( kvs::Vector3ui( dimx, dimy, dimz ) );
    setVeclen( 1 );
    setValues( values );
    setMinMaxValues( min_value, max_value );

    if ( grid_type == kvs::StructuredVolumeObject::Rectilinear )
    {
        // Coordinate array of the structured volume object (rectilinear grid).
        // NOTE: The coordinate values are stored in the array 'coords' as follows:
        //       coords = {x0,x1,...,xi,y0,y1,...,yj,z0,z1,...,zk}
        kvs::ValueArray<float> coords( dimx + dimy + dimz );

        float* xcoords = coords.pointer();
        switch ( file->dataDescriptor().xdef().mapping )
        {
        case kvs::grads::XDef::Levels:
        {
            const float* values = file->dataDescriptor().xdef().values.pointer();
            memcpy( xcoords, values, sizeof( float ) * dimx );
            break;
        }
        case kvs::grads::XDef::Linear:
        {
            kvs::Real32 value = file->dataDescriptor().xdef().values[0];
            const kvs::Real32 increment = file->dataDescriptor().xdef().values[1];
            for ( size_t i = 0; i < dimx; i++, value += increment ) xcoords[i] = value;
            break;
        }
        default: break;
        }

        float* ycoords = coords.pointer() + dimx;
        switch ( file->dataDescriptor().ydef().mapping )
        {
        case kvs::grads::YDef::Levels:
        {
            const float* values = file->dataDescriptor().ydef().values.pointer();
            memcpy( ycoords, values, sizeof( float ) * dimy );
            break;
        }
        case kvs::grads::YDef::Linear:
        {
            kvs::Real32 value = file->dataDescriptor().ydef().values[0];
            const kvs::Real32 increment = file->dataDescriptor().ydef().values[1];
            for ( size_t i = 0; i < dimy; i++, value += increment ) ycoords[i] = value;
            break;
        }
        default: break;
        }

        float* zcoords = coords.pointer() + dimx + dimy;
        switch ( file->dataDescriptor().zdef().mapping )
        {
        case kvs::grads::ZDef::Levels:
        {
            const float* values = file->dataDescriptor().zdef().values.pointer();
            memcpy( zcoords, values, sizeof( float ) * dimz );
            break;
        }
        case kvs::grads::ZDef::Linear:
        {
            kvs::Real32 value = file->dataDescriptor().zdef().values[0];
            const kvs::Real32 increment = file->dataDescriptor().zdef().values[1];
            for ( size_t i = 0; i < dimz; i++, value += increment ) zcoords[i] = value;
            break;
        }
        default: break;
        }

        setCoords( coords );
    }
    updateMinMaxCoords();
}

} // end of namespace util
