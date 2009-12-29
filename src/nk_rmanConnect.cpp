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
            r = g = b = a = 0.f;
        }

        // data
        float r, g, b, a;
};

///=====
/// @class RmanBuffer
/// @brief describes an image buffer or pixels
class RmanBuffer
{
    public:
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
        bool m_killThread;
        float m_lastUpdate;
        FormatPair m_formats;
        Lock m_mutex;

        // the renderman image dimensions
        unsigned int m_rmanWidth;
        unsigned int m_rmanHeight;

        // our pixel buffer
        RmanBuffer m_buffer;
        unsigned int hash_counter; // our refresh hash counter
        rmanconnect::Server* mp_server;
        int m_port;

        RmanConnect(Node* node) :
            Iop(node), m_port(9201), mp_server(0)
        {
            // create server
            resetServer(m_port);

            // spawn the listening thread on node creation
            Thread::spawn(::rmanConnectListen, 1, this);
        }

        // Destroying the Op should get rid of the parallel threads.
        // Unfortunatly currently Nuke does not destroy one of the Ops on a
        // deleted node, as it is saving it for Undo. This bug will be fixed
        // in an upcoming version, so you should implement this:
        ~RmanConnect()
        {
            m_killThread = true;
            Thread::wait(this);
            delete mp_server;
        }

        void resetServer(int port, bool search = true)
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
                        std::cerr << "Opened rmanconnect server at port "
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

        void _validate(bool for_real)
        {
            // TODO: create a new format from our m_buffer->width & m_buffer->height
            // and return that

            info_.full_size_format(*m_formats.fullSizeFormat());
            info_.format(*m_formats.format());
            info_.channels(Mask_RGBA);
            info_.set(format());
        }

        void _request(int x, int y, int r, int t, ChannelMask channels,
                int count)
        {
            input0().validate();
        }

        // The hash value must change or Nuke will think the picture is the
        // same. If you can't determine some id for the picture, you should
        // use the current time or something.
        void append(Hash& hash)
        {
            hash.append(m_lastUpdate);
        }

        void engine(int y, int xx, int r, ChannelMask channels, Row& out)
        {
            unsigned int imageSize =
                    static_cast<unsigned int> (info().format().width()
                            * info().format().height());

            // fill up the output image from the buffer
            if (m_buffer.size() == imageSize)
            {
                float *rOut = out.writable(Chan_Red) + xx;
                float *gOut = out.writable(Chan_Green) + xx;
                float *bOut = out.writable(Chan_Blue) + xx;
                float *aOut = out.writable(Chan_Alpha) + xx;

                const float *END = rOut + (r - xx);

                unsigned int xxx = static_cast<unsigned int> (xx);
                unsigned int yyy = static_cast<unsigned int> (y);

                while (rOut < END)
                {
                    *rOut = m_buffer.get(xxx, yyy).r;
                    *gOut = m_buffer.get(xxx, yyy).g;
                    *bOut = m_buffer.get(xxx, yyy).b;
                    *aOut = m_buffer.get(xxx, yyy).a;

                    ++rOut;
                    ++gOut;
                    ++bOut;
                    ++aOut;

                    ++xxx;
                }
            }
            else
            {
                Row input(xx, r);
                input0().get(y, xx, r, Mask_All, input);
                out.copy(input, Mask_All, xx, r);
            }
        }

        int minimum_inputs() const
        {
            return 1;
        }

        int maximum_inputs() const
        {
            return 1;
        }

        void knobs(Knob_Callback f)
        {
            Format_knob(f, &m_formats, "m_formats_knob", "format");
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

        const char* Class() const
        {
            return CLASS;
        }
        const char* node_help() const
        {
            return HELP;
        }
        static const Iop::Description d;
};
//=====

//=====
// @brief our listening thread method
static void rmanConnectListen(unsigned index, unsigned nthreads, void* data)
{
    RmanConnect * node = reinterpret_cast<RmanConnect*> (data);
    if (!node->mp_server)
    {
        std::cerr << "Could not find opened rmanconnect port!" << std::endl;
        node->m_port = -1;
        return;
    }

    while (!node->m_killThread)
    {
        // block here until we get some data
        rmanconnect::Data d = node->mp_server->listen();

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

                unsigned int _x, _x0, _y, _y0, offset;
                _x = _x0 = _y = _y0 = 0;

                const float* pixel_data = d.data();
                for (_x = 0; _x < d.width(); ++_x)
                    for (_y = 0; _y < d.height(); ++_y)
                    {
                        RmanColour &pix = node->m_buffer.get(_x
                                + d.x(), _h - (_y + d.y() + 1));
                        offset = (d.width() * _y * d.spp()) + (_x * d.spp());
                        pix.r = pixel_data[offset + 0];
                        pix.g = pixel_data[offset + 1];
                        pix.b = pixel_data[offset + 2];
                        pix.a = pixel_data[offset + 3];
                    }
                delete[] pixel_data;

                // release buffer
                node->m_mutex.unlock();
                break;
            }
            case 2: // close image
            {
                break;
            }
        }

        // after the update set the last update time
        node->m_lastUpdate = static_cast<float> (time(NULL));
        node->invalidate();
        node->asapUpdate();
    }
}

//=====
// nuke builder stuff
static Iop* build(Node* node)
{
    return new RmanConnect(node);
}
const Iop::Description RmanConnect::d(CLASS, 0, build);
