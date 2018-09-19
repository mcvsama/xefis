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
class BasicPropertyIn:
	virtual public PropertyVirtualInterface,
	virtual public BasicProperty
{ };


template<class pValue>
	class PropertyIn final:
		public BasicPropertyIn,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's coupled to given owner, but doesn't have any data source yet.
		 */
		explicit
		PropertyIn (ModuleIO* owner, std::string_view const& path);

		/**
		 * Same as PropertyIn (ModuleIO*, std::string_view), but additionally set up the fallback value.
		 */
		explicit
		PropertyIn (ModuleIO* owner, std::string_view const& path, Value fallback_value);

		// Dtor
		~PropertyIn();

		// Forbid copying
		PropertyIn<Value>&
		operator= (PropertyIn<Value> const&) = delete;

		// Move operator
		[[nodiscard]]
		PropertyIn<Value>&
		operator= (PropertyIn<Value>&&) = default;

		// BasicPropertyIn API
		void
		operator<< (NoDataSource) override;

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
		[[nodiscard]]
		std::size_t
		use_count() const noexcept override;

		// BasicProperty API
		void
		fetch (Cycle const&) override;

		// BasicProperty API
		[[nodiscard]]
		bool
		has_constant_blob_size() const noexcept override;

		// BasicProperty API
		[[nodiscard]]
		size_t
		constant_blob_size() const noexcept override;

		// BasicProperty API
		[[nodiscard]]
		std::string
		to_string (PropertyConversionSettings const& = {}) const override;

		// BasicProperty API
		[[nodiscard]]
		std::optional<float128_t>
		to_floating_point (PropertyConversionSettings const& = {}) const override;

		// BasicProperty API
		[[nodiscard]]
		Blob
		to_blob() const override;

		// PropertyVirtualInterface API
		void
		deregister() override;

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
	PropertyIn<V>::PropertyIn (ModuleIO* owner, std::string_view const& path):
		BasicProperty (owner, path)
	{
		ModuleIO::ProcessingLoopAPI (*this->io()).register_input_property (*this);
	}


template<class V>
	inline
	PropertyIn<V>::PropertyIn (ModuleIO* owner, std::string_view const& path, Value fallback_value):
		PropertyIn (owner, path)
	{
		this->set_fallback (fallback_value);
	}


template<class V>
	inline
	PropertyIn<V>::~PropertyIn()
	{
		deregister();
	}


template<class V>
	inline void
	PropertyIn<V>::operator<< (NoDataSource)
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
	inline std::size_t
	PropertyIn<V>::use_count() const noexcept
	{
		return 0;
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
	PropertyIn<V>::to_string (PropertyConversionSettings const& settings) const
	{
		return PropertyTraits<V>::to_string (*this, settings);
	}


template<class V>
	inline std::optional<float128_t>
	PropertyIn<V>::to_floating_point (PropertyConversionSettings const& settings) const
	{
		return PropertyTraits<V>::to_floating_point (*this, settings);
	}


template<class V>
	inline Blob
	PropertyIn<V>::to_blob() const
	{
		return PropertyTraits<V>::to_blob (*this);
	}


template<class V>
	inline void
	PropertyIn<V>::deregister()
	{
		if (this->io())
			ModuleIO::ProcessingLoopAPI (*this->io()).unregister_input_property (*this);

		// Order is important:
		(*this) << no_data_source;
		this->_owner = nullptr;
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
				property_source->inc_use_count (this);
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
				property_source->dec_use_count (this);
			},
			[&] (ConstantSource<Value>&) noexcept {
				// No action
			}
		}, _data_source);
	}

} // namespace xf

#endif

