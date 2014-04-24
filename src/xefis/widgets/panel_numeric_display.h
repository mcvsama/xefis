/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__WIDGETS__PANEL_NUMERIC_DISPLAY_H__INCLUDED
#define XEFIS__WIDGETS__PANEL_NUMERIC_DISPLAY_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtWidgets/QDial>
#include <QtGui/QPixmap>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/widgets/panel_widget.h>


namespace Xefis {

class PanelNumericDisplay: public PanelWidget
{
	static constexpr int BorderWidth	= 2;
	static constexpr int Margin			= 2;

	static constexpr std::size_t MinusSymbolIndex	= 10;
	static constexpr std::size_t EmptySymbolIndex	= 11;

  public:
	/**
	 * Create simple 7-segment numeric display with given number of digits.
	 */
	PanelNumericDisplay (QWidget* parent, Panel*, unsigned int num_digits, bool pad_with_zeros, PropertyInteger value_property);

  protected:
	void
	paintEvent (QPaintEvent*) override;

	void
	data_updated() override;

  private slots:
	/**
	 * Read data from property.
	 */
	void
	read();

  private:
	/**
	 * Convert an integer to string of characters to display.
	 * Returned string will always have size equal to num_digits.
	 * Minimum value for num_digits is 1.
	 * If pad_with_zeros is true and value > 0, then the result will be
	 * padded with '0' chars instead of spaces.
	 */
	static QString
	convert_to_digits (int64_t value, unsigned int num_digits, bool pad_with_zeros);

  private:
	unsigned int			_num_digits;
	bool					_pad_with_zeros;
	std::vector<QPixmap>	_digits_to_display;
	PropertyInteger			_value_property;
	std::array<QPixmap, 12>	_digit_images; // [10] is minus sign, [11] is empty.
};

} // namespace Xefis

#endif

