#include "Server.h"
#include "Client.h"
#include <boost/lexical_cast.hpp>
#include <vector>
#include <iostream>

using namespace rmanconnect;
using boost::asio::ip::tcp;

Server::Server( int port ) :
        mPort( port ),
        mSocket( mIoService ),
        mAcceptor( mIoService, tcp::endpoint(boost::asio::ip::tcp::v4(), port) )
{
}

Server::~Server()
{
}

void Server::reconnect( int port )
{
}

void Server::quit( int port )
{
    rmanconnect::Client client(mPort);
    client.quit();
}

Data Server::listen()
{
    mAcceptor.accept(mSocket);

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
