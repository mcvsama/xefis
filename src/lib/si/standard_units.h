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

#ifndef SI__STANDARD_UNITS_H__INCLUDED
#define SI__STANDARD_UNITS_H__INCLUDED

// Standard:
#include <cstddef>
#include <ratio>

// Local:
#include "unit.h"


namespace si {
namespace units {

// Pi ~ std::ratio<314159265358979323846264338327950288419717, 100000000000000000000000000000000000000000>,
// but that may be too large for most compilers now.
typedef std::ratio<3141592653589793238, 1000000000000000000> Pi;

// Smaller-precision Pi to avoid overflows when multiplying/dividing by 180:
typedef std::ratio<31415926535897932, 10000000000000000> SmallPi;


// Basic SI units:
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 0,  0,  0,  0,  0,  0,  0,  0>	Dimensionless;
typedef Unit< 1,  0,  0,  0,  0,  0,  0,  0>	Meter;
typedef Unit< 0,  1,  0,  0,  0,  0,  0,  0>	Kilogram;
typedef Unit< 0,  0,  1,  0,  0,  0,  0,  0>	Second;
typedef Unit< 0,  0,  0,  1,  0,  0,  0,  0>	Ampere;
typedef Unit< 0,  0,  0,  0,  1,  0,  0,  0>	Kelvin;
typedef Unit< 0,  0,  0,  0,  0,  1,  0,  0>	Mole;
typedef Unit< 0,  0,  0,  0,  0,  0,  1,  0>	Candela;
typedef Unit< 0,  0,  0,  0,  0,  0,  0,  1>	Radian;

// Named derived units:
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 0,  0, -1,  0,  0,  0,  0,  0>	Hertz;
typedef Unit< 0,  0,  0,  0,  0,  0,  0,  2>	Steradian;
typedef Unit< 1,  1, -2,  0,  0,  0,  0,  0>	Newton;
typedef Unit<-1,  1, -2,  0,  0,  0,  0,  0>	Pascal;
typedef Unit< 2,  1, -2,  0,  0,  0,  0,  0>	Joule;
typedef Unit< 2,  1, -3,  0,  0,  0,  0,  0>	Watt;
typedef Unit< 0,  0,  1,  1,  0,  0,  0,  0>	Coulomb;
typedef Unit< 2,  1, -3, -1,  0,  0,  0,  0>	Volt;
typedef Unit<-2, -1,  4,  2,  0,  0,  0,  0>	Farad;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 2,  1, -3, -2,  0,  0,  0,  0>	Ohm;
typedef Unit<-2, -1,  3,  2,  0,  0,  0,  0>	Siemens;
typedef Unit< 2,  1, -2, -1,  0,  0,  0,  0>	Weber;
typedef Unit< 0,  1, -2, -1,  0,  0,  0,  0>	Tesla;
typedef Unit< 2,  1, -2, -2,  0,  0,  0,  0>	Henry;
typedef Unit< 0,  0,  0,  0,  0,  0,  1,  2>	Lumen;
typedef Unit<-2,  0,  0,  0,  0,  0,  1,  2>	Lux;
typedef Unit< 0,  0, -1,  0,  0,  0,  0,  0>	Becquerel;
typedef Unit< 2,  0, -2,  0,  0,  0,  0,  0>	Gray;
typedef Unit< 0,  0, -1,  0,  0,  1,  0,  0>	Katal;

// Unnamed derived units:
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 2,  0,  0,  0,  0,  0,  0,  0>	SquareMeter;
typedef Unit< 3,  0,  0,  0,  0,  0,  0,  0>	CubicMeter;
typedef Unit< 1,  0, -1,  0,  0,  0,  0,  0>	MeterPerSecond;
typedef Unit< 3,  0, -1,  0,  0,  0,  0,  0>	CubicMeterPerSecond;
typedef Unit< 1,  0, -2,  0,  0,  0,  0,  0>	MeterPerSecondSquared;
typedef Unit< 1,  0, -3,  0,  0,  0,  0,  0>	MeterPerSecondCubed;
typedef Unit< 1,  0, -4,  0,  0,  0,  0,  0>	MeterPerQuarticSecond;
typedef Unit< 0,  0, -1,  0,  0,  0,  0,  1>	RadianPerSecond;
typedef Unit< 0,  0, -2,  0,  0,  0,  0,  1>	RadianPerSecondSquared;
typedef Unit< 1,  1, -1,  0,  0,  0,  0,  0>	NewtonSecond;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 2,  1, -1,  0,  0,  0,  0,  0>	NewtonMeterSecond;
typedef Unit< 2,  1, -2,  0,  0,  0,  0,  0>	NewtonMeter;
typedef Unit< 1,  1, -3,  0,  0,  0,  0,  0>	NewtonPerSecond;
typedef Unit<-1,  0,  0,  0,  0,  0,  0,  0>	ReciprocalMeter;
typedef Unit<-2,  1,  0,  0,  0,  0,  0,  0>	KilogramPerSquareMeter;
typedef Unit<-3,  1,  0,  0,  0,  0,  0,  0>	KilogramPerCubicMeter;
typedef Unit< 3, -1,  0,  0,  0,  0,  0,  0>	CubicMeterPerKilogram;
typedef Unit<-3,  0,  0,  0,  0,  1,  0,  0>	MolePerCubicMeter;
typedef Unit< 3,  0,  0,  0,  0, -1,  0,  0>	CubicMeterPerMole;
typedef Unit< 2,  1, -1,  0,  0,  0,  0,  0>	JouleSecond;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 2,  1, -2,  0, -1,  0,  0,  0>	JoulePerKelvin;
typedef Unit< 2,  1, -2,  0, -1, -1,  0,  0>	JoulePerKelvinMole;
typedef Unit< 2,  0, -2,  0, -1,  0,  0,  0>	JoulePerKilogramKelvin;
typedef Unit< 2,  1, -2,  0,  0, -1,  0,  0>	JoulePerMole;
typedef Unit< 2,  0, -2,  0,  0,  0,  0,  0>	JoulePerKilogram;
typedef Unit<-1,  1, -2,  0,  0,  0,  0,  0>	JoulePerCubicMeter;
typedef Unit< 0,  1, -2,  0,  0,  0,  0,  0>	NewtonPerMeter;
typedef Unit< 0,  1, -3,  0,  0,  0,  0,  0>	WattPerSquareMeter;
typedef Unit< 1,  1, -3,  0, -1,  0,  0,  0>	WattPerMeterKelvin;
typedef Unit< 2,  0, -1,  0,  0,  0,  0,  0>	SquareMeterPerSecond;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit<-1,  1, -1,  0,  0,  0,  0,  0>	PascalSecond;
typedef Unit<-2,  0,  1,  1,  0,  0,  0,  0>	CoulombPerSquareMeter;
typedef Unit<-3,  0,  1,  1,  0,  0,  0,  0>	CoulombPerCubicMeter;
typedef Unit<-2,  0,  0,  1,  0,  0,  0,  0>	AmperePerSquareMeter;
typedef Unit<-3, -1,  3,  2,  0,  0,  0,  0>	SiemensPerMeter;
typedef Unit< 0, -1,  3,  2,  0, -1,  0,  0>	SiemensSquareMeterPerMole;
typedef Unit<-3, -1,  4,  2,  0,  0,  0,  0>	FaradPerMeter;
typedef Unit< 1,  1, -2, -2,  0,  0,  0,  0>	HenryPerMeter;
typedef Unit< 1,  1, -3, -1,  0,  0,  0,  0>	VoltPerMeter;
typedef Unit<-1,  0,  0,  1,  0,  0,  0,  0>	AmperePerMeter;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit<-2,  0,  0,  0,  0,  0,  1,  0>	CandelaPerSquareMeter;
typedef Unit< 0,  0,  1,  0,  0,  0,  1,  2>	LumenSecond;
typedef Unit<-2,  0,  1,  0,  0,  0,  1,  2>	LuxSecond;
typedef Unit< 0, -1,  1,  1,  0,  0,  0,  0>	CoulombPerKilogram;
typedef Unit< 2,  0, -3,  0,  0,  0,  0,  0>	GrayPerSecond;
typedef Unit< 3,  1, -3, -2,  0,  0,  0,  0>	OhmMeter;
typedef Unit<-1,  1,  0,  0,  0,  0,  0,  0>	KilogramPerMeter;
typedef Unit<-1,  0,  1,  1,  0,  0,  0,  0>	CoulombPerMeter;
typedef Unit< 0, -1,  0,  0,  0,  1,  0,  0>	MolePerKilogram;
typedef Unit< 0,  1,  0,  0,  0, -1,  0,  0>	KilogramPerMole;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit<-2,  0,  0,  0,  0,  0,  0,  0>	MeterPerCubicMeter;
typedef Unit< 0,  1, -1,  0,  0,  0,  0,  0>	KilogramPerSecond;
typedef Unit< 2,  0,  0,  1,  0,  0,  0,  0>	JoulePerTesla;
typedef Unit<-1,  1, -3,  0,  0,  0,  0,  0>	WattPerCubicMeter;
typedef Unit<-2, -1,  3,  0,  1,  0,  0,  0>	KelvinPerWatt;
typedef Unit< 0,  0,  0,  0, -1,  0,  0,  0>	ReciprocalKelvin;
typedef Unit<-1,  0,  0,  0,  1,  0,  0,  0>	KelvinPerMeter;
typedef Unit< 0, -1,  2,  1,  0,  0,  0,  0>	SquareMeterPerVoltSecond;
typedef Unit< 0,  1, -3,  0,  0,  0,  0,  0>	JoulePerSquareMeterSecond;
typedef Unit< 1, -1,  2,  0,  0,  0,  0,  0>	ReciprocalPascal;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit<-2, -1,  2,  2,  0,  0,  0,  0>	ReciprocalHenry;
typedef Unit< 1,  1, -2, -1,  0,  0,  0,  0>	WeberPerMeter;
typedef Unit< 3,  1, -2, -1,  0,  0,  0,  0>	WeberMeter;
typedef Unit< 1,  1, -2, -1,  0,  0,  0,  0>	TeslaMeter;
typedef Unit< 0,  1, -2,  0,  0,  0,  0,  0>	JoulePerSquareMeter;
typedef Unit< 3,  0, -1,  0,  0, -1,  0,  0>	CubicMeterPerMoleSecond;
typedef Unit< 2,  1,  0,  0,  0,  0,  0,  0>	KilogramSquareMeter;
typedef Unit< 2,  0, -1,  0,  0,  0,  0,  0>	NewtonMeterSecondPerKilogram;
typedef Unit< 0,  0, -2,  0,  0,  0,  0,  0>	HertzPerSecond;
typedef Unit<-2, -1,  3,  0,  0,  0,  1,  2>	LumenPerWatt;
//            m  kg   s   A   K mol  cd rad
//            ↓   ↓   ↓   ↓   ↓   ↓   ↓   ↓
typedef Unit< 0,  0,  0,  1,  0,  0,  0,  1>	AmpereRadian;
typedef Unit<-1, -1,  2,  2,  0,  0,  0,  0>	MeterPerHenry;
typedef Unit< 2,  1, -3,  0,  0,  0,  0, -2>	WattPerSteradian;
typedef Unit< 1,  1, -3,  0,  0,  0,  0, -2>	WattPerSteradianMeter;
typedef Unit< 0,  1, -3,  0,  0,  0,  0, -2>	WattPerSteradianSquareMeter;
typedef Unit<-1,  1, -3,  0,  0,  0,  0, -2>	WattPerSteradianCubicMeter;
typedef Unit< 1,  1, -3,  0,  0,  0,  0,  0>	WattPerMeter;

// Often-used scaled units:
typedef ScaledUnit<Meter, std::kilo>				Kilometer;
typedef ScaledUnit<Meter, std::centi>				Centimeter;
typedef ScaledUnit<Meter, std::milli>				Millimeter;
typedef ScaledUnit<Kilogram, std::kilo>				Tonne;
typedef ScaledUnit<Kilogram, std::milli>			Gram;
typedef ScaledUnit<Kilogram, std::micro>			Milligram;
typedef ScaledUnit<Kilogram, std::nano>				Microgram;
typedef ScaledUnit<Second, std::ratio<3600, 1>>		Hour;
typedef ScaledUnit<Second, std::ratio<60, 1>>		Minute;
typedef ScaledUnit<Second, std::milli>				Millisecond;
typedef ScaledUnit<Second, std::micro>				Microsecond;
typedef ScaledUnit<Second, std::nano>				Nanosecond;
typedef ScaledUnit<Ampere, std::milli>				MilliAmpere;
typedef ScaledUnit<Ampere, std::micro>				MicroAmpere;
typedef ScaledUnit<Coulomb, std::ratio<1, 3600>>	Amperehour;
typedef ScaledUnit<Coulomb, std::ratio<10, 36>>		MilliAmperehour;
typedef ScaledUnit<Newton, std::kilo>				KiloNewton;
typedef ScaledUnit<Hertz, std::mega>				MegaHertz;
typedef ScaledUnit<Hertz, std::kilo>				KiloHertz;
typedef ScaledUnit<Watt, std::mega>					MegaWatt;
typedef ScaledUnit<Watt, std::kilo>					KiloWatt;
typedef ScaledUnit<Watt, std::milli>				MilliWatt;
typedef ScaledUnit<Watt, std::micro>				MicroWatt;
typedef ScaledUnit<Pascal, std::kilo>				KiloPascal;
typedef ScaledUnit<Pascal, std::hecto>				HectoPascal;

// Often-used non-standard units:
typedef ScaledUnit<Meter, std::ratio<1'200, 3'937>>													Foot;
typedef ScaledUnit<Meter, std::ratio<1'609'344, 1'000>>												Mile;
typedef ScaledUnit<Meter, std::ratio<1'852, 1>>														NauticalMile;
typedef ScaledUnit<Meter, std::ratio<254, 10'000>>													Inch;
typedef ScaledUnit<Kilogram, std::ratio<45'359'237, 100'000'000>>									PoundMass;
typedef ScaledUnit<MeterPerSecondSquared, std::ratio<980'665, 100'000>>								Gravity;
typedef ScaledUnit<Kelvin, std::ratio<5, 9>>														Rankine;
typedef ScaledUnit<Radian, std::ratio_divide<SmallPi, std::ratio<180>>>								Degree;
typedef ScaledUnit<RadianPerSecond, std::ratio_divide<Pi, std::ratio<30>>>							RotationPerMinute;
typedef ScaledUnit<Pascal, std::ratio<3'386'389, 1000>>												InchOfMercury;
typedef ScaledUnit<MeterPerSecond, std::ratio<10, 36>>												KilometerPerHour;
typedef ScaledUnit<MeterPerSecond, Foot::Scale>														FootPerSecond;
typedef ScaledUnit<FootPerSecond, std::ratio<1, 60>>												FootPerMinute;
typedef ScaledUnit<MeterPerSecond, std::ratio_multiply<NauticalMile::Scale, std::ratio<1, 3600>>>	Knot;

// Units with offset:
typedef ScaledUnit<Kelvin, std::ratio<1>, std::ratio<27'315, 100>>			Celsius;
typedef ScaledUnit<Celsius, std::ratio<5, 9>, std::ratio<-32 * 5, 9>>		Fahrenheit;

// Equivalent units:
typedef Gray Sievert;

} // namespace units
} // namespace si

#endif

