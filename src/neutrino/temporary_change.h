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

#ifndef NEUTRINO__TEMPORARY_CHANGE_H__INCLUDED
#define NEUTRINO__TEMPORARY_CHANGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <utility>


namespace neutrino {

/**
 * Sets a new value to given object. Restores its original value
 * upon destruction.
 */
template<class pValue, class pNewValue>
	class TemporaryChange
	{
		using Value		= pValue;
		using NewValue	= pNewValue;

	  public:
		/**
		 * \param	value
		 *			Value to modify. Must live longer than this object.
		 */
		explicit
		TemporaryChange (Value& value, NewValue&& new_value);

		/**
		 * \param	value
		 *			Value to modify. Must live longer than this object.
		 */
		explicit
		TemporaryChange (Value& value, NewValue const& new_value);

		// Dtor
		~TemporaryChange();

		/**
		 * Explicitly restore original value.
		 */
		void
		restore();

	  private:
		bool	_modified	{ true };
		Value&	_value;
		Value	_original_value;
	};


template<class V, class N>
	inline
	TemporaryChange<V, N>::TemporaryChange (Value& value, NewValue&& new_value):
		_original_value (std::exchange (value, new_value))
	{ }


template<class V, class N>
	inline
	TemporaryChange<V, N>::TemporaryChange (Value& value, NewValue const& new_value):
		_original_value (std::exchange (value, new_value))
	{ }


template<class V, class N>
	inline
	TemporaryChange<V, N>::~TemporaryChange()
	{
		restore();
	}


template<class V, class N>
	inline void
	TemporaryChange<V, N>::restore()
	{
		if (_modified)
		{
			_value = std::move (_original_value);
			_modified = false;
		}
	}

} // namespace neutrino

#endif

