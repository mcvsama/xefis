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

#ifndef XEFIS__CORE__PROPERTY_IN_H__INCLUDED
#define XEFIS__CORE__PROPERTY_IN_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/property_traits.h>
#include <xefis/utility/variant.h>


namespace xf {

template<class Value>
	class PropertyOut;


/**
 * Wrapper for values that are supposed to act as a constant source
 * for PropertyIn objects.
 */
template<class Value>
	class ConstantSource
	{
	  public:
		// Ctor
		explicit
		ConstantSource (Value value):
			value (value)
		{ }

	  public:
		Value value;
	};


/**
 * Mixin base class for all PropertyIn<*>
 */
class BasicPropertyIn: virtual public PropertyVirtualInterface
{ };


template<class pValue>
	class PropertyIn:
		public BasicPropertyIn,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's coupled to given owner, but doesn't have any data source yet.
		 */
		explicit
		PropertyIn (ModuleIO* owner, std::string const& path);

		/**
		 * Same as PropertyIn (ModuleIO*, std::string), but additionally set up the fallback value.
		 */
		explicit
		PropertyIn (ModuleIO* owner, std::string const& path, Value&& fallback_value);

		// Dtor
		~PropertyIn();

		/**
		 * Set no data source for this property.
		 */
		void
		operator<< (Nil);

		/**
		 * Set PropertyOut as a data source for this property.
		 */
		void
		operator<< (PropertyOut<Value>& other);

		/**
		 * Set a ConstantSource as a data source for this property.
		 */
		template<class ConstantValue>
			void
			operator<< (ConstantSource<ConstantValue> const&);

		// BasicProperty API
		void
		fetch (Cycle const&) override;

		// BasicProperty API
		bool
		has_constant_blob_size() const noexcept override;

		// BasicProperty API
		size_t
		constant_blob_size() const noexcept override;

		// BasicProperty API
		std::string
		to_string() const override;

		// BasicProperty API
		std::string
		to_string (PropertyConversionSettings const&) const override;

		// BasicProperty API
		void
		from_string (std::string const&) override;

		// BasicProperty API
		Blob
		to_blob() const override;

		// BasicProperty API
		void
		from_blob (BlobView) override;

	  private:
		void
		inc_source_use_count() noexcept;

		void
		dec_source_use_count() noexcept;

	  private:
		std::variant<std::monostate, PropertyOut<Value>*, ConstantSource<Value>>	_data_source;
		Cycle::Number																_fetch_cycle_number { 0 };
	};


template<class V>
	inline
	PropertyIn<V>::PropertyIn (ModuleIO* owner, std::string const& path):
		Property<V> (owner, path)
	{
		ModuleIO::ProcessingLoopAPI (*this->io()).register_input_property (*this);
	}


template<class V>
	inline
	PropertyIn<V>::PropertyIn (ModuleIO* owner, std::string const& path, Value&& fallback_value):
		PropertyIn (owner, path)
	{
		this->set_fallback (std::forward<Value> (fallback_value));
	}


template<class V>
	inline
	PropertyIn<V>::~PropertyIn()
	{
		ModuleIO::ProcessingLoopAPI (*this->io()).unregister_input_property (*this);
	}


template<class V>
	inline void
	PropertyIn<V>::operator<< (Nil)
	{
		dec_source_use_count();
		_data_source = std::monostate{};
		this->protected_set_nil();
	}


template<class V>
	inline void
	PropertyIn<V>::operator<< (PropertyOut<Value>& other)
	{
		dec_source_use_count();
		_data_source = &other;
		inc_source_use_count();
		this->protected_set (other);
	}


template<class V>
	template<class C>
		inline void
		PropertyIn<V>::operator<< (ConstantSource<C> const& source)
		{
			dec_source_use_count();
			_data_source = ConstantSource<Value> { source.value };
			inc_source_use_count();
			this->protected_set (source.value);
		}


template<class V>
	inline void
	PropertyIn<V>::fetch (Cycle const& cycle)
	{
		if (_fetch_cycle_number < cycle.number())
		{
			_fetch_cycle_number = cycle.number();
			std::visit (overload {
				[&] (std::monostate) {
					this->protected_set_nil();
				},
				[&] (PropertyOut<Value>* property_source) {
					property_source->fetch (cycle);
					this->protected_set (*property_source);
				},
				[&] (ConstantSource<Value>& constant_source) {
					this->protected_set (constant_source.value);
				}
			}, _data_source);
		}
	}


template<class V>
	inline bool
	PropertyIn<V>::has_constant_blob_size() const noexcept
	{
		return PropertyTraits<V>::has_constant_blob_size();
	}


template<class V>
	inline size_t
	PropertyIn<V>::constant_blob_size() const noexcept
	{
		return PropertyTraits<V>::constant_blob_size();
	}


template<class V>
	inline std::string
	PropertyIn<V>::to_string() const
	{
		return PropertyTraits<V>::to_string (*this, PropertyConversionSettings());
	}


template<class V>
	inline std::string
	PropertyIn<V>::to_string (PropertyConversionSettings const& settings) const
	{
		return PropertyTraits<V>::to_string (*this, settings);
	}


template<class V>
	inline void
	PropertyIn<V>::from_string (std::string const& str)
	{
		PropertyTraits<V>::from_string (*this, str);
	}


template<class V>
	inline Blob
	PropertyIn<V>::to_blob() const
	{
		return PropertyTraits<V>::to_blob (*this);
	}


template<class V>
	inline void
	PropertyIn<V>::from_blob (BlobView blob)
	{
		PropertyTraits<V>::from_blob (*this, blob);
	}


template<class V>
	inline void
	PropertyIn<V>::inc_source_use_count() noexcept
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (PropertyOut<Value>* property_source) noexcept {
				property_source->inc_use_count();
			},
			[&] (ConstantSource<Value>&) noexcept {
				// No action
			}
		}, _data_source);
	}


template<class V>
	inline void
	PropertyIn<V>::dec_source_use_count() noexcept
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (PropertyOut<Value>* property_source) noexcept {
				property_source->dec_use_count();
			},
			[&] (ConstantSource<Value>&) noexcept {
				// No action
			}
		}, _data_source);
	}

} // namespace xf

#endif

