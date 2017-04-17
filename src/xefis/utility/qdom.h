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

#ifndef XEFIS__UTILITY__QDOM_H__INCLUDED
#define XEFIS__UTILITY__QDOM_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>

// Qt:
#include <QtXml/QDomElement>


/**
 * Compare element's name to a string.
 */
inline bool
operator== (QDomElement const& element, const char* string) noexcept
{
	return element.tagName() == string;
}


/**
 * Compare element's name to a string.
 */
inline bool
operator== (QDomElement const& element, std::string const& string) noexcept
{
	return element.tagName() == string.c_str();
}


/**
 * Compare element's name to a string.
 */
inline bool
operator== (QDomElement const& element, QString const& string) noexcept
{
	return element.tagName() == string;
}


/**
 * Compare element's name to a string.
 */
inline bool
operator!= (QDomElement const& element, const char* string) noexcept
{
	return !(element == string);
}


/**
 * Compare element's name to a string.
 */
inline bool
operator!= (QDomElement const& element, std::string const& string) noexcept
{
	return !(element == string);
}


/**
 * Compare element's name to a string.
 */
inline bool
operator!= (QDomElement const& element, QString const& string) noexcept
{
	return !(element == string);
}


/*
 * Utility functions
 */


namespace xf {

/**
 * Throw BadDomAttribute if there are attributes in the element
 * that are not listed in the allowed_attributes.
 */
inline void
only_allow_attributes (QDomElement const& e, std::set<QString> const& allowed_attributes)
{
	auto const attrs = e.attributes();
	for (int i = 0; i < attrs.size(); ++i)
	{
		QString name = attrs.item (i).toAttr().name();
		if (allowed_attributes.find (name) != allowed_attributes.end())
			throw BadDomAttribute (e, name);
	}
}


/**
 * Throw MissingDomAttribute if at least one of listed attributes
 * is missing from the DOM element.
 */
inline void
require_attributes (QDomElement const& e, std::set<QString> const& required_attributes)
{
	for (auto const ra: required_attributes)
		if (!e.hasAttribute (ra))
			throw MissingDomAttribute (e, ra);
}


/**
 * Calls both only_allow_attributes() and required_attributes() on given list.
 */
inline void
require_and_only_allow_attributes (QDomElement const& e, std::set<QString> const& attributes)
{
	only_allow_attributes (e, attributes);
	require_attributes (e, attributes);
}

} // namespace xf

#endif

