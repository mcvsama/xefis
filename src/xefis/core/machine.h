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

#ifndef XEFIS__CORE__MACHINE_H__INCLUDED
#define XEFIS__CORE__MACHINE_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/processing_loop.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/sequence.h>
#include <xefis/utility/tracker.h>


namespace xf {

class Machine: private Noncopyable
{
  private:
	using ProcessingLoopsTracker = Tracker<ProcessingLoop>;

  public:
	// Ctor
	explicit
	Machine (Xefis*);

	/**
	 * Dtor
	 * Has to be virtual since we'll possibly refer to machines thorugh base class
	 * (that is Machine).
	 */
	virtual
	~Machine() = default;

	/**
	 * Return main Xefis object.
	 */
	Xefis*
	xefis() const noexcept;

	/**
	 * A sequence of processing loops.
	 */
	Sequence<ProcessingLoopsTracker::Iterator>
	processing_loops() noexcept;

	/**
	 * A sequence of processing loops.
	 */
	Sequence<ProcessingLoopsTracker::ConstIterator>
	processing_loops() const noexcept;

	/**
	 * Register a processing loop.
	 */
	template<class Compatible>
		void
		register_processing_loop (Registrant<Compatible>&);

  private:
	Xefis*						_xefis;
	Tracker<ProcessingLoop>		_processing_loops;
};


inline Xefis*
Machine::xefis() const noexcept
{
	return _xefis;
}


inline auto
Machine::processing_loops() noexcept -> Sequence<ProcessingLoopsTracker::Iterator>
{
	return { _processing_loops.begin(), _processing_loops.end() };
}


inline auto
Machine::processing_loops() const noexcept -> Sequence<ProcessingLoopsTracker::ConstIterator>
{
	return { _processing_loops.cbegin(), _processing_loops.cend() };
}


template<class Compatible>
	inline void
	Machine::register_processing_loop (Registrant<Compatible>& processing_loop)
	{
		// Don't give access to public to Registry interface, so that user doesn't
		// register instrument with some weird Details value.
		_processing_loops.register_object (processing_loop);
	}

} // namespace xf

#endif

