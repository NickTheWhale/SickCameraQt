
# SickCameraQt

Siemens Step 7 PLC interface for Sick Visionary-T Mini TOF camera with flexible image filtering.
## Installation

*[Prebuilt binaries](google.com)* 
1. **Install Qt**
* [Download Qt 6.5.1](https://www.qt.io/download)
> **Note:** The program was written with Qt 6.5.1, but should be compatable with 6.x.x. If you do decide to use a version other than 6.5.1, you will have to set the SickGUI 'Configuration Properties \> Qt Project Settings \> Qt Installation' to the appropriate value.
* Follow the steps of the installer
* Select the following when prompted (this is what I *think* is the minimum required, but if you run into issues, you can always rerun the installer and update what components you want)
![qt installer options](./docs/images/qt_install.png)

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
* Set environment variable VCINSTALLDIR (most likely "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC")

> **Note:** Visual Studio needs to be closed and reopenned to recognize changes in environment variables.