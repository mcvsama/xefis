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
#include <optional>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/module_io.h>
#include <xefis/utility/variant.h>


namespace xf {

template<class Value>
	class PropertyIn;


/**
 * Mixin base class for all PropertyOut<*>
 */
class BasicPropertyOut: virtual public PropertyVirtualInterface
{
  public:
	/**
	 * Set property to nil-value.
	 */
	virtual void
	operator= (Nil) = 0;
};


template<class pValue>
	class PropertyOut:
		public BasicPropertyOut,
		public Property<pValue>
	{
	  public:
		typedef pValue Value;

		/**
		 * Create Property that's not coupled to any ModuleIO and don't have any data source yet.
		 */
		//explicit
		//TODO PropertyOut (std::string const& path);

		/**
		 * Create Property that's coupled to a ModuleIO and set the module as data source.
		 */
		explicit
		PropertyOut (ModuleIO* owner_and_data_source, std::string const& path);

		// Dtor
		~PropertyOut();

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
		operator<< (Nil);

		/**
		 * Set PropertyOut as a data source for this property.
		 */
		void
		operator<< (PropertyOut<Value>& other);

		// BasicProperty API
		void
		fetch (Cycle const&) override;

	  private:
		void
		inc_source_use_count() noexcept;

		void
		dec_source_use_count() noexcept;

	  private:
		std::variant<std::monostate, ModuleIO*, PropertyOut<Value>*>	_data_source;
		Cycle::Number													_fetch_cycle_number { 0 };
	};


// TODO
#if 0
template<class V>
	inline
	PropertyOut<V>::PropertyOut (std::string const& path):
		Property<V> (path)
	{ }
#endif


template<class V>
	inline
	PropertyOut<V>::PropertyOut (ModuleIO* owner_and_data_source, std::string const& path):
		Property<V> (owner_and_data_source, path)
	{
		_data_source = owner_and_data_source;
		inc_source_use_count();
		ModuleIO::ProcessingLoopAPI (*this->io()).register_output_property (*this);
	}


template<class V>
	inline
	PropertyOut<V>::~PropertyOut()
	{
		if (this->io())
			ModuleIO::ProcessingLoopAPI (*this->io()).unregister_output_property (*this);
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
	PropertyOut<V>::operator<< (Nil)
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
				property_source->inc_use_count();
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
				property_source->dec_use_count();
			}
		}, _data_source);
	}

} // namespace xf

#endif

