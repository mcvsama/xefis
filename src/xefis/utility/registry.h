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

#ifndef XEFIS__UTILITY__REGISTRY_H__INCLUDED
#define XEFIS__UTILITY__REGISTRY_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <algorithm>
#include <variant>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "registration_proof.h"


namespace xf {

template<class pRegistrant, class pDetails = std::monostate>
	class Registry
	{
	  public:
		class Disclosure;

		using Registrant			= pRegistrant;
		using Details				= pDetails;
		using RegistrationProof		= xf::RegistrationProof<Registrant, Details>;
		using RegisteredCallback	= std::function<void (Disclosure&)>;
		using UnregisteredCallback	= std::function<void (Disclosure&)>;

		class Disclosure
		{
			template<class R, class D>
				friend class xf::RegistrationProof;

		  public:
			explicit
			Disclosure (Registrant& registrant, Details& details, std::weak_ptr<typename Registry::SharedData> registry_data):
				_registrant (registrant),
				_details (details),
				_registry_data (registry_data)
			{ }

			Registrant&
			registrant() noexcept
			{
				return _registrant;
			}

			Registrant const&
			registrant() const noexcept
			{
				return _registrant;
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

		  private:
			Registrant&										_registrant;
			Details											_details;
			std::weak_ptr<typename Registry::SharedData>	_registry_data;
		};

	  private:
		using DisclosuresVector		= std::vector<Disclosure*>;

		template<class R, class D>
			friend class xf::RegistrationProof;

		struct SharedData
		{
			DisclosuresVector disclosures;

			// Ctor
			explicit
			SharedData (RegisteredCallback, UnregisteredCallback);

			void
			insert (Disclosure&);

			void
			remove (Disclosure&);

		  private:
			RegisteredCallback			_registered_callback;
			UnregisteredCallback		_unregistered_callback;
		};

	  public:
		/**
		 * Allows iteration over registered objects of this Registry.
		 * It's a wrapper for DisclosuresVector::iterator which gives Registrant&
		 * when dereferenced, instead of Disclosure&.
		 */
		template<class pWrappedIterator>
			class IteratorWrapper
			{
				using WrappedIterator = pWrappedIterator;

			  public:
				// Ctor
				explicit
				IteratorWrapper (WrappedIterator);

				// Ctor
				explicit
				IteratorWrapper (IteratorWrapper const&);

				IteratorWrapper&
				operator= (IteratorWrapper const&);

				bool
				operator== (IteratorWrapper const&) const noexcept;

				bool
				operator!= (IteratorWrapper const&) const noexcept;

				IteratorWrapper&
				operator++();

				Disclosure&
				operator*();

				Disclosure const&
				operator*() const;

			  private:
				WrappedIterator _disclosure_iterator;
			};

		using Iterator		= IteratorWrapper<typename DisclosuresVector::iterator>;
		using ConstIterator	= IteratorWrapper<typename DisclosuresVector::const_iterator>;

	  public:
		// Ctor
		explicit
		Registry();

		// Ctor
		explicit
		Registry (RegisteredCallback, UnregisteredCallback);

		/**
		 * Register given object in this registry.
		 * Return a registration proof.
		 */
		RegistrationProof
		register_object (Registrant&);

		/**
		 * Register given object in this registry.
		 * Return a registration proof.
		 */
		RegistrationProof
		register_object (Registrant&, Details);

		/**
		 * Return iterator pointing to the first element of registry.
		 *
		 * Iterators get invalidated when any new registration or deregistration
		 * happens.
		 */
		Iterator
		begin();

		/**
		 * Const-version of begin().
		 */
		ConstIterator
		begin() const;

		/**
		 * Const-version of begin().
		 */
		ConstIterator
		cbegin() const;

		/**
		 * Return iterator pointing to the after-the-last element of registry.
		 */
		Iterator
		end();

		/**
		 * Const-version of end().
		 */
		ConstIterator
		end() const;

		/**
		 * Const-version of end().
		 */
		ConstIterator
		cend() const;

