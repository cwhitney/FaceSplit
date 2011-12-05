#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Capture.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class FaceSplitApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void saveImg();
    
    Capture mCapture;
    gl::Texture mTexture;
    gl::Fbo mFbo;
    
    params::InterfaceGl mParams;
    
    int numSlices;
};

void FaceSplitApp::setup()
{
    numSlices = 9;
    mTexture = gl::Texture();
    
    mFbo = gl::Fbo( getWindowSize().x, getWindowSize().y );
    
    try {
		mCapture = Capture( 640, 480 );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
    
    mParams = params::InterfaceGl( "Cam", Vec2i( 225, 200 ) );
    mParams.addParam( "Number of slices", &numSlices );
    mParams.addButton( "Save", std::bind( &FaceSplitApp::saveImg, this ) );
}

void FaceSplitApp::saveImg()
{
    string pngPath = getSaveFilePath( getHomeDirectory() );
    
    if( ! pngPath.empty() ) {
        
        // make sure the path is valid
        int pos = pngPath.find_last_of(".");
        if(pos == -1){
            pngPath += ".png";
        }else if( pngPath.substr( pos ) != ".png"){
            pngPath = pngPath.substr(0, pos) + ".png";
        }else{
           
        }
        
        // convoluted texture flipping
        gl::Texture t = mFbo.getTexture();
        t.setFlipped( true );
        gl::setMatricesWindow( t.getSize() );
        
        gl::Fbo tmp( 640, 480);
        tmp.bindFramebuffer();
        gl::clear();
            gl::translate( tmp.getSize()/2 );
            gl::rotate( 180 );
            gl::translate( tmp.getSize()/-2 );
            
            gl::scale(-1, 1);
            gl::translate( -t.getWidth(), 0 );
        
            gl::draw( t );
        tmp.unbindFramebuffer();
        
        gl::setMatricesWindow( getWindowSize() );
        
        // save it finally
        Surface srf = Surface( tmp.getTexture() );
        writeImage( pngPath, srf, ImageTarget::Options().colorModel( ImageIo::CM_RGB ).quality( 1.0f ), "png" );
    }
}

void FaceSplitApp::mouseDown( MouseEvent event )
{
}

void FaceSplitApp::update()
{
    if( mCapture && mCapture.checkNewFrame() ) {
		mTexture = gl::Texture( mCapture.getSurface() );
	}
}

void FaceSplitApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color( Color::white() );
    
    if( mTexture ){
        
        mFbo.bindFramebuffer();
        gl::clear();
        
        gl::draw( mTexture );
        
        mTexture.enableAndBind();
        glBegin( GL_TRIANGLES );{
            float scale = 1.0;
            for(int i=0; i<numSlices/2; i++){
                
                scale = 1.0 - (i / float(numSlices / 2.0 ) );
                scale -= (scale / float(numSlices));
                
                // left tris
                glTexCoord2f( 0, 1 );
                glVertex2f( Vec2f( 0, mTexture.getHeight()  * scale) );
                
                glTexCoord2f( 0, 0 );
                glVertex2f( Vec2f( 0, 0 ) );
                
                glTexCoord2f( 1, 0 );
                glVertex2f( Vec2f( mTexture.getWidth() * scale, 0  ) );
                
                
                // right tris
                glTexCoord2f( 0, 1 );   // BL
                glVertex2f( Vec2f( mTexture.getWidth() - mTexture.getWidth() * scale, mTexture.getHeight()) );
                
                glTexCoord2f( 1, 0 );   // TR
                glVertex2f( Vec2f( mTexture.getWidth(), mTexture.getHeight() - mTexture.getHeight() * scale ) );
                
                glTexCoord2f( 1, 1 );   // BR
                glVertex2f( Vec2f( mTexture.getWidth(), mTexture.getHeight() ) );
            }
        }glEnd();
        mTexture.unbind();
        
        mFbo.unbindFramebuffer();
        gl::Texture t = mFbo.getTexture();
        t.setFlipped( true );
        
        gl::draw( t );
    }
    
    
    
    mParams.draw();
}


CINDER_APP_BASIC( FaceSplitApp, RendererGl )
