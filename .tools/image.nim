#
# Copyright (C) 2018 - 2022 bolthur project.
#
# This file is part of bolthur/kernel.
#
# bolthur/kernel is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# bolthur/kernel is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with bolthur/kernel.  If not, see <http://www.gnu.org/licenses/>.
#

import std/os
# local stuff
from util/image import createPlainImageFile

# check argument count
let argc: int = paramCount()
if argc != 3:
  echo "Usage: ramdisk <image name> <image type> <destination folder>"
  quit( 1 )

# get command line arguments
let imageName: string = paramStr( 1 )
let imageType: string = paramStr( 2 )
let destinationFolder: string = paramStr( 3 )

# remove tmp dir again
removeDir( joinPath( "tmp_" & imageType ) )
# create tmp directories
createDir( joinPath( "tmp_" & imageType ) )

# create image
createPlainImageFile( imageType, imageName, destinationFolder )

# remove tmp dir again
removeDir( joinPath( "tmp_" & imageType ) )
