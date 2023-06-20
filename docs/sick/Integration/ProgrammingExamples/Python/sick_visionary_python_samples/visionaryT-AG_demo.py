#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script demonstrates how to retrieve data from a V3SCamera device. It
does not depend on the V3SCamera API, but the functionality is comparable.
Please have a look at the samples point cloud conversion examples in order
to get more elaborate examples (e.g. how to transform distance values into
a 3D point cloud).

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

from common.Control import Control
from common.Streaming import Data
from common.Stream import Streaming

# === Device specific protocol & control_port mapping ===
# Visionary-T CX / AG / DT => CoLaB / Port 2112
parser = argparse.ArgumentParser(description="Exemplary data reception from SICK Visionary AG devices.")
parser.add_argument('-i', '--ipAddress', required=False, type=str,
                    default="192.168.1.10", help="The ip address of the device.")
parser.add_argument('-p', '--protocol', required=False, choices=['ColaB', 'Cola2'],
                    default="ColaB", help="The SICK Cola protocol version.")
parser.add_argument('-n', '--channel', required=False, type=str, default='DEPTHMAP',
                    help="The selected data reduction channel: 'DEPTHMAP', 'POLAR2D' or 'CARTESIAN'")
parser.add_argument('-c', '--control_port', required=False, type=int,
                    default=2112, help="The control port to change settings.")
parser.add_argument('-s', '--streaming_port', required=False, type=int,
                    default=2114, help="The tcp port of the data channel.")
args = parser.parse_args()

# create and open a control connection to the device
deviceControl = Control(args.ipAddress, args.protocol, args.control_port)
deviceControl.open()

name, version = deviceControl.getIdent()

print()
print("DeviceIdent: {} {}".format(name.decode('utf-8'), version.decode('utf-8')))
print()

def channelType(x):
    return {
        'DEPTHMAP': 1,
        'POLAR2D': 2,
        'CARTESIAN': 3,
    }.get(x, 1)    # 1 is default if x not found


# access the device via a set account to change settings
deviceControl.login(Control.USERLEVEL_AUTH_CLIENT, 'CLIENT')


if channelType(args.channel) == 1:
    deviceControl.enableDepthMapDataTransfer()

elif channelType(args.channel) == 2:
    # activate polar 2D data reduction
    deviceControl.activatePolar2DReduction()
    # wait while params are applied

    deviceControl.disableDepthMapDataTransfer()
    deviceControl.enablePolar2DDataTransfer()
elif channelType(args.channel) == 3:
    # activate Cartesian data reduction
    deviceControl.activateCartesianReduction()
    # wait while params are applied
    deviceControl.disableDepthMapDataTransfer()
    deviceControl.enableCartesianDataTransfer()

# logout after settings have been done
deviceControl.logout()

deviceControl.startStream() # in case the stream is stopped
deviceControl.waitForReductionParamsApplied()

deviceControl.initStream()
deviceControl.stopStream() # the device starts stream automatically

deviceStreaming = Streaming(args.ipAddress, args.streaming_port)

# reopen the stream
deviceStreaming.openStream()

# send binary data to blob server to initialize the communication
deviceStreaming.sendBlobRequest()

# request camera to produce a single frame
deviceControl.singleStep()
# request the whole frame data
deviceStreaming.getFrame()

# access the new frame via the corresponding class attribute
wholeFrame = deviceStreaming.frame

# create new Data object to hold/parse/handle frame data
myData = Data.Data()
# parse frame data to Data object for further data processing
myData.read(wholeFrame)

if myData.hasDepthMap:
    print()
    print("Data contains depth map data:")
    print("Frame number: {}".format(myData.depthmap.frameNumber))
    distanceData = list(myData.depthmap.distance)

    numCols = myData.cameraParams.width
    numRows = myData.cameraParams.height

    midIndex = int(numCols * (numRows/2) + numCols/2)
    print("- center pixel distance: {:.2f}mm".format(distanceData[midIndex]))
    print("- confidence of this measurement: {}".format(myData.depthmap.confidence[midIndex]))
    meanDistance = sum(distanceData) / len(distanceData)
    print("- mean distance: {}mm".format(meanDistance))
    print("- min/max distance: {}mm/{}mm".format(min(distanceData), max(distanceData)))

if myData.hasPolar2D:
    polarData = myData.polarData2D
    distanceData = polarData.distance
    print()
    print("Data contains polar scan data:")
    numScans = len(distanceData)
    print("- angle of first scan point: {:.2f}".format(polarData.angleFirstScanPoint))
    print("- angular resolution: {:.2f}".format(polarData.angularResolution))
    print("- number of scan points: {}".format(numScans))
    midIndex = int(numScans / 2)
    print("- distance in middle sector: {:.2f}mm".format(distanceData[midIndex]))
    print("- confidence in middle sector: {}".format(polarData.confidence[midIndex]))
    meanDistance = sum(distanceData) / numScans
    print(" -mean distance: {:.2f}mm".format(meanDistance))
    print("- min/max distance: {:.2f}mm/{:.2f}mm".format(min(distanceData), max(distanceData)))
    # Comparison to Sopas parameters
    startAngle = polarData.angleFirstScanPoint - (polarData.angularResolution / 2)
    endAngle = polarData.angleFirstScanPoint + (polarData.angularResolution * (numScans - 0.5))
    print("- start angle (Sopas): {:.2f}".format(startAngle))
    print("- end angle (Sopas): {:.2f}".format(endAngle))
    print("- number of sectors (Sopas): {}".format(numScans))
    print("- all distance values:")
    for val in polarData.distance:
        print("    {:.2f}".format(val))

if myData.hasCartesian:
    print()
    print("Data contains Cartesian point cloud:")
    cartesianData = myData.cartesianData
    print("- number of points: {}".format(cartesianData.numPoints))
    print("- points (x,y,z) in mm / confidence:")
    for x, y, z, conf in zip(cartesianData.x, cartesianData.y, cartesianData.z, cartesianData.confidence):
        print("    {:+7.1f}, {:+7.1f}, {:+7.1f}, {:5.1f}%".format(x, y, z, conf))

deviceStreaming.closeStream()
# restore settings to their defaults
deviceControl.login(Control.USERLEVEL_AUTH_CLIENT, 'CLIENT')
if channelType(args.channel) == 2:
    deviceControl.deactivatePolar2DReduction()
    deviceControl.disablePolar2DDataTransfer()
elif channelType(args.channel) == 3:
    deviceControl.deactivateCartesianReduction()
    deviceControl.disableCartesianDataTransfer()
deviceControl.enableDepthMapDataTransfer()
#deviceControl.applySettings()
deviceControl.logout()

deviceControl.startStream()
deviceControl.close()
