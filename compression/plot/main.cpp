#include <kvs/StructuredVolumeObject>
#include <kvs/StructuredVolumeImporter>
#include <kvs/TableObject>
#include <kvs/ScatterPlotRenderer>
#include <kvs/glut/Application>
#include <kvs/glut/Screen>
#include <kvs/glut/Axis2DRenderer>
#include <kvs/ColorMap>

int main( int argc, char** argv )
{
    kvs::glut::Application app( argc, argv );

    kvs::StructuredVolumeObject* volume1 = new kvs::StructuredVolumeImporter( argv[1] );
    kvs::StructuredVolumeObject* volume2 = new kvs::StructuredVolumeImporter( argv[2] );

    kvs::TableObject* object = new kvs::TableObject();
    object->addColumn( volume1->values(), argv[1] );
    object->addColumn( volume2->values(), argv[2] );

    delete volume1;
    delete volume2;

    kvs::ScatterPlotRenderer* renderer = new kvs::ScatterPlotRenderer();
    renderer->setBackgroundColor( kvs::RGBAColor( 255, 255, 255, 0.5f ) );
    renderer->setPointSize( 5.0f );
    renderer->setPointOpacity( 128 );
    
//    kvs::TransferFunction tfunc(256);
//    renderer->setColorMap( tfunc.colorMap() );

    kvs::glut::Axis2DRenderer* axis = new kvs::glut::Axis2DRenderer();
    axis->setAxisWidth( 5.0 );

    kvs::glut::Screen screen( &app );
    screen.registerObject( object, renderer );
    screen.registerObject( object, axis );
    screen.show();

    return app.run();
}
