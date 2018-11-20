/* Copyright (C) 2018, Gepard Graphics
 * Copyright (C) 2018, Szilard Ledan <szledan@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gepard.h"
#include "nanoargparse.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#include "surfaces/gepard-png-surface.h"
#include "surfaces/gepard-xsurface.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

static const bool isCollinear(const float x, const float y, const float* pts)
{
    const float threshold = 1.0f / float(0x1 << 20);
    const bool first3pointCollinear = std::fabs(pts[0] * (pts[3] - y) + pts[2] * (y - pts[1]) + x * (pts[1] - pts[3])) < threshold;
    const bool last3pointCollinear = std::fabs(pts[0] * (pts[3] - pts[5]) + pts[2] * (pts[5] - pts[1]) + pts[4] * (pts[1] - pts[3])) < threshold;
    return first3pointCollinear && last3pointCollinear;
}

static void convertRawDataToRGBA(const uint32_t raw, int& red, int& green, int& blue, float& alpha)
{
    red = raw & 0x000000ff;
    green = (raw & 0x00ff00) >> 8;
    blue = (raw & 0x00ff0000) >> 16;
    alpha = float((raw & 0xff000000) >> 24) / 255.0f;
}

void parseNSVGimage(gepard::Gepard& ctx, const NSVGimage* img)
{
    NSVGshape* shp = img->shapes;
    while (shp) {
        NSVGpath* pth = shp->paths;

        // Parse path (aka 'd' property).
        ctx.beginPath();
        while(pth) {
            const float* pts = pth->pts;
            float fromX = pts[0];
            float fromY = pts[1];
            ctx.moveTo(fromX, fromY);

            // Every path element is a cubic (Bézier) curve in NSVG struct.
            // We need to try replace these functions to easier ones.
            for (int i = 1; i < pth->npts; i += 3) {
                const float toX = pts[i * 2 + 4];
                const float toY = pts[i * 2 + 5];
                if (isCollinear(fromX, fromY, &(pts[i * 2]))) {
                    ctx.lineTo(toX, toY);
                } else {
                    ctx.bezierCurveTo(pts[i * 2], pts[i * 2 + 1], pts[i * 2 + 2], pts[i * 2 + 3], toX, toY);
                }
                fromX = toX;
                fromY = toY;
            }
            if (pth->closed) {
                ctx.closePath();
            }
            pth = pth->next;
        }

        int red, green, blue;
        float alpha;

        // Parse fill style and call fill().
        if (shp->fill.type != NSVG_PAINT_NONE) {
            switch (shp->fill.type) {
            case NSVG_PAINT_COLOR:
                convertRawDataToRGBA(shp->fill.color, red, green, blue, alpha);
                ctx.setFillColor(red * shp->opacity, green * shp->opacity, blue * shp->opacity, alpha * shp->opacity);
                break;
            case NSVG_PAINT_LINEAR_GRADIENT:
                /*! \todo: unipmlemnted */;
                break;
            case NSVG_PAINT_RADIAL_GRADIENT:
                /*! \todo: unipmlemnted */;
                break;
            case NSVG_PAINT_NONE:
            default:
                break;
            }

            // Fill the path.
            ctx.fill(); //! \todo: filltype: fill("nonzero"), fill("evenodd")
        }

        // Parse stroke style and stroke().
        if (shp->stroke.type != NSVG_PAINT_NONE) {
            // Parse stroke color style.
            switch (shp->stroke.type) {
            case NSVG_PAINT_COLOR:
                convertRawDataToRGBA(shp->stroke.color, red, green, blue, alpha);
                ctx.setStrokeColor(red * shp->opacity, green * shp->opacity, blue * shp->opacity, alpha * shp->opacity);
                break;
            case NSVG_PAINT_LINEAR_GRADIENT:
                /*! \todo: unipmlemnted */;
                break;
            case NSVG_PAINT_RADIAL_GRADIENT:
                /*! \todo: unipmlemnted */;
                break;
            case NSVG_PAINT_NONE:
            default:
                break;
            }

            // Parse stroke line width.
            ctx.lineWidth = shp->strokeWidth;

            //! \todo: strokeDashOffset
            //! \todo: strokeDashArray
            //! \todo: strokeDashCount

            // Parse join's type of lines.
            switch (shp->strokeLineJoin) {
            case NSVG_JOIN_MITER:
                ctx.lineJoin = "miter";
                break;
            case NSVG_JOIN_ROUND:
                ctx.lineJoin = "round";
                break;
            case NSVG_JOIN_BEVEL:
                ctx.lineJoin = "bevel";
                break;
            default:
                break;
            }

            // Parse cap's type of lines.
            switch (shp->strokeLineCap) {
            case NSVG_CAP_BUTT:
                ctx.lineCap = "butt";
                break;
            case NSVG_CAP_ROUND:
                ctx.lineCap = "round";
                break;
            case NSVG_CAP_SQUARE:
                ctx.lineCap = "square";
                break;
            default:
                break;
            }

            // Parse miter limit.
            ctx.miterLimit = shp->miterLimit;

            // Draw the stroke of path.
            ctx.stroke();
        }

        shp = shp->next;
    }
}

int main(int argc, char* argv[])
{
    // Parse arguments.
    INIT_ARGS(argc, argv);
    if (CHECK_FLAG("-h, --help, --usage")) {
        std::cout << "SVG rastering with Gepard *** (C) 2018. Szilard Ledan" << std::endl;
        std::cout << "Usage: " << std::string(argv[0]) << " [options] SVGFILE" << std::endl
                  << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help      show this help." << std::endl;
        std::cout << "  -p, --png FILE  use png output and set file name." << std::endl;
        std::cout << "  -C, --no-clear  disable canvas clear." << std::endl;
        return 0;
    }

    struct {
        const int isOn;
        const std::string file;
    } a_png = {
        CHECK_FLAG("-p, --png"),
        GET_VALUE("-p, --png", "build/tiger.png")
    };
    const bool a_disableClear = CHECK_FLAG("-C, --no-clear");
    const std::string a_svgFile = GET_VALUE("", "./apps/svggepard/tiger.svg");

    // Open and parse SVG file.
    NSVGimage* pImage = nsvgParseFromFile(a_svgFile.c_str(), "px", 96);
    if (!pImage) {
        std::cerr << "Wrong SVG file or not exist." << std::endl;
        return 1;
    }

    // Set surface and initial the Gepard context.
    const uint width = pImage->width;
    const uint height = pImage->height;
    gepard::Surface* surface = a_png.isOn ? (gepard::Surface*)new gepard::PNGSurface(width, height)
                                          : (gepard::Surface*)new gepard::XSurface(width, height);
    gepard::Gepard gepard(surface);

    // Clear the canvas.
    if (!a_disableClear) {
        gepard.fillStyle = "#fff";
        gepard.fillRect(0, 0, width, height);
    }

    // Parse NSVGImage.
    parseNSVGimage(gepard, pImage);

    nsvgDelete(pImage);

    // Put the results.
    if (a_png.isOn) {
        gepard::PNGSurface* pngSurface = (gepard::PNGSurface*)surface;
        pngSurface->save(a_png.file);
        if (pngSurface) {
            delete pngSurface;
        }
    } else {
        gepard::XSurface* xSurface = (gepard::XSurface*)(surface);

        XEvent xEvent;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (XCheckWindowEvent((Display*)xSurface->getDisplay(), (Window)xSurface->getWindow(), KeyPress | ClientMessage, &xEvent)) {
                break;
            }
        }
        if (xSurface) {
            delete xSurface;
        }
    }

    return 0;
}
