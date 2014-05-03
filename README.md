XEFIS
=====

Qt-based EFIS and EICAS that can work on Raspberry-Pi, but utterly slowly.

Right now it can be used with FlightGear.

![XEFIS](http://mcv.mulabs.org/app/xefis/screenshot-005.png)

Required depenencies
====================

uuid-dev
gcc-4.7
boost
boost-endian
qt-5
mhash-0.9.9.9

C++11 compiler required
=======================

Compiles fine with both gcc and clang.

Use at least gcc-4.7 or clang-3.4. If you're using old Ubuntu, you may need to
install package **gcc-snapshot** and create **src/Makefile.local**
file with the following contents:

```
CXX = /usr/lib/gcc-snapshot/bin/gcc
```
