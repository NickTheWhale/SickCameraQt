#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This script demonstrates how to use the auto exposure functions
by invoking the method 'TriggerAutoExposureParameterized'.
It's also shown how the region of interest (ROI) can be set.
The sample is based on the AcquisitionModeStereo = NORMAL.

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
import struct
import time

from common.Control import Control
from common.Streaming import Data
from common.Stream import Streaming

# === Device specific protocol & control_port mapping ===
# Visionary-S CX           => CoLaB / Port 2112
parser = argparse.ArgumentParser(description=
                                 'Exemplary triggering of auto exposure functions from SICK Visionary-S devices.')
parser.add_argument('-i', '--ipAddress', required=False, type=str,
                    default="192.168.1.10", help='The ip address of the device.')
parser.add_argument('-p', '--protocol', required=False, choices=['ColaB', 'Cola2'],
                    default="ColaB", help='The SICK Cola protocol version.')
parser.add_argument('-c', '--control_port', required=False, type=int,
                    default=2112, help='The control port to change settings.')
parser.add_argument('-s', '--streaming_port', required=False, type=int,
                    default=2114, help='The tcp port of the data channel.')
args = parser.parse_args()

deviceControl = Control(args.ipAddress, args.protocol, args.control_port)
deviceControl.open()

name, version = deviceControl.getIdent()

print()
print("DeviceIdent: {} {}".format(name.decode('utf-8'), version.decode('utf-8')))
print()

# ===== Adjust AcquisitionModeStereo to NORMAL =====
deviceControl.login(Control.USERLEVEL_AUTH_CLIENT, 'CLIENT')

acquisitionModeStereo = 0
deviceControl.setAcquisitionModeStereo(acquisitionModeStereo)

# ===== Set region of interest (ROI) =====
left = 160
right = 480
top = 128
bottom = 384

autoExposureROI = struct.pack('>IIII', left, right, top, bottom)

# Set ROI for Auto Exposure 3D
deviceControl.setAutoExposure3D(autoExposureROI)

# Set ROI for Auto Exposure RGB
deviceControl.setAutoExposureColorROI(autoExposureROI)

# Set ROI for Auto White Balance
# NOTE: The user is responisble to make sure that the region he sets the ROI to, is actually white.
deviceControl.setAutoWhiteBalanceROI(autoExposureROI)

deviceControl.logout()

# ===== Read out actual integration time values (before auto exposure was triggered) =====
# ATTENTION:
# This sample is based on the NORMAL acquisition mode;
# other modes may refer to other integration time variables

integrationTime_old = deviceControl.getIntegrationTimeUs()
print("integrationTime (old): {}us".format(integrationTime_old))

integrationTimeUsColor_old = deviceControl.getIntegrationTimeUsColor()
print("integrationTimeUsColor (old)us: {}".format(integrationTimeUsColor_old))

# Info: For White Balance exists no SOPAS variable;
#       the changes are done internally in the device and applied to the image.
#       If you open SOPAS and you are running this sample in parallel
#       you can see how the image changes.

# ===== Trigger all auto exposure functions =============================
# Transfer Enum for exposureAlgorithms for parallel execution
for i in range(3):
    # Trigger each function individually
    print("Invoke method 'TriggerAutoExposureParameterized' (Param: {}) ...".format(i))
    data = struct.pack('>HB', 1, i) # INTEGRATION_TIME=0 / INTEGRATION_TIME_COLOR=1 / WHITE_BALANCE=2
    deviceControl.login(Control.USERLEVEL_SERVICE, "CUST_SERV")
    success = deviceControl.startAutoExposureParameterized(data)
    deviceControl.logout()

    # Check method return value (success == 1)
    if success:
        # Wait until auto exposure method is finished
        startTime = time.time()
        while deviceControl.getAutoExposureParameterizedRunning():
            if ((time.time() - startTime) <= 10):
                time.sleep(0.2)
            else:
                print("TIMEOUT: auto exposure method (Param: {}) needs longer than expected!".format(i))
    else:
        print("ERROR: Invoking auto exposure method (Param: {}) was not sucessful! (return value: {})".format(i, ord(success[-1])))

# ===== Read out new integration time values (after auto exposure was triggered) =========
integrationTime_old = deviceControl.getIntegrationTimeUs()
print("integrationTime (new): {}µs".format(integrationTime_old))

integrationTimeUsColor_old = deviceControl.getIntegrationTimeUsColor()
print("integrationTimeUsColor (new): {}µs".format(integrationTimeUsColor_old))

# ===== Receive some frames =============================================
deviceControl.initStream()
deviceControl.stopStream() # the device starts stream automatically

deviceStreaming = Streaming(args.ipAddress, args.streaming_port)
deviceStreaming.openStream()
deviceStreaming.sendBlobRequest()

deviceControl.singleStep()

deviceStreaming.getFrame()
wholeFrame = deviceStreaming.frame

myData = Data.Data()
myData.read(wholeFrame)

if myData.hasDepthMap:
    print("\nData contains depth map data:")
    distanceData = myData.depthmap.distance

    numCols = myData.cameraParams.width
    numRows = myData.cameraParams.height
    print("Image resolution is: {} (Width) x {} (Height)".format(numCols, numRows))

    frameNumber = myData.depthmap.frameNumber
    print("Received frame number: {}".format(frameNumber))

deviceStreaming.closeStream()

deviceControl.startStream()
deviceControl.close()
