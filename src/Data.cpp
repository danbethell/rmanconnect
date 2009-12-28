#include "Data.h"
#include <cstring>
#include <iostream>

using namespace rmanconnect;

Data::Data() :
        mX(0),
        mY(0),
        mWidth(0),
        mHeight(0),
        mSpp(0),
        mpData(0),
        mType(-1)
{
}

Data::Data( int x, int y, 
            int width, int height, 
            int spp, const float *data, bool auto_cleanup ) :
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height),
    mSpp(spp),
    mType(1)
{
    if ( data!=0 )
    {
        mpData = const_cast<float*>(data);
        mObjectOwnsData = auto_cleanup;
    }
}

Data::~Data()
{
    if ( mObjectOwnsData )
        delete [] mpData;
}
