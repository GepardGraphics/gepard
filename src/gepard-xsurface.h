/* Copyright (C) 2016, Gepard Graphics
 * Copyright (C) 2016, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
 * Copyright (C) 2016, Szilard Ledan <szledan@gmail.com>
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

#ifndef GEPARD_XSURFACE_H
#define GEPARD_XSURFACE_H

#include "gepard-defs.h"
#include "gepard-surface.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace gepard {

/*!
 * \brief A simple XSurface class for _Gepard_
 *
 * \todo: documentation is missing.
 */
class XSurface : public Surface {
public:
    XSurface(uint32_t width = 0, uint32_t height = 0)
        : Surface(width, height)
    {
        XSetWindowAttributes windowAttributes;
        XWMHints hints;
        Window root = 0;

        _display = XOpenDisplay(NULL);
        if(_display == NULL) {
            //! \todo: LOG: "XOpenDisplay returned NULL");
        }
        root = XDefaultRootWindow(_display);
        windowAttributes.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
        _window = XCreateWindow ( // create a window with the provided parameters
            _display, root, 0, 0, width, height, 0,
            CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &windowAttributes);
        hints.input = True;
        hints.flags = InputHint;
        XSetWMHints(_display, _window, &hints);

        XMapWindow(_display, _window); // make the window visible on the screen
        XStoreName(_display, _window, "XSurface for gepard"); //! \todo: set title.
    }

    virtual void* getDisplay() { return (void*)_display; }
    virtual unsigned long getWindow() { return _window; }

private:
    Display* _display;
    Window _window;
};

} //  namespace gepard

#endif // GEPARD_XSURFACE_H
