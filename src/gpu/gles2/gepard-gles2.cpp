/* Copyright (C) 2016, Gepard Graphics
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

#ifdef USE_GLES2

#include "gepard-gles2.h"

#include "gepard-defs.h"

namespace gepard {

namespace gles2 {

GepardGLES2::GepardGLES2(Surface* surface)
    : _surface(surface)
{
    const EGLint configAttribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,

        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_SAMPLES, 0,
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        // Identify OpenGL 2 ES context
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };

    EGLDisplay eglDisplay = 0;
    EGLSurface eglSurface = 0;
    EGLint numOfConfigs = 0;
    EGLConfig eglConfig = 0;
    void* display = _surface->getDisplay();
    if (display) {
        eglDisplay = eglGetDisplay((EGLNativeDisplayType) display);

        // Initialize EGL
        if (eglDisplay == EGL_NO_DISPLAY) {
            //! \todo: LOG: "eglGetDisplay returned EGL_DEFAULT_DISPLAY");
            exit(-1);
        }
        if (eglInitialize(eglDisplay, NULL, NULL) != EGL_TRUE) {
            //! \todo: LOG:  "eglInitialize returned with EGL_FALSE");
            exit(-1);
        }
        if (eglChooseConfig(eglDisplay, configAttribs, &eglConfig, 1, &numOfConfigs) != EGL_TRUE) {
            //! \todo: LOG: "eglChooseConfig returned with EGL_FALSE");
            exit(-1);
        }
        EGLNativeWindowType window = _surface->getWindow();
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, window, NULL);
        if (eglSurface == EGL_NO_SURFACE) {
            //! \todo: LOG:  "eglCreateWindowSurface returned EGL_NO_SURFACE");
            exit(-1);
        }
        _eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
        if (_eglContext == EGL_NO_CONTEXT) {
            //! \todo: LOG: "eglCreateContext returned EGL_NO_CONTEXT");
            exit(-1);
        }
        if (eglMakeCurrent(eglDisplay, eglSurface, eglSurface, _eglContext) != EGL_TRUE) {
            //! \todo: LOG: "eglMakeCurrent returned EGL_FALSE");
            exit(-1);
        }

        // Set EGL display & surface.
        _eglDisplay = eglDisplay;
        _eglSurface = eglSurface;
    }
}

/*!
 * \brief GepardGLES2::closePath
 *
 * \todo unimplemented function
 */
