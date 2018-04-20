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

#ifndef XEFIS__UTILITY__REGISTRATION_PROOF_H__INCLUDED
#define XEFIS__UTILITY__REGISTRATION_PROOF_H__INCLUDED

// Standard:
#include <cstddef>
#include <variant>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

template<class Registrant, class Details>
	class Registry;


/**
 * An object that "unregisters" thing from another thing when it gets deleted.
 * Returned by various "register" methods. The receiver should hold this object
 * as long as it wants to be "registered".
 */
template<class pRegistrant, class pDetails = std::monostate>
	class RegistrationProof: private Noncopyable
	{
		template<class R, class D>
			friend class Registry;

	  public:
		using Registrant	= pRegistrant;
		using Details		= pDetails;
		using Registry		= xf::Registry<Registrant, Details>;
		using Disclosure	= typename Registry::Disclosure;

	  private:
		// Ctor
		explicit
		RegistrationProof (Registrant&, Details, Registry&);

	  public:
		// Ctor
		explicit
		RegistrationProof() = default;

		// Ctor
		explicit
		RegistrationProof (RegistrationProof&&);

		// Dtor
		~RegistrationProof();

		// Move-assignment operator
		RegistrationProof&
		operator= (RegistrationProof&&);

		/**
		 * Return true if RegistrationProof is valid and registrant object
		 * can be accessed.
		 */
		operator bool() const noexcept;

		Registrant&
		registrant() noexcept;

		Registrant const&
		registrant() const noexcept;

		void
		reset();

	  private:
		std::unique_ptr<Disclosure> _disclosure;
	};


template<class R, class D>
	inline
	RegistrationProof<R, D>::RegistrationProof (Registrant& registrant, Details details, Registry& registry):
		_disclosure (std::make_unique<Disclosure> (registrant, details, registry._shared_data))
	{
		if (auto ptr = _disclosure->_registry_data.lock())
			ptr->insert (*_disclosure.get());
	}


template<class R, class D>
	inline
	RegistrationProof<R, D>::RegistrationProof (RegistrationProof&& other):
		_disclosure (std::move (other._disclosure))
	{ }


template<class R, class D>
	inline
	RegistrationProof<R, D>::~RegistrationProof()
	{
		if (_disclosure)
			if (auto ptr = _disclosure->_registry_data.lock())
				ptr->remove (*_disclosure.get());
	}


template<class R, class D>
	inline RegistrationProof<R, D>&
	RegistrationProof<R, D>::operator= (RegistrationProof&& other)
	{
		_disclosure = std::move (other._disclosure);
		return *this;
	}


template<class R, class D>
	inline void
	RegistrationProof<R, D>::reset()
	{
		_disclosure.reset();
	}


template<class R, class D>
	inline
	RegistrationProof<R, D>::operator bool() const noexcept
	{
		return _disclosure;
	}


template<class R, class D>
	inline auto
	RegistrationProof<R, D>::registrant() noexcept
		-> Registrant&
	{
		return _disclosure->registrant;
	}


template<class R, class D>
	inline auto
	RegistrationProof<R, D>::registrant() const noexcept
		-> Registrant const&
	{
		return _disclosure->registrant;
	}

} // namespace xf

#endif

