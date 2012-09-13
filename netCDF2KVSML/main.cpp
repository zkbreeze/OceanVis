#include <iostream>
#include <list>
#include <string>
#include <cstdio>
#include <netcdfcpp.h>

#include <kvs/Message>
#include <kvs/AnyValueArray>
#include <kvs/File>
#include <kvs/FileList>
#include <kvs/Directory>
#include <kvs/Timer>
#include <kvs/KVSMLObjectStructuredVolume>


/*===========================================================================*/
/**
 *  @brief  Dimension class.
 */
/*===========================================================================*/
class Dim
{
private:

    const NcDim* m_dim;

public:

    Dim( const NcDim* dim ):
        m_dim( dim ) {}

    const std::string name( void ) const
    {
        return( m_dim->name() );
    }

    const size_t size( void ) const
    {
        return( static_cast<size_t>( m_dim->size() ) );
    }

    friend std::ostream& operator <<( std::ostream& stream, const Dim* dim )
    {
        stream << "name = " << dim->name() << ", "
               << "size = " << dim->size();

        return( stream );
    }
};

typedef std::list<Dim*> DimList;

/*===========================================================================*/
/**
 *  @brief  Variable class.
 */
/*===========================================================================*/
class Var
{
private:

    const NcVar* m_var;
    DimList      m_dims;

public:

    Var( const NcVar* var ):
        m_var( var )
    {
        const int ndims = m_var->num_dims();
        for ( int i = 0; i < ndims; i++ )
        {
            m_dims.push_back( new Dim( m_var->get_dim(i) ) );
        }
    }

    ~Var( void )
    {
        DimList::iterator dim = m_dims.begin();
        DimList::const_iterator last_dim = m_dims.end();
        while ( dim != last_dim )
        {
            if ( *dim ) delete *dim;
            dim++;
        }
        m_dims.clear();
    }

    const bool isValid( void ) const
    {
        return( m_var->is_valid() != 0 );
    }

    const std::string name( void ) const
    {
        return( m_var->name() );
    }

    const size_t nvals( void ) const
    {
        return( static_cast<size_t>( m_var->num_vals() ) );
    }

    const size_t ndims( void ) const
    {
        return( m_dims.size() );
    }

    const Dim* dim( const size_t index ) const
    {
        DimList::const_iterator dim = m_dims.begin();
        DimList::const_iterator last = m_dims.end();
        size_t counter = 0;
        while ( dim != last )
        {
            if ( counter++ == index ) break;
            dim++;
        }

        return( *dim );
    }

    const Dim* dim( const std::string name ) const
    {
        DimList::const_iterator dim = m_dims.begin();
        DimList::const_iterator last = m_dims.end();
        while ( dim != last )
        {
            if ( (*dim)->name() == name ) break;

            dim++;
        }

        return( *dim );
    }

    const std::string type( void ) const
    {
        switch ( m_var->type() )
        {
        case ncByte:   return( "unsigned char" );
        case ncChar:   return( "char" );
        case ncShort:  return( "short" );
        case ncInt:    return( "int" );
        case ncFloat:  return( "float" );
        case ncDouble: return( "double" );
        default:       return( "" );
        }
    }

