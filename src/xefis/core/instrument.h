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

// Standard:
#include <cstddef>
#include <atomic>
#include <future>
#include <string>
#include <type_traits>

// Lib:
#include <boost/circular_buffer.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/paint_request.h>
#include <xefis/utility/noncopyable.h>


namespace xf {

class Screen;

class BasicInstrument: public BasicModule
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
		AccountingAPI (BasicInstrument&);

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
		BasicInstrument& _instrument;
	};

  public:
	using BasicModule::BasicModule;

	// Dtor
	virtual
	~BasicInstrument() = default;

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


template<class IO = ModuleIO>
	class Instrument: public BasicInstrument
	{
	  public:
		/**
		 * Ctor
		 * Version for modules that do have their own IO class.
		 */
		template<class = std::enable_if_t<!std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::unique_ptr<IO> io, std::string_view const& instance = {});

		/**
		 * Ctor
		 * Version for modules that do not have any IO class.
		 */
		template<class = std::enable_if_t<std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::string_view const& instance = {});

	  protected:
		IO& io;
	};


inline
BasicInstrument::AccountingAPI::AccountingAPI (BasicInstrument& instrument):
	_instrument (instrument)
{ }


inline si::Time
BasicInstrument::AccountingAPI::frame_time() const noexcept
{
	return _instrument._frame_time;
}


inline void
BasicInstrument::AccountingAPI::set_frame_time (si::Time frame_time)
{
	_instrument._frame_time = frame_time;
}


inline void
BasicInstrument::AccountingAPI::add_painting_time (si::Time time)
{
	_instrument._painting_times.push_back (time);
}


inline boost::circular_buffer<si::Time> const&
BasicInstrument::AccountingAPI::painting_times() const noexcept
{
	return _instrument._painting_times;
}


inline bool
BasicInstrument::dirty_since_last_check() noexcept
{
	return _dirty.exchange (false);
}


inline void
BasicInstrument::mark_dirty() noexcept
{
	_dirty.store (true);
}


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::unique_ptr<IO> module_io, std::string_view const& instance):
			BasicInstrument (std::move (module_io), instance),
			io (static_cast<IO&> (*io_base()))
		{ }


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::string_view const& instance):
			BasicInstrument (std::make_unique<IO>(), instance),
			io (static_cast<IO&> (*io_base()))
		{ }

} // namespace xf

#endif

