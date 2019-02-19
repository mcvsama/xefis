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

#ifndef NEUTRINO__SI__STANDARD_QUANTITIES_H__INCLUDED
#define NEUTRINO__SI__STANDARD_QUANTITIES_H__INCLUDED

// Standard:
#include <cstddef>

// Local:
#include "quantity.h"
#include "standard_units.h"


namespace si {
namespace quantities {

using namespace si::units;

// Basic SI quantities:
typedef Quantity<Dimensionless>					Scalar;
typedef Quantity<Meter>							Length;
typedef Quantity<Kilogram>						Mass;
typedef Quantity<Second>						Time;
typedef Quantity<Ampere>						Current;
typedef Quantity<Kelvin>						Temperature;
typedef Quantity<Mole>							Amount;
typedef Quantity<Candela>						LuminousIntensity;

// Named derived quantities:
typedef Quantity<Hertz>							Frequency;
typedef Quantity<Radian>						Angle;
typedef Quantity<Steradian>						SolidAngle;
typedef Quantity<Newton>						Force;
typedef Quantity<Pascal>						Pressure;
typedef Quantity<Pascal>						Stress;
typedef Quantity<Joule>							Energy;
typedef Quantity<Joule>							Work;
typedef Quantity<Joule>							Heat;
typedef Quantity<Watt>							Power;
typedef Quantity<Watt>							RadiantFlux;
typedef Quantity<Coulomb>						Charge;
typedef Quantity<Volt>							Voltage;
typedef Quantity<Farad>							Capacitance;
typedef Quantity<Ohm>							Resistance;
typedef Quantity<Ohm>							Impedance;
typedef Quantity<Ohm>							Reactance;
typedef Quantity<Siemens>						Conductance;
typedef Quantity<Weber>							MagneticFlux;
typedef Quantity<Tesla>							MagneticField;
typedef Quantity<Henry>							Inductance;
typedef Quantity<Lumen>							LuminousFlux;
typedef Quantity<Lux>							Illuminance;
typedef Quantity<Becquerel>						Radioactivity;
typedef Quantity<Gray>							AbsorbedDose;
typedef Quantity<Katal>							CatalyticActivity;

// Derived quantities of unnamed units:
typedef Quantity<SquareMeter>					Area;
typedef Quantity<CubicMeter>					Volume;
typedef Quantity<MeterPerSecond>				Velocity;
typedef Quantity<CubicMeterPerSecond>			VolumetricFlow;
typedef Quantity<MeterPerSecondSquared>			Acceleration;
typedef Quantity<MeterPerSecondCubed>			Jerk;
typedef Quantity<MeterPerQuarticSecond>			Jounce;
typedef Quantity<RadianPerSecond>				AngularVelocity;
typedef Quantity<BaseRadianPerSecond>			BaseAngularVelocity;
typedef Quantity<RadianPerSecondSquared>		AngularAcceleration;
typedef Quantity<BaseRadianPerSecondSquared>	BaseAngularAcceleration;
typedef Quantity<NewtonSecond>					Momentum;
typedef Quantity<NewtonSecond>					Impulse;
typedef Quantity<NewtonMeterSecond>				AngularMomentum;
typedef Quantity<NewtonMeter>					Torque;
typedef Quantity<NewtonPerSecond>				Yank;
typedef Quantity<ReciprocalMeter>				Wavenumber;
typedef Quantity<ReciprocalMeter>				OpticalPower;
typedef Quantity<ReciprocalMeter>				Curvature;
typedef Quantity<ReciprocalMeter>				SpatialFrequency;
typedef Quantity<KilogramPerSquareMeter>		AreaDensity;
typedef Quantity<KilogramPerCubicMeter>			Density;
typedef Quantity<CubicMeterPerKilogram>			SpecificVolume;
typedef Quantity<MolePerCubicMeter>				Molarity;
typedef Quantity<CubicMeterPerMole>				MolarVolume;
typedef Quantity<JouleSecond>					Action;
typedef Quantity<JoulePerKelvin>				HeatCapacity;
typedef Quantity<JoulePerKelvin>				Entropy;
typedef Quantity<JoulePerKelvinMole>			MolarHeatCapacity;
typedef Quantity<JoulePerKelvinMole>			MolarHeatEntropy;
typedef Quantity<JoulePerKilogramKelvin>		SpecificHeatCapacity;
typedef Quantity<JoulePerKilogramKelvin>		SpecificEntropy;
typedef Quantity<JoulePerMole>					MolarEnergy;
typedef Quantity<JoulePerKilogram>				SpecificEnergy;
typedef Quantity<JoulePerCubicMeter>			EnergyDensity;
typedef Quantity<NewtonPerMeter>				SurfaceTension;
typedef Quantity<NewtonPerMeter>				Stiffness;
typedef Quantity<WattPerSquareMeter>			HeatFluxDensity;
typedef Quantity<WattPerSquareMeter>			Irradiance;
typedef Quantity<WattPerMeterKelvin>			ThermalConductivity;
typedef Quantity<SquareMeterPerSecond>			KinematicViscosity;
typedef Quantity<SquareMeterPerSecond>			ThermalDiffusivity;
typedef Quantity<SquareMeterPerSecond>			DiffusionCoefficient;
typedef Quantity<PascalSecond>					DynamicViscosity;
typedef Quantity<CoulombPerSquareMeter>			ElectricDisplacementField;
typedef Quantity<CoulombPerSquareMeter>			PolarizationDensity;
typedef Quantity<CoulombPerCubicMeter>			ChargeDensity;
typedef Quantity<AmperePerSquareMeter>			CurrentDensity;
typedef Quantity<SiemensPerMeter>				ElectricalConductivity;
typedef Quantity<SiemensSquareMeterPerMole>		MolarConductivity;
typedef Quantity<FaradPerMeter>					Permittivity;
typedef Quantity<HenryPerMeter>					MagneticPermeability;
typedef Quantity<VoltPerMeter>					ElectricFieldStrength;
typedef Quantity<AmperePerMeter>				Magnetization;
typedef Quantity<AmperePerMeter>				MagneticFieldStrength;
typedef Quantity<CandelaPerSquareMeter>			Luminance;
typedef Quantity<LumenSecond>					LuminousEnergy;
typedef Quantity<LuxSecond>						LuminousExposure;
typedef Quantity<CoulombPerKilogram>			Exposure;
typedef Quantity<GrayPerSecond>					AbsorbedDoseRate;
typedef Quantity<OhmMeter>						Resistivity;
typedef Quantity<KilogramPerMeter>				LinearMassDensity;
typedef Quantity<CoulombPerMeter>				LinearChargeDensity;
typedef Quantity<MolePerKilogram>				Molality;
typedef Quantity<KilogramPerMole>				MolarMass;
typedef Quantity<MeterPerCubicMeter>			FuelEfficiency;
typedef Quantity<KilogramPerSecond>				MassFlowRate;
typedef Quantity<JoulePerTesla>					MagneticDipoleMoment;
typedef Quantity<WattPerCubicMeter>				SpecralIrradiance;
typedef Quantity<WattPerCubicMeter>				PowerDensity;
typedef Quantity<KelvinPerWatt>					ThermalResistance;
typedef Quantity<ReciprocalKelvin>				ThermalExpansionCoefficient;
typedef Quantity<KelvinPerMeter>				TemperatureGradient;
typedef Quantity<SquareMeterPerVoltSecond>		ElectronMobility;
typedef Quantity<JoulePerSquareMeterSecond>		EnergyFluxDensity;
typedef Quantity<ReciprocalPascal>				Compressibility;
typedef Quantity<ReciprocalHenry>				MagneticReluctance;
typedef Quantity<WeberPerMeter>					MagneticVectorPotential;
typedef Quantity<WeberMeter>					MagneticMoment;
typedef Quantity<TeslaMeter>					MagneticRigidity;
typedef Quantity<JoulePerSquareMeter>			RadiantExposure;
typedef Quantity<CubicMeterPerMoleSecond>		CatalyticEfficiency;
typedef Quantity<KilogramSquareMeter>			MomentOfInertia;
typedef Quantity<NewtonMeterSecondPerKilogram>	SpecificAngularMomentum;
typedef Quantity<HertzPerSecond>				FrequencyDrift;
typedef Quantity<LumenPerWatt>					LuminousEfficacy;
typedef Quantity<AmpereRadian>					MagnetomotiveForce;
typedef Quantity<MeterPerHenry>					MagneticSusceptibility;
typedef Quantity<WattPerSteradian>				RadiantIntensity;
typedef Quantity<WattPerSteradianMeter>			SpectralIntensity;
typedef Quantity<WattPerSteradianSquareMeter>	Radiance;
typedef Quantity<WattPerSteradianCubicMeter>	SpectralRadiance;
typedef Quantity<WattPerMeter>					SpectralPower;

// Equivalent quantities:
typedef Quantity<Sievert>						EquivalentDose;
typedef Quantity<MeterPerSecond>				Speed; // Alias

} // namespace quantities
} // namespace si

#endif

