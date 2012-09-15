#include "CubeToTetrahedraBspline.h"

namespace kvs
{

CubeToTetrahedraBspline::CubeToTetrahedraBspline( const kvs::StructuredVolumeObject* volume, size_t i )
{
    block_size = i;
    this->exec( volume );
}

kvs::ObjectBase* CubeToTetrahedraBspline::exec( const kvs::ObjectBase* object )
{
    const kvs::StructuredVolumeObject* volume = kvs::StructuredVolumeObject::DownCast( object );

//    std::cout << volume->values().typeInfo()->typeName() <<std::endl;
    const size_t ndivisions = 24;
    const kvs::Vector3ui ncells = volume->resolution() - kvs::Vector3ui( 1, 1, 1 );
    
    const int remainder_x = ncells.x() % block_size;
    const int remainder_y = ncells.y() % block_size;
    const int remainder_z = ncells.z() % block_size;
    
    const size_t block_x = ncells.x() / block_size + (bool)( remainder_x );  std::cout << "block_x: " << block_x <<std::endl;
    const size_t block_y = ncells.y() / block_size + (bool)( remainder_y );  std::cout << "block_y: " << block_y <<std::endl;
    const size_t block_z = ncells.z() / block_size + (bool)( remainder_z );  std::cout << "block_z: " << block_z <<std::endl;
    
    const size_t ncubes = block_x * block_y * block_z;
    const size_t nvertices = ( block_x + 1 ) * ( block_y + 1 ) * ( block_z + 1);
    std::cout << "nvertices: " << nvertices << std::endl;
    const size_t nnodes = nvertices + block_x * block_y * ( block_z + 1 ) + block_y * block_z * ( block_x + 1 ) + block_z * block_x * ( block_y + 1 ) + ncubes;
    std::cout << "nnodes: " << nnodes << std::endl;

    const size_t ntets = ncubes * ndivisions;
    std::cout << "ntets: " << ntets << std::endl;

    const kvs::UInt32 line_size = block_x + 1;
    std::cout<< "nnodesPerLine: " << line_size << std::endl;
    const kvs::UInt32 slice_size = ( block_x + 1 ) * ( block_y + 1 );
    std::cout<< "nnodesPerSlice: " << slice_size << std::endl;
    
    //Calculate the new value array
    float* ori_values = ( float* )volume->values().pointer();
    
    //Calculate the Bspline value with EvaluateCubicSplineAtGridPoint
    const size_t nx = volume->resolution().x();
    const size_t ny = volume->resolution().y();
    const size_t nz = volume->resolution().z();
    const size_t ori_nnodes = volume->nnodes();
    float* coefficient = new float[ori_nnodes];
    // convertion for Bspline
    size_t index_values = 0;
    size_t index_coefficient = 0;
    for( size_t i = 0; i < nx; i++ )
        for( size_t j = 0; j < ny; j++ )
            for( size_t k = 0; k < nz; k ++ )
            {
                index_coefficient = i * ny * nz + j * nz + k;
                index_values = k * nx * ny + j * nx + i;
                coefficient[index_coefficient] = ori_values[index_values];
            }
    
    //for( size_t  i = 0; i < ori_nnodes; i++ ) coefficient[i] = ori_values[i];
    ConvertToInterpolationCoefficients_3D( coefficient, nx, ny, nz, CVC_DBL_EPSILON );
    for ( size_t k = 0; k < nz ; k++ ) 
        for( size_t j = 0; j < ny; j++ )
            for( size_t i = 0; i < nx; i++ )
            {
                size_t index = i + j * nx + k * nx * ny;
                ori_values[index] = EvaluateCubicSplineAtGridPoint( coefficient, nx, ny, nz, i, j, k );
            }
    
    //Calculate the value and coord of every vertex
    kvs::AnyValueArray values;
    float* pvalues = static_cast<float*>( values.allocate<float>( nnodes ) );
    kvs::ValueArray<kvs::Real32> coords( nnodes * 3 );
    kvs::Real32* pcoords = coords.pointer();
    size_t count_x = 0;
    size_t count_y = 0;
    size_t count_z = 0;
    float coord_x = 0.0;
    float coord_y = 0.0;
    float coord_z = 0.0;
    size_t count = 0;

    for ( size_t k = 0; k < ( block_z + 1 ); k++ )
    {
        if ( k == block_z )
        {
            count_z = nx * ny * ncells.z();
            coord_z = static_cast<float>( ncells.z() );
        }
        for ( size_t j = 0; j < ( block_y + 1 ); j++ )
        {
            if ( j == block_y )
            {
                count_y = nx * ncells.y();
                coord_y = static_cast<float>( ncells.y() );
            }
            for ( size_t i = 0; i < ( block_x + 1 ); i++ )
            {
                if ( i == block_x )
                {
                    count_x = ncells.x();
                    coord_x = static_cast<float>( ncells.x() );
                }
                //values
                *(pvalues + (count++)) = *(ori_values + count_x + count_y + count_z );
                count_x += block_size;
                
                //coords
                *(pcoords++) = coord_x;   
                *(pcoords++) = coord_y;   
                *(pcoords++) = coord_z;
                coord_x += static_cast<float>( block_size );
            }
            count_x = 0;
            count_y += ( volume->resolution().x() ) * block_size;
            
            coord_x = 0.0;
            coord_y += static_cast<float>( block_size );
        }
        count_y = 0;
        count_z += ( volume->nnodesPerSlice() ) * block_size;
        
        coord_y = 0.0;
        coord_z += static_cast<float>( block_size );
    }
    size_t index_x = count;
    
//    //Build the basic coordinate of every vertex
//    kvs::ValueArray<kvs::Real32> coords( nnodes * 3 );
//    kvs::Real32* pcoords = coords.pointer();
//    for ( size_t k = 0; k < ( block_z + 1 ); k++ )
//    {
//        for ( size_t j = 0; j < ( block_y + 1 ); j++ )
//        {
//            for ( size_t i = 0; i < ( block_x + 1 ); i++ )
//            {
//                    const kvs::Real32 x = static_cast<kvs::Real32>( i );
//                    const kvs::Real32 y = static_cast<kvs::Real32>( j );
//                    const kvs::Real32 z = static_cast<kvs::Real32>( k );
//                    
//                    *(pcoords++) = x * block_size;   
//                    *(pcoords++) = y * block_size;   
//                    *(pcoords++) = z * block_size;
//            }
//        }
//    }
    
    //Evalute the value of the divided point with Bspline
    float* partials = new float[4];
    float* p = new float[3];
    float minCoord[3] = { 0.0, 0.0, 0.0 };
    float maxCoord[3] = { nx, ny, nz };
    float dxyz[3] = { 1, 1, 1 };
    
    int initial_index = (int)block_size/2;
    float initial_coord = (float)block_size/2.0;
    
    int edge_index_x = remainder_x ? (int)( remainder_x )/2 : initial_index;
    int edge_index_y = remainder_y ? (int)( remainder_y )/2 : initial_index;
    int edge_index_z = remainder_z ? (int)( remainder_z )/2 : initial_index;
    float edge_coord_x = remainder_x ? (float)( remainder_x )/2.0 : initial_coord;
    float edge_coord_y = remainder_y ? (float)( remainder_y )/2.0 : initial_coord;
    float edge_coord_z = remainder_z ? (float)( remainder_z )/2.0 : initial_coord;
    
    int temp_index_x = 0;
    int temp_index_y = 0;
    int temp_index_z = 0;
    float temp_coord_x = 0.0;
    float temp_coord_y = 0.0;
    float temp_coord_z = 0.0;
    
    //Calculate the center of the face z = 0, 1, 2 ....
//    temp_index_x = initial_index;
//    temp_index_y = initial_index;
//    temp_index_z = 0;
//    temp_coord_x = initial_coord;
//    temp_coord_y = initial_coord;
//    temp_coord_z = 0.0;
    
    for ( size_t k = 0; k < ( block_z + 1 ); k++ )
    {
        temp_index_z = 0;
        temp_coord_z = 0.0;
        
        if ( k == block_z )
        {
            temp_index_z = remainder_z ? ( remainder_z - (int)block_size ) : 0; std::cout << "temp_index_z: " << temp_index_z << std::endl;
            temp_coord_z = remainder_z ? (float)( remainder_z - (int)block_size ) : 0.0;  std::cout << "temp_coord_z: " << temp_coord_z << std::endl;
        }
        for ( size_t j = 0; j < ( block_y ); j++ )
        {
            temp_index_y = initial_index;
            temp_coord_y = initial_coord;

            if ( j == block_y - 1 )
            {
                temp_index_y = edge_index_y;
                temp_coord_y = edge_coord_y;
            }
            for ( size_t i = 0; i < ( block_x ); i++ )
            { 
                temp_index_x = initial_index;
                temp_coord_x = initial_coord;

                if ( i == block_x - 1 )
                {
                    temp_index_x = edge_index_x;
                    temp_coord_x = edge_coord_x;
                }

                    int u = temp_index_x + (i * block_size);
                    int v = temp_index_y + (j * block_size);
                    int w = temp_index_z + (k * block_size);
                    p[0] = temp_coord_x + (float)(i * block_size);
                    p[1] = temp_coord_y + (float)(j * block_size);
                    p[2] = temp_coord_z + (float)(k * block_size);
                    *(pcoords++) = p[0];
                    *(pcoords++) = p[1];
                    *(pcoords++) = p[2];
                
                EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint( coefficient, nx, ny, nz, u, v, w, p, partials, minCoord, maxCoord, dxyz );
                *(pvalues + (count++)) = partials[0];                
            }
        }
    }
    size_t index_y = count;
    std::cout<< "x_c: " << count <<std::endl;
    
    //Calculate the center of the face x = 0, 1, 2 ....
//    temp_index_x = 0;
//    temp_index_y = initial_index;
//    temp_index_z = initial_index;
//    temp_coord_x = 0.0;
//    temp_coord_y = initial_coord;
//    temp_coord_z = initial_coord;
    
    for ( size_t k = 0; k < ( block_z ); k++ )
    {
        temp_index_z = initial_index;
        temp_coord_z = initial_coord;
        
        if ( k == block_z - 1 )
        {
            temp_index_z = edge_index_z;
            temp_coord_z = edge_coord_z;
        }
        for ( size_t j = 0; j < ( block_y ); j++ )
        {
            temp_index_y = initial_index;
            temp_coord_y = initial_coord;

            if ( j == block_y - 1 )
            {
                temp_index_y = edge_index_y;
                temp_coord_y = edge_coord_y;
            }
            for ( size_t i = 0; i < ( block_x + 1 ); i++ )
            {
                temp_index_x = 0;
                temp_coord_x = 0.0;

                if ( i == block_x )
                {
                    temp_index_x = remainder_x ? ( remainder_x - (int)block_size ) : 0;
                    temp_coord_x = remainder_x ? (float)( remainder_x - (int)block_size ) : 0.0;
                }
                
                int u = temp_index_x + (i * block_size);
                int v = temp_index_y + (j * block_size);
                int w = temp_index_z + (k * block_size);
                p[0] = temp_coord_x + (float)(i * block_size);
                p[1] = temp_coord_y + (float)(j * block_size);
                p[2] = temp_coord_z + (float)(k * block_size);
                *(pcoords++) = p[0];
                *(pcoords++) = p[1];
                *(pcoords++) = p[2];
                
                EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint( coefficient, nx, ny, nz, u, v, w, p, partials, minCoord, maxCoord, dxyz );
                *(pvalues + (count++)) = partials[0];
            }
        }
    }
    size_t index_z = count;
    std::cout<< "y_c: " << count <<std::endl;
    
    //Calculate the center of the face y = 0, 1, 2 ....
//    temp_index_x = initial_index;
//    temp_index_y = 0;
//    temp_index_z = initial_index;
//    temp_coord_x = initial_coord;
//    temp_coord_y = 0.0;
//    temp_coord_z = initial_coord;
    
    for ( size_t k = 0; k < ( block_z ); k++ )
    {
        temp_index_z = initial_index;
        temp_coord_z = initial_coord;
        
        if ( k == block_z - 1 )
        {
            temp_index_z = edge_index_z;
            temp_coord_z = edge_coord_z;
        }
        for ( size_t j = 0; j < ( block_y + 1 ); j++ )
        {
            temp_index_y = 0;
            temp_coord_y = 0.0;
            
            if ( j == block_y )
            {
                temp_index_y = remainder_y ? ( remainder_y - (int)block_size ) : 0;
                temp_coord_y = remainder_y ? (float)( remainder_y - (int)block_size ) : 0.0;
            }
            for ( size_t i = 0; i < ( block_x ); i++ )
            {
                temp_index_x = initial_index;
                temp_coord_x = initial_coord;
                
                if ( i == block_x - 1 )
                {
                    temp_index_x = edge_index_x;
                    temp_coord_x = edge_coord_x;
                }
                
                int u = temp_index_x + (i * block_size);
                int v = temp_index_y + (j * block_size);
                int w = temp_index_z + (k * block_size);
                p[0] = temp_coord_x + (float)(i * block_size);
                p[1] = temp_coord_y + (float)(j * block_size);
                p[2] = temp_coord_z + (float)(k * block_size);
                *(pcoords++) = p[0];
                *(pcoords++) = p[1];
                *(pcoords++) = p[2];
                
                EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint( coefficient, nx, ny, nz, u, v, w, p, partials, minCoord, maxCoord, dxyz );
                *(pvalues + (count++)) = partials[0];
            }
        }
    }
    size_t index_gravity = count;
    std::cout<< "z_c: " << count <<std::endl;
    
    //Calculate the gravity center of every cell
    temp_index_x = initial_index;
    temp_index_y = initial_index;
    temp_index_z = initial_index;
    temp_coord_x = initial_coord;
    temp_coord_y = initial_coord;
    temp_coord_z = initial_coord;
    
    for ( size_t k = 0; k < ( block_z ); k++ )
    {
        if ( k == block_z - 1 )
        {
            temp_index_z = edge_index_z;
            temp_coord_z = edge_coord_z;
        }
        for ( size_t j = 0; j < ( block_y ); j++ )
        {
            if ( j == block_y - 1 )
            {
                temp_index_y = edge_index_y;
                temp_coord_y = edge_coord_y;
            }
            for ( size_t i = 0; i < ( block_x  ); i++ )
            {
                if ( i == block_x - 1 )
                {
                    temp_index_x = edge_index_x;
                    temp_coord_x = edge_coord_x;
                }
                
                int u = temp_index_x + (i * block_size);
                int v = temp_index_y + (j * block_size);
                int w = temp_index_z + (k * block_size);
                p[0] = temp_coord_x + (float)(i * block_size);
                p[1] = temp_coord_y + (float)(j * block_size);
                p[2] = temp_coord_z + (float)(k * block_size);
                *(pcoords++) = p[0];
                *(pcoords++) = p[1];
                *(pcoords++) = p[2];
                
                EvaluateCubicSplineOrder1PartialsAtAnyGivenPoint( coefficient, nx, ny, nz, u, v, w, p, partials, minCoord, maxCoord, dxyz );
                *(pvalues + (count++)) = partials[0];
            }
            temp_index_x = initial_index;
            temp_coord_x = initial_coord;
        }
        temp_index_y = initial_coord;
        temp_coord_y = initial_coord;
    }

    //Calculate the new connection
    kvs::ValueArray<kvs::UInt32> connections( ntets * 4 );
    kvs::UInt32* pconnections = connections.pointer();
    
    for ( size_t k = 0; k < block_z; k++ )
    {
        for ( size_t j = 0; j < block_y; j++ )
        {
            for ( size_t i = 0; i < block_x; i++ )
            {
                const size_t index = i + j * line_size + k * slice_size;
                const kvs::UInt32 id0 = index;
                const kvs::UInt32 id1 = id0 + 1;
                const kvs::UInt32 id2 = id0 + line_size;
                const kvs::UInt32 id3 = id1 + line_size;
                const kvs::UInt32 id4 = id0 + slice_size;
                const kvs::UInt32 id5 = id1 + slice_size;
                const kvs::UInt32 id6 = id2 + slice_size;
                const kvs::UInt32 id7 = id3 + slice_size;
                
                const size_t index_center = i + j * block_x + k * block_x * block_y;
                const kvs::UInt32 id8 = index_x + index_center;
                const kvs::UInt32 id10 = id8 + (block_x * block_y);
                const kvs::UInt32 id11 = index_y + index_center;
                const kvs::UInt32 id9 = id11 + 1;
                const kvs::UInt32 id12 = index_z + index_center;
                const kvs::UInt32 id13 = id12 + block_x;
                const kvs::UInt32 id14 = index_gravity + index_center;

                //tet0
                *(pconnections++) = id14;
                *(pconnections++) = id8;
                *(pconnections++) = id0;
                *(pconnections++) = id1;
                
                //tet1
                *(pconnections++) = id14;
                *(pconnections++) = id8;
                *(pconnections++) = id1;
                *(pconnections++) = id3;
                
                //tet2
                *(pconnections++) = id14;
                *(pconnections++) = id8;
                *(pconnections++) = id3;
                *(pconnections++) = id2;
                
                //tet3
                *(pconnections++) = id14;
                *(pconnections++) = id8;
                *(pconnections++) = id2;
                *(pconnections++) = id0;
                
                //tet4
                *(pconnections++) = id14;
                *(pconnections++) = id9;
                *(pconnections++) = id1;
                *(pconnections++) = id5;
                
                //tet5
                *(pconnections++) = id14;
                *(pconnections++) = id9;
                *(pconnections++) = id5;
                *(pconnections++) = id7;
                
                //tet6
                *(pconnections++) = id14;
                *(pconnections++) = id9;
                *(pconnections++) = id7;
                *(pconnections++) = id3;
                
                //tet7
                *(pconnections++) = id14;
                *(pconnections++) = id9;
                *(pconnections++) = id3;
                *(pconnections++) = id1;
                
                //tet8
                *(pconnections++) = id14;
                *(pconnections++) = id10;
                *(pconnections++) = id5;
                *(pconnections++) = id4;
                
                //tet9
                *(pconnections++) = id14;
                *(pconnections++) = id10;
                *(pconnections++) = id4;
                *(pconnections++) = id6;
                
                //tet10
                *(pconnections++) = id14;
                *(pconnections++) = id10;
                *(pconnections++) = id6;
                *(pconnections++) = id7;
                
                //tet11
                *(pconnections++) = id14;
                *(pconnections++) = id10;
                *(pconnections++) = id7;
                *(pconnections++) = id5;
                
                //tet12
                *(pconnections++) = id14;
                *(pconnections++) = id11;
                *(pconnections++) = id4;
                *(pconnections++) = id0;
                
                //tet13
                *(pconnections++) = id14;
                *(pconnections++) = id11;
                *(pconnections++) = id0;
                *(pconnections++) = id2;
                
                //tet14
                *(pconnections++) = id14;
                *(pconnections++) = id11;
                *(pconnections++) = id2;
                *(pconnections++) = id6;
                
                //tet15
                *(pconnections++) = id14;
                *(pconnections++) = id11;
                *(pconnections++) = id6;
                *(pconnections++) = id4;
                
                //tet16
                *(pconnections++) = id14;
                *(pconnections++) = id12;
                *(pconnections++) = id1;
                *(pconnections++) = id0;
                
                //tet17
                *(pconnections++) = id14;
                *(pconnections++) = id12;
                *(pconnections++) = id0;
                *(pconnections++) = id4;
                
                //tet18
                *(pconnections++) = id14;
                *(pconnections++) = id12;
                *(pconnections++) = id4;
                *(pconnections++) = id5;
                
                //tet19
                *(pconnections++) = id14;
                *(pconnections++) = id12;
                *(pconnections++) = id5;
                *(pconnections++) = id1;
                
                //tet20
                *(pconnections++) = id14;
                *(pconnections++) = id13;
                *(pconnections++) = id2;
                *(pconnections++) = id3;
                
                //tet21
                *(pconnections++) = id14;
                *(pconnections++) = id13;
                *(pconnections++) = id3;
                *(pconnections++) = id7;
                
                //tet22
                *(pconnections++) = id14;
                *(pconnections++) = id13;
                *(pconnections++) = id7;
                *(pconnections++) = id6;
                
                //tet23
                *(pconnections++) = id14;
                *(pconnections++) = id13;
                *(pconnections++) = id6;
                *(pconnections++) = id2;

            }
            index_y ++;
        }
        index_z += block_x;
    }    

    if ( volume->hasMinMaxExternalCoords() )
    {
        const kvs::Vector3f min_coord( volume->minExternalCoord() );
        const kvs::Vector3f max_coord( volume->maxExternalCoord() );
        SuperClass::setMinMaxExternalCoords( min_coord, max_coord );
    }

    if ( volume->hasMinMaxObjectCoords() )
    {
        const kvs::Vector3f min_coord( volume->minObjectCoord() );
        const kvs::Vector3f max_coord( volume->maxObjectCoord() );
        SuperClass::setMinMaxObjectCoords( min_coord, max_coord );
    }

    if ( volume->hasMinMaxValues() )
    {
        const kvs::Real64 min_value( volume->minValue() );
        const kvs::Real64 max_value( volume->maxValue() );
        SuperClass::setMinMaxValues( min_value, max_value );
    }

    SuperClass::setVeclen( volume->veclen() );
    SuperClass::setNNodes( nnodes );
    SuperClass::setNCells( ntets );
    SuperClass::setCellType( kvs::UnstructuredVolumeObject::Tetrahedra );
    SuperClass::setCoords( coords );
    SuperClass::setConnections( connections );
    SuperClass::setValues( values );
    SuperClass::updateMinMaxCoords();
    SuperClass::updateMinMaxValues();

    return this;
}

} // end of namespace kvs
