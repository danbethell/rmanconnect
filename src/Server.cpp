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

#include "Server.h"
#include "Client.h"
#include <boost/lexical_cast.hpp>
#include <vector>
#include <iostream>
#include <OpenEXR/Iex.h>

using namespace rmanconnect;
using boost::asio::ip::tcp;

Server::Server() :
        mPort(0),
        mSocket( mIoService ),
        mAcceptor( mIoService )
{
}

Server::Server( int port ) :
        mPort(0),
        mSocket( mIoService ),
        mAcceptor( mIoService )
{
    connect( port );
}

Server::~Server()
{
    if ( mAcceptor.is_open() )
        mAcceptor.close();
}

void Server::connect( int port, bool search )
{
    // disconnect if necessary
    if ( mAcceptor.is_open() )
        mAcceptor.close();

    // reconnect at specified port
    int start_port = port;
    while (!mAcceptor.is_open() && port < start_port + 99)
    {
        try
        {
            tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );
            mAcceptor.open(endpoint.protocol());
            mAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            mAcceptor.bind(endpoint);
            mAcceptor.listen();
            mPort = port;
        }
        catch (...)
        {
            mAcceptor.close();
            if (!search)
                break;
            else
                port++;
        }
    }

    // handle failed connection
    if ( !mAcceptor.is_open() )
    {
        char buffer[32];
        sprintf(buffer, "port: %d", start_port);
        if (search)
            sprintf(buffer, "port: %d-%d", start_port, start_port + 99);
        THROW( Iex::BaseExc, "Failed to connect to port " << buffer );
    }
}

void Server::quit()
{
    std::string hostname("localhost");
    rmanconnect::Client client(hostname, mPort);
    client.quit();
}

void Server::accept()
{
    if ( mSocket.is_open() )
        mSocket.close();
    mAcceptor.accept(mSocket);
}

Data Server::listen()
{
    Data d;

    // read the key from the incoming data
    try
    {
        int key = -1;
        boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&key), sizeof(int)) );

        switch( key )
        {
            case 0: // open image
            {
                // send back an image id
                int image_id = 1;
                boost::asio::write( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&image_id), sizeof(int) ) );

                // get width & height
                int width, height;
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&width), sizeof(int)) );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&height), sizeof(int)) );

                // create data object
                d.mType = key;
                d.mWidth = width;
                d.mHeight = height;
                break;
            }
            case 1: // image data
            {
                d.mType = key;

                // receive image id
                int image_id;
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&image_id), sizeof(int)) );

                // get data info
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mX), sizeof(int)) );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mY), sizeof(int)) );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mWidth), sizeof(int)) );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mHeight), sizeof(int)) );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mSpp), sizeof(int)) );

                // get pixels
                int num_samples = d.width() * d.height() * d.spp();
                d.mPixelStore.resize( num_samples );
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&d.mPixelStore[0]), sizeof(float)*num_samples ) ) ;
                break;
            }
            case 2: // close image
            {
                int image_id;
                d.mType = key;
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&image_id), sizeof(int)) );
                mSocket.close();
                break;
            }
            case 9: // quit
            {
                d.mType = 9;
                mSocket.close();
                break;
            }
        }
    }
    catch( ... )
    {
        mSocket.close();
        THROW( Iex::BaseExc, "Could not read from socket!" );
    }

    return d;
}
