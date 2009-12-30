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
        mpAcceptor(0)
{
}

Server::~Server()
{
    delete mpAcceptor;
    mpAcceptor = 0;
}

void Server::reconnect( int port, bool search )
{
    // disconnect if necessary
    if ( mpAcceptor )
    {
        mpAcceptor->close();
        delete mpAcceptor;
        mpAcceptor = 0;
    }

    // reconnect at specified port
    int start_port = port;
    while (!mpAcceptor && port < start_port + 99)
    {
        try
        {
            mpAcceptor = new tcp::acceptor( mIoService, tcp::endpoint(boost::asio::ip::tcp::v4(), port) );
            mPort = port;
        }
        catch (...)
        {
            mpAcceptor = 0;
            if (!search)
                break;
            else
                port++;
        }
    }

    // handle failed connection
    if ( !mpAcceptor )
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
    rmanconnect::Client client(mPort);
    client.quit();
}

Data Server::listen()
{
    mpAcceptor->accept(mSocket);

    Data d;
    try
    {
        // read the key from the incoming data
        int key;
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
                float* pixel_data = new float[num_samples];
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&pixel_data[0]), sizeof(float)*num_samples ) ) ;
                d.mpData = pixel_data;

                break;
            }
            case 2: // close image
            {
                int image_id;
                d.mType = key;
                boost::asio::read( mSocket, boost::asio::buffer(reinterpret_cast<char*>(&image_id), sizeof(int)) );
                break;
            }
            case 9: // quit
            {
                d.mType = 9;
                break;
            }
        }
    }
    catch( ... )
    {
    }

    mSocket.close();
    return d;
}
