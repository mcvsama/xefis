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

#ifndef XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED
#define XEFIS__CORE__V2__INSTRUMENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <type_traits>

// Qt:
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/services.h>


namespace v2 {
using namespace xf; // XXX

template<class IO = ModuleIO>
	class Instrument:
		public Module<IO>,
		public QWidget // XXX will not be a widget in the future - instead it will draw on a canvas.
	{
	  public:
		/**
		 * Ctor
		 * Version for modules that do have their own IO class.
		 */
		template<class = std::enable_if_t<!std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::unique_ptr<IO> io, std::string const& instance = {});

		/**
		 * Ctor
		 * Version for modules that do not have any IO class.
		 */
		template<class = std::enable_if_t<std::is_same_v<IO, ModuleIO>>>
			explicit
			Instrument (std::string const& instance = {});

	  private:
		void
		configure();
	};


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::unique_ptr<IO> io, std::string const& instance):
			Module<IO> (std::move (io), instance),
			QWidget (nullptr)
		{
			configure();
		}


template<class IO>
	template<class>
		inline
		Instrument<IO>::Instrument (std::string const& instance):
			Module<IO> (instance),
			QWidget (nullptr)
		{
			configure();
		}


template<class IO>
	inline void
	Instrument<IO>::configure()
	{
		setFont (xf::Services::instrument_font());
		setCursor (QCursor (Qt::CrossCursor));
	}

} // namespace v2

#endif

