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

#ifndef XEFIS__CORE__PROPERTY_OUT_H__INCLUDED
#define XEFIS__CORE__PROPERTY_OUT_H__INCLUDED

// Standard:
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/core/property_traits.h>
#include <xefis/utility/utility.h>
#include <xefis/utility/variant.h>


namespace xf {

template<class Value>
	class PropertyIn;


/**
 * Mixin base class for all PropertyOut<*>
 */
class BasicPropertyOut:
	virtual public PropertyVirtualInterface,
	virtual public BasicProperty
{
  public:
	/**
	 * Set property to nil-value.
	 */
	virtual void
	operator= (Nil) = 0;

	/**
	 * Unserializes property from string.
	 */
	virtual void
	from_string (std::string_view const&, PropertyConversionSettings const& = {}) = 0;

	/**
	 * Unserializes property from Blob.
	 *
	 * \throw	InvalidBlobSize
	 *			If blob has size not corresponding to this property type.
	 */
	virtual void
	from_blob (BlobView) = 0;
};


template<class pValue>
	class PropertyOut final:
		public BasicPropertyOut,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's coupled to a ModuleIO and set the module as data source.
		 */
		explicit
		PropertyOut (ModuleIO* owner_and_data_source, std::string_view const& path);

		// Dtor
		~PropertyOut();

		// Copy operator
		PropertyOut<Value> const&
		operator= (PropertyOut<Value> const&);

		/**
		 * Alias for Property::protected_set_nil().
		 */
		void
		operator= (Nil) override;

		/**
		 * Alias for Property::protected_set (std::optional<Value>)
		 */
		PropertyOut const&
		operator= (std::optional<Value>);

		/**
		 * Alias for Property::protected_set (Property<Value> const&)
		 */
		PropertyOut const&
		operator= (Property<Value> const&);

		/**
		 * Set this property as data source for the other property.
		 */
		void
		operator>> (PropertyIn<Value>& other);

		/**
		 * Set this property as data source for the other property.
		 */
		void
		operator>> (PropertyOut<Value>& other);

		/**
		 * Set no data source for this property.
		 */
		void
		operator<< (NoDataSource) override;

		/**
		 * Set PropertyOut as a data source for this property.
		 */
		void
		operator<< (PropertyOut<Value>& other);

		/**
		 * Increase use-count of this property.
		 * Add given property to list of users of this property.
		 */
		void
		inc_use_count (BasicProperty*) noexcept;

		/**
		 * Decrease use-count of this property.
		 * Remove given property from list of users of this property.
		 */
		void
		dec_use_count (BasicProperty*) noexcept;

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

		// BasicPropertyOut API
		void
		from_string (std::string_view const&, PropertyConversionSettings const& = {}) override;

		// BasicPropertyOut API
		void
		from_blob (BlobView) override;

		// PropertyVirtualInterface API
		void
		deregister() override;

	  private:
		void
		inc_source_use_count() noexcept;

		void
		dec_source_use_count() noexcept;

	  private:
		std::variant<std::monostate, ModuleIO*, PropertyOut<Value>*>	_data_source;
		std::vector<BasicProperty*>										_data_sinks;
		Cycle::Number													_fetch_cycle_number { 0 };
	};


template<class V>
	inline
	PropertyOut<V>::PropertyOut (ModuleIO* owner_and_data_source, std::string_view const& path):
		BasicProperty (owner_and_data_source, path)
	{
		_data_source = owner_and_data_source;
		_data_sinks.reserve (8);
		inc_source_use_count();
		ModuleIO::ProcessingLoopAPI (*this->io()).register_output_property (*this);
	}


template<class V>
	inline
	PropertyOut<V>::~PropertyOut()
	{
		for (auto* data_sink: clone (_data_sinks))
			(*data_sink) << no_data_source;

		deregister();
	}


