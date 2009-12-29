#ifndef RMAN_CONNECT_DATA_H_
#define RMAN_CONNECT_DATA_H_

namespace rmanconnect
{
    class Data
    {
    friend class Client;
    friend class Server;
    public:
        Data( int x=0, int y=0,
              int width=0, int height=0,
              int spp=0, const float *data=0 );
        ~Data();
        
        int type(){ return mType; }
        int x(){ return mX; }
        int y(){ return mY; }
        int width(){ return mWidth; }
        int height(){ return mHeight; }
        int spp(){ return mSpp; }
        const float *data(){ return mpData; }

    private:
        // what type of data is this?
        // 0: open image
        // 1: image data
        // 2: close image
        // 9: quit listening loop
        int mType;

        // x & y position
        int mX, mY; 
        
        // width, height, num channels (samples)
        unsigned int mWidth, mHeight, mSpp;

        // our pixel data, interleaved
        float *mpData; 
    };
}

#endif // RMAN_CONNECT_DATA_H_
