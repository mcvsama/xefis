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
#include <xefis/core/v2/cycle.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_digitizer.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/instrument_aids.h>


class BasicIndicator:
	virtual protected xf::InstrumentAids,
	public v2::Instrument
{
  public:
	/**
	 * Precision is number of digits after decimal point.
	 * Negative values are accepted and have different meaning: value will be divided by 10^n.
	 */
	v2::Setting<int>					precision				{ this, 0 };

	/**
	 * Set modulo value. If > 0, value will be converted to int,
	 * divided by n and the multipled by n again.
	 */
	v2::Setting<unsigned int>			modulo					{ this, false };

	/**
	 * Number of digits displayed.
	 */
	v2::Setting<unsigned int>			digits					{ this, 3 };

	v2::Setting<double>					value_minimum			{ this };
	v2::Setting<std::optional<double>>	value_minimum_critical	{ this, std::nullopt };
	v2::Setting<std::optional<double>>	value_minimum_warning	{ this, std::nullopt };
	v2::Setting<std::optional<double>>	value_maximum_warning	{ this, std::nullopt };
	v2::Setting<std::optional<double>>	value_maximum_critical	{ this, std::nullopt };
	v2::Setting<double>					value_maximum			{ this };

  public:
	// Ctor
	BasicIndicator (std::string const& instance);

  protected:
	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

	QString
	stringify_value (double value) const;
};


class LinearIndicator: public BasicIndicator
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<bool>					mirrored_style			{ this, false };

  public:
	// Ctor
	explicit
	LinearIndicator (v2::PropertyDigitizer, std::string const& instance = {});

	// Module API
	void
	process (v2::Cycle const&) override;

  protected:
	// QWidget API
	void
	paintEvent (QPaintEvent*) override;

	QString
	pad_string (QString const& input) const;

  private:
	v2::PropertyDigitizer	_value_digitizer;
	v2::PropertyObserver	_inputs_observer;
};

#endif

