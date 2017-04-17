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

#ifndef XEFIS__UTILITY__QDOM_ITERATOR_H__INCLUDED
#define XEFIS__UTILITY__QDOM_ITERATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>

// Qt:
#include <QtXml/QDomElement>


namespace xf {

/**
 * Sequence iterator for use with for(:) loops.
 */
class QDomSubElementsIterator
{
  public:
	/**
	 * Create past-the-end iterator.
	 */
	QDomSubElementsIterator() = default;

	explicit
	QDomSubElementsIterator (QDomElement element);

	bool
	operator== (QDomSubElementsIterator const& other) const;

	bool
	operator!= (QDomSubElementsIterator const& other) const;

	void
	operator++();

	QDomElement&
	operator*();

	QDomElement*
	operator->();

  private:
	QDomElement _element;
};


inline
QDomSubElementsIterator::QDomSubElementsIterator (QDomElement element):
	_element (element)
{ }


inline bool
QDomSubElementsIterator::operator== (QDomSubElementsIterator const& other) const
{
	return _element == other._element;
}


inline bool
QDomSubElementsIterator::operator!= (QDomSubElementsIterator const& other) const
{
	return _element != other._element;
}


inline void
QDomSubElementsIterator::operator++()
{
	_element = _element.nextSiblingElement();
}


inline QDomElement&
QDomSubElementsIterator::operator*()
{
	return _element;
}


inline QDomElement*
QDomSubElementsIterator::operator->()
{
	return &_element;
}


/*
 * Global functions
 */


inline QDomSubElementsIterator
iterate_sub_elements (QDomElement element)
{
	return QDomSubElementsIterator (element);
}


/**
 * Support for generic iterating over element's children
 * with range-for.
 */
inline xf::QDomSubElementsIterator
begin (xf::QDomSubElementsIterator element_iterator)
{
	return xf::QDomSubElementsIterator (element_iterator->firstChildElement());
}


/**
 * Support for generic iterating over element's children
 * with range-for.
 */
inline xf::QDomSubElementsIterator
end (xf::QDomSubElementsIterator)
{
	return xf::QDomSubElementsIterator();
}

} // namespace xf

#endif

