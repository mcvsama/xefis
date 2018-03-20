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

	  private:
		class Disclosure
		{
			friend class RegistrationProof;

		  public:
			explicit
			Disclosure (Registrant& registrant, Details& details, std::weak_ptr<typename Registry::SharedData> registry_data):
				_registrant (registrant),
				_details (details),
				_registry_data (registry_data)
			{ }

			Registrant&
			registrant() const
			{
				return _registrant;
			}

			Details&
			details()
			{
				return _details;
			}

		  private:
			Registrant&										_registrant;
			Details											_details;
			std::weak_ptr<typename Registry::SharedData>	_registry_data;
		};

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
		std::unique_ptr<Disclosure> _unique_data;
	};


template<class R, class D>
	inline
	RegistrationProof<R, D>::RegistrationProof (Registrant& registrant, Details details, Registry& registry):
		_unique_data (std::make_unique<Disclosure> (registrant, details, registry._shared_data))
	{
		if (auto ptr = _unique_data->_registry_data.lock())
			ptr->insert (*_unique_data.get());
	}


template<class R, class D>
	inline
	RegistrationProof<R, D>::RegistrationProof (RegistrationProof&& other):
		_unique_data (std::move (other._unique_data))
	{ }


template<class R, class D>
	inline
	RegistrationProof<R, D>::~RegistrationProof()
	{
		if (_unique_data)
			if (auto ptr = _unique_data->_registry_data.lock())
				ptr->remove (*_unique_data.get());
	}


template<class R, class D>
	inline RegistrationProof<R, D>&
	RegistrationProof<R, D>::operator= (RegistrationProof&& other)
	{
		_unique_data = std::move (other._unique_data);
		return *this;
	}


template<class R, class D>
	inline void
	RegistrationProof<R, D>::reset()
	{
		_unique_data.reset();
	}


template<class R, class D>
	inline
	RegistrationProof<R, D>::operator bool() const noexcept
	{
		return _unique_data;
	}


template<class R, class D>
	inline auto
	RegistrationProof<R, D>::registrant() noexcept
		-> Registrant&
	{
		return _unique_data->registrant;
	}


template<class R, class D>
	inline auto
	RegistrationProof<R, D>::registrant() const noexcept
		-> Registrant const&
	{
		return _unique_data->registrant;
	}

} // namespace xf

#endif

