#ifndef RMAN_CONNECT_SERVER_H_
#define RMAN_CONNECT_SERVER_H_

#include "Data.h"
#include <boost/asio.hpp>

namespace rmanconnect
{
    class Server
    {
    public:
        Server( int port ); // start up the listening server
        ~Server(); // shutdown the server
        
        void reconnect( int port );
        Data listen();
        void quit(int port);

    private:
        int mPort; // the port we should listen to

        // tcp stuff
        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
        boost::asio::ip::tcp::acceptor mAcceptor;
    };
}

#endif // RMAN_CONNECT_SERVER_H_
