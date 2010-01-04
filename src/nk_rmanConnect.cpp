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

static const char* const CLASS = "RmanConnect";
static const char* const HELP =
        "Connects to renderman in a output through the corresponding display driver";

#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Thread.h"
#include "DDImage/Knobs.h"
#include "DDImage/DDMath.h"
using namespace DD::Image;

// TODO: format output automatically
// TODO: kill thread on exit nuke
// TODO: client port based on port parameter

#include "Data.h"
#include "Server.h"

// our default port
const int rmanconnect_default_port = 9201;

// our listener method
static void rmanConnectListen(unsigned index, unsigned nthreads, void* data);

/// @class Colour
/// @brief lightweight class for describing an individual pixel
class RmanColour
{
    public:
        RmanColour()
        {
            _val[0] = _val[1] = _val[2] = 0.f;
            _val[3] = 1.f;
        }

        float& operator[](int i){ return _val[i]; }
        const float& operator[](int i) const { return _val[i]; }

        // data
        float _val[4];
};

///=====
/// @class RmanBuffer
/// @brief describes an image buffer or pixels
class RmanBuffer
{
    public:
        RmanBuffer() :
            _width(0),
            _height(0)
        {
        }

        void init(const unsigned int width, const unsigned int height)
        {
            _width = width;
            _height = height;
            _data.resize(_width * _height);
        }

        RmanColour& get(unsigned int x, unsigned int y)
        {
            unsigned int index = (_width * y) + x;
            return _data[index];
        }

        const RmanColour& get(unsigned int x, unsigned int y) const
        {
            unsigned int index = (_width * y) + x;
            return _data[index];
        }

        const unsigned int size() const
        {
            return _data.size();
        }

        // data
        std::vector<RmanColour> _data;
        unsigned int _width;
        unsigned int _height;
};
///=====

///=====
/// @class RmanConnect
/// @brief our nuke node that receives data from an rman display driver
class RmanConnect: public Iop
{
    public:
        FormatPair m_fmt; // our buffer format (knob)
        int m_port; // the port we're listening on (knob)

        RmanBuffer m_buffer; // our pixel buffer
        Lock m_mutex; // mutex for locking the pixel buffer
        unsigned int hash_counter; // our refresh hash counter
        rmanconnect::Server m_server; // our rmanconnect::Server
        bool m_inError; // some error handling
        std::string m_connectionError;
        bool m_legit;

        RmanConnect(Node* node) :
            Iop(node),
            m_port(rmanconnect_default_port),
            m_inError(false),
            m_connectionError(""),
            m_legit(false)
        {
            inputs(0);
        }

        // Destroying the Op should get rid of the parallel threads.
        // Unfortunatly currently Nuke does not destroy one of the Ops on a
        // deleted node, as it is saving it for Undo. This bug will be fixed
        // in an upcoming version, so you should implement this:
        ~RmanConnect()
        {
            disconnect();
        }

        // It seems additional instances of a node get copied/constructed upon 
        // very frequent calls to asapUpdate() and this causes us a few
        // problems - we don't want new sockets getting opened etc.
        // Fortunately attach() only gets called for nodes in the dag so we can
        // use this to mark the DAG node as 'legit' and open the port accordingly.
        void attach()
        {
            m_legit = true;
        }

        void detach()
        {
            // even though a node still exists once removed from a scene (in the
            // undo stack) we should close the port and reopen if attach() gets
            // called.
            m_legit = false;
            disconnect();
        }

        void flagForUpdate()
        {
            if ( hash_counter==UINT_MAX )
                hash_counter=0;
            else
                hash_counter++;
            asapUpdate();
        }

        // we can use this to change our tcp port
        void changePort( int port )
        {
            m_inError = false;
            m_connectionError = "";

            // try to reconnect
            disconnect();
            try
            {
                m_server.connect( m_port );
            }
            catch ( ... )
            {
                std::stringstream ss;
                ss << "Could not connect to port: " << port;
                m_connectionError = ss.str();
                m_inError = true;
                std::cerr << ss.str() << std::endl;
                return;
            }

            // success
            if ( m_server.isConnected() )
            {
                Thread::spawn(::rmanConnectListen, 1, this);
                std::cout << "Connected to port: " << m_server.getPort() << std::endl;
            }
        }

        // disconnect the server for it's port
        void disconnect()
        {
            if ( m_server.isConnected() )
            {
                m_server.quit();
                Thread::wait(this);
                std::cout << "Disconnected from port: " << m_server.getPort() << std::endl;
            }
        }

        // The hash value must change or Nuke will think the picture is the
        // same. If you can't determine some id for the picture, you should
        // use the current time or something.
        void append(Hash& hash)
        {
            hash.append(hash_counter);
        }

