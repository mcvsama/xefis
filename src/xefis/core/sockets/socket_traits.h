/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__SOCKET_TRAITS_H__INCLUDED
#define XEFIS__CORE__SOCKETS__SOCKET_TRAITS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/utility/blob.h>

// Neutrino:
#include <neutrino/c++20.h>
#include <neutrino/stdexcept.h>

// Boost:
#include <boost/lexical_cast.hpp>

// Standard:
#include <cstddef>
#include <string>
#include <utility>


namespace xf::detail {

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
 * Since AssignableSocket might not defined yet at this point, use template function
 * to allow compilation at all.
 */
template<class AssignableSocketType, class Value>
	extern void
	assign (AssignableSocketType& socket, Value&& value);


template<class Value>
	inline Blob
	apply_generic_value_to_blob (Socket<Value> const& socket, size_t constant_blob_size)
	{
		Blob result (constant_blob_size, 0);
		result[0] = socket ? not_nil : nil;

		if (socket)
		{
			Blob converted;
			value_to_blob (*socket, converted);
			std::copy (converted.begin(), converted.end(), std::next (result.begin()));
		}

		return result;
	}


template<class Value, template<class> class AnySocket>
	inline void
	apply_generic_blob_to_value (AnySocket<Value>& module_out, BlobView blob, size_t constant_blob_size)
	{
		if (blob.size() == constant_blob_size)
		{
			if (blob[0] == not_nil)
			{
				blob.remove_prefix (1);
				Value value;
				blob_to_value (blob, value);
				assign (module_out, value);
			}
			else
				assign (module_out, xf::nil);
		}
		else
			throw InvalidBlobSize (blob.size(), constant_blob_size);
	}


template<class Value, template<class> class AnySocket>
	inline void
	generic_from_string (AnySocket<Value>& socket, std::string_view const& str, SocketConversionSettings const& settings)
	{
		if (str == settings.nil_value)
			assign (socket, xf::nil);
		else
		{
			try {
				assign (socket, boost::lexical_cast<Value> (str));
			}
			catch (boost::bad_lexical_cast&)
			{
				assign (socket, xf::nil);
			}
		}
	}

} // namespace xf::detail


namespace xf {

template<class Value>
	class AssignableSocket;


/**
 * Checks whether given enum contains a value named "Nil".
 */
template<class Enum>
	concept EnumWithNilValue = std::is_enum_v<Enum> && requires { Enum::Nil; };


/**
 * Inherit this utility class if you want to provide generic traits for enum values.
 * Beware that MSB bit is used as nil-indicator with these traits class.
 */
template<class Enum>
	struct EnumSocketTraits
	{
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
			if constexpr (EnumWithNilValue<Enum>)
				return sizeof (Enum);
			else
				return 1 + sizeof (Enum);
		}

		static inline std::string
		to_string (Socket<Enum> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
				return detail::to_string_proxy (*socket);
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<Enum>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (module_out, xf::nil);
			else
			{
				Enum value;
				parse (str, value);
				detail::assign (module_out, value);
			}
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<Enum> const&, SocketConversionSettings const&)
		{
			return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<Enum> const& socket)
		{
			Blob result (constant_blob_size(), 0);

			if constexpr (EnumWithNilValue<Enum>)
			{
				if (socket)
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (*socket), result);
				else
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (Enum::xf_nil_value), result);
			}
			else
			{
				result[0] = socket ? detail::not_nil : detail::nil;

				if (socket)
				{
					Blob tmp;
					value_to_blob (static_cast<std::underlying_type_t<Enum>> (*socket), tmp);
					std::copy (tmp.begin(), tmp.end(), std::next (result.begin()));
				}
			}

			return result;
		}

		static inline void
		from_blob (AssignableSocket<Enum>& module_out, BlobView blob)
		{
			if constexpr (EnumWithNilValue<Enum>)
			{
				if (blob.size() == constant_blob_size())
				{
					Enum result;
					blob_to_value (blob, result);

					if (result == Enum::xf_nil_value)
						detail::assign (module_out, xf::nil);
					else
						detail::assign (module_out, result);
				}
				else
					throw InvalidBlobSize (blob.size(), constant_blob_size());
			}
			else
				detail::apply_generic_blob_to_value (module_out, blob, constant_blob_size());
		}
	};