void GepardGLES2::closePath()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::moveTo
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardGLES2::moveTo(Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::lineTo
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardGLES2::lineTo(Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::quadraticCurveTo
 * \param cpx  X-axis value of _control_ poin
 * \param cpy  Y-axis value of _control_ poin
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardGLES2::quadraticCurveTo(Float cpx, Float cpy, Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::bezierCurveTo
 * \param cp1x  X-axis value of _first control_ point
 * \param cp1y  Y-axis value of _first control_ point
 * \param cp2x  X-axis value of _second control_ point
 * \param cp2y  Y-axis value of _second control_ point
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardGLES2::bezierCurveTo(Float cp1x, Float cp1y, Float cp2x, Float cp2y, Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::arcTo
 * \param x1  X-axis value of _tangent_ point
 * \param y1  Y-axis value of _tangent_ point
 * \param x2  X-axis value of _end_ point
 * \param y2  Y-axis value of _end_ point
 * \param radius  size of arc
 *
 * \todo unimplemented function
 */
void GepardGLES2::arcTo(Float x1, Float y1, Float x2, Float y2, Float radius)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::rect
 * \param x  X-axis value of _start_ and _end_ point
 * \param y  Y-axis value of _start_ and _end_ point
 * \param w  size on X-axis
 * \param h  size on Y-axis
 *
 * \todo unimplemented function
 */
void GepardGLES2::rect(Float x, Float y, Float w, Float h)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::arc
 * \param x  X-axis value of _center_ point
 * \param y  Y-axis value of _center_ point
 * \param radius  size of arc
 * \param startAngle  specify the _start_ point on arc
 * \param endAngle  specify the _end_ point on arc
 * \param counterclockwise  specify the draw direction on arc
 *
 * \todo unimplemented function
 */
void GepardGLES2::arc(Float x, Float y, Float radius, Float startAngle, Float endAngle, bool counterclockwise)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::beginPath
 *
 * \todo unimplemented function
 */
void GepardGLES2::beginPath()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::fill
 *
 * \todo unimplemented function
 */
void GepardGLES2::fill()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::stroke
 *
 * \todo unimplemented function
 */
void GepardGLES2::stroke()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::drawFocusIfNeeded
 *
 * \todo unimplemented function
 */
void GepardGLES2::drawFocusIfNeeded(/*Element element*/)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::clip
 *
 * \todo unimplemented function
 */
void GepardGLES2::clip()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardGLES2::isPointInPath
 * \param x  X-axis value of the given point
 * \param y  Y-axis value of the given point
 * \return  true if the given _point_ is in the current path
 *
 * \todo unimplemented function
 */
bool GepardGLES2::isPointInPath(Float x, Float y)
{
    NOT_IMPLEMENTED();
    return false;
}

#include <iostream>

/*!
 * \brief GepardGLES2::fillRect
 * \param x  X-axis value of _start_ and _end_ point
 * \param y  Y-axis value of _start_ and _end_ point
 * \param w  size on X-axis
 * \param h  size on Y-axis
 *
 * \todo unimplemented function
 */
void  GepardGLES2::fillRect(float x, float y, float w, float h)
{
    //! \todo: implement fillRect with GLES2

    eglSwapBuffers(_eglDisplay, _eglSurface);
}

} // namespace gles2

// GepardEngine functions

/*!
 * \brief GepardEngine::closePath
 */
void GepardEngine::closePath()
{
    _engineBackend->closePath();
}

/*!
 * \brief GepardEngine::moveTo
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardEngine::moveTo(Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::lineTo
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardEngine::lineTo(Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::quadraticCurveTo
 * \param cpx  X-axis value of _control_ poin
 * \param cpy  Y-axis value of _control_ poin
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardEngine::quadraticCurveTo(Float cpx, Float cpy, Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::bezierCurveTo
 * \param cp1x  X-axis value of _first control_ point
 * \param cp1y  Y-axis value of _first control_ point
 * \param cp2x  X-axis value of _second control_ point
 * \param cp2y  Y-axis value of _second control_ point
 * \param x  X-axis value of _end_ point
 * \param y  Y-axis value of _end_ point
 *
 * \todo unimplemented function
 */
void GepardEngine::bezierCurveTo(Float cp1x, Float cp1y, Float cp2x, Float cp2y, Float x, Float y)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::arcTo
 * \param x1  X-axis value of _tangent_ point
 * \param y1  Y-axis value of _tangent_ point
 * \param x2  X-axis value of _end_ point
 * \param y2  Y-axis value of _end_ point
 * \param radius  size of arc
 *
 * \todo unimplemented function
 */
void GepardEngine::arcTo(Float x1, Float y1, Float x2, Float y2, Float radius)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::rect
 * \param x  X-axis value of _start_ and _end_ point
 * \param y  Y-axis value of _start_ and _end_ point
 * \param w  size on X-axis
 * \param h  size on Y-axis
 *
 * \todo unimplemented function
 */
void GepardEngine::rect(Float x, Float y, Float w, Float h)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::arc
 * \param x  X-axis value of _center_ point
 * \param y  Y-axis value of _center_ point
 * \param radius  size of arc
 * \param startAngle  specify the _start_ point on arc
 * \param endAngle  specify the _end_ point on arc
 * \param counterclockwise  specify the draw direction on arc
 *
 * \todo unimplemented function
 */
void GepardEngine::arc(Float x, Float y, Float radius, Float startAngle, Float endAngle, bool counterclockwise)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::beginPath
 *
 * \todo unimplemented function
 */
void GepardEngine::beginPath()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::fill
 *
 * \todo unimplemented function
 */
void GepardEngine::fill()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::stroke
 *
 * \todo unimplemented function
 */
void GepardEngine::stroke()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::drawFocusIfNeeded
 *
 * \todo unimplemented function
 */
void GepardEngine::drawFocusIfNeeded(/*Element element*/)
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::clip
 *
 * \todo unimplemented function
 */
void GepardEngine::clip()
{
    NOT_IMPLEMENTED();
}

/*!
 * \brief GepardEngine::isPointInPath
 * \param x  X-axis value of the given point
 * \param y  Y-axis value of the given point
 * \return  true if the given _point_ is in the current path
 *
 * \todo unimplemented function
 */
bool GepardEngine::isPointInPath(Float x, Float y)
{
    NOT_IMPLEMENTED();
    return false;
}

/*!
 * \brief GepardEngine::fillRect
 * \param x  X-axis value of _start_ and _end_ point
 * \param y  Y-axis value of _start_ and _end_ point
 * \param w  size on X-axis
 * \param h  size on Y-axis
 *
 * \todo documentation is missing
 */
void  GepardEngine::fillRect(float x, float y, float w, float h)
{
    _engineBackend->fillRect(x, y, w, h);
}

void GepardEngine::engineBackendInit()
{
    _engineBackend = reinterpret_cast<gles2::GepardGLES2*>(new gles2::GepardGLES2(_surface));
}

void GepardEngine::engineBackendDestroy()
{
    if (_engineBackend) {
        delete reinterpret_cast<gles2::GepardGLES2*>(_engineBackend);
    }
}

} // namespace gepard

#endif // USE_GLES2
