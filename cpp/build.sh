#!/bin/sh
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
# source distribution.
# 
# This file is part of REDHAWK Basic Components TuneFilterDecimate.
# 
# REDHAWK Basic Components TuneFilterDecimate is free software: you can redistribute it and/or modify it under the terms of 
# the GNU General Public License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any later version.
# 
# REDHAWK Basic Components TuneFilterDecimate is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with this 
# program.  If not, see http://www.gnu.org/licenses/.
#

if [ "$1" = "clean" ]; then
  make clean
else
  # Checks if build is newer than makefile (based on modification time)
  if [ ! -e configure ] || [ ! -e Makefile ] || [ configure.ac -nt Makefile ] || [ Makefile.am -nt Makefile ]; then
    ./reconf
    ./configure
  fi
  make -j
  exit 0
fi
