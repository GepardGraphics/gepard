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

#ifndef SAVANNA_BENCHMARKS_H
#define SAVANNA_BENCHMARKS_H

#include "gepard.h"
#include "surfaces/gepard-xsurface.h"
#include "savanna.h"

namespace savanna {

/*!
 * \brief The MonkeyMark class
 */
class MonkeyMark : public ZygoteMark {
public:
    explicit MonkeyMark(ConfigMap& configMap);
    virtual ~MonkeyMark();

    virtual Savanna::React init(gepard::XSurface*);
    virtual Savanna::React start();
    virtual Savanna::React stop();
    virtual Savanna::React run();

    int iterateCount;
    int rectHeight;
    int rectNumbers;
    int rectWidth;
    int windowHeight;
    int windowWidth;

protected:
    gepard::XSurface* surface;
    gepard::Gepard* gepard;

private:
    void drawRects();
};

/*!
 * \brief The SnakeMark class
 */
class SnakeMark : public MonkeyMark {
public:
    explicit SnakeMark(ConfigMap& configMap);

    Savanna::React init(gepard::XSurface*);
    Savanna::React start();
    Savanna::React stop();
    Savanna::React run();

    int maxVelocity;
    int paintRectSize;

    int maxSize2;
    int maxFlet;
    float ratio;

private:
    void drawRects(int x, int y);
    int newVelocity(int x);
};

} // savanna namespace

#endif // SAVANNA_BENCHMARKS_H