    const kvs::AnyValueArray data(
        const size_t offset = 0,
        const size_t dim1 = 1,
        const size_t dim2 = 1,
        const size_t dim3 = 1 ) const
    {
        const void* head = m_var->values()->base();
        const size_t nvalues = dim1 * dim2 * dim3;

        switch ( m_var->type() )
        {
        case ncByte:
        {
            kvs::UInt8* values = (kvs::UInt8*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        case ncChar:
        {
            kvs::Int8* values = (kvs::Int8*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        case ncShort:
        {
            kvs::Int16* values = (kvs::Int16*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        case ncInt:
        {
            kvs::Int32* values = (kvs::Int32*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        case ncFloat:
        {
            kvs::Real32* values = (kvs::Real32*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        case ncDouble:
        {
            kvs::Real64* values = (kvs::Real64*)( head ) + offset;
            return( kvs::AnyValueArray( this->flip( dim1, dim2, dim3, values ), nvalues ) );
        }
        default: return( kvs::AnyValueArray() );
        }
    }

    friend std::ostream& operator <<( std::ostream& stream, const Var* var )
    {
        stream << "name = " << var->name() << ", "
               << "type = " << var->type() << ", "
               << "ndims = " << var->ndims() << ", "
               << "nvals = " << var->nvals();

        return( stream );
    }

    template <typename T>
    const T* flip( const size_t dim1, const size_t dim2, const size_t dim3, T* data ) const
    {
        const size_t stride = dim1 * dim2;
        const size_t end = dim3 / 2;
        for ( size_t z = 0; z < end; z++ )
        {
            T* src = data + z * stride;
            T* dst = data + ( dim3 - z - 1 ) * stride;
            for ( size_t xy = 0; xy < stride; xy++ )
            {
                T tmp = *src;
                *src = *dst;
                *dst = tmp;
                src++;
                dst++;
            }
        }

        return( data );
    }
};

typedef std::list<Var*> VarList;

/*===========================================================================*/
/**
 *  @brief  File class.
 */
/*===========================================================================*/
class File
{
private:

    std::string m_filename;
    NcFile*     m_file;
    DimList     m_dims;
    VarList     m_vars;

public:

    File( void ):
        m_file( 0 ) {}

    File( const std::string filename ):
        m_file( 0 )
    {
        this->read( filename );
    }

    ~File( void )
    {
        if ( m_file ) delete m_file;

        DimList::iterator dim = m_dims.begin();
        DimList::const_iterator last_dim = m_dims.end();
        while ( dim != last_dim )
        {
            if ( *dim ) delete *dim;
            dim++;
        }
        m_dims.clear();

        VarList::iterator var = m_vars.begin();
        VarList::const_iterator last_var = m_vars.end();
        while ( var != last_var )
        {
            if ( *var ) delete *var;
            var++;
        }
        m_vars.clear();
    }

    const std::string filename( void ) const
    {
        return( m_filename );
    }

    const size_t ndims( void ) const
    {
        return( m_dims.size() );
    }

    const DimList& dims( void ) const
    {
        return( m_dims );
    }

    const Dim* dim( const size_t index ) const
    {
        DimList::const_iterator dim = m_dims.begin();
        DimList::const_iterator last = m_dims.end();
        size_t counter = 0;
        while ( dim != last )
        {
            if ( counter++ == index ) break;
            dim++;
        }

        return( *dim );
    }

    const Dim* dim( const std::string name ) const
    {
        DimList::const_iterator dim = m_dims.begin();
        DimList::const_iterator last = m_dims.end();
        while ( dim != last )
        {
            if ( (*dim)->name() == name ) break;
            dim++;
        }

        return( *dim );
    }

    const size_t nvars( void ) const
    {
        return( m_vars.size() );
    }

    const VarList& vars( void ) const
    {
        return( m_vars );
    }

    const Var* var( const size_t index ) const
    {
        VarList::const_iterator var = m_vars.begin();
        VarList::const_iterator last = m_vars.end();
        size_t counter = 0;
        while ( var != last )
        {
            if ( counter++ == index ) break;
            var++;
        }

        return( *var );
    }

    const Var* var( const std::string name ) const
    {
        VarList::const_iterator var = m_vars.begin();
        VarList::const_iterator last = m_vars.end();
        while ( var != last )
        {
            if ( (*var)->name() == name ) break;

            var++;
        }

        return( *var );
    }

    const bool read( const std::string filename )
    {
        if ( m_file ) delete m_file;

        m_filename = filename;
        m_file = new NcFile( filename.c_str(), NcFile::ReadOnly );
        if ( !m_file ) return( false );
        if ( !m_file->is_valid() ) return( false );

        const int ndims = m_file->num_dims();
        for ( int i = 0; i < ndims; i++ )
        {
            m_dims.push_back( new Dim( m_file->get_dim(i) ) );
        }

        const int nvars = m_file->num_vars();
        for ( int i = 0; i < nvars; i++ )
        {
            m_vars.push_back( new Var( m_file->get_var(i) ) );
        }

        return( true );
    }
};


/*===========================================================================*/
/**
 *  @brief  Output header information.
 *  @param  file [i] netCDF file
 */
/*===========================================================================*/
void OutputHeader( const File& file )
{
    std::cout << "Num. of dims: " << file.ndims() << std::endl;
    for ( size_t i = 0; i < file.ndims(); i++ )
    {
        std::cout << "dim" << i << " : " << file.dim(i) << std::endl;
    }

    std::cout << "Num. of vars: " << file.nvars() << std::endl;
    for ( size_t i = 0; i < file.nvars(); i++ )
    {
        std::cout << "var" << i << " : " << file.var(i) << std::endl;
        for ( size_t j = 0; j < file.var(i)->ndims(); j++ )
        {
            std::cout << "\tdim" << j << " : " << file.var(i)->dim(j) << std::endl;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Output KVSML data.
 *  @param  file [in] netCDF file
 */
/*===========================================================================*/
void OutputKVSML( const File& file )
{
    const size_t nvars = file.nvars();
    for ( size_t i = 0; i < nvars; i++ )
    {
        const Var* var = file.var(i);
        if ( !var->isValid() ) continue;

        const std::string basename = kvs::File( file.filename() ).baseName();
        switch ( var->ndims() )
        {
        case 1:
        {
            const Dim* dim = var->dim(0);
            if ( dim->name() == "time" )
            {
                const size_t dim1 = dim->size();
                const size_t dim2 = 1;
                const size_t dim3 = 1;
                const kvs::Vector3ui resolution( dim1, dim2, dim3 );
                const std::string filename = basename + "__" + var->name() + ".kvsml";
                std::cout << filename << " ... ";
                kvs::Timer timer( kvs::Timer::Start );
                const kvs::AnyValueArray values = var->data( 0, dim1, dim2, dim3 );
                {
                    kvs::KVSMLObjectStructuredVolume kvsml;
                    kvsml.setWritingDataType( kvs::KVSMLObjectStructuredVolume::Ascii );
                    kvsml.setGridType( "uniform" );
                    kvsml.setVeclen( 1 );
                    kvsml.setResolution( resolution );
                    kvsml.setValues( values );
                    kvsml.write( filename );
                }
                timer.stop();
                const float msec = timer.msec();
                const size_t mbytes = values.byteSize() / 1024 / 1024;
                std::cout << "done. [" << msec << " msec, " << mbytes << " MB]" << std::endl;
            }
            break;
        }
        case 2:
        {
            const Dim* dim = var->dim(0);
            if ( dim->name() == "time" )
            {
                const size_t nsteps = dim->size();
                const size_t dim1 = var->dim(1)->size();
                const size_t dim2 = 1;
                const size_t dim3 = 1;
                const kvs::Vector3ui resolution( dim1, dim2, dim3 );
                for ( size_t i = 0; i < nsteps; i++ )
                {
                    char no[6]; sprintf( no, "%04d", static_cast<int>(i) );
                    const size_t offset = dim1 * dim2 * dim3 * i;
                    const std::string filename = basename + "__" + var->name() + "_" + no + ".kvsml";
                    std::cout << filename << " ... ";
                    kvs::Timer timer( kvs::Timer::Start );
                    const kvs::AnyValueArray values = var->data( offset, dim1, dim2, dim3 );
                    {
                        kvs::KVSMLObjectStructuredVolume kvsml;
                        kvsml.setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
                        kvsml.setGridType( "uniform" );
                        kvsml.setVeclen( 1 );
                        kvsml.setResolution( resolution );
                        kvsml.setValues( values );
                        kvsml.write( filename );
                    }
                    timer.stop();
                    const float msec = timer.msec();
                    const size_t mbytes = values.byteSize() / 1024 / 1024;
                    std::cout << "done. [" << msec << " msec, " << mbytes << " MB]" << std::endl;
                }
            }
            break;
        }
        case 3:
        {
            const Dim* dim = var->dim(0);
            if ( dim->name() == "time" )
            {
                const size_t nsteps = dim->size();
                const size_t dim1 = var->dim(2)->size();
                const size_t dim2 = var->dim(1)->size();
                const size_t dim3 = 1;
                const kvs::Vector3ui resolution( dim1, dim2, dim3 );
                for ( size_t i = 0; i < nsteps; i++ )
                {
                    char no[6]; sprintf( no, "%04d", static_cast<int>(i) );
                    const size_t offset = dim1 * dim2 * dim3 * i;
                    const std::string filename = basename + "__" + var->name() + "_" + no + ".kvsml";
                    std::cout << filename << " ... ";
                    kvs::Timer timer( kvs::Timer::Start );
                    const kvs::AnyValueArray values = var->data( offset, dim1, dim2, dim3 );
                    {
                        kvs::KVSMLObjectStructuredVolume kvsml;
                        kvsml.setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
                        kvsml.setGridType( "uniform" );
                        kvsml.setVeclen( 1 );
                        kvsml.setResolution( resolution );
                        kvsml.setValues( values );
                        kvsml.write( filename );
                    }
                    timer.stop();
                    const float msec = timer.msec();
                    const size_t mbytes = values.byteSize() / 1024 / 1024;
                    std::cout << "done. [" << msec << " msec, " << mbytes << " MB]" << std::endl;
                }
            }
            break;
        }
        case 4:
        {
            const Dim* dim = var->dim(0);
            if ( dim->name() == "time" )
            {
                const size_t nsteps = dim->size();
                const size_t dim1 = var->dim(3)->size();
                const size_t dim2 = var->dim(2)->size();
                const size_t dim3 = var->dim(1)->size();
                const kvs::Vector3ui resolution( dim1, dim2, dim3 );
                for ( size_t i = 0; i < nsteps; i++ )
                {
                    char no[6]; sprintf( no, "%04d", static_cast<int>(i) );
                    const size_t offset = dim1 * dim2 * dim3 * i;
                    const std::string filename = basename + "__" + var->name() + "_" + no + ".kvsml";
                    std::cout << filename << " ... " << std::flush;
                    kvs::Timer timer( kvs::Timer::Start );
                    const kvs::AnyValueArray& values = var->data( offset, dim1, dim2, dim3 );
                    {
                        kvs::KVSMLObjectStructuredVolume kvsml;
                        kvsml.setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
                        kvsml.setGridType( "uniform" );
                        kvsml.setVeclen( 1 );
                        kvsml.setResolution( resolution );
                        kvsml.setValues( values );
                        kvsml.write( filename );
                    }
                    timer.stop();
                    const float msec = timer.msec();
                    const size_t mbytes = values.byteSize() / 1024 / 1024;
                    std::cout << "done. [" << msec << " msec, " << mbytes << " MB]" << std::endl;
                }
            }
            break;
        }
        default: break;
        }
    }
}

void CombineUVW( const std::string filename )
{
    const std::string basename = kvs::File( filename ).baseName();
    const size_t max_iterations = 1000;
    for ( size_t i = 0; i < max_iterations; i++ )
    {
        char no[6]; sprintf( no, "%04d", static_cast<int>(i) );
        const std::string ufile = basename + "__u_" + no + ".kvsml";
        const std::string vfile = basename + "__v_" + no + ".kvsml";
        const std::string wfile = basename + "__w_" + no + ".kvsml";
        if ( kvs::File( ufile ).isExisted() &&
             kvs::File( vfile ).isExisted() &&
             kvs::File( wfile ).isExisted() )
        {
            kvs::KVSMLObjectStructuredVolume ukvsml( ufile );
            kvs::KVSMLObjectStructuredVolume vkvsml( vfile );
            kvs::KVSMLObjectStructuredVolume wkvsml( wfile );

            KVS_ASSERT( ukvsml.veclen() == 1 );
            KVS_ASSERT( vkvsml.veclen() == 1 );
            KVS_ASSERT( wkvsml.veclen() == 1 );
            KVS_ASSERT( ukvsml.resolution() == vkvsml.resolution() );
            KVS_ASSERT( vkvsml.resolution() == wkvsml.resolution() );

            const std::string uvwfile = basename + "__uvw_" + no + ".kvsml";
            std::cout << uvwfile << " ... ";
            kvs::Timer timer( kvs::Timer::Start );

            kvs::AnyValueArray values;
            const size_t nvalues = ukvsml.values().size() * 3;
            const kvs::AnyValueArray uvalues = ukvsml.values();
            const kvs::AnyValueArray vvalues = vkvsml.values();
            const kvs::AnyValueArray wvalues = wkvsml.values();
            const std::type_info& type = uvalues.typeInfo()->type();
            if ( type == typeid(kvs::UInt8) )
            {
                kvs::UInt8* pvalues = static_cast<kvs::UInt8*>( values.allocate<kvs::UInt8>( nvalues ) );
                const kvs::UInt8* puvalues = static_cast<const kvs::UInt8*>( uvalues.pointer() );
                const kvs::UInt8* pvvalues = static_cast<const kvs::UInt8*>( vvalues.pointer() );
                const kvs::UInt8* pwvalues = static_cast<const kvs::UInt8*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else if ( type == typeid(kvs::Int8) )
            {
                kvs::Int8* pvalues = static_cast<kvs::Int8*>( values.allocate<kvs::Int8>( nvalues ) );
                const kvs::Int8* puvalues = static_cast<const kvs::Int8*>( uvalues.pointer() );
                const kvs::Int8* pvvalues = static_cast<const kvs::Int8*>( vvalues.pointer() );
                const kvs::Int8* pwvalues = static_cast<const kvs::Int8*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else if ( type == typeid(kvs::Int16) )
            {
                kvs::Int16* pvalues = static_cast<kvs::Int16*>( values.allocate<kvs::Int16>( nvalues ) );
                const kvs::Int16* puvalues = static_cast<const kvs::Int16*>( uvalues.pointer() );
                const kvs::Int16* pvvalues = static_cast<const kvs::Int16*>( vvalues.pointer() );
                const kvs::Int16* pwvalues = static_cast<const kvs::Int16*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else if ( type == typeid(kvs::Int32) )
            {
                kvs::Int32* pvalues = static_cast<kvs::Int32*>( values.allocate<kvs::Int32>( nvalues ) );
                const kvs::Int32* puvalues = static_cast<const kvs::Int32*>( uvalues.pointer() );
                const kvs::Int32* pvvalues = static_cast<const kvs::Int32*>( vvalues.pointer() );
                const kvs::Int32* pwvalues = static_cast<const kvs::Int32*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else if ( type == typeid(kvs::Real32) )
            {
                kvs::Real32* pvalues = static_cast<kvs::Real32*>( values.allocate<kvs::Real32>( nvalues ) );
                const kvs::Real32* puvalues = static_cast<const kvs::Real32*>( uvalues.pointer() );
                const kvs::Real32* pvvalues = static_cast<const kvs::Real32*>( vvalues.pointer() );
                const kvs::Real32* pwvalues = static_cast<const kvs::Real32*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else if ( type == typeid(kvs::Real64) )
            {
                kvs::Real64* pvalues = static_cast<kvs::Real64*>( values.allocate<kvs::Real64>( nvalues ) );
                const kvs::Real64* puvalues = static_cast<const kvs::Real64*>( uvalues.pointer() );
                const kvs::Real64* pvvalues = static_cast<const kvs::Real64*>( vvalues.pointer() );
                const kvs::Real64* pwvalues = static_cast<const kvs::Real64*>( wvalues.pointer() );
                for ( size_t i = 0; i < nvalues / 3; i++ )
                {
                    *(pvalues++) = *(puvalues++);
                    *(pvalues++) = *(pvvalues++);
                    *(pvalues++) = *(pwvalues++);
                }
            }
            else continue;

            kvs::KVSMLObjectStructuredVolume kvsml;
            kvsml.setWritingDataType( kvs::KVSMLObjectStructuredVolume::ExternalBinary );
            kvsml.setGridType( "uniform" );
            kvsml.setVeclen( 3 );
            kvsml.setResolution( ukvsml.resolution() );
            kvsml.setValues( values );
            kvsml.write( uvwfile );

            timer.stop();
            const float msec = timer.msec();
            const size_t mbytes = values.byteSize() / 1024 / 1024;
            std::cout << "done. [" << msec << " msec, " << mbytes << " MB]" << std::endl;
        }
        else break;
    }
}

int main( int argc, char** argv )
{
    if ( argc < 1 ) exit( EXIT_FAILURE );

    if ( kvs::File( std::string( argv[1] ) ).isFile() )
    {
        const std::string filename = argv[1];
        {
            File file( filename );
            OutputKVSML( file );
        }
        {
            CombineUVW( filename );
        }
    }
    else if ( kvs::Directory( std::string( argv[1] ) ).isDirectory() )
    {
        const std::string extension = "nc";
        const std::string dirname = argv[1];
        kvs::Directory dir( dirname );
        kvs::FileList files = dir.fileList();
        kvs::FileList::const_iterator file = files.begin();
        kvs::FileList::const_iterator last = files.end();
        while ( file != last )
        {
            if ( file->extension() == extension )
            {
                const std::string filename = file->filePath();
                {
                    File file( filename );
                    OutputKVSML( file );
                }
                {
                    CombineUVW( filename );
                }
            }
            file++;
        }
    }

    return( 0 );
}
