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

#include "savanna-benchmarks.h"

#include "gepard.h"
#include "surfaces/gepard-xsurface.h"
#include "savanna.h"
#include <chrono>
#include <math.h>
#include <thread>

namespace savanna {

// MonkeyMark

MonkeyMark::MonkeyMark(ConfigMap& configMap)
    : ZygoteMark(configMap)
    , iterateCount(configs["iterateCount"])
    , rectHeight(configs["rectHeight"])
    , rectNumbers(configs["rectNumbers"])
    , rectWidth(configs["rectWidth"])
    , windowHeight(configs["windowHeight"])
    , windowWidth(configs["windowWidth"])
    , surface(nullptr)
    , gepard(nullptr)
{
}

MonkeyMark::~MonkeyMark()
{
    if (surface) {
        delete surface;
    }
    if (gepard) {
        delete gepard;
    }
}

Savanna::React MonkeyMark::init(gepard::XSurface* srfc)
{
    std::srand(configs["seed"]);

    surface = srfc;

    return Savanna::pass();
}

Savanna::React MonkeyMark::start()
{
    if (gepard)
        return Savanna::React(Savanna::MEM_LEAK);
    gepard = new gepard::Gepard(surface);

    SN_LOG("MonkeyBenchMark START.");
    return Savanna::pass();
}

Savanna::React MonkeyMark::stop()
{
    SN_LOG("MonkeyBenchMark STOP.");

    if (gepard) {
        delete gepard;
    }

    return Savanna::pass();
}

Savanna::React MonkeyMark::run()
{
    for (int i = 0; i <= iterateCount; ++i) {
        drawRects();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));   // Only for CPU sparing.
        if (surface->hasToQuit()) {
            return Savanna::React(Savanna::EXIT);
        }
    }

    return Savanna::pass();
}

void MonkeyMark::drawRects()
{
    for (int i = 0; i < rectNumbers; ++i) {
        int x = std::rand() % (windowWidth);
        int y = std::rand() % (windowHeight);

        gepard->setFillColor(std::rand() % 256, std::rand() % 256, std::rand() % 256, std::rand() % 256);
        gepard->fillRect(x, y, rectWidth, rectHeight);

        step();
     }
}

// SnakeMark

SnakeMark::SnakeMark(ConfigMap &configMap)
    : MonkeyMark(configMap)
    , maxVelocity(configs["maxVelocity"])
    , paintRectSize(configs["paintRectSize"])
    , maxSize2(2 * paintRectSize)
    , maxFlet(paintRectSize * paintRectSize)
    , ratio(float(255) / float(maxFlet))
{
}

Savanna::React SnakeMark::init(gepard::XSurface* srfc)
{
    return MonkeyMark::init(srfc);
}

Savanna::React SnakeMark::start()
{
    if (gepard)
        return Savanna::React(Savanna::MEM_LEAK);
    gepard = new gepard::Gepard(surface);

    SN_LOG("SnakeBenchMark START.");
    return Savanna::pass();
}

Savanna::React SnakeMark::stop()
{
    SN_LOG("SnakeBenchMark STOP.");

    if (gepard) {
        delete gepard;
    }

    return Savanna::pass();
}

Savanna::React SnakeMark::run()
{
    int x = (windowWidth / 4) + std::rand() % (windowWidth / 2);
    int y = (windowHeight / 4) + std::rand() % (windowHeight / 2);
    int dx = std::rand() % maxVelocity - trunc(maxVelocity / 2);
    int dy = std::rand() % maxVelocity - trunc(maxVelocity / 2);

    for (int i = 0; i <= iterateCount; ++i) {
        x += dx;
        if (x - paintRectSize < 0 || x + paintRectSize > windowWidth) {
            dx = -dx;
            x += dx;
            dx = newVelocity(dx);
        }

        y += dy;
        if (y - paintRectSize < 0 || y + paintRectSize > windowHeight) {
            dy = -dy;
            y += dy;
            dy = newVelocity(dy);
        }

        drawRects(x, y);

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));   // Only for CPU sparing.
        if (surface->hasToQuit()) {
            return Savanna::React(Savanna::EXIT);
        }
    }

    return Savanna::pass();
}

void SnakeMark::drawRects(int x, int y)
{
    for (int i = 0; i < rectNumbers; ++i) {
        int dx = std::rand() % (maxSize2) - paintRectSize;
        int dy = std::rand() % (maxSize2) - paintRectSize;
        int a = 255 - int(float(dx * dx + dy * dy) * ratio);

        gepard->setFillColor(std::rand() % 256, std::rand() % 256, std::rand() % 256, a);
        gepard->fillRect(x + dx, y + dy, rectWidth, rectHeight);

        step();
     }
}

int SnakeMark::newVelocity(const int x)
{
    const int sign = x < 0 ? -1 : 1;

    return sign * clamp(abs(x + std::rand() % 7 - 3), 1, maxVelocity);
}

} // savanna namespace
