/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__LOOKAHEAD_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__LOOKAHEAD_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/lookahead.h>


class Lookahead: public Xefis::Module
{
  public:
	// Ctor
	Lookahead (Xefis::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	Xefis::PropertyFloat		_input;
	Xefis::PropertyFloat		_output;
	Xefis::PropertyTime			_lookahead_time;
	Xefis::Smoother<double>		_output_smoother;
	Xefis::Lookahead<double>	_output_estimator;
};

#endif
