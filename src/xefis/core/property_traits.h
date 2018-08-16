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

#ifndef XEFIS__CORE__PROPERTY_TRAITS_H__INCLUDED
#define XEFIS__CORE__PROPERTY_TRAITS_H__INCLUDED

// Standard:
#include <cstddef>

// Boost:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/c++20.h>
#include <xefis/core/property.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/blob.h>


namespace xf {

class BasicPropertyIn;
class BasicPropertyOut;

template<class Value>
	class PropertyIn;

template<class Value>
	class PropertyOut;

template<class Value>
	class ConstantSource;


namespace detail {

constexpr Blob::value_type nil = 0;
constexpr Blob::value_type not_nil = 1;


/**
 * Re-enabler of Argument-Dependent-Lookup.
 */
template<class T>
	constexpr std::string
	to_string_proxy (T value)
	{
		return std::string (to_string (value));
	}


template<class PropertyInOut, class Value>
	inline void
	assign (PropertyInOut& property, Value&& value)
	{
		if constexpr (std::is_base_of<BasicPropertyIn, PropertyInOut>())
		{
			if constexpr (std::is_base_of<Nil, std::remove_cvref_t<Value>>())
				property << xf::no_data_source;
			else
				property << ConstantSource (std::forward<Value> (value));
		}
		else
			property = std::forward<Value> (value);
	}


template<class Value>
	inline Blob
	apply_generic_value_to_blob (Property<Value> const& property, size_t constant_blob_size)
	{
		Blob result (constant_blob_size, 0);
		result[0] = property ? not_nil : nil;

		if (property)
		{
			Blob converted;
			value_to_blob (*property, converted);
			std::copy (converted.begin(), converted.end(), std::next (result.begin()));
		}

		return result;
	}


template<class PropertyInOut>
	inline void
	apply_generic_blob_to_value (PropertyInOut& property, BlobView blob, size_t constant_blob_size)
	{
		if (blob.size() == constant_blob_size)
		{
			if (blob[0] == not_nil)
			{
				blob.remove_prefix (1);
				typename PropertyInOut::Value value;
				blob_to_value (blob, value);
				assign (property, value);
			}
			else
				assign (property, xf::nil);
		}
		else
			throw InvalidBlobSize (blob.size(), constant_blob_size);
	}

} // namespace detail


/**
 * Inherit this utility class if you want to provide generic traits for enum values.
 * Beware that MSB bit is used as nil-indicator with these traits class.
 */
template<class Enum>
	struct EnumPropertyTraits
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			// 1 additional byte is for nil-indication:
			return 1 + sizeof (Enum);
		}

		static inline std::string
		to_string (Property<Enum> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return detail::to_string_proxy (*property);
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<Enum>& property, std::string const& str)
		{
			Enum value;
			parse (str, value);
			property << ConstantSource (value); // TODO detail::assign
		}

		static inline void
		from_string (PropertyOut<Enum>& property, std::string const& str)
		{
			Enum value;
			parse (str, value);
			property = value; // TODO detail::assign
		}

		static inline Blob
		to_blob (Property<Enum> const& property)
		{
			Blob result;

			if (property)
				value_to_blob (static_cast<std::underlying_type_t<Enum>> (*property), result);
			else
			{
				// Store nil-value info in the most significant bit:
				constexpr uint8_t shift = sizeof (Enum) * 8 - 1;
				value_to_blob (std::make_unsigned_t<std::underlying_type_t<Enum>> (1) << shift, result);
			}

			return result;
		}

		static inline void
		from_blob (PropertyIn<Enum>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

		static inline void
		from_blob (PropertyOut<Enum>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

	  private:
		template<class PropertyInOut>
			static inline void
			generic_from_blob (PropertyInOut&, BlobView)
			{
				// TODO if MSB is 1, it's a nil value
				//if (blob.size() == constant_blob_size())
				//{
				//	if (blob[0] == 2)
				//		detail::assign (property, xf::nil);
				//	else
				//		detail::assign (property, !!blob[0]);
				//}
				//else
				//	throw InvalidBlobSize();
			}
	};


template<class Integer>
	struct IntegerPropertyTraits
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			// 1 additional byte is for nil-indication:
			return 1 + sizeof (Integer);
		}

		static inline std::string
		to_string (Property<Integer> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return (boost::format (settings.numeric_format) % *property).str();
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<Integer>&, std::string const&)
		{
			// TODO
		}

		static inline void
		from_string (PropertyOut<Integer>&, std::string const&)
		{
			// TODO
		}

		static inline Blob
		to_blob (Property<Integer> const& property)
		{
			return detail::apply_generic_value_to_blob (property, constant_blob_size());
		}

		static inline void
		from_blob (PropertyIn<Integer>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}

		static inline void
		from_blob (PropertyOut<Integer>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}
	};


// TODO for double/float use nan as nil indicator
// TODO or just use generic IntegerPropertyTraits (renamed to NumericPropertyTraits)
template<class FloatingPoint>
	struct FloatingPointPropertyTraits
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			// 1 additional byte is for nil-indication:
			return 1 + sizeof (FloatingPoint);
		}

		static inline std::string
		to_string (Property<FloatingPoint> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return std::to_string (*property);
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<FloatingPoint>&, std::string const&)
		{
			// TODO
		}

		static inline void
		from_string (PropertyOut<FloatingPoint>&, std::string const&)
		{
			// TODO
		}

		static inline Blob
		to_blob (Property<FloatingPoint> const& property)
		{
			return detail::apply_generic_value_to_blob (property, constant_blob_size());
		}

		static inline void
		from_blob (PropertyIn<FloatingPoint>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}

		static inline void
		from_blob (PropertyOut<FloatingPoint>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}
	};


