#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This sample shows how to select a specific job by its id.

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

# === Device specific protocol & control_port mapping ===
# Visionary-T CX / AG / DT => CoLaB / Port 2112
# Visionary-S CX           => CoLaB / Port 2112
#process cmd arguments
parser = argparse.ArgumentParser(description="Select a new job on a SICK Visionary device.")
parser.add_argument('-i', '--ipAddress', required=False, type=str,
                    default="192.168.1.10", help="The ip address of the device.")
parser.add_argument('-p', '--protocol', required=False, choices=['ColaB', 'Cola2'],
                    default="ColaB", help="The SICK Cola protocol version.")
parser.add_argument('-c', '--control_port', required=False, type=int,
                    default=2112, help="The control port to change settings.")
args = parser.parse_args()

deviceControl = Control(args.ipAddress, args.protocol, args.control_port)
deviceControl.open()

name, version = deviceControl.getIdent()

print()
print("DeviceIdent: {} {}".format(name.decode('utf-8'), version.decode('utf-8')))
print()

deviceControl.login(Control.USERLEVEL_SERVICE, "CUST_SERV")

# specify the desired job id
jobId = 0

# show current job id
currentJobId = deviceControl.getCurrentJobId()
print("Job id before job selection: {}".format(currentJobId))

# select job
jobId = 1
print("Trying to select job id: {}".format(jobId))
deviceControl.selectJobByID(jobId)
currentJobId = deviceControl.getCurrentJobId()
print("Job id after job selection: {}".format(currentJobId))

deviceControl.logout()
deviceControl.close()
