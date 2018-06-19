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

#ifndef XEFIS__MODULES__INSTRUMENTS__LINEAR_INDICATOR_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__LINEAR_INDICATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_digitizer.h>
#include <xefis/core/property_observer.h>
#include <xefis/core/setting.h>
#include <xefis/support/instrument/instrument_support.h>


// TODO extract as separate file
class BasicIndicatorIO: public xf::ModuleIO
{
  public:
	/**
	 * Precision is number of digits after decimal point.
	 * Negative values are accepted and have different meaning: value will be divided by 10^n.
	 */
	xf::Setting<int>					precision				{ this, "precision", 0 };

	/**
	 * Set modulo value. If > 0, value will be converted to int,
	 * divided by n and the multipled by n again.
	 */
	xf::Setting<unsigned int>			modulo					{ this, "modulo", false };

	/**
	 * Number of digits displayed.
	 */
	xf::Setting<unsigned int>			digits					{ this, "digits", 3 };

	xf::Setting<double>					value_minimum			{ this, "value_minimum" };
	xf::Setting<std::optional<double>>	value_minimum_critical	{ this, "value_minimum_critical", std::nullopt };
	xf::Setting<std::optional<double>>	value_minimum_warning	{ this, "value_minimum_warning", std::nullopt };
	xf::Setting<std::optional<double>>	value_maximum_warning	{ this, "value_maximum_warning", std::nullopt };
	xf::Setting<std::optional<double>>	value_maximum_critical	{ this, "value_maximum_critical", std::nullopt };
	xf::Setting<double>					value_maximum			{ this, "value_maximum" };
};


class LinearIndicatorIO: public BasicIndicatorIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>					mirrored_style			{ this, "mirrored_style", false };
};


template<class IO>
	class BasicIndicator:
		public xf::Instrument<IO>,
		virtual protected xf::InstrumentSupport
	{
	  public:
		// Ctor
		BasicIndicator (std::unique_ptr<IO>, std::string const& instance);

	  protected:
		QString
		stringify_value (double value) const;
	};


class LinearIndicator: public BasicIndicator<LinearIndicatorIO>
{
  public:
	// Ctor
	explicit
	LinearIndicator (std::unique_ptr<LinearIndicatorIO>, xf::PropertyDigitizer, std::string const& instance = {});

	// Module API
	void
	process (xf::Cycle const&) override;

	// Instrument API
	void
	paint (xf::PaintRequest&) const override;

  protected:
	QString
	pad_string (QString const& input) const;

  private:
	xf::PropertyDigitizer	_value_digitizer;
	xf::PropertyObserver	_inputs_observer;
};


template<class IO>
	inline
	BasicIndicator<IO>::BasicIndicator (std::unique_ptr<IO> module_io, std::string const& instance):
		xf::Instrument<IO> (std::move (module_io), instance)
	{ }


template<class IO>
	inline QString
	BasicIndicator<IO>::stringify_value (double value) const
	{
		double numeric_value = value;
		auto& io = this->io;

		if (*io.precision < 0)
			numeric_value /= std::pow (10.0, -*io.precision);

		if (*io.modulo > 0)
			numeric_value = static_cast<int> (numeric_value) / *io.modulo * *io.modulo;

		return QString ("%1").arg (numeric_value, 0, 'f', std::max (0, *io.precision));
	}

#endif

