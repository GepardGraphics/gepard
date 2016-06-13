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

#ifndef GEPARD_ENGINE_H
#define GEPARD_ENGINE_H

#include "gepard-defs.h"

#include "gepard-image.h"
#include "gepard-surface.h"
#include "gepard-types.h"
#ifdef USE_GLES2
#include "gepard-gles2.h"
#endif // USE_GLES2

namespace gepard {

class Image;
class Surface;

#ifdef USE_GLES2
namespace gles2 {
class GepardGLES2;
} // namespace gles2
typedef gles2::GepardGLES2 GepardEngineBackend;
#else
typedef void GepardEngineBackend;
#endif

class GepardEngine {
public:
    explicit GepardEngine(Surface* surface)
        : _surface(surface)
    {
        engineBackendInit();
    }
    ~GepardEngine()
    {
        engineBackendDestroy();
    }

    /* 5. Building paths */
    void closePath();
    void moveTo(Float x, Float y);
    void lineTo(Float x, Float y);
    void quadraticCurveTo(Float cpx, Float cpy, Float x, Float y);
    void bezierCurveTo(Float cp1x, Float cp1y, Float cp2x, Float cp2y, Float x, Float y);
    void arcTo(Float x1, Float y1, Float x2, Float y2, Float radius);
    void rect(Float x, Float y, Float w, Float h);
    void arc(Float x, Float y, Float radius, Float startAngle, Float endAngle, bool counterclockwise = false);

    /* 11. Drawing paths to the canvas */
    void beginPath();
    void fill();
    void stroke();
    void drawFocusIfNeeded(/*Element element*/);
    void clip();
    bool isPointInPath(Float x, Float y);

    void fillRect(float x, float y, float w, float h);

private:
    void engineBackendInit();
    void engineBackendDestroy();

    Surface* _surface;
    GepardEngineBackend* _engineBackend;
};

} // namespace gepard

#endif // GEPARD_ENGINE_H
