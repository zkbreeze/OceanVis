#include <iostream>
#include <fstream>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/LineObject>
#include <kvs/PointObject>
#include <kvs/glew/RayCastingRenderer>

#include <kvs/TransferFunction>
#include <kvs/glut/TransferFunctionEditor>


using namespace std;

class TransferFunctionEditor : public kvs::glut::TransferFunctionEditor
{
    
public:
    
    TransferFunctionEditor( kvs::ScreenBase* screen ) :
    kvs::glut::TransferFunctionEditor( screen )
    {
    }
    
    void apply( void )
    {
        kvs::RendererBase* r = screen()->rendererManager()->renderer();
        kvs::glew::RayCastingRenderer* renderer = static_cast<kvs::glew::RayCastingRenderer*>( r );
        renderer->setTransferFunction( transferFunction() );
        screen()->redraw();
    }
    
};

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );
    kvs::glut::Screen screen( &app );
    screen.show();    
    
    //Load Volume Data
    kvs::StructuredVolumeObject* volume = new kvs::StructuredVolumeImporter( argv[1] );
    
    kvs::glew::RayCastingRenderer* renderer
    = new kvs::glew::RayCastingRenderer();
    renderer->disableShading();
    float ka = 0.3;
    float kd = 0.5;
    float ks = 0.8;
    float n  = 100;
    renderer->setShader( kvs::Shader::Phong( ka, kd, ks, n ) );
    
    TransferFunctionEditor editor( &screen );
    if ( argc > 2 )
    {
        editor.setTransferFunction( kvs::TransferFunction( argv[2] ) );
        renderer->setTransferFunction( editor.transferFunction() );
    }
    editor.setVolumeObject( volume );
    editor.show();
    
    screen.registerObject( volume, renderer );
    
    return( app.run() );
}
