/*
Copyright (c) 2010, Dan Bethell, Johannes Saam.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    * Neither the name of RmanConnect nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Client.h"
#include <boost/lexical_cast.hpp>
#include <OpenEXR/Iex.h>
#include <iostream>

using namespace rmanconnect;
using boost::asio::ip::tcp;

Client::Client( std::string hostname, int port ) :
        mHost( hostname ),
        mPort( port ),
        mImageId( -1 ),
        mSocket( mIoService )
{
}

void Client::connect( std::string hostname, int port )
{
    bool result = true;
    tcp::resolver resolver(mIoService);
    tcp::resolver::query query( hostname.c_str(), boost::lexical_cast<std::string>(port).c_str() );
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
        mSocket.close();
        mSocket.connect(*endpoint_iterator++, error);
    }
    if (error)
        throw boost::system::system_error(error);
}

void Client::disconnect()
{
    mSocket.close();
}

Client::~Client()
{
    disconnect();
}

void Client::openImage( Data &header )
{
    // connect to port
    connect(mHost, mPort);

    // send image header message with image desc information
    int key = 0;
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&key), sizeof(int)) );

    // read our imageid
    boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&mImageId), sizeof(int)) );

    // send our width & height
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&header.mWidth), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&header.mHeight), sizeof(int)) );

    // disconnect
    disconnect();
}

void Client::sendPixels( Data &data )
{
    if ( mImageId<0 )
        THROW( Iex::BaseExc, "Could not send data - image id is not valid!" );

    // connect to port
    connect(mHost, mPort);

    boost::system::error_code ignored_error;
    // send data for image_id
    int key = 1;
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&key), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&mImageId), sizeof(int)) );

    // send pixel data
    int num_samples = data.mWidth * data.mHeight * data.mSpp;
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mX), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mY), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mWidth), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mHeight), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mSpp), sizeof(int)) );
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&data.mpData[0]), sizeof(float)*num_samples) );

    // disconnect
    disconnect();
}

void Client::closeImage( )
{
    // connect to port
    connect(mHost, mPort);

    // send image complete message for image_id
    int key = 2;
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&key), sizeof(int)) );

    // tell the server which image we're closing
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&mImageId), sizeof(int)) );

    // disconnect
    disconnect();
}

void Client::quit()
{
    connect(mHost, mPort);
    int key = 9;
    boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&key), sizeof(int)) );
    disconnect();
}
