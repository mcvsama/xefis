/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__INSTRUMENTS__EFIS_H__INCLUDED
#define XEFIS__INSTRUMENTS__EFIS_H__INCLUDED

// Qt:
#include <QtGui/QWidget>
#include <QtNetwork/QUdpSocket>

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <widgets/efis_widget.h>


class EFIS: public QWidget
{
	Q_OBJECT

  public:
	EFIS (QWidget* parent);

	/**
	 * Show input alert when data is not coming for given period of time.
	 * Pass 0.f to disable. Default is 110 ms.
	 */
	void
	set_input_alert_timeout (Seconds timeout);

  private slots:
	/**
	 * Read and apply FlightGear datagrams from UDP socket.
	 */
	void
	read_input();

	/**
	 * Show input alert (when there's no incoming
	 * data from external source).
	 */
	void
	input_timeout();

	/**
	 * Hide input alert.
	 */
	void
	input_ok();

  private:
	/**
	 * Hide all indicators.
	 */
	void
	hide_all();

  private:
	EFISWidget*	_efis_widget				= nullptr;
	Seconds		_input_alert_timeout		= 0.0f;
	QTimer*		_input_alert_timer			= nullptr;
	QTimer*		_input_alert_hide_timer		= nullptr;
	bool		_show_input_alert			= false;
	QUdpSocket*	_input						= nullptr;
};

#endif
