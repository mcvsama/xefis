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

#ifndef XEFIS__CORE__PROPERTY_UNION_H__INCLUDED
#define XEFIS__CORE__PROPERTY_UNION_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property.h"
#include "property_node.h"


namespace Xefis {

/**
 * Holds one of the basic Property types.
 * Kind of union which tells you which one
 * component is valid.
 */
class PropertyUnion
{
  public:
	PropertyUnion (PropertyBoolean&);

	PropertyUnion (PropertyInteger&);

	PropertyUnion (PropertyFloat&);

	PropertyUnion (PropertyString&);

	/**
	 * Return property type.
	 */
	PropertyType
	type() const;

	/**
	 * Return BaseProperty object.
	 */
	BaseProperty&
	access();

	/**
	 * Return property reference.
	 * Throw Exception if the real type is different.
	 */
	PropertyBoolean&
	access_bool();

	/**
	 * Return property reference.
	 * Throw Exception if the real type is different.
	 */
	PropertyInteger&
	access_int();

	/**
	 * Return property reference.
	 * Throw Exception if the real type is different.
	 */
	PropertyFloat&
	access_float();

	/**
	 * Return property reference.
	 * Throw Exception if the real type is different.
	 */
	PropertyString&
	access_string();

  private:
	PropertyType		_type;
	BaseProperty*		_property			= nullptr;
	PropertyBoolean*	_property_bool		= nullptr;
	PropertyInteger*	_property_int		= nullptr;
	PropertyFloat*		_property_float		= nullptr;
	PropertyString*		_property_string	= nullptr;
};


inline
PropertyUnion::PropertyUnion (PropertyBoolean& property):
	_type (PropBoolean),
	_property (&property),
	_property_bool (&property)
{ }


inline
PropertyUnion::PropertyUnion (PropertyInteger& property):
	_type (PropInteger),
	_property (&property),
	_property_int (&property)
{ }


inline
PropertyUnion::PropertyUnion (PropertyFloat& property):
	_type (PropFloat),
	_property (&property),
	_property_float (&property)
{ }


inline
PropertyUnion::PropertyUnion (PropertyString& property):
	_type (PropString),
	_property (&property),
	_property_string (&property)
{ }


inline PropertyType
PropertyUnion::type() const
{
	return _type;
}


inline BaseProperty&
PropertyUnion::access()
{
	return *_property;
}


inline PropertyBoolean&
PropertyUnion::access_bool()
{
	return *_property_bool;
}


inline PropertyInteger&
PropertyUnion::access_int()
{
	return *_property_int;
}


inline PropertyFloat&
PropertyUnion::access_float()
{
	return *_property_float;
}


inline PropertyString&
PropertyUnion::access_string()
{
	return *_property_string;
}

} // namespace Xefis

#endif

