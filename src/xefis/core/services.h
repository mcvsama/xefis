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

#ifndef XEFIS__CORE__SERVICES_H__INCLUDED
#define XEFIS__CORE__SERVICES_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/function.hpp>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtGui/QFont>
#include <QtWidgets/QApplication>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Private API, but can't put it inside of Services class,
 * because MOC won't be able to handle it.
 */
class CallOutDispatcher: public QObject
{
	Q_OBJECT

  private:
	/**
	 * Receive CallOut events.
	 */
	void
	customEvent (QEvent*) override;
};


/**
 * Common public services.
 */
class Services
{
  public:
	/**
	 * Allows calling out given function from within main Qt event queue.
	 * You can use boost::bind() as function callback.
	 */
	class CallOutEvent: public QEvent
	{
	  public:
		explicit
		CallOutEvent (boost::function<void()> callback);

		/**
		 * Cancel CallOut. Prevent calling callback from call_out() method.
		 * It's safe to call this method from the callback.
		 */
		void
		cancel();

		/**
		 * Call the callback, unless CallOut has been cancelled.
		 */
		void
		call_out();

	  private:
		bool					_cancelled;
		boost::function<void()> _callback;
	};

  public:
	/**
	 * Initialize services.
	 * Call AFTER initialization of QApplication.
	 */
	static void
	initialize();

	/**
	 * Deinit.
	 * Frees resources. Call it BEFORE deletion of QApplication.
	 */
	static void
	deinitialize();

	/**
	 * Return vector of compiled-in feature names.
	 */
	static std::vector<const char*>
	features();

	/**
	 * Register given callback to be called from within main Qt event queue.
	 * \return	CallOutEvent object. This object is deleted after call has been made.
	 */
	static CallOutEvent*
	call_out (boost::function<void()> callback);

	/**
	 * Return number of pixels per point on the screen. Takes into account
	 * screen DPI reported by the Qt.
	 */
	static float
	x_pixels_per_point (float x_dpi);

	/**
	 * Return number of pixels per point on the screen. Takes into account
	 * screen DPI reported by the Qt.
	 */
	static float
	y_pixels_per_point (float y_dpi);

	/**
	 * Return default font size in pixels.
	 */
	static float
	default_font_size (float y_dpi);

	/**
	 * Return font used for rendering instruments.
	 */
	static QFont
	instrument_font();

	/**
	 * Return font suitable for panels.
	 */
	static QFont
	panel_font();

  private:
	static Unique<CallOutDispatcher>	_call_out_dispatcher;
	static QFont						_instrument_font;
	static QFont						_panel_font;
};


inline
Services::CallOutEvent::CallOutEvent (boost::function<void()> callback):
	QEvent (QEvent::User),
	_cancelled (false),
	_callback (callback)
{ }


inline void
Services::CallOutEvent::cancel()
{
	_cancelled = true;
}


inline void
Services::CallOutEvent::call_out()
{
	if (!_cancelled)
		_callback();
}


inline float
Services::x_pixels_per_point (float dpi)
{
	return dpi / 72.0f;
}


inline float
Services::y_pixels_per_point (float dpi)
{
	return dpi / 72.0f;
}


inline float
Services::default_font_size (float y_dpi)
{
	QFont font = QApplication::font();
	return font.pointSize() * y_pixels_per_point (y_dpi);
}


inline QFont
Services::instrument_font()
{
	return _instrument_font;
}


inline QFont
Services::panel_font()
{
	return _panel_font;
}

} // namespace xf

#endif

