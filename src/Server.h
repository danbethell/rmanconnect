#ifndef RMAN_CONNECT_SERVER_H_
#define RMAN_CONNECT_SERVER_H_

#include "Data.h"
#include <boost/asio.hpp>

namespace rmanconnect
{
    class Server
    {
    public:
        Server(); // start up the listening server
        ~Server(); // shutdown the server
        
        void reconnect( int port, bool seach=false );
        Data listen();
        void quit();

        int getPort(){ return mPort; }
        bool isConnected(){ return ( mpAcceptor!=0 ); }

    private:

        int mPort; // the port we're listening to

        // tcp stuff
        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
        boost::asio::ip::tcp::acceptor* mpAcceptor;
    };
}

#endif // RMAN_CONNECT_SERVER_H_