        void _validate(bool for_real)
        {
            // do we need to open a port?
            if ( m_server.isConnected()==false && !m_inError && m_legit )
                changePort(m_port);

            // handle any connection error
            if ( m_inError )
                error(m_connectionError.c_str());

            // setup format etc
            info_.format(*m_fmt.fullSizeFormat());
            info_.full_size_format(*m_fmt.format());
            info_.channels(Mask_RGBA);
            info_.set(info().format());
        }

        void engine(int y, int xx, int r, ChannelMask channels, Row& out)
        {
            float *rOut = out.writable(Chan_Red) + xx;
            float *gOut = out.writable(Chan_Green) + xx;
            float *bOut = out.writable(Chan_Blue) + xx;
            float *aOut = out.writable(Chan_Alpha) + xx;
            const float *END = rOut + (r - xx);
            unsigned int xxx = static_cast<unsigned int> (xx);
            unsigned int yyy = static_cast<unsigned int> (y);

            // don't have a buffer yet
            if ( m_buffer._width==0 && m_buffer._height==0 )
            {
                while (rOut < END)
                {
                    *rOut = *gOut = *bOut = *aOut = 0.f;
                    ++rOut;
                    ++gOut;
                    ++bOut;
                    ++aOut;
                    ++xxx;
                }
            }
            else
            {
                while (rOut < END)
                {
                    if ( xxx >= m_buffer._width || yyy >= m_buffer._height )
                    {
                        *rOut = *gOut = *bOut = *aOut = 0.f;
                    }
                    else
                    {
                        *rOut = m_buffer.get(xxx, yyy)[0];
                        *gOut = m_buffer.get(xxx, yyy)[1];
                        *bOut = m_buffer.get(xxx, yyy)[2];
                        *aOut = m_buffer.get(xxx, yyy)[3];
                    }
                    ++rOut;
                    ++gOut;
                    ++bOut;
                    ++aOut;
                    ++xxx;
                }
            }
        }

        void knobs(Knob_Callback f)
        {
            Format_knob(f, &m_fmt, "m_formats_knob", "format");
            Tooltip(f, "Output render format");
            Int_knob(f, &m_port, "port_number", "port");
        }

        int knob_changed(Knob* knob)
        {
            if (knob->name() && strcmp(knob->name(), "port_number") == 0)
            {
                changePort(m_port);
                return 1;
            }
            return 0;
        }

        const char* Class() const { return CLASS; }
        const char* displayName() const { return CLASS; }
        const char* node_help() const { return HELP; }
        static const Iop::Description desc;
};
//=====

//=====
// @brief our listening thread method
static void rmanConnectListen(unsigned index, unsigned nthreads, void* data)
{
    bool killThread = false;

    RmanConnect * node = reinterpret_cast<RmanConnect*> (data);
    while (!killThread)
    {
        // block here until we get some data
        rmanconnect::Data d = node->m_server.listen();

        // handle the data we received
        switch (d.type())
        {
            case 0: // open a new image
            {
                node->m_mutex.lock();
                node->m_buffer.init(d.width(), d.height());
                node->m_mutex.unlock();
                break;
            }
            case 1: // image data
            {
                // lock buffer
                node->m_mutex.lock();

                // copy data from d into node->m_buffer
                int _w = node->m_buffer._width;
                int _h = node->m_buffer._height;

                unsigned int _x, _x0, _y, _y0, _s, offset;
                _x = _x0 = _y = _y0 = _s = 0;

                int _xorigin = d.x();
                int _yorigin = d.y();
                int _width = d.width();
                int _height = d.height();
                int _spp = d.spp();

                const float* pixel_data = d.data();
                for (_x = 0; _x < _width; ++_x)
                    for (_y = 0; _y < _height; ++_y)
                    {
                        RmanColour &pix = node->m_buffer.get(_x
                                + _xorigin, _h - (_y + _yorigin + 1));
                        offset = (_width * _y * _spp) + (_x * _spp);
                        for (_s = 0; _s < _spp; ++_s)
                            pix[_s] = pixel_data[offset+_s];
                    }

                // TODO: ideally when 'd' goes out of scope this could be cleaned up for us
                delete[] pixel_data;

                // release buffer
                node->m_mutex.unlock();

                // update the image
                node->flagForUpdate();
                break;
            }
            case 2: // close image
            {
                // update the image
                node->flagForUpdate();
                break;
            }
            case 9: // this is sent when the parent process want to kill
                    // the listening thread
            {
                killThread = true;
                break;
            }
        }
    }
}

//=====
// nuke builder stuff
static Iop* constructor(Node* node){ return new RmanConnect(node); }
const Iop::Description RmanConnect::desc(CLASS, "Image/RmanConnect", constructor);