template<class Integer>
	struct IntegerSocketTraits
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
		to_string (Socket<Integer> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
				return (boost::format (settings.numeric_format) % *socket).str();
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<Integer>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			detail::generic_from_string (module_out, str, settings);
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<Integer> const& socket, SocketConversionSettings const&)
		{
			if (socket)
				return static_cast<float128_t> (*socket);
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<Integer> const& socket)
		{
			return detail::apply_generic_value_to_blob (socket, constant_blob_size());
		}

		static inline void
		from_blob (AssignableSocket<Integer>& module_out, BlobView blob)
		{
			detail::apply_generic_blob_to_value (module_out, blob, constant_blob_size());
		}
	};


template<class FloatingPoint>
	struct FloatingPointSocketTraits
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
		to_string (Socket<FloatingPoint> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
				return std::to_string (*socket);
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<FloatingPoint>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			detail::generic_from_string (module_out, str, settings);
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<FloatingPoint> const& socket, SocketConversionSettings const&)
		{
			if (socket)
				return static_cast<float128_t> (*socket);
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<FloatingPoint> const& socket)
		{
			Blob result;

			if (socket)
				value_to_blob (*socket, result);
			else
				value_to_blob (std::numeric_limits<FloatingPoint>::quiet_NaN(), result);

			return result;
		}

		static inline void
		from_blob (AssignableSocket<FloatingPoint>& module_out, BlobView blob)
		{
			if (blob.size() == constant_blob_size())
			{
				FloatingPoint fp;
				blob_to_value (blob, fp);

				if (std::isnan (fp))
					detail::assign (module_out, xf::nil);
				else
					detail::assign (module_out, fp);
			}
			else
				throw InvalidBlobSize (blob.size(), constant_blob_size());
		}
	};


template<class Value, class Enabled = void>
	struct SocketTraits
	{
	  private:
		static constexpr bool
		has_constant_blob_size();

		static constexpr size_t
		constant_blob_size();

		static inline std::string
		to_string (Socket<Value> const&, SocketConversionSettings const&);

		static inline void
		from_string (AssignableSocket<Value>&, std::string_view const&, SocketConversionSettings const&);

		static inline std::optional<float128_t>
		to_floating_point (Socket<Value> const&, SocketConversionSettings const&);

		static inline Blob
		to_blob (Socket<Value> const&);

		static inline void
		from_blob (AssignableSocket<Value>&, BlobView);
	};


template<>
	struct SocketTraits<bool>
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
		to_string (Socket<bool> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
				return *socket ? settings.true_value : settings.false_value;
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<bool>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			if (str == settings.true_value)
				detail::assign (module_out, true);
			else if (str == settings.false_value)
				detail::assign (module_out, false);
			else
				detail::assign (module_out, xf::nil);
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<bool> const&, SocketConversionSettings const&)
		{
			return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<bool> const& socket)
		{
			if (socket)
				return { *socket ? Blob::value_type (1) : Blob::value_type (0) };
			else
				return { 2 };
		}

		static inline void
		from_blob (AssignableSocket<bool>& module_out, BlobView blob)
		{
			if (blob.size() == constant_blob_size())
			{
				if (blob[0] == 2)
					detail::assign (module_out, xf::nil);
				else
					detail::assign (module_out, !!blob[0]);
			}
			else
				throw InvalidBlobSize (blob.size(), constant_blob_size());
		}
	};


template<>
	struct SocketTraits<int8_t>: public IntegerSocketTraits<int8_t>
	{ };


template<>
	struct SocketTraits<int16_t>: public IntegerSocketTraits<int16_t>
	{ };


template<>
	struct SocketTraits<int32_t>: public IntegerSocketTraits<int32_t>
	{ };


template<>
	struct SocketTraits<int64_t>: public IntegerSocketTraits<int64_t>
	{ };


template<>
	struct SocketTraits<uint8_t>: public IntegerSocketTraits<uint8_t>
	{ };


template<>
	struct SocketTraits<uint16_t>: public IntegerSocketTraits<uint16_t>
	{ };


