#!/bin/bash

# Copyright (C) 2016, Gepard Graphics
# Copyright (C) 2016, Attila Dusnoki <dati91@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

usage() {
  echo "Usage: $0 [OPTIONS]"
  echo "Options:"
  echo "-d, --developer: install developer libraries"
  echo "-q, --no-prompt: no progress info"
  echo "--gles2: install gles2 libraries"
  echo "--vulkan: install vulkan libraries"
  exit 1
}

while test "$1" != ""
do
  case "$1" in
    -d|--developer) do_inst_dev=1;;
    -q|--no-prompt) do_quietly="-qq --assume-yes";;
    --gles2) do_inst_gles2=1;;
    --vulkan) do_inst_vulkan=1;;
    *) usage;;
  esac
  shift
done

common_list="cmake libx11-dev libpng-dev"
gles2_list="libegl1-mesa-dev libgles2-mesa-dev"
vulkan_list=""

dev_list="cppcheck doxygen graphviz"

if test "$do_inst_dev" = "1"; then
  echo "Including developer dependencies."
else
  echo "Skipping developer dependencies."
  dev_list=
fi

if test "$do_inst_gles2" != "1" && test "$BACKEND" != "gles2"; then
  gles2_list=
fi

if test "$do_inst_vulkan" != "1" && test "$BACKEND" != "vulkan"; then
  vulkan_list=
fi

packages="$(
echo "${dev_list} ${common_list} ${gles2_list} ${vulkan_list}" | tr " " "\n"
)"

sudo apt-get update ${do_quietly}
sudo apt-get install ${do_quietly} ${packages}
