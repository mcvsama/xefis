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
#include <xefis/utility/sequence.h>


namespace xf {

class Machine
{
  private:
	using ProcessingLoopsContainer = std::vector<std::unique_ptr<ProcessingLoop>>;

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
	Sequence<ProcessingLoopsContainer::iterator>
	processing_loops() noexcept;

	/**
	 * A sequence of processing loops.
	 */
	Sequence<ProcessingLoopsContainer::const_iterator>
	processing_loops() const noexcept;

  protected:
	/**
	 * Create new processing loop with given frequency.
	 */
	template<class pProcessingLoop = ProcessingLoop, class ...Arg>
		pProcessingLoop*
		make_processing_loop (Arg&& ...args);

  private:
	Xefis*						_xefis;
	ProcessingLoopsContainer	_processing_loops;
};


template<class pProcessingLoop, class ...Arg>
	inline pProcessingLoop*
	Machine::make_processing_loop (Arg&& ...args)
	{
		_processing_loops.push_back (std::make_unique<pProcessingLoop> (this, std::forward<Arg> (args)...));

		return static_cast<pProcessingLoop*> (_processing_loops.back().get());
	}


inline Xefis*
Machine::xefis() const noexcept
{
	return _xefis;
}


inline auto
Machine::processing_loops() noexcept -> Sequence<ProcessingLoopsContainer::iterator>
{
	return { _processing_loops.begin(), _processing_loops.end() };
}


inline auto
Machine::processing_loops() const noexcept -> Sequence<ProcessingLoopsContainer::const_iterator>
{
	return { _processing_loops.cbegin(), _processing_loops.cend() };
}

} // namespace xf

#endif