	  private:
		std::shared_ptr<SharedData> _shared_data;
	};


template<class R, class D>
	template<class W>
		inline
		Registry<R, D>::IteratorWrapper<W>::IteratorWrapper (WrappedIterator wit):
			_disclosure_iterator (wit)
		{ }


template<class R, class D>
	template<class W>
		inline
		Registry<R, D>::IteratorWrapper<W>::IteratorWrapper (IteratorWrapper const& other):
			_disclosure_iterator (other._disclosure_iterator)
		{ }


template<class R, class D>
	template<class W>
		inline auto
		Registry<R, D>::IteratorWrapper<W>::operator= (IteratorWrapper const& other)
			-> IteratorWrapper&
		{
			_disclosure_iterator = other._disclosure_iterator;
			return *this;
		}


template<class R, class D>
	template<class W>
		inline bool
		Registry<R, D>::IteratorWrapper<W>::operator== (IteratorWrapper const& other) const noexcept
		{
			return _disclosure_iterator == other._disclosure_iterator;
		}


template<class R, class D>
	template<class W>
		inline bool
		Registry<R, D>::IteratorWrapper<W>::operator!= (IteratorWrapper const& other) const noexcept
		{
			return !(*this == other);
		}


template<class R, class D>
	template<class W>
		inline auto
		Registry<R, D>::IteratorWrapper<W>::operator*()
			-> Disclosure&
		{
			return **_disclosure_iterator;
		}


template<class R, class D>
	template<class W>
		inline auto
		Registry<R, D>::IteratorWrapper<W>::operator*() const
			-> Disclosure const&
		{
			return **_disclosure_iterator;
		}


template<class R, class D>
	template<class W>
		inline auto
		Registry<R, D>::IteratorWrapper<W>::operator++()
			-> IteratorWrapper&
		{
			++_disclosure_iterator;
			return *this;
		}


template<class R, class D>
	inline
	Registry<R, D>::SharedData::SharedData (RegisteredCallback registered_callback, UnregisteredCallback unregistered_callback):
		_registered_callback (registered_callback),
		_unregistered_callback (unregistered_callback)
	{ }


template<class R, class D>
	inline void
	Registry<R, D>::SharedData::insert (Disclosure& disclosure)
	{
		disclosures.push_back (&disclosure);

		if (_registered_callback)
			_registered_callback (disclosure);
	}


template<class R, class D>
	inline void
	Registry<R, D>::SharedData::remove (Disclosure& disclosure)
	{
		if (_unregistered_callback)
			_unregistered_callback (disclosure);

		auto new_end = std::remove (disclosures.begin(), disclosures.end(), &disclosure);
		disclosures.resize (std::distance (disclosures.begin(), new_end));
	}


template<class R, class D>
	inline
	Registry<R, D>::Registry():
		_shared_data (std::make_shared<SharedData>())
	{ }


template<class R, class D>
	inline
	Registry<R, D>::Registry (RegisteredCallback registered_callback, UnregisteredCallback unregistered_callback):
		_shared_data (std::make_shared<SharedData> (registered_callback, unregistered_callback))
	{ }


template<class R, class D>
	inline auto
	Registry<R, D>::register_object (Registrant& registrant)
		-> RegistrationProof
	{
		return RegistrationProof (registrant, Details(), *this);
	}


template<class R, class D>
	inline auto
	Registry<R, D>::register_object (Registrant& registrant, Details details)
		-> RegistrationProof
	{
		return RegistrationProof (registrant, details, *this);
	}


template<class R, class D>
	inline auto
	Registry<R, D>::begin()
		-> Iterator
	{
		return Iterator (_shared_data->disclosures.begin());
	}


template<class R, class D>
	inline auto
	Registry<R, D>::begin() const
		-> ConstIterator
	{
		return ConstIterator (_shared_data->disclosures.cbegin());
	}


template<class R, class D>
	inline auto
	Registry<R, D>::cbegin() const
		-> ConstIterator
	{
		return ConstIterator (_shared_data->disclosures.cbegin());
	}


template<class R, class D>
	inline auto
	Registry<R, D>::end()
		-> Iterator
	{
		return Iterator (_shared_data->disclosures.end());
	}


template<class R, class D>
	inline auto
	Registry<R, D>::end() const
		-> ConstIterator
	{
		return ConstIterator (_shared_data->disclosures.cend());
	}


template<class R, class D>
	inline auto
	Registry<R, D>::cend() const
		-> ConstIterator
	{
		return ConstIterator (_shared_data->disclosures.cend());
	}

} // namespace xf

#endif

