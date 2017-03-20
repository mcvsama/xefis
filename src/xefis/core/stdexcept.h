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

#ifndef XEFIS__CORE__STDEXCEPT_H__INCLUDED
#define XEFIS__CORE__STDEXCEPT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class DomException: public Exception
{
  public:
	using Exception::Exception;

  protected:
	static QString
	get_path (QDomElement const& element)
	{
		QString result;
		QDomNode node = element;

		while (!node.isNull() && node.isElement())
		{
			result = "/" + node.toElement().tagName() + result;
			node = node.parentNode();
		}

		return result;
	}
};


/**
 * Throw to indicate that subelement required but missing.
 */
class MissingDomElement: public DomException
{
  public:
	// Ctor
	explicit
	MissingDomElement (QDomElement const& parent, QString const& child_name):
		DomException ("missing subelement <" + child_name + "> in " + get_path (parent))
	{
		hide_backtrace();
	}
};


/**
 * Throw when an element is not supported in given context.
 */
class BadDomElement: public DomException
{
  public:
	// Ctor
	explicit
	BadDomElement (QDomElement const& element, QString const& additional_message = QString()):
		DomException ("element '" + element.tagName() + "' is not supported in " + get_path (element) +
					  (additional_message.isEmpty() ? "" : ("; " + additional_message)))
	{
		hide_backtrace();
	}

	// Ctor
	BadDomElement (QString const& message):
		DomException (message)
	{
		hide_backtrace();
	}
};


/**
 * Throw to indicate that element needs an attribute.
 */
class MissingDomAttribute: public DomException
{
  public:
	// Ctor
	explicit
	MissingDomAttribute (QDomElement const& element, QString const& attribute_name):
		DomException ("element <" + element.tagName() + "> needs attribute '" + attribute_name + "'")
	{
		hide_backtrace();
	}
};


/**
 * Throw to indicate that an attribute has invalid value.
 */
class BadDomAttribute: public DomException
{
  public:
	// Ctor
	explicit
	BadDomAttribute (QDomElement const& element, QString const& attribute_name, QString const& message = QString()):
		DomException ("invalid value for attribute '" + attribute_name + "' in " + get_path (element) + ": " + message)
	{
		hide_backtrace();
	}
};


/**
 * General configuration error.
 */
class BadConfiguration: public Exception
{
  public:
	// Ctor
	explicit
	BadConfiguration (const char* message, Exception const* inner = nullptr):
		Exception (message, inner)
	{
		hide_backtrace();
	}

	// Ctor
	explicit
	BadConfiguration (std::string const& message, Exception const* inner = nullptr):
		Exception (message, inner)
	{
		hide_backtrace();
	}

	// Ctor
	explicit
	BadConfiguration (QString const& message, Exception const* inner = nullptr):
		Exception (message, inner)
	{
		hide_backtrace();
	}
};


/**
 * General IO error.
 */
class IOError: public Exception
{
  public:
	using Exception::Exception;
};


/**
 * Invalid format during conversion (eg. forbidden character in numeric string).
 */
class InvalidFormat: public Exception
{
  public:
	using Exception::Exception;
};


/**
 * Invalid call (a function should not be called by user, etc).
 */
class InvalidCall: public Exception
{
  public:
	using Exception::Exception;
};

} // namespace xf

#endif

