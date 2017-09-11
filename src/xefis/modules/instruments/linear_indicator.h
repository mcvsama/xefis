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
#include <xefis/core/v1/window.h>
#include <xefis/core/v2/cycle.h>
#include <xefis/core/v2/instrument.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_digitizer.h>
#include <xefis/core/v2/property_observer.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/instrument_aids.h>


class BasicIndicatorIO: public v2::ModuleIO
{
  public:
	/**
	 * Precision is number of digits after decimal point.
	 * Negative values are accepted and have different meaning: value will be divided by 10^n.
	 */
	v2::Setting<int>					precision				{ this, "precision", 0 };

	/**
	 * Set modulo value. If > 0, value will be converted to int,
	 * divided by n and the multipled by n again.
	 */
	v2::Setting<unsigned int>			modulo					{ this, "modulo", false };

	/**
	 * Number of digits displayed.
	 */
	v2::Setting<unsigned int>			digits					{ this, "digits", 3 };

	v2::Setting<double>					value_minimum			{ this, "value_minimum" };
	v2::Setting<std::optional<double>>	value_minimum_critical	{ this, "value_minimum_critical", std::nullopt };
	v2::Setting<std::optional<double>>	value_minimum_warning	{ this, "value_minimum_warning", std::nullopt };
	v2::Setting<std::optional<double>>	value_maximum_warning	{ this, "value_maximum_warning", std::nullopt };
	v2::Setting<std::optional<double>>	value_maximum_critical	{ this, "value_maximum_critical", std::nullopt };
	v2::Setting<double>					value_maximum			{ this, "value_maximum" };
};


class LinearIndicatorIO: public BasicIndicatorIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<bool>					mirrored_style			{ this, "mirrored_style", false };
};


template<class IO>
	class BasicIndicator:
		virtual protected xf::InstrumentAids,
		public v2::Instrument<IO>
	{
	  public:
		// Ctor
		BasicIndicator (std::unique_ptr<IO>, std::string const& instance);

	  protected:
		// QWidget API
		void
		resizeEvent (QResizeEvent*) override;

		QString
		stringify_value (double value) const;
	};


template<class IO>
	inline
	BasicIndicator<IO>::BasicIndicator (std::unique_ptr<IO> module_io, std::string const& instance):
		xf::InstrumentAids (1.0f),
		v2::Instrument<IO> (std::move (module_io), instance)
	{ }


template<class IO>
	inline void
	BasicIndicator<IO>::resizeEvent (QResizeEvent*)
	{
		auto xw = dynamic_cast<v1::Window*> (this->window());
		if (xw)
			InstrumentAids::set_scaling (1.2f * xw->pen_scale(), 0.95f * xw->font_scale());

		InstrumentAids::update_sizes (this->size(), this->window()->size());
	}


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


class LinearIndicator: public BasicIndicator<LinearIndicatorIO>
{
  public:
	// Ctor
	explicit
	LinearIndicator (std::unique_ptr<LinearIndicatorIO>, v2::PropertyDigitizer, std::string const& instance = {});

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

