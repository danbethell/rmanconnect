#ifndef RMAN_CONNECT_DATA_H_
#define RMAN_CONNECT_DATA_H_

namespace rmanconnect
{
    class ImageDesc
    {
        public:
            int x, y;
            unsigned int width, height, spp;
    };

    class Data
    {
    friend class Client;
    friend class Server;
    public:
        Data();
        Data( int x, int y, 
              int width, int height, 
              int spp, const float *data=0, bool auto_cleanup=false );
        ~Data();
        
        int type(){ return mType; } // 0: open, 1: data, 2:close
        int x(){ return mX; }
        int y(){ return mY; }
        int width(){ return mWidth; }
        int height(){ return mHeight; }
        int spp(){ return mSpp; }
        const float *data(){ return mpData; }

    private:
        // x & y position
        int mX, mY; 
        
        // width, height, num channels (samples)
        unsigned int mWidth, mHeight, mSpp; 

        // do we cleanup data automatically?
        bool mObjectOwnsData;

        // our pixel data, interleaved
        float *mpData; 

        // what type of data is this?
        int mType;
    };
}

#endif // RMAN_CONNECT_DATA_H_
