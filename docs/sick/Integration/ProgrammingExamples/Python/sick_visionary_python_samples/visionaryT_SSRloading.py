#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This sample shows how to load an SSR file

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

import common.data_io.SsrLoader as ssrloader
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description="Example for loading SSR data read recorded using a SICK Visionary-T device.")
parser.add_argument('-f', '--filename', required=False, type=str,
                    default="sample_data/visionaryT_sample.ssr", help="The filename of the SSR file")
parser.add_argument('-l', '--start_frame', required=False, type=int,
                    default=0, help="The frame to start the processing with. The first available frame has number 0.")
parser.add_argument('-m', '--frames_to_process', required=False, type=int,
                    default=0, help="The number of frames to process from the SSR file. 0 means all frames in file.")
args = parser.parse_args()

# setting values to rows and column variables for the matplot figure
rows = 1
columns = 3

images_distance, images_intensity, images_confidence, cam_params, is_stereo = ssrloader.readSsrData(args.filename, args.start_frame, args.frames_to_process)

for i in range(len(images_distance)):
    # create figure
    fig = plt.figure(figsize=(21, 7))
    fig.suptitle("Frame {}\n(press ctrl-C in the command line window to stop)".format(i))

    # showing distance image at 1st position
    fig.add_subplot(rows, columns, 1)
    plt.imshow(images_distance[i])
    plt.title("distance")

    # showing intensity image at 2nd position
    fig.add_subplot(rows, columns, 2)
    plt.imshow(images_intensity[i])
    plt.title("intensity")

    # showing confidence image at 3rd position
    fig.add_subplot(rows, columns, 3)
    plt.imshow(images_confidence[i])
    plt.title("confidence")

    plt.show()
