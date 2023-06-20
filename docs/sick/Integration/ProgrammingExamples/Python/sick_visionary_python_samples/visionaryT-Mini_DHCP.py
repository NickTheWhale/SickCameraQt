#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This sample shows how to setup DHCP via CoLa on the device and to revert the device IP back to static IP.

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
from common.Protocol.AutoIp import AutoIp

def activate_dhcp(deviceControl):
    # open connection to device
    deviceControl.open()
    deviceControl.login(Control.USERLEVEL_SERVICE, 'CUST_SERV')
    # setup ethernet mode to DHCP
    deviceControl.writeVariable(b'EIAddrMode', struct.pack('B', 1))
    time.sleep(1)
    deviceControl.invokeMethod(b'mEthUpdt')
    deviceControl.logout()

    deviceControl.login(Control.USERLEVEL_SERVICE, 'CUST_SERV')
    deviceControl.writeVariable(b'EIAddrMode', struct.pack('B', 1))
    deviceControl.logout()

    deviceControl.close()

def activate_static_ip(deviceControl, ip):
    # open connection to device
    deviceControl.open()
    deviceControl.login(Control.USERLEVEL_SERVICE, 'CUST_SERV')
    # setup ethernet mode to static IP
    deviceControl.writeVariable(b'EIIpAddr', struct.pack('>4B', *[int(x) for x in ip.split(".")]))
    deviceControl.writeVariable(b'EIAddrMode', struct.pack('B', 0))
    time.sleep(1)
    deviceControl.invokeMethod(b'mEthUpdt')
    deviceControl.logout()

    deviceControl.login(Control.USERLEVEL_SERVICE, 'CUST_SERV')
    deviceControl.writeVariable(b'EIAddrMode', struct.pack('B', 0))
    deviceControl.logout()

    deviceControl.close()

def find_device_ip(deviceType, interfaceIPAddress, interfaceNetMask):
    autoIp = AutoIp(interfaceIPAddress, interfaceNetMask)
    print('Scanning for device ...')
    devices = autoIp.scan()
    for dev in devices:
        if dev.items["DeviceType"][0] == deviceType:
            return dev.items['IPAddress'][0]

# === Device specific protocol & control_port mapping ===
# Visionary-T Mini CX      => CoLa2 / Port 2122
# handle argument parsing from cmd arguments
parser = argparse.ArgumentParser(description="Exemplary DHCP activation for SICK Visionary devices.")
parser.add_argument('-i', '--interfaceIPAddress', required=False, type=str,
                    default="192.168.1.2", help="The ip address of the network interface where device is connected to.")
parser.add_argument('-n', '--interfaceNetMask', required=False, type=str,
                    default="255.255.255.0", help="The subnet mask of the network interface where device is connected to.")
parser.add_argument('-si', '--staticIPAddress', required=False, type=str,
                    default="192.168.1.10", help="The static ip address of the device.")
parser.add_argument('-p', '--protocol', required=False, choices=['ColaB', 'Cola2'],
                    default="Cola2", help="The SICK CoLa protocol version.")
parser.add_argument('-c', '--control-port', required=False, type=int,
                    default=2122, help="The control port to change settings.")
parser.add_argument('-s', '--setback_static', required=False, type=bool,
                    default=False, help="The option to setback ethernet mode to static ip.")
args = parser.parse_args()


if __name__ == '__main__':
    deviceType = 'Visionary-T Mini CX V3S105-1x'
    deviceIP = find_device_ip(deviceType, args.interfaceIPAddress, args.interfaceNetMask)
    print("Initial IP Address of the device {}".format(deviceIP))


    # Obtain IP address from DHCP server
    print("Activating DHCP ethernet addressing mode ...")
    deviceControl = Control(ipAddress=deviceIP, protocol=args.protocol, control_port=args.control_port)
    activate_dhcp(deviceControl)
    print("Wait till IP address is obtained from DHCP server ...")
    time.sleep(30) # Default DHCP Server timeout
    deviceIP = find_device_ip(deviceType, args.interfaceIPAddress, args.interfaceNetMask)
    print("IP Address of the device obtained from DHCP Server {}".format(deviceIP))

    if args.setback_static:
        print("Revert ethernet addressing mode to static IP {}".format(args.staticIPAddress))
        del deviceControl
        deviceControl = Control(ipAddress=deviceIP, protocol=args.protocol, control_port=args.control_port)
        activate_static_ip(deviceControl, args.staticIPAddress)
        time.sleep(5)
        deviceIP = find_device_ip(deviceType, args.interfaceIPAddress, args.interfaceNetMask)
        print("IP Address of the device after setting static IP {}".format(deviceIP))