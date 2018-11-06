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
#include <string>
#include <utility>

// Boost:
#include <boost/lexical_cast.hpp>

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


/**
 * Since PropertyOut is not defined yet at this point, use template function
 * to allow compilation at all.
 */
template<class Property, class Value>
	void
	assign (Property& property, Value&& value)
	{
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


template<class Value, template<class> class PropertyOut>
	inline void
	apply_generic_blob_to_value (PropertyOut<Value>& property, BlobView blob, size_t constant_blob_size)
	{
		if (blob.size() == constant_blob_size)
		{
			if (blob[0] == not_nil)
			{
				blob.remove_prefix (1);
				Value value;
				blob_to_value (blob, value);
				assign (property, value);
			}
			else
				assign (property, xf::nil);
		}
		else
			throw InvalidBlobSize (blob.size(), constant_blob_size);
	}


template<class Value, template<class> class Property>
	inline void
	generic_from_string (Property<Value>& property, std::string_view const& str, PropertyConversionSettings const& settings)
	{
		if (str == settings.nil_value)
			assign (property, xf::nil);
		else
		{
			try {
				assign (property, boost::lexical_cast<Value> (str));
			}
			catch (boost::bad_lexical_cast&)
			{
				assign (property, xf::nil);
			}
		}
	}

} // namespace detail


/**
 * Inherit this utility class if you want to provide generic traits for enum values.
 * Beware that MSB bit is used as nil-indicator with these traits class.
 */
template<class Enum>
	struct EnumPropertyTraits
	{
	  private:
		template<class E, class Enabler = void>
			struct HasSpecialNilValue
			{
				constexpr
				operator bool() const
				{
					return false;
				};
			};

		template<class E>
			struct HasSpecialNilValue<E, std::enable_if_t<std::is_enum_v<E> && std::is_void_v<std::void_t<decltype (E::Nil)>>>>
			{
				constexpr
				operator bool() const
				{
					return true;
				};
			};

	  public:
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static constexpr size_t
		constant_blob_size()
		{
			// 1 additional byte is for nil-indication:
			if constexpr (HasSpecialNilValue<Enum>())
				return sizeof (Enum);
			else
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
		from_string (PropertyOut<Enum>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (property, xf::nil);
			else
			{
				Enum value;
				parse (str, value);
				detail::assign (property, value);
			}
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<Enum> const&, PropertyConversionSettings const&)
		{
			return std::nullopt;
		}

		static inline Blob
		to_blob (Property<Enum> const& property)
		{
			Blob result (constant_blob_size(), 0);

			if constexpr (HasSpecialNilValue<Enum>())
			{
				if (property)
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (*property), result);
				else
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (Enum::xf_nil_value), result);
			}
			else
			{
				result[0] = property ? detail::not_nil : detail::nil;

				if (property)
				{
					Blob tmp;
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (*property), tmp);
					std::copy (tmp.begin(), tmp.end(), std::next (result.begin()));
				}
			}

			return result;
		}

		static inline void
		from_blob (PropertyOut<Enum>& property, BlobView blob)
		{
			if constexpr (HasSpecialNilValue<Enum>())
			{
				if (blob.size() == constant_blob_size())
				{
					Enum result;
					blob_to_value (blob, result);

					if (result == Enum::xf_nil_value)
						detail::assign (property, xf::nil);
					else
						detail::assign (property, result);
				}
				else
					throw InvalidBlobSize (blob.size(), constant_blob_size());
			}
			else
				detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
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
		from_string (PropertyOut<Integer>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			detail::generic_from_string (property, str, settings);
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<Integer> const& property, PropertyConversionSettings const&)
		{
			if (property)
				return static_cast<float128_t> (*property);
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Property<Integer> const& property)
		{
			return detail::apply_generic_value_to_blob (property, constant_blob_size());
		}

		static inline void
		from_blob (PropertyOut<Integer>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}
	};


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
			// NaN is used as a nil value.
			return sizeof (FloatingPoint);
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
		from_string (PropertyOut<FloatingPoint>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			detail::generic_from_string (property, str, settings);
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<FloatingPoint> const& property, PropertyConversionSettings const&)
		{
			if (property)
				return static_cast<float128_t> (*property);
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Property<FloatingPoint> const& property)
		{
			Blob result;

			if (property)
				value_to_blob (*property, result);
			else
				value_to_blob (std::numeric_limits<FloatingPoint>::quiet_NaN(), result);

			return result;
		}

		static inline void
		from_blob (PropertyOut<FloatingPoint>& property, BlobView blob)
		{
			if (blob.size() == constant_blob_size())
			{
				FloatingPoint fp;
				blob_to_value (blob, fp);

				if (std::isnan (fp))
					detail::assign (property, xf::nil);
				else
					detail::assign (property, fp);
			}
			else
				throw InvalidBlobSize (blob.size(), constant_blob_size());
		}
	};


