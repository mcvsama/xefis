XEFIS
=====

Xefis is a framework that helps writing software for remote controlled vehicles
and their ground stations.

It focuses on flying vehicles.

It has:
    * display modules (EFIS) for ADI, HSI, various gauges
    * communication modules with encryption and authentication support (eg. to connect GCS and the aircraft)
    * a couple of hardware communication modules (BMP085 pressure sensor, EagleTree Airspeed sensor, XBee modem,
      NMEA GPS modules, CHR UM6 IMU)
    * computation modules to obtain performance and navigation data from sensor data
      (airspeed, altitude, bearing, track, various other speeds, etc),
    * automation modules that can automatically control a model
    * rigid-body simulation framework to help testing of models
    * basic system that allows data exchange between the modules

Notes:
No-one uses this besides the author. Don't expect any support.

![XEFIS](http://mcv.mulabs.org/app/xefis/screenshot-006.png)

Required depenencies
====================

pkg-config
uuid-dev
gcc or clang
boost
boost-endian
qt-5
crypto++-8.5.0-2

Packages required on Ubuntu 18:
	pkg-config
    qtbase5-dev
	libqt5svg5
	libqt5x11extras5-dev
	libboost-all-dev
	uuid-dev
	libmhash-dev


