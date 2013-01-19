XEFIS
=====

Qt-based EFIS and EICAS that can work on Raspberry-Pi, but utterly slowly.

Right now it can be used with FlightGear.

Required depenencies
====================

uuid-dev
gcc-4.7
boost
qt-5

C++11 compiler required
=======================

Use gcc-4.7 at least. If you're using Ubuntu, you may need to
install package **gcc-snapshot** and create **src/Makefile.local**
file with the following contents:

```
CXX = /usr/lib/gcc-snapshot/bin/gcc
```
