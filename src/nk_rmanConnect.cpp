static const char* const CLASS = "RmanConnect";
static const char* const HELP =
        "Connects to renderman in a output through the corresponding display driver";

#include <time.h>
#include <stdio.h>
#include <vector>
#include <string>

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
        FormatPair m_fmt;
        RmanBuffer m_buffer; // our pixel buffer
        Lock m_mutex; // mutex for locking the pixel buffer
        unsigned int hash_counter; // our refresh hash counter
        rmanconnect::Server* mp_server; // our rmanconnect::Server
        int m_port; // the port we're listening on

        RmanConnect(Node* node) :
            Iop(node), m_port(9201), mp_server(0)
        {
            inputs(0);

            // setup our tcp listener
            startServer(m_port);

            // spawn the listening thread on node creation
            Thread::spawn(::rmanConnectListen, 1, this);
        }

        // Destroying the Op should get rid of the parallel threads.
        // Unfortunatly currently Nuke does not destroy one of the Ops on a
        // deleted node, as it is saving it for Undo. This bug will be fixed
        // in an upcoming version, so you should implement this:
        ~RmanConnect()
        {
            mp_server->quit(m_port);
            Thread::wait(this);
            delete mp_server;
        }

        void startServer(int port, bool search = true)
        {
            int start_port = port;
            if (!mp_server)
            {
                // try and find an open port
                while (!mp_server && port < start_port + 99)
                {
                    try
                    {
                        mp_server = new rmanconnect::Server(port);
                        std::cerr << "Opened RmanConnect at localhost:"
                                << port << std::endl;
                        m_port = port;
                        break;
                    }
                    catch (...)
                    {
                        mp_server = 0;
                        if (!search)
                            break;
                        else
                            port++;
                    }
                }
            }
            else
            {
                mp_server->reconnect(port);
                std::cerr << "Opened rmanconnect server at port " << port
                        << std::endl;
            }

            if (!mp_server)
            {
                m_port = -1;
                char buffer[32];
                sprintf(buffer, "port: %d", start_port);
                if (search)
                    sprintf(buffer, "port: %d-%d", start_port, start_port + 99);
                error("%s %s", "Could not open port for rmanconnect!", buffer);
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
            info_.format(*m_fmt.fullSizeFormat());
            info_.full_size_format(*m_fmt.format());
            info_.channels(Mask_RGBA);
            info_.set(info().format());
        }

/*
        void _request(int x, int y, int r, int t, ChannelMask channels,
                int count)
        {
            input0().validate();
        }
*/

        void engine(int y, int xx, int r, ChannelMask channels, Row& out)
        {
            validate(false);

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
                //resetServer( m_port, false );
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
    if (!node->mp_server)
    {
        std::cerr << "Could not find opened RmanConnect port!" << std::endl;
        node->m_port = -1;
        return;
    }

    while (!killThread)
    {
        // block here until we get some data
        rmanconnect::Data d = node->mp_server->listen();

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
                break;
            }
            case 2: // close image
            {
                // do nothing
                break;
            }
            case 9: // this is sent when the parent process want to kill
                    // the listening thread
            {
                killThread = true;
                break;
            }
        }

        // increment our hash_counter
        if ( node->hash_counter==UINT_MAX )
            node->hash_counter=0;
        else
            node->hash_counter++;
        node->asapUpdate();
    }
}

//=====
// nuke builder stuff
static Iop* constructor(Node* node){ return new RmanConnect(node); }
const Iop::Description RmanConnect::desc(CLASS, "Image/RmanConnect", constructor);
