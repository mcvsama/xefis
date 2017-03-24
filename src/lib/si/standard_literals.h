/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__STANDARD_LITERALS_H__INCLUDED
#define SI__STANDARD_LITERALS_H__INCLUDED

// Standard:
#include <cstddef>

// Local:
#include "standard_units.h"
#include "quantity.h"


namespace si {
namespace literals {

#define SI_DEFINE_LITERAL(xUnit, xliteral)			\
	constexpr Quantity<units::xUnit>				\
	operator"" xliteral (long double value)			\
	{												\
		return Quantity<units::xUnit> (value);		\
	}												\
													\
	constexpr Quantity<units::xUnit>				\
	operator"" xliteral (unsigned long long value)	\
	{												\
		return Quantity<units::xUnit> (value);		\
	}

// Base SI units:
SI_DEFINE_LITERAL (Meter, _m)
SI_DEFINE_LITERAL (Kilogram, _kg)
SI_DEFINE_LITERAL (Second, _s)
SI_DEFINE_LITERAL (Ampere, _A)
SI_DEFINE_LITERAL (Kelvin, _K)
SI_DEFINE_LITERAL (Mole, _mol)
SI_DEFINE_LITERAL (Candela, _cd)
SI_DEFINE_LITERAL (Radian, _rad)

// Named derived units:
SI_DEFINE_LITERAL (Hertz, _Hz)
SI_DEFINE_LITERAL (Steradian, _sr)
SI_DEFINE_LITERAL (Newton, _N)
SI_DEFINE_LITERAL (Pascal, _Pa)
SI_DEFINE_LITERAL (Joule, _J)
SI_DEFINE_LITERAL (Watt, _W)
SI_DEFINE_LITERAL (Coulomb, _C)
SI_DEFINE_LITERAL (Volt, _V)
SI_DEFINE_LITERAL (Farad, _F)
SI_DEFINE_LITERAL (Ohm, _ohm)
SI_DEFINE_LITERAL (Siemens, _S)
SI_DEFINE_LITERAL (Weber, _Wb)
SI_DEFINE_LITERAL (Tesla, _T)
SI_DEFINE_LITERAL (Henry, _H)
SI_DEFINE_LITERAL (Lumen, _lm)
SI_DEFINE_LITERAL (Lux, _lx)
SI_DEFINE_LITERAL (Becquerel, _Bq)
SI_DEFINE_LITERAL (Gray, _Gy)
SI_DEFINE_LITERAL (Katal, _kat)

// Some of unnamed derived units:
SI_DEFINE_LITERAL (SquareMeter, _m2)
SI_DEFINE_LITERAL (CubicMeter, _m3)
SI_DEFINE_LITERAL (MeterPerSecond, _mps)
SI_DEFINE_LITERAL (MeterPerSecondSquared, _mps2)
SI_DEFINE_LITERAL (RadianPerSecond, _radps)
SI_DEFINE_LITERAL (RadianPerSecondSquared, _radps2)
SI_DEFINE_LITERAL (NewtonMeter, _Nm)
SI_DEFINE_LITERAL (NewtonSecond, _Ns)
SI_DEFINE_LITERAL (PascalSecond, _Pas)

// Often-used scaled units:
SI_DEFINE_LITERAL (Kilometer, _km)
SI_DEFINE_LITERAL (Centimeter, _cm)
SI_DEFINE_LITERAL (Millimeter, _mm)
SI_DEFINE_LITERAL (Tonne, _ton)
SI_DEFINE_LITERAL (Gram, _gr)
SI_DEFINE_LITERAL (Milligram, _mg)
SI_DEFINE_LITERAL (Microgram, _ug)
SI_DEFINE_LITERAL (Hour, _h)
SI_DEFINE_LITERAL (Minute, _min)
SI_DEFINE_LITERAL (Millisecond, _ms)
SI_DEFINE_LITERAL (Microsecond, _us)
SI_DEFINE_LITERAL (Nanosecond, _ns)
SI_DEFINE_LITERAL (MilliAmpere, _mA)
SI_DEFINE_LITERAL (MicroAmpere, _uA)
SI_DEFINE_LITERAL (Amperehour, _Ah)
SI_DEFINE_LITERAL (MilliAmperehour, _mAh)
SI_DEFINE_LITERAL (KiloNewton, _kN)
SI_DEFINE_LITERAL (MegaHertz, _MHz)
SI_DEFINE_LITERAL (KiloHertz, _kHz)
SI_DEFINE_LITERAL (MegaWatt, _MW)
SI_DEFINE_LITERAL (KiloWatt, _kW)
SI_DEFINE_LITERAL (MilliWatt, _mW)
SI_DEFINE_LITERAL (MicroWatt, _uW)
SI_DEFINE_LITERAL (KiloPascal, _kPa)
SI_DEFINE_LITERAL (HectoPascal, _hPa)
SI_DEFINE_LITERAL (Foot, _ft)
SI_DEFINE_LITERAL (Mile, _mi)
SI_DEFINE_LITERAL (NauticalMile, _nmi)
SI_DEFINE_LITERAL (Inch, _in)
SI_DEFINE_LITERAL (PoundMass, _lb)
SI_DEFINE_LITERAL (Gravity, _g)
SI_DEFINE_LITERAL (Rankine, _Ra)
SI_DEFINE_LITERAL (Degree, _deg)
SI_DEFINE_LITERAL (RotationPerMinute, _rpm)
SI_DEFINE_LITERAL (InchOfMercury, _inHg)
SI_DEFINE_LITERAL (KilometerPerHour, _kph)
SI_DEFINE_LITERAL (FootPerMinute, _fpm)
SI_DEFINE_LITERAL (Knot, _kt)
SI_DEFINE_LITERAL (Celsius, _degC)
SI_DEFINE_LITERAL (Fahrenheit, _degF)

// Equivalent units:
SI_DEFINE_LITERAL (Sievert, _Sv)

#undef SI_DEFINE_LITERAL

} // namespace literals
} // namespace si

#endif

