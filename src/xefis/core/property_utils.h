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

#ifndef XEFIS__CORE__PROPERTY_UTILS_H__INCLUDED
#define XEFIS__CORE__PROPERTY_UTILS_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class PropertyPath;


/**
 * Indicates a nil-node, for example when trying to read
 * the value of such node.
 * XXX
 */
class NilNode: public Exception
{
  public:
	NilNode();
};


/**
 * Indicates invalid operation on node of a specific type,
 * for example attempt to read int value of a directory-type
 * node.
 * XXX
 */
class PropertyAccessError: public Exception
{
  public:
	PropertyAccessError (std::string const& message);
};


/**
 * Indicates that given string is not a valid supported type.
 * XXX
 */
class BadType: public Exception
{
  public:
	explicit BadType (std::string const& name);
};


/**
 * Indicates that there was a path conflict while creating
 * directory path with mkpath().
 * XXX
 */
class PropertyPathConflict: public Exception
{
  public:
	PropertyPathConflict (std::string const& message);
};


/**
 * Indicates that property tried to be read
 * could not be found in the node tree.
 * XXX
 */
class PropertyNotFound: public Exception
{
  public:
	explicit PropertyNotFound (std::string const& message);
};


/**
 * Indicates that the property is singular (not attached to any tree)
 * and can't be written or read.
 * XXX
 */
class SingularProperty: public Exception
{
  public:
	explicit SingularProperty (std::string const& message);
};


/**
 * Indicates that the operation is invalid on certain
 * node type.
 * XXX
 */
class InvalidOperation: public Exception
{
  public:
	explicit InvalidOperation (std::string const& message);
};


/**
 * Indicates that there's type conflict between existing property
 * and property requested to be created.
 * XXX
 */
class TypeConflict: public Exception
{
  public:
	explicit TypeConflict (PropertyPath const& path);
};


/**
 * Indicates that there was an error during stringify operation.
 */
class StringifyError: public Exception
{
  public:
	explicit StringifyError (std::string const& message);
};


/**
 * Encapsulates string used as path, for better type safety.
 */
class PropertyPath
{
  public:
	// Ctor
	PropertyPath() = default;

	// Ctor
	explicit PropertyPath (const char* path);

	// Ctor
	explicit PropertyPath (std::string const& path);

	// Ctor
	explicit PropertyPath (QString const& path);

	// Ctor
	PropertyPath (PropertyPath const&) = default;

	// Ctor
	PropertyPath (PropertyPath&&) = default;

	PropertyPath&
	operator= (PropertyPath const&) = default;

	PropertyPath&
	operator= (PropertyPath&&) = default;

	bool
	operator== (PropertyPath const& other) const noexcept;

	/**
	 * Return string reference.
	 */
	std::string const&
	string() const noexcept;

  private:
	std::string _path;
};


/**
 * Encapsulates string used as property type, for better type safety.
 * XXX
 */
class PropertyType
{
  public:
	// Ctor
	PropertyType() = default;

	// Ctor
	explicit PropertyType (const char* type);

	// Ctor
	explicit PropertyType (std::string const& type);

	// Ctor
	explicit PropertyType (QString const& type);

	// Ctor
	PropertyType (PropertyType const&) = default;

	// Ctor
	PropertyType (PropertyType&&) = default;

	PropertyType&
	operator= (PropertyType const&) = default;

	PropertyType&
	operator= (PropertyType&&) = default;

	bool
	operator== (PropertyType const& other) const noexcept;

	/**
	 * Return string reference.
	 */
	std::string const&
	string() const noexcept;

  private:
	/**
	 * Check if type is valid, throw an error if not.
	 */
	static std::string const&
	check_validity (std::string const& type);

  private:
	std::string _type;
};


inline
NilNode::NilNode():
	Exception ("accessed a nil-node")
{ }


inline
PropertyAccessError::PropertyAccessError (std::string const& message):
	Exception (message)
{ }


inline
BadType::BadType (std::string const& name):
	Exception ("'" + name + "' is not valid type name")
{ }


inline
PropertyPathConflict::PropertyPathConflict (std::string const& message):
	Exception (message)
{ }


inline
PropertyNotFound::PropertyNotFound (std::string const& message):
	Exception (message)
{ }


inline
SingularProperty::SingularProperty (std::string const& message):
	Exception (message)
{ }


inline
InvalidOperation::InvalidOperation (std::string const& message):
	Exception (message)
{ }


inline
TypeConflict::TypeConflict (PropertyPath const& path):
	Exception ("property under path '" + path.string() + "' already exists and has different type")
{ }


inline
StringifyError::StringifyError (std::string const& message):
	Exception (message)
{ }


inline
PropertyPath::PropertyPath (const char* path):
	_path (path)
{ }


inline
PropertyPath::PropertyPath (std::string const& path):
	_path (path)
{ }


inline
PropertyPath::PropertyPath (QString const& path):
	PropertyPath (path.toStdString())
{ }


inline bool
PropertyPath::operator== (PropertyPath const& other) const noexcept
{
	return _path == other._path;
}


inline std::string const&
PropertyPath::string() const noexcept
{
	return _path;
}


inline
PropertyType::PropertyType (const char* type):
	_type (check_validity (type))
{ }


inline
PropertyType::PropertyType (std::string const& type):
	_type (check_validity (type))
{ }


inline
PropertyType::PropertyType (QString const& type):
	PropertyType (type.toStdString())
{ }


inline bool
PropertyType::operator== (PropertyType const& other) const noexcept
{
	return _type == other._type;
}


inline std::string const&
PropertyType::string() const noexcept
{
	return _type;
}

} // namespace xf

#endif

