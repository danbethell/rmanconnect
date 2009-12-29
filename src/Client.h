#ifndef RMAN_CONNECT_CLIENT_H_
#define RMAN_CONNECT_CLIENT_H_

#include "Data.h"
#include <boost/asio.hpp>

namespace rmanconnect
{
    class Client
    {
    public:
        // startup client & test connection
        Client( int port );
        
        // shutdown client
        ~Client();
        
        // send data to the server
        void openImage( Data &header );
        void send( Data &data );
        void closeImage();
        void quit();
        
    private:
        void connect( int port );
        void disconnect();

        // store the port we should connect to
        int mPort, mImageId;
        bool mIsConnected;

        // tcp stuff
        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
    };
}

#endif // RMAN_CONNECT_CLIENT_H_
