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

#ifndef XEFIS__CORE__SI_PROPERTY_H__INCLUDED
#define XEFIS__CORE__SI_PROPERTY_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "property.h"


namespace Xefis {

template<class siType>
	class SIProperty: public PropertyFloat
	{
	  public:
		typedef siType SIType;

	  public:
		/*
		 * Functions overridden from PropertyFloat.
		 */

		SIType
		read (SIType default_value = SIType()) const;

		SIType
		read_signalling() const;

		SIType
		operator*() const;

		void
		write (SIType const&);

		void
		write_signalling (SIType const&);

		// Overridden from PropertyFloat
		virtual bool
		is_specialized() const noexcept override;

		// Overridden from PropertyFloat
		virtual std::string
		stringify() const override;

		// Overridden from PropertyFloat
		virtual void
		parse (std::string const&) override;

	  private:
		/*
		 * Make some methods from PropertyFloat private, so user can't use them.
		 */

		Type
		read (Type default_value = Type()) const;

		void
		write (Type const&);

		void
		write_signalling (Type const&);
	};


template<class T>
	inline typename
	SIProperty<T>::SIType
	SIProperty<T>::read (SIType default_value) const
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
			{
				SIType ret;
				ret.internal() = node->read<typename SIType::ValueType> (default_value.internal());
				return ret;
			}
		}
		return default_value;
	}


template<class T>
	inline typename
	SIProperty<T>::SIType
	SIProperty<T>::read_signalling() const
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
			{
				SIType ret;
				ret.internal() = node->read<SIType::ValueType>();
				return ret;
			}
			throw PropertyNotFound ("could not find property by path");
		}
		else
			throw SingularProperty ("can't read from a singular property");
	}


template<class T>
	inline typename
	SIProperty<T>::SIType
	SIProperty<T>::operator*() const
	{
		return read (SIType());
	}


template<class T>
	inline void
	SIProperty<T>::write (SIType const& value)
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				node->write<typename SIType::ValueType> (value.internal());
			else
				ensure_path (_path, value.internal());
		}
		else
			throw SingularProperty ("can't write to a singular property");
	}


template<class T>
	inline void
	SIProperty<T>::write_signalling (SIType const& value)
	{
		if (_root)
		{
			PropertyNode* node = get_node();
			if (node)
				node->write<SIType::ValueType> (value.internal());
			else
				throw PropertyNotFound ("could not find property by path");
		}
		else
			throw SingularProperty ("can't write to a singular property");
	}


template<class T>
	inline bool
	SIProperty<T>::is_specialized() const noexcept
	{
		return true;
	}


template<>
	inline std::string
	SIProperty<Angle>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Angle()).deg()) + " deg";
	}


template<>
	inline std::string
	SIProperty<Frequency>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Frequency()).Hz()) + " Hz";
	}


template<>
	inline std::string
	SIProperty<Length>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Length()).ft()) + " ft";
	}


template<>
	inline std::string
	SIProperty<Pressure>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Pressure()).inHg()) + " inHg";
	}


template<>
	inline std::string
	SIProperty<Speed>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Speed()).kt()) + " kt";
	}


template<>
	inline std::string
	SIProperty<Time>::stringify() const
	{
		return boost::lexical_cast<std::string> (read (Time()).s()) + " s";
	}


template<>
	inline void
	SIProperty<Angle>::parse (std::string const& str)
	{
		write (Angle().parse (str));
	}


template<>
	inline void
	SIProperty<Frequency>::parse (std::string const& str)
	{
		write (Frequency().parse (str));
	}


template<>
	inline void
	SIProperty<Length>::parse (std::string const& str)
	{
		write (Length().parse (str));
	}


template<>
	inline void
	SIProperty<Pressure>::parse (std::string const& str)
	{
		write (Pressure().parse (str));
	}


template<>
	inline void
	SIProperty<Speed>::parse (std::string const& str)
	{
		write (Speed().parse (str));
	}


template<>
	inline void
	SIProperty<Time>::parse (std::string const& str)
	{
		write (Time().parse (str));
	}


/*
 * Shortcut types
 */


typedef SIProperty<Angle>		PropertyAngle;
typedef SIProperty<Pressure>	PropertyPressure;
typedef SIProperty<Frequency>	PropertyFrequency;
typedef SIProperty<Length>		PropertyLength;
typedef SIProperty<Time>		PropertyTime;
typedef SIProperty<Speed>		PropertySpeed;

} // namespace Xefis

#endif

