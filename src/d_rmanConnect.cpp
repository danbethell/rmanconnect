#include "Client.h"
#include "Data.h"
#include "ImageDesc.h"

#include <ndspy.h>

#include <iostream>
#include <exception>
#include <cstring>

extern "C"
{
    // open our display driver
    PtDspyError DspyImageOpen(PtDspyImageHandle *pvImage,
            const char *drivername, const char *filename, int width,
            int height, int paramCount, const UserParameter *parameters,
            int formatCount, PtDspyDevFormat *format, PtFlagStuff *flagstuff)
    {
        std::cerr << "display open" << std::endl;
        try
        {
            // create a new rmanConnect object
            rmanconnect::Client *client = new rmanconnect::Client();

            // make image header & send to server
            rmanconnect::ImageDesc header;
            header.x = 0;
            header.y = 0;
            header.width = width;
            header.height = height;
            client->openImage( header );

            // create passable pointer for our client object
            *pvImage = reinterpret_cast<PtDspyImageHandle>(client);
        }
        catch (const std::exception &e)
        {
            DspyError("RmanConnect display driver", "%s\n", e.what());
            return PkDspyErrorUndefined;
        }

        return PkDspyErrorNone;
    }

    // handle image data sent from the renderer
    PtDspyError DspyImageData(PtDspyImageHandle pvImage, int xmin,
            int xmax_plusone, int ymin, int ymax_plusone, int entrysize,
            const unsigned char *data)
    {
        try
        {
            rmanconnect::Client *client =
                    reinterpret_cast<rmanconnect::Client*> (pvImage);
            const float *ptr = reinterpret_cast<const float*> (data);

            // create our data object
            rmanconnect::Data data(xmin, ymin, xmax_plusone - xmin,
                    ymax_plusone - ymin, entrysize / sizeof(float), ptr);

            // send it to the server
            client->send(data);
        }
        catch (const std::exception &e)
        {
            DspyError("RmanConnect display driver", "%s\n", e.what());
            return PkDspyErrorUndefined;
        }
        return PkDspyErrorNone;
    }

    // close the display driver
    PtDspyError DspyImageClose(PtDspyImageHandle pvImage)
    {
        std::cerr << "display close" << std::endl;

        try
        {
            rmanconnect::Client *client =
                    reinterpret_cast<rmanconnect::Client*> (pvImage);
            client->closeImage();
            delete client;
        }
        catch (const std::exception &e)
        {
            DspyError("RmanConnect display driver", "%s\n", e.what());
            return PkDspyErrorUndefined;
        }
        return PkDspyErrorNone;
    }

    // query the renderer for image details
    PtDspyError DspyImageQuery(PtDspyImageHandle pvImage,
            PtDspyQueryType querytype, size_t datalen, void *data)
    {
        if (datalen > 0 && data)
        {
            switch (querytype)
            {
                case PkSizeQuery:
                {
                    PtDspySizeInfo sizeInfo;
                    if (datalen > sizeof(sizeInfo))
                        datalen = sizeof(sizeInfo);
                    sizeInfo.width = 512;
                    sizeInfo.height = 512;
                    sizeInfo.aspectRatio = 1.f;
                    memcpy(data, &sizeInfo, datalen);
                    break;
                }

                case PkOverwriteQuery:
                    // not used in 3delight
                    break;

                default:
                    return PkDspyErrorUnsupported;
            }
        }
        else
        {
            return PkDspyErrorBadParams;
        }
        return PkDspyErrorNone;
    }

} // extern C