template<class Value>
	struct PropertyTraits
	{
		static constexpr bool
		has_constant_blob_size();

		static constexpr size_t
		constant_blob_size();

		static inline std::string
		to_string (Property<Value> const&, PropertyConversionSettings const&);

		static inline void
		from_string (PropertyIn<Value>&, std::string const&);

		static inline void
		from_string (PropertyOut<Value>&, std::string const&);

		static inline Blob
		to_blob (Property<Value> const&);

		static inline void
		from_blob (PropertyIn<Value>&, BlobView);

		static inline void
		from_blob (PropertyOut<Value>&, BlobView);
	};


template<>
	struct PropertyTraits<bool>
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			return 1;
		}

		static inline std::string
		to_string (Property<bool> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return *property ? settings.true_value : settings.false_value;
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<bool>&, std::string const&)
		{
			// TODO _property << ConstantSource (s == _true_value);
		}

		static inline void
		from_string (PropertyOut<bool>&, std::string const&)
		{
			// TODO _property = (s == _true_value); // TODO
		}

		static inline Blob
		to_blob (Property<bool> const& property)
		{
			if (property)
				return { *property ? Blob::value_type (1) : Blob::value_type (0) };
			else
				return { 2 };
		}

		static inline void
		from_blob (PropertyIn<bool>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

		static inline void
		from_blob (PropertyOut<bool>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

	  private:
		template<class PropertyInOut>
			static inline void
			generic_from_blob (PropertyInOut& property, BlobView blob)
			{
				if (blob.size() == constant_blob_size())
				{
					if (blob[0] == 2)
						detail::assign (property, xf::nil);
					else
						detail::assign (property, !!blob[0]);
				}
				else
					throw InvalidBlobSize (blob.size(), constant_blob_size());
			}
	};


template<>
	struct PropertyTraits<int8_t>: public IntegerPropertyTraits<int8_t>
	{ };


template<>
	struct PropertyTraits<int16_t>: public IntegerPropertyTraits<int16_t>
	{ };


template<>
	struct PropertyTraits<int32_t>: public IntegerPropertyTraits<int32_t>
	{ };


template<>
	struct PropertyTraits<int64_t>: public IntegerPropertyTraits<int64_t>
	{ };


template<>
	struct PropertyTraits<uint8_t>: public IntegerPropertyTraits<uint8_t>
	{ };


template<>
	struct PropertyTraits<uint16_t>: public IntegerPropertyTraits<uint16_t>
	{ };


template<>
	struct PropertyTraits<uint32_t>: public IntegerPropertyTraits<uint32_t>
	{ };


template<>
	struct PropertyTraits<uint64_t>: public IntegerPropertyTraits<uint64_t>
	{ };


template<>
	struct PropertyTraits<float16_t>: public FloatingPointPropertyTraits<float16_t>
	{ };


template<>
	struct PropertyTraits<float32_t>: public FloatingPointPropertyTraits<float32_t>
	{ };


template<>
	struct PropertyTraits<float64_t>: public FloatingPointPropertyTraits<float64_t>
	{ };


template<>
	struct PropertyTraits<std::string>
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static size_t
		constant_blob_size()
		{
			throw InvalidCall ("PropertyTraits<std::string>::constant_blob_size()");
		}

		static inline std::string
		to_string (Property<std::string> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return *property;
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<std::string>&, std::string const&)
		{
			// TODO
		}

		static inline void
		from_string (PropertyOut<std::string>&, std::string const&)
		{
			// TODO
		}

		static inline Blob
		to_blob (Property<std::string> const& property)
		{
			if (property)
			{
				Blob result (1 + property->size(), 0);
				result[0] = detail::not_nil;
				std::copy (property->begin(), property->end(), std::next (result.begin()));
				return result;
			}
			else
				return { detail::nil };
		}

		static inline void
		from_blob (PropertyIn<std::string>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

		static inline void
		from_blob (PropertyOut<std::string>& property, BlobView blob)
		{
			generic_from_blob (property, blob);
		}

	  private:
		template<class PropertyInOut>
			static inline void
			generic_from_blob (PropertyInOut& property, BlobView blob)
			{
				if (blob.empty())
					throw InvalidBlobSize (0);
				else
				{
					if (blob[0] == detail::not_nil)
						detail::assign (property, std::string (std::next (blob.begin()), blob.end()));
					else
						detail::assign (property, xf::nil);
				}
			}
	};


template<class Unit>
	struct PropertyTraits<si::Quantity<Unit>>
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			return 1 + sizeof (typename si::Quantity<Unit>::Value);
		}

		static inline std::string
		to_string (Property<si::Quantity<Unit>> const& property, PropertyConversionSettings const& settings)
		{
			if (property)
				return (boost::format (settings.numeric_format) % *property).str() + " " + unit_to_string (*property);
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyIn<si::Quantity<Unit>>&, std::string const&)
		{
			// TODO
		}

		static inline void
		from_string (PropertyOut<si::Quantity<Unit>>&, std::string const&)
		{
			// TODO
		}

		static inline Blob
		to_blob (Property<si::Quantity<Unit>> const& property)
		{
			return detail::apply_generic_value_to_blob (property, constant_blob_size());
		}

		static inline void
		from_blob (PropertyIn<si::Quantity<Unit>>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}

		static inline void
		from_blob (PropertyOut<si::Quantity<Unit>>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}
	};

} // namespace xf

#endif

