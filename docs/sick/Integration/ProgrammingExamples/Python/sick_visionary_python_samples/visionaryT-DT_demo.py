#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This sample shows how to read the detection results from the device.

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

import struct
import argparse

from common.Control import Control

# === Device specific protocol & control_port mapping ===
# Visionary-T CX / AG / DT => CoLaB / Port 2112
parser = argparse.ArgumentParser(description="Exemplary data reception from SICK Visionary DT devices.")
parser.add_argument('-i', '--ipAddress', required=False, type=str,
                    default="192.168.1.10", help="The ip address of the device.")
parser.add_argument('-p', '--protocol', required=False, choices=['ColaB', 'Cola2'],
                    default="ColaB", help="The SICK Cola protocol version.")
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

# access the device via a set account to change settings
deviceControl.login(Control.USERLEVEL_SERVICE, "CUST_SERV")

# arrays representing the complete cuboid grid have a fixed length of 900 entries (30 rows and 30 columns)
maxRows = 30
maxCols = 30
# but not all have valid values. thus get the user specified numRows and numCols
dims = deviceControl.getRangeDimensions()

numRows = dims[0]
numCols = dims[1]

print("Range dimensions (rows: {}, columns: {})".format(numRows, numCols))

# get all cuboids with detection information as array of struct. Each value contains the following information:
# - the location of the cuboids center in x and y.
# - the height values of the top and bottom ends of the cuboids
# - if a detection has occurred inside a cuboid.
# Note that the device uses the term cell instead of cuboid internally.
cellsInfo = deviceControl.getAllCuboidCellsWithDetectionInfo()

# check which cuboids (a.k.a. cells) have detections
for row in range(numRows):
    for col in range(numCols):
        startPosReturnVal = (row * maxRows + col) * 17
        infoCell = struct.unpack('>ffii?', cellsInfo[startPosReturnVal:startPosReturnVal+17])
        print("Cuboid ({},{}) has its center at: {},{}".format(row, col, infoCell[0], infoCell[1]))
        print("Cuboid ({},{}) has its bottom and top end at a height of: {},{}".format(row, col, infoCell[2], infoCell[3]))
        print("Cuboid ({},{}) has a detection: {}".format(row, col, infoCell[4]))

# get all cuboid groups as an array of uint32. Each value contains the information if a cuboid belongs to a group or not (bitwise)
cell2GroupMappings = deviceControl.cuboitGroupsToArray()

# find out for each cuboid to which cuboid groups it has been assigned
print()
print("Group indices are represented by bit positions of '1'")
for row in range(numRows):
    for col in range(numCols):
        startPosReturnVal = (row * maxRows + col) * 4
        groupMap = struct.unpack('>I', cell2GroupMappings[startPosReturnVal:startPosReturnVal + 4])
        # convert the groupMap to binary representation
        print("Cuboid ({0},{1}): {2:020b}".format(row, col, groupMap[0]))

# logout after settings have been done
deviceControl.logout()
deviceControl.close()