template<>
	struct SocketTraits<uint32_t>: public IntegerSocketTraits<uint32_t>
	{ };


template<>
	struct SocketTraits<uint64_t>: public IntegerSocketTraits<uint64_t>
	{ };


template<>
	struct SocketTraits<float16_t>: public FloatingPointSocketTraits<float16_t>
	{ };


template<>
	struct SocketTraits<float32_t>: public FloatingPointSocketTraits<float32_t>
	{ };


template<>
	struct SocketTraits<float64_t>: public FloatingPointSocketTraits<float64_t>
	{ };


template<>
	struct SocketTraits<float128_t>: public FloatingPointSocketTraits<float128_t>
	{ };


template<>
	struct SocketTraits<std::string>
	{
		static constexpr bool
		has_constant_blob_size()
		{
			return true;
		}

		static size_t
		constant_blob_size()
		{
			throw InvalidCall ("SocketTraits<std::string>::constant_blob_size()");
		}

		static inline std::string
		to_string (Socket<std::string> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
				return *socket;
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<std::string>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (module_out, xf::nil);
			else
				detail::assign (module_out, std::string (str));
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<std::string> const&, SocketConversionSettings const&)
		{
			return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<std::string> const& socket)
		{
			if (socket)
			{
				Blob result (1 + socket->size(), 0);
				result[0] = detail::not_nil;
				std::copy (socket->begin(), socket->end(), std::next (result.begin()));
				return result;
			}
			else
				return { detail::nil };
		}

		static inline void
		from_blob (AssignableSocket<std::string>& module_out, BlobView blob)
		{
			if (blob.empty())
				throw InvalidBlobSize (0);
			else
			{
				if (blob[0] == detail::not_nil)
					detail::assign (module_out, std::string (std::next (blob.begin()), blob.end()));
				else
					detail::assign (module_out, xf::nil);
			}
		}
	};


template<class Unit>
	struct SocketTraits<si::Quantity<Unit>>
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
		to_string (Socket<si::Quantity<Unit>> const& socket, SocketConversionSettings const& settings)
		{
			if (socket)
			{
				if (settings.preferred_units.empty())
					return (boost::format (settings.numeric_format) % *socket).str();
				else
				{
					for (si::DynamicUnit const& du: settings.preferred_units)
						if (si::is_convertible (Unit::dynamic_unit(), du))
							return (boost::format (settings.numeric_format) % si::convert (Unit::dynamic_unit(), socket->value(), du)).str() + " " + du.symbol();

					return (boost::format (settings.numeric_format) % *socket).str();
				}
			}
			else
				return settings.nil_value;
		}

		static inline void
		from_string (AssignableSocket<si::Quantity<Unit>>& module_out, std::string_view const& str, SocketConversionSettings const& settings)
		{
			if (str == settings.nil_value)
				detail::assign (module_out, xf::nil);
			else
			{
				si::Quantity<Unit> result;
				si::parse (str, result);
				detail::assign (module_out, result);
			}
		}

		static inline std::optional<float128_t>
		to_floating_point (Socket<si::Quantity<Unit>> const& socket, SocketConversionSettings const&)
		{
			if (socket)
				return static_cast<float128_t> (socket->value());
			else
				return std::nullopt;
		}

		static inline Blob
		to_blob (Socket<si::Quantity<Unit>> const& socket)
		{
			return detail::apply_generic_value_to_blob (socket, constant_blob_size());
		}

		static inline void
		from_blob (AssignableSocket<si::Quantity<Unit>>& module_out, BlobView blob)
		{
			detail::apply_generic_blob_to_value (module_out, blob, constant_blob_size());
		}
	};


/**
 * Automatically applies EnumSocketTraits to all enum types.
 */
template<class Enum>
	struct SocketTraits<Enum, std::enable_if_t<std::is_enum_v<Enum>>>: public EnumSocketTraits<Enum>
	{ };

} // namespace xf


// Xefis:
#include <xefis/core/sockets/assignable_socket.h>


namespace xf::detail {

template<class AssignableSocketType, class Value>
	inline void
	assign (AssignableSocketType& socket, Value&& value)
	{
		socket = std::forward<Value> (value);
	}

} // namespace xf::detail

#endif

