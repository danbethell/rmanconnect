#include "Data.h"
#include <cstring>
#include <iostream>

using namespace rmanconnect;

Data::Data( int x, int y, 
            int width, int height, 
            int spp, const float *data ) :
    mType(1),
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height),
    mSpp(spp)
{
    if ( data!=0 )
        mpData = const_cast<float*>(data);
}

Data::~Data()
{
}
