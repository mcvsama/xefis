/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CONFIGS__CTHULHU_SHARED__LINK_IO_H__INCLUDED
#define XEFIS__CONFIGS__CTHULHU_SHARED__LINK_IO_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/io/link.h>


template<template<class> class PropertyType>
	class CthulhuGCS2AircraftLinkIO: public LinkIO
	{
	  public:
		PropertyType<si::Angle>		stick_elevator		{ this, "/controls/stick-elevator" };
		PropertyType<si::Angle>		stick_ailerons		{ this, "/controls/stick-ailerons" };
		PropertyType<si::Angle>		rudder_pedals		{ this, "/controls/rudder-pedals" };
		PropertyType<si::Force>		throttle_left		{ this, "/controls/throttle-left" };
		PropertyType<si::Force>		throttle_right		{ this, "/controls/throttle-right" };
		PropertyType<bool>			test_bool			{ this, "" }; // XXX
		PropertyType<uint64_t>		test_uint			{ this, "" }; // XXX
	};


template<template<class> class PropertyType>
	class CthulhuAircraft2GCSLinkIO: public LinkIO
	{
	  public:
		PropertyType<si::Angle>		home_latitude		{ this, "/home/latitude" };
		PropertyType<si::Angle>		home_longitude		{ this, "/home/longitude" };
	};


using CthulhuGCS_Tx_LinkIO = CthulhuGCS2AircraftLinkIO<xf::PropertyIn>;
using CthulhuGCS_Rx_LinkIO = CthulhuAircraft2GCSLinkIO<xf::PropertyOut>;

using CthulhuAircraft_Tx_LinkIO = CthulhuAircraft2GCSLinkIO<xf::PropertyIn>;
using CthulhuAircraft_Rx_LinkIO = CthulhuGCS2AircraftLinkIO<xf::PropertyOut>;

#endif

