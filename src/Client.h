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

#ifndef RMAN_CONNECT_CLIENT_H_
#define RMAN_CONNECT_CLIENT_H_

#include "Data.h"
#include <boost/asio.hpp>

//! \namespace rmanconnect
namespace rmanconnect
{
    /*! \class Client
     * \brief Used to send an image to a Server
     *
     * The Client class is created each time an application wants to send
     * an image to the Server. Once it is instantiated the application should
     * call openImage(), send(), and closeImage() to send an image to the
     * Server.
     */
    class Client
    {
    friend class Server;
    public:
        /*! \brief Constructor
         *
         * Creates a new Client object and tell it to connect any messages to
         * the specified port.
         */
        Client( int port );

        //! Destructor
        ~Client();

        /*! \brief Sends a message to the Server to open a new image.
         *
         * The header parameter is used to tell the Server the size of image
         * buffer to allocate.
         */
        void openImage( Data &header );
        
        /*! \brief Sends a section of image data to the Server.
         *
         * Once an image is open a Client can use this to send a series of
         * pixel blocks to the Server. The Data object passed must correctly
         * specify the block position and dimensions as well as provide a
         * pointer to pixel data.
         */
        void sendPixels( Data &data );

        /*! \brief Sends a message to the Server that the Clients has finished
         *
         * This tells the Server that a Client has finished sending pixel
         * information for an image.
         */
        void closeImage();
        
    private:
        void connect( int port );
        void disconnect();
        void quit();

        // store the port we should connect to
        int mPort, mImageId;
        bool mIsConnected;

        // tcp stuff
        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
    };
}

#endif // RMAN_CONNECT_CLIENT_H_
