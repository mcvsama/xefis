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

#ifndef XEFIS__CORE__MODULE_H__INCLUDED
#define XEFIS__CORE__MODULE_H__INCLUDED

// Standard:
#include <cstddef>
#include <initializer_list>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property_union.h>


namespace Xefis {

class Module
{
  public:
	struct NameAndProperty
	{
		QString			name;
		PropertyUnion	property;
		bool			required;
	};

	typedef std::vector<NameAndProperty> PropertiesList;

  public:
	// Dtor
	virtual ~Module();

  protected:
	/**
	 * Parse the <properties> element and initialize properties
	 * by their names matching the <properties> children.
	 */
	void
	parse_properties (QDomElement& properties_element, PropertiesList);
};


inline
Module::~Module()
{ }

} // namespace Xefis

#endif

