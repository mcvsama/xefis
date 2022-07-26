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

#ifndef XEFIS__CORE__GRAPHICS_H__INCLUDED
#define XEFIS__CORE__GRAPHICS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QFont>

// Standard:
#include <cstddef>


namespace xf {

class Graphics
{
  public:
	// Ctor
	explicit
	Graphics (Logger const&);

	/**
	 * Basic instrument font.
	 */
	[[nodiscard]]
	QFont const&
	instrument_font() const noexcept;

	/**
	 * Basic instrument font.
	 */
	[[nodiscard]]
	QFont const&
	panel_font() const noexcept;

  private:
	/**
	 * Load fonts used by Xefis and setup default QFonts to use.
	 */
	void
	setup_fonts();

  private:
	Logger	_logger;
	QFont	_instrument_font;
	QFont	_panel_font;
};


inline QFont const&
Graphics::instrument_font() const noexcept
{
	return _instrument_font;
}


inline QFont const&
Graphics::panel_font() const noexcept
{
	return _panel_font;
}

} // namespace xf

#endif

