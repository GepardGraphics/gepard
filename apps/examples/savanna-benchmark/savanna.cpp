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

#include "savanna.h"

#include "gepard.h"
#include "surfaces/gepard-xsurface.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <sstream>
#include <string>
#include <time.h>

namespace savanna {

template<typename T>
inline static std::unique_ptr<T> makeUnique(T* ptr)
{
    std::unique_ptr<T> uniquePtr(ptr);
    return std::move(uniquePtr);
}

// Savanna

void Savanna::init(int argc, char* argv[])
{
    SN_LOG("*** Savanna BenchMark for Gepard, 2016. *** ");
    SN_LOG("Usage: " << argv[0] << " [config-file]");

    std::string configFile = "";
    if (argc > 1) {
        configFile = argv[1];
    }

    // Set default configs.
    configs["iterateCount"] = 10;
    configs["maxVelocity"] = 51;
    configs["paintRectSize"] = 30;
    configs["rectHeight"] = 5;
    configs["rectNumbers"] = 20;
    configs["rectWidth"] = 5;
    configs["seed"] = 1985;
    configs["warmupCount"] = 3;
    configs["windowHeight"] = 500;
    configs["windowWidth"] = 500;

    SN_LOG("Read config file...");
    if (!configFile.empty()) {
        std::fstream fs(configFile, std::fstream::in);
        std::string line;

        while (std::getline(fs, line)) {
            // Romove '#' comment part and ' ' chars.
            line = line.substr(0, line.find('#'));
            if (std::remove(line.begin(), line.end(), ' ') == line.end()) {
                continue;
            }

            std::istringstream issLine(line);
            std::string key;

            if (std::getline(issLine, key, '=')) {
                std::string value;

                if (std::getline(issLine, value)) {
                    configs[key] = atoi(value.c_str());
                }
            }
        }

        SN_LOG("...done.");
    } else {
        SN_LOG("...not found. Use default configs.");
    }
    SN_LOG("");

    SN_LOG(" * Configs:");
    for (ConfigMap::iterator config = configs.begin(); config != configs.end(); ++config) {
        SN_LOG(config->first << "=" << config->second);
    }
    SN_LOG("");

    surface = makeUnique(new gepard::XSurface(configs["windowWidth"], configs["windowHeight"]));
}

void Savanna::add(ZygoteMark* zygoteMark)
{
    benchMarks.push_back(zygoteMark);
}

int Savanna::run()
{
    SN_LOG("Savanna BenchMark START");
    Savanna::React reaction;
    for (std::list<ZygoteMark*>::iterator it = benchMarks.begin(); it != benchMarks.end() && reaction.isPass(); ++it) {
        (*it)->init(surface.get());
        if ((*it)->start().isPass()) {
            reaction = (*it)->run();
            (*it)->stop();
        }
    }
    SN_LOG("Savanna BenchMark STOP");
    return reaction.isFail() ? int(reaction.ra()) : 0;
}

// ZygoteMark

ZygoteMark::ZygoteMark(ConfigMap &configMap)
    : configs(configMap)
    , sampleCount(-configs["warmupCount"])
    , steps(0)
    , sum(0)
    , currentTime(0)
    , oldTime(0)
{
}

ZygoteMark::~ZygoteMark()
{
}

void ZygoteMark::step()
{
    steps++;
    time(&currentTime);
    const time_t oneSec = 1;

    if (currentTime - oldTime >= oneSec) {
        float avg = 0;
        sampleCount++;
        if (sampleCount > 0) {
            sum += steps;
            avg = float(sum) / float(sampleCount);
        }
        SN_LOG(sampleCount << ". FPS: " << steps << ", AVG: " << avg << "");

        // Reset.
        steps = 0;
        time(&oldTime);
    }
}

} // savanna namespace