template<class V>
	inline PropertyOut<V> const&
	PropertyOut<V>::operator= (PropertyOut<Value> const& other)
	{
		this->protected_set (other);
		return *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator= (Nil)
	{
		this->protected_set_nil();
	}


template<class V>
	inline PropertyOut<V> const&
	PropertyOut<V>::operator= (std::optional<Value> value)
	{
		this->protected_set (value);
		return *this;
	}


template<class V>
	inline PropertyOut<V> const&
	PropertyOut<V>::operator= (Property<Value> const& value)
	{
		this->protected_set (value);
		return *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator>> (PropertyIn<Value>& other)
	{
		other << *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator>> (PropertyOut<Value>& other)
	{
		other << *this;
	}


template<class V>
	inline void
	PropertyOut<V>::operator<< (NoDataSource)
	{
		dec_source_use_count();
		_data_source = std::monostate();
		inc_source_use_count();
	}


template<class V>
	inline void
	PropertyOut<V>::operator<< (PropertyOut<Value>& other)
	{
		dec_source_use_count();
		_data_source = &other;
		inc_source_use_count();
	}


template<class V>
	inline void
	PropertyOut<V>::inc_use_count (BasicProperty* data_sink) noexcept
	{
		_data_sinks.push_back (data_sink);
	}


template<class V>
	inline void
	PropertyOut<V>::dec_use_count (BasicProperty* data_sink) noexcept
	{
		auto new_end = std::remove (_data_sinks.begin(), _data_sinks.end(), data_sink);
		_data_sinks.resize (std::distance (_data_sinks.begin(), new_end));
	}


template<class V>
	inline std::size_t
	PropertyOut<V>::use_count() const noexcept
	{
		return _data_sinks.size();
	}


template<class V>
	inline void
	PropertyOut<V>::fetch (Cycle const& cycle)
	{
		if (_fetch_cycle_number < cycle.number())
		{
			_fetch_cycle_number = cycle.number();
			std::visit (overload {
				[&] (std::monostate) {
					this->protected_set_nil();
				},
				[&] (ModuleIO* module_source) {
					BasicModule::ProcessingLoopAPI (module_source->module()).fetch_and_process (cycle);
				},
				[&] (PropertyOut<Value>* property_source) {
					property_source->fetch (cycle);
					this->protected_set (*property_source);
				}
			}, _data_source);
		}
	}


template<class V>
	inline bool
	PropertyOut<V>::has_constant_blob_size() const noexcept
	{
		return PropertyTraits<V>::has_constant_blob_size();
	}


template<class V>
	inline size_t
	PropertyOut<V>::constant_blob_size() const noexcept
	{
		return PropertyTraits<V>::constant_blob_size();
	}


template<class V>
	inline std::string
	PropertyOut<V>::to_string (PropertyConversionSettings const& settings) const
	{
		return PropertyTraits<V>::to_string (*this, settings);
	}


template<class V>
	inline std::optional<float128_t>
	PropertyOut<V>::to_floating_point (PropertyConversionSettings const& settings) const
	{
		return PropertyTraits<V>::to_floating_point (*this, settings);
	}


template<class V>
	inline Blob
	PropertyOut<V>::to_blob() const
	{
		return PropertyTraits<V>::to_blob (*this);
	}


template<class V>
	inline void
	PropertyOut<V>::from_string (std::string_view const& str, PropertyConversionSettings const& settings)
	{
		PropertyTraits<V>::from_string (*this, str, settings);
	}


template<class V>
	inline void
	PropertyOut<V>::from_blob (BlobView blob)
	{
		PropertyTraits<V>::from_blob (*this, blob);
	}


template<class V>
	inline void
	PropertyOut<V>::deregister()
	{
		if (this->io())
			ModuleIO::ProcessingLoopAPI (*this->io()).unregister_output_property (*this);

		// Order is important:
		(*this) << no_data_source;
		this->_owner = nullptr;
	}


template<class V>
	inline void
	PropertyOut<V>::inc_source_use_count() noexcept
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ModuleIO*) noexcept {
				// No action
			},
			[&] (PropertyOut<Value>* property_source) noexcept {
				property_source->inc_use_count (this);
			}
		}, _data_source);
	}


template<class V>
	inline void
	PropertyOut<V>::dec_source_use_count() noexcept
	{
		std::visit (overload {
			[&] (std::monostate) noexcept {
				// No action
			},
			[&] (ModuleIO*) noexcept {
				// No action
			},
			[&] (PropertyOut<Value>* property_source) noexcept {
				property_source->dec_use_count (this);
			}
		}, _data_source);
	}

} // namespace xf

#endif