template<class Value, class Enabled = void>
	struct PropertyTraits
	{
	  private:
		static constexpr bool
		has_constant_blob_size();

		static constexpr size_t
		constant_blob_size();

		static inline std::string
		to_string (Property<Value> const&, PropertyConversionSettings const&);

		static inline void
		from_string (PropertyOut<Value>&, std::string_view const&, PropertyConversionSettings const&);

		static inline std::optional<float128_t>
		to_floating_point (Property<Value> const&, PropertyConversionSettings const&);

		static inline Blob
		to_blob (Property<Value> const&);

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
		from_string (PropertyOut<bool>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			if (str == settings.true_value)
				detail::assign (property, true);
			else if (str == settings.false_value)
				detail::assign (property, false);
			else
				detail::assign (property, xf::nil);
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<bool> const&, PropertyConversionSettings const&)
		{
			return std::nullopt;
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
		from_blob (PropertyOut<bool>& property, BlobView blob)
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
	struct PropertyTraits<float128_t>: public FloatingPointPropertyTraits<float128_t>
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
		from_string (PropertyOut<std::string>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (property, xf::nil);
			else
				detail::assign (property, std::string (str));
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<std::string> const&, PropertyConversionSettings const&)
		{
			return std::nullopt;
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
		from_blob (PropertyOut<std::string>& property, BlobView blob)
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
			{
				if (settings.preferred_units.empty())
					return (boost::format (settings.numeric_format) % *property).str();
				else
				{
					for (si::DynamicUnit const& du: settings.preferred_units)
						if (si::is_convertible (Unit::dynamic_unit(), du))
							return (boost::format (settings.numeric_format) % si::convert (Unit::dynamic_unit(), property->value(), du)).str() + " " + du.symbol();

					return (boost::format (settings.numeric_format) % *property).str();
				}
			}
			else
				return settings.nil_value;
		}

		static inline void
		from_string (PropertyOut<si::Quantity<Unit>>& property, std::string_view const& str, PropertyConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (property, xf::nil);
			else
			{
				si::Quantity<Unit> result;
				si::parse (str, result);
				detail::assign (property, result);
			}
		}

		static inline std::optional<float128_t>
		to_floating_point (Property<si::Quantity<Unit>> const& property, PropertyConversionSettings const&)
		{
			if (property)
				return static_cast<float128_t> (property->value());
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Property<si::Quantity<Unit>> const& property)
		{
			return detail::apply_generic_value_to_blob (property, constant_blob_size());
		}

		static inline void
		from_blob (PropertyOut<si::Quantity<Unit>>& property, BlobView blob)
		{
			detail::apply_generic_blob_to_value (property, blob, constant_blob_size());
		}
	};


/**
 * Automatically applies EnumPropertyTraits to all enum types.
 */
template<class Enum>
	struct PropertyTraits<Enum, std::enable_if_t<std::is_enum_v<Enum>>>: public EnumPropertyTraits<Enum>
	{ };

} // namespace xf

#endif

