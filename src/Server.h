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

#ifndef RMAN_CONNECT_SERVER_H_
#define RMAN_CONNECT_SERVER_H_

#include "Data.h"
#include <boost/asio.hpp>

//! \namespace rmanconnect
namespace rmanconnect
{
    /*! \class Server
     * \brief Represents a listening Server, ready to accept incoming images.
     *
     * This class wraps up the provision of a TCP port, and handles incoming
     * connections from Client objects when they're ready to send image data.
     */
    class Server
    {
    public:
        /*! \brief Constructor.
         *
         * Creates a new server. By default the Server is not connected at
         * creation time.
         */
        Server();
        /*! \brief Constructor.
         *
         * Creates a new server and calls connect() with the specified port
         * number.
         */
        Server( int port );
        /*! \brief Destructor.
         *
         *  Shuts down the server, closing any open ports if the server is
         *  connected.
         */
        ~Server();
        
        /*! \brief Connects the server to a port.
         *
         * If true is passed as the second parameter then the server will
         * search for the first available port if the specified one is not
         * available. To find out which port the server managed to connect to,
         * call getPort() afterwards.
         */
        void connect( int port, bool seach=false );

        /*! \brief Listens for incoming Client connections.
         *
         * This function blocks (and so may be require running on a separate
         * thread), returning once a Client has sent a message.
         *
         * The returned Data object is filled with the relevant information and
         * passed back ready for handling by the parent application.
         */
        Data listen();

        /*! \brief Sends a 'quit' message to the server.
         *
         * This can be used to exit a listening loop running on a separate
         * thread.
         */
        void quit();

        //! Returns whether or not the server is connected to a port.
        bool isConnected(){ return ( mpAcceptor!=0 ); }

        //! Returns the port the server is currently connected to.
        int getPort(){ return mPort; }

    private:

        // the port we're listening to
        int mPort;

        // boost::asio tcp stuff
        boost::asio::io_service mIoService;
        boost::asio::ip::tcp::socket mSocket;
        boost::asio::ip::tcp::acceptor* mpAcceptor;
    };
}

/*! \mainpage RmanConnect
 *
 * The RmanConnect project is a RenderMan Interface-compatible display driver
 * and Nuke plugin for direct rendering in the Nuke interface.
 *
 * RenderMan® is a registered trademark of Pixar.<br>
 * Nuke® is a registered trademark of The Foundry.
 *
 * \image html rmanconnect_grab.jpg
 *
 * RmanConnect is based on a simple Client/Server model and the classes for
 * those are described <a href="annotated.html">here</a>.
 *
 * The code is freely available from http://github.com/danbethell/rmanconnect
 * and is released under the Revised BSD license. See \link COPYING \endlink
 * for more details.
 *
 * © Copyright 2010, Dan Bethell, Johannes Saam.
 */

/*! \page COPYING
© Copyright 2010, Dan Bethell, Johannes Saam.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

<ul>
    <li>Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.</li>

    <li>Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.</li>

    <li>Neither the name of RmanConnect nor the names of its contributors may be
    used to endorse or promote products derived from this software without
    specific prior written permission.</li>
</ul>
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

#endif // RMAN_CONNECT_SERVER_H_
