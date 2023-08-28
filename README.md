
# SickCameraQt

Siemens Step 7 PLC interface for Sick Visionary-T Mini TOF camera with flexible image filtering.
## Installation

*[Prebuilt binaries](google.com)* 
1. **Install Qt**
* [Download Qt](https://www.qt.io/download)
> **Note:** The program was written with Qt 6.5.1, but should be compatable with 6.x.x
* Follow the steps of the installer
* Select the following when prompted

2. **Install OpenCV**
* [Download OpenCV 4.8.0](https://github.com/opencv/opencv/releases/tag/4.8.0)
* Set environment variable "OPENCV_DIR" to root install directory

3. **Install Boost**
* [Download Boost 1.8.3](https://www.boost.org/users/download/)
* Set environment variable "BOOST_DIR" to root install directory

4. **Install Visual Studio 2022**
* [Download Visual Studio](https://visualstudio.microsoft.com/downloads/)
* Follow the steps of the installer and when prompted, check the "Desktop development with C++" workload
* Once installed, add "Qt Visual Studio Tools" extension