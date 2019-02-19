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

#ifndef NEUTRINO__TRACKER_H__INCLUDED
#define NEUTRINO__TRACKER_H__INCLUDED

// Standard:
#include <algorithm>
#include <any>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/utility.h>


namespace neutrino {

class BasicRegistrant;

template<class V>
	class Registrant;


namespace detail {

struct TrackerInfo
{
	std::function<void (BasicRegistrant&)>	deregister;
	std::function<void (BasicRegistrant&)>	moved_to;
};

} // namespace detail


template<class pValue, class pDetails = std::monostate>
	class Tracker
	{
	  public:
		using Value					= pValue;
		using Details				= pDetails;

		class Disclosure
		{
			friend class Tracker;

		  public:
			// Ctor
			Disclosure (Value* value, Details&& details, BasicRegistrant& registrant):
				_value (value),
				_details (std::forward<Details> (details)),
				_registrant (&registrant)
			{ }

			// Move ctor
			Disclosure (Disclosure&&) = default;

			// Move operator
			Disclosure&
			operator= (Disclosure&&) = default;

			Value&
			value() noexcept
			{
				return *_value;
			}

			Value const&
			value() const noexcept
			{
				return *_value;
			}

			Details&
			details() noexcept
			{
				return _details;
			}

			Details const&
			details() const noexcept
			{
				return _details;
			}

			Registrant<Value>&
			registrant() noexcept
			{
				return static_cast<Registrant<Value>&> (*_registrant);
			}

			Registrant<Value> const&
			registrant() const noexcept
			{
				return static_cast<Registrant<Value>&> (*_registrant);
			}

		  private:
			Value*										_value;
			Details										_details;
			std::list<detail::TrackerInfo>::iterator	_tracker_info_iterator;
			BasicRegistrant*							_registrant;
		};

		using RegisteredCallback	= std::function<void (Disclosure&)>;
		using DeregisteredCallback	= std::function<void (Disclosure&)>;
		using Disclosures			= std::list<Disclosure>;
		using Iterator				= typename Disclosures::iterator;
		using ConstIterator			= typename Disclosures::const_iterator;

	  public:
		// Ctor
		explicit
		Tracker (RegisteredCallback = nullptr, DeregisteredCallback = nullptr);

		// Dtor
		~Tracker();

		template<class CompatibleValue>
			void
			register_object (Registrant<CompatibleValue>&, Details&& = Details());

		template<class CompatibleValue>
			void
			deregister_object (Registrant<CompatibleValue>&);

		bool
		empty() const noexcept;

		std::size_t
		size() const noexcept;

		Iterator
		begin();

		ConstIterator
		begin() const;

		ConstIterator
		cbegin() const;

		Iterator
		end();

		ConstIterator
		end() const;

		ConstIterator
		cend() const;

	  private:
		std::list<Disclosure>	_disclosures;
		RegisteredCallback		_object_registered;
		DeregisteredCallback	_object_deregistered;
	};


class BasicRegistrant
{
	template<class V>
		friend class neutrino::Tracker;

