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

from std/os import splitPath, fileExists, copyFile, removeFile, joinPath, getCurrentDir, walkDirRec, pcFile, pcDir, getFileInfo
from std/strutils import intToStr, replace
from std/math import pow
from std/osproc import execCmdEx

var mega: int = int( pow( 2.0, 20.0 ) )

proc imageDd( source: string, target: string, size: int, blockSize: int, offset: int ): void =
  var cmd:string = "dd if=" & source & " of=" & target & " bs=" & intToStr( blockSize ) & " count=" & intToStr( int( size / blockSize ) ) & " skip=" & intToStr( int( offset / blockSize ) )
  if -1 == size:
    cmd = "dd if=" & source & " of=" & target & " bs=" & intToStr( blockSize ) & " seek=" & intToStr( int( offset / blockSize ) )

  let result: tuple[output: string, exitCode: int] = execCmdEx( cmd )
  if 0 != result.exitCode:
    echo "Unable to create raw image: " & result.output
    quit( 1 )

proc imageCreatePartition( target: string, contentFolder: string, imageType: string ): void =
  let basePath: string = joinPath( getCurrentDir(), "tmp_" & imageType )
  let partitionImage: string = joinPath( basePath, "image.img" )

  let blockSizeImage: int = 512
  let blockSize: int = 1024
  var currentOffset: int = 0

  var useMtools: bool = false
  var imageMegaSize: float = 0
  var command: string = ""
  var partitionType: string = ""
  var partitionSubtract: int = 0
  # determine image size
  case imageType:
    of "fat12":
      useMtools = true
      imageMegaSize = 1.44
      command = "mkfs.fat -F12 " & partitionImage & " --mbr=yes"
      partitionType = "01"
      partitionSubtract = blockSizeImage
    of "fat16":
      useMtools = true
      imageMegaSize = 32
      command = "mkfs.vfat -F16 " & partitionImage & " --mbr=yes"
      partitionType = "04"
      partitionSubtract = blockSizeImage
    of "fat32":
      useMtools = true
      imageMegaSize = 64
      command = "mkfs.vfat -F32 " & partitionImage & " --mbr=yes"
      partitionType = "0c"
      partitionSubtract = blockSizeImage
    of "ext2":
      useMtools = false
      imageMegaSize = 32
      command = "mke2fs -t ext2 -d " & joinPath( contentFolder, "test" ) & " -r 1 -N 0 -m 5 " & partitionImage & " " & intToStr( imageMegaSize.toInt ) & "M"
    else:
      echo "Unsupported image type passed"
      quit( 1 )
  # calculate partition size by mega size
  let partitionSize: int = int( imageMegaSize * float( mega ) )

  var cmdResult: tuple[output: string, exitCode: int]
  # create raw files for boot, root and destination image
  imageDd( "/dev/zero", partitionImage, partitionSize, blockSize, currentOffset )
  # format image
  cmdResult = execCmdEx( command )
  if 0 != cmdResult.exitCode:
    echo "Unable to format partition: " & cmdResult.output
    quit( 1 )
  # copy files with mtools if flag is set
  if true == useMtools:
    # copy normal files
    let filesToCopy:string = joinPath( contentFolder, "test" )
    for file in walkDirRec( filesToCopy, { pcFile, pcDir } ):
      var info = getFileInfo( file )
      if info.kind == pcDir:
        var localfolder = file.replace( filesToCopy )
        cmdResult = execCmdEx( "mmd -i " & partitionImage & " ::" & localfolder )
      else:
        var splitted = splitPath( file )
        if splitted.tail == ".gitkeep":
          continue
        var localfolder = splitted.head.replace( filesToCopy )
        cmdResult = execCmdEx( "mcopy -i " & partitionImage & " " & file & " ::" & localfolder )
      if 0 != cmdResult.exitCode:
        echo "Unable to copy content to boot partition: " & cmdResult.output
        quit( 1 )
    # copy possible additional files for type
    let additionalFile:string = joinPath( contentFolder, imageType, "test" )
    for file in walkDirRec( additionalFile, { pcFile, pcDir } ):
      var info = getFileInfo( file )
      if info.kind == pcDir:
        var localfolder = file.replace( additionalFile )
        cmdResult = execCmdEx( "mmd -i " & partitionImage & " ::" & localfolder )
      else:
        var splitted = splitPath( file )
        var localfolder = splitted.head.replace( additionalFile )
        cmdResult = execCmdEx( "mcopy -i " & partitionImage & " " & file & " ::" & localfolder )
      if 0 != cmdResult.exitCode:
        echo "Unable to copy content to boot partition: " & cmdResult.output
        quit( 1 )
  # set partitions
  cmdResult = execCmdEx( """printf "
type=""" & partitionType & """, size=""" & intToStr( int( partitionSize / blockSizeImage ) ) & """
" | sfdisk """" & partitionImage & """"""" )
  if 0 != cmdResult.exitCode:
    echo "Unable to set partition types in target image: " & cmdResult.output
    quit( 1 )

proc createPlainImageFile*( imageType: string, imageName: string, destinationFolder: string ): void =
  # bunch of variables
  let basePath: string = joinPath( getCurrentDir(), "tmp_" & imageType )
  let imagePath: string = joinPath( basePath, "image.img" )
  let contentDirectoryPath: string = joinPath( getCurrentDir(), "files" )
  let destinationPath: string = joinPath( getCurrentDir(), imageType & ".img" )
  # create partition
  imageCreatePartition( imagePath, contentDirectoryPath, imageType )
  if fileExists( destinationPath ): removeFile( destinationPath )
  copyFile( imagePath, joinPath( destinationFolder, imageType & ".img" ) )
