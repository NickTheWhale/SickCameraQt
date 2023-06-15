#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script demonstrates how to retrieve depth data from a Visionary-S device
and save the data as png images. The following images are stored:
- Z-map image
- RGBA image
- Statemap image
- Z-map image with applied statemap

The sample assumes the data streaming on the Visionary is already running
(configured in Sopas-ET to be either free-running or externally triggered with
 a proper external trigger input).

Author: GBC09 / BU05 / SW
SICK AG, Waldkirch
email: techsupport0905@sick.de

Copyright note: Redistribution and use in source, with or without modification, are permitted.

Liability clause: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import argparse

from skimage import io
import numpy as np

from common.Stream import Streaming as Streaming
from common.Streaming import Data

# === Device specific protocol & control_port mapping ===
# Visionary-S CX           => CoLaB / Port 2112
# handle argument parsing from cmd arguments
parser = argparse.ArgumentParser(description="Exemplary data reception from SICK Visionary devices.")
parser.add_argument('-i', '--ipAddress', required=False, type=str,
                    default="192.168.1.10", help="The ip address of the device.")
parser.add_argument('-c', '--control_port', required=False, type=int,
                    default=2112, help="The control port to change settings.")
parser.add_argument('-s', '--streaming_port', required=False, type=int,
                    default=2114, help="The tcp port of the data channel.")
args = parser.parse_args()

deviceStreaming = Streaming(args.ipAddress, args.streaming_port)
deviceStreaming.openStream()
deviceStreaming.sendBlobRequest()

myData = Data.Data()

for imageNum in range(5):
    try:
        deviceStreaming.getFrame()
        wholeFrame = deviceStreaming.frame

        myData.read(wholeFrame)

        if myData.hasDepthMap:
            print("=== Saving images ({}) ===".format(imageNum))
            print("Frame number: {}".format(myData.depthmap.frameNumber))

            numCols = myData.cameraParams.width
            numRows = myData.cameraParams.height

            # ========== Z-map ==========
            zmapData = list(myData.depthmap.distance)

            # Save Z-map as *.png-file
            zmapDataArray = np.uint16(np.reshape(zmapData, (512, 640)))
            io.imsave("z_map_image{}.png".format(imageNum), zmapDataArray)

            # ========== RGBA image ==========
            rgbaData = myData.depthmap.intensity

            # Save RGBA image as *.png-file
            rgbaDataArray = np.uint32(np.reshape(rgbaData, (512, 640)))
            rgbaDataArray = np.frombuffer(rgbaDataArray, np.uint8)
            rgbaDataArray = np.reshape(rgbaDataArray, (512, 640, 4))

            io.imsave("rgba_image{}.png".format(imageNum), rgbaDataArray)

            # ========== Statemap ==========
            statemapData = myData.depthmap.confidence

            # Save Statemap as *.png-file
            statemapDataArray = np.uint16(np.reshape(statemapData, (512, 640)))
            io.imsave("statemap_image{}.png".format(imageNum), statemapDataArray)

            # Apply the Statemap to the Z-map
            zmapData_with_statemap = zmapDataArray
            z = 0
            for i in range(numRows):
                for j in range(numCols):
                    if statemapDataArray[i][j] != 0:
                        zmapData_with_statemap[i][j] = 0 # Set invalid pixels to lowest value

            # Save RGBA image with applied statemap as *.png-file
            io.imsave("z_map_image_with_applied_statemap{}.png".format(imageNum), zmapData_with_statemap)

    except KeyboardInterrupt:
        print("Ctrl-C pressed (1), terminating.")

deviceStreaming.closeStream()