  protected:
	std::list<detail::TrackerInfo> _trackers;
};


template<class pValue>
	class Registrant:
		public BasicRegistrant,
		private Noncopyable
	{
		template<class V>
			friend class neutrino::Tracker;

	  public:
		using Value = pValue;

	  public:
		// Ctor
		Registrant (Value&&);

		// Ctor
		template<class ...Args>
			Registrant (Args&& ...);

		// Move ctor
		template<class CompatibleValue>
			Registrant (Registrant<CompatibleValue>&&);

		// Dtor
		~Registrant();

		// Copy operator
		template<class CompatibleValue>
			Registrant&
			operator= (Registrant<CompatibleValue>&&);

		Value&
		operator*() noexcept;

		Value const&
		operator*() const noexcept;

		Value*
		operator->() noexcept;

		Value const*
		operator->() const noexcept;

	  private:
		// Would use std::optional<>, but it cant't handle abstract types.
		std::unique_ptr<Value> _value;
	};


template<class V, class D>
	inline
	Tracker<V, D>::Tracker (RegisteredCallback object_registered, DeregisteredCallback object_deregistered):
		_object_registered (object_registered),
		_object_deregistered (object_deregistered)
	{ }


template<class V, class D>
	inline
	Tracker<V, D>::~Tracker()
	{
		// Do not iterate _disclosures as it will be modified in the loop, get only pointers to registrants:
		std::list<BasicRegistrant*> registrant_ptrs;

		for (auto& disclosure: _disclosures)
			registrant_ptrs.push_back (disclosure._registrant);

		// Deregister in reverse order:
		std::reverse (registrant_ptrs.begin(), registrant_ptrs.end());

		for (auto r_ptr: registrant_ptrs)
			deregister_object (static_cast<Registrant<Value>&> (*r_ptr));
	}


template<class V, class D>
	template<class CompatibleValue>
		inline void
		Tracker<V, D>::register_object (Registrant<CompatibleValue>& registrant, Details&& details)
		{
			// Perhaps it would be useful to have a Registrant that has a "unique" flag, which would cause
			// unregistering from any other trackers before registering to new one.
			// But for now it's not needed.

			// Make sure that new registrant isn't registered in this tracker:
			deregister_object (registrant);

			_disclosures.emplace_back (&*registrant, std::forward<Details> (details), registrant);
			auto* the_disclosure = &_disclosures.back();
			registrant._trackers.push_back (detail::TrackerInfo());
			auto tracker_info = std::prev (registrant._trackers.end());
			the_disclosure->_tracker_info_iterator = tracker_info;

			tracker_info->deregister = [this] (BasicRegistrant& o) {
				this->deregister_object (static_cast<Registrant<Value>&> (o));
			};
			tracker_info->moved_to = [this, the_disclosure] (BasicRegistrant& basic_to) {
				auto& to = static_cast<Registrant<Value>&> (basic_to);
				the_disclosure->_value = &*to._value;
				the_disclosure->_registrant = &to;
			};

			if (_object_registered)
				_object_registered (*the_disclosure);
		}


template<class V, class D>
	template<class CompatibleValue>
		inline void
		Tracker<V, D>::deregister_object (Registrant<CompatibleValue>& registrant)
		{
			_disclosures.remove_if ([this, &registrant] (auto& disclosure) {
				if (disclosure._value == &*registrant)
				{
					if (_object_deregistered)
						_object_deregistered (disclosure);

					registrant._trackers.erase (disclosure._tracker_info_iterator);
					return true;
				}
				else
					return false;
			});
		}


template<class V, class D>
	inline bool
	Tracker<V, D>::empty() const noexcept
	{
		return _disclosures.empty();
	}


template<class V, class D>
	inline std::size_t
	Tracker<V, D>::size() const noexcept
	{
		return _disclosures.size();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::begin() -> Iterator
	{
		return _disclosures.begin();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::begin() const -> ConstIterator
	{
		return _disclosures.begin();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::cbegin() const -> ConstIterator
	{
		return _disclosures.cbegin();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::end() -> Iterator
	{
		return _disclosures.end();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::end() const -> ConstIterator
	{
		return _disclosures.end();
	}


template<class V, class D>
	inline auto
	Tracker<V, D>::cend() const -> ConstIterator
	{
		return _disclosures.cend();
	}


template<class V>
	inline
	Registrant<V>::Registrant (Value&& value):
		_value (std::forward<Value> (value))
	{ }


template<class V>
	template<class ...Args>
		inline
		Registrant<V>::Registrant (Args&& ...args):
			_value (std::make_unique<Value> (std::forward<Args> (args)...))
		{ }


template<class V>
	template<class CompatibleValue>
		inline
		Registrant<V>::Registrant (Registrant<CompatibleValue>&& other)
		{
			*this = std::forward<Registrant<CompatibleValue>> (other);
		}


template<class V>
	inline
	Registrant<V>::~Registrant()
	{
		// Deregistration modifies _trackers, so iterate over a copy:
		for (auto& t: clone (_trackers))
			if (t.deregister)
				t.deregister (*this);
	}


template<class V>
	template<class CompatibleValue>
		inline
		Registrant<V>&
		Registrant<V>::operator= (Registrant<CompatibleValue>&& other)
		{
			_trackers = std::move (other._trackers);
			_value = std::move (other._value);

			for (auto& t: _trackers)
				if (t.moved_to)
					t.moved_to (*this);

			return *this;
		}


template<class V>
	inline auto
	Registrant<V>::operator*() noexcept -> Value&
	{
		return *_value;
	}


template<class V>
	inline auto
	Registrant<V>::operator*() const noexcept -> Value const&
	{
		return *_value;
	}


template<class V>
	inline auto
	Registrant<V>::operator->() noexcept -> Value*
	{
		return &*_value;
	}


template<class V>
	inline auto
	Registrant<V>::operator->() const noexcept -> Value const*
	{
		return &*_value;
	}

} // namespace neutrino

#endif

