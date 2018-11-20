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

#ifndef SAVANNA_H
#define SAVANNA_H

#include "gepard.h"
#include "surfaces/gepard-xsurface.h"
#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <string>
#include <time.h>

namespace savanna {

#ifdef SN_LOG
#undef SN_LOG
#endif
/*!
 * \brief A simple LOG macro.
 */
#define SN_LOG(X) do { std::cout << X << std::endl; } while(false)

template<typename T>
T clamp(T x, const T min, const T max)
{
    return std::min(std::max(x, min), max);
}

typedef std::map<std::string, int> ConfigMap;

class ZygoteMark;

/*!
 * \brief The Savanna class
 */
class Savanna {
public:
    typedef enum Reactions {
        // Passes == 0.
        PASS = 0,
        // Controls < 0.
        EXIT = -1,
        // Fails > 0.
        FAIL = 1,
        MEM_LEAK = 1,
    } Reactions;

    struct React {
        explicit React(Reactions ra = PASS) : _ra(ra) {}

        Reactions ra() const { return _ra; }

        bool isPass() const { return this->_ra == PASS; }
        bool isControl() const { return this->_ra < PASS; }
        bool isFail() const { return this->_ra > PASS; }

    private:
        Reactions _ra;
    };

    static React pass() { return React(); }

    void init(int argc, char* argv[]);
    void add(ZygoteMark*);
    int run();

    ConfigMap configs;

private:
    std::list<ZygoteMark*> benchMarks;
    std::unique_ptr<gepard::XSurface> surface;
};

/*!
 * \brief The ZygoteMark class
 */
class ZygoteMark {
public:
    explicit ZygoteMark(ConfigMap& configMap);
    virtual ~ZygoteMark();

    virtual Savanna::React init(gepard::XSurface*) = 0;
    virtual Savanna::React start() = 0;
    virtual Savanna::React run() = 0;
    virtual Savanna::React stop() = 0;

    void step();

    ConfigMap configs;
    int sampleCount;

    int steps;
    long long unsigned int sum;
    time_t currentTime;
    time_t oldTime ;
};

} // savanna namespace

#endif // SAVANNA_H
