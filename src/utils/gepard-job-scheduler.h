/* Copyright (C) 2019, Gepard Graphics
 * Copyright (C) 2019, Szilard Ledan <szledan@gmail.com>
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

#ifndef GEPARD_JOB_SCHEDULER_H
#define GEPARD_JOB_SCHEDULER_H

#include "gepard-defs.h"
#include "gepard-job-runner.h"
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

namespace gepard {

/*!
 * \brief The JobScheduler class
 */
class JobScheduler {
public:
    JobScheduler(JobRunner&, const int64_t timeout = INT64_MAX);
    ~JobScheduler();

    void addJob(std::function<void()> func, std::function<void()> callback = nullptr);

    void waitForJobs() { waitForJobs(_timeout); }
    void waitForJobs(const int64_t timeout);

    const bool timeouted() { return _timeouted; }

private:
    void bindFunc(std::function<void()>);
    void callbackFunc(std::function<void()>);

    JobRunner& _jobRunner;
    const int64_t _timeout;
    const int64_t _waitTime = 100; //! \todo: CMake variable?

    std::mutex _mutex;
    int32_t _activeJobCount = 0;
    bool _timeouted = false;

};

} // namespace gepard

#endif // GEPARD_JOB_SCHEDULER_H
