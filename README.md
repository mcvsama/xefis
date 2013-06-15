XEFIS
=====

Qt-based EFIS and EICAS that can work on Raspberry-Pi, but utterly slowly.

Right now it can be used with FlightGear.

![XEFIS](http://mcv.mulabs.org/app/xefis/screenshot-002.png)

Required depenencies
====================

uuid-dev
gcc-4.7
boost
qt-5
mhash-0.9.9.9

C++11 compiler required
=======================

Use gcc-4.7 at least. If you're using Ubuntu, you may need to
install package **gcc-snapshot** and create **src/Makefile.local**
file with the following contents:

```
CXX = /usr/lib/gcc-snapshot/bin/gcc
```
