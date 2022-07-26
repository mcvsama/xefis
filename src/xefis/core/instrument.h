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

#ifndef XEFIS__CORE__INSTRUMENT_H__INCLUDED
#define XEFIS__CORE__INSTRUMENT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/paint_request.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Lib:
#include <boost/circular_buffer.hpp>

// Standard:
#include <cstddef>
#include <atomic>
#include <future>
#include <string>
#include <type_traits>


namespace xf {

class Screen;

class Instrument: public Module
{
	static constexpr std::size_t kMaxPaintingTimesBackLog = 1000;

  public:
	/**
	 * Accesses accounting data (time spent on processing, etc.
	 */
	class AccountingAPI
	{
	  public:
		// Ctor
		explicit
		AccountingAPI (Instrument&);

		/**
		 * Frame time of the Screen that this instrument is being painted on.
		 */
		[[nodiscard]]
		si::Time
		frame_time() const noexcept;

		/**
		 * Set frame time of the Screen that this instrument is being painted on.
		 */
		void
		set_frame_time (si::Time);

		/**
		 * Add new measured painting time (time spent in the paint() method).
		 */
		void
		add_painting_time (si::Time);

		/**
		 * Painting times buffer.
		 */
		[[nodiscard]]
		boost::circular_buffer<si::Time> const&
		painting_times() const noexcept;

	  private:
		Instrument& _instrument;
	};

  public:
	using Module::Module;

	// Dtor
	virtual
	~Instrument() = default;

	/**
	 * Paint the instrument onto given canvas.
	 */
	virtual std::packaged_task<void()>
	paint (PaintRequest) const = 0;

	/**
	 * Return true if instrument wants to be repainted.
	 * Also unmark the instrument as dirty atomically.
	 */
	[[nodiscard]]
	bool
	dirty_since_last_check() noexcept;

	/**
	 * Mark instrument as dirty (to be repainted).
	 */
	void
	mark_dirty() noexcept;

  private:
	std::atomic<bool>					_dirty			{ true };
	boost::circular_buffer<si::Time>	_painting_times	{ kMaxPaintingTimesBackLog };
	si::Time							_frame_time		{ 0_s };
};


inline
Instrument::AccountingAPI::AccountingAPI (Instrument& instrument):
	_instrument (instrument)
{ }


inline si::Time
Instrument::AccountingAPI::frame_time() const noexcept
{
	return _instrument._frame_time;
}


inline void
Instrument::AccountingAPI::set_frame_time (si::Time frame_time)
{
	_instrument._frame_time = frame_time;
}


inline void
Instrument::AccountingAPI::add_painting_time (si::Time time)
{
	_instrument._painting_times.push_back (time);
}


inline boost::circular_buffer<si::Time> const&
Instrument::AccountingAPI::painting_times() const noexcept
{
	return _instrument._painting_times;
}


inline bool
Instrument::dirty_since_last_check() noexcept
{
	return _dirty.exchange (false);
}


inline void
Instrument::mark_dirty() noexcept
{
	_dirty.store (true);
}

} // namespace xf

#endif

