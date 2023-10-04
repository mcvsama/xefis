/* vim:ts=4
 *
 * Copyleft 2012…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "input_link.h"


using namespace neutrino::si::literals;


InputLink::InputLink (std::unique_ptr<LinkProtocol> protocol, InputLinkParams const& params, xf::Logger const& logger, std::string_view const& instance):
	Module (instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	_protocol (std::move (protocol)),
	_params (params)
{
	_input_blob.reserve (2 * _protocol->size());

	if (_params.failsafe_after)
	{
		_failsafe_timer = std::make_unique<QTimer>();
		_failsafe_timer->setSingleShot (true);
		_failsafe_timer->setInterval (_params.failsafe_after->in<si::Millisecond>());
		QObject::connect (_failsafe_timer.get(), &QTimer::timeout, [this] { failsafe(); });
	}

	if (_params.reacquire_after)
	{
		_reacquire_timer = std::make_unique<QTimer>();
		_reacquire_timer->setSingleShot (true);
		_reacquire_timer->setInterval (_params.reacquire_after->in<si::Millisecond>());
		QObject::connect (_reacquire_timer.get(), &QTimer::timeout, [this] { reacquire(); });
	}
}


void
InputLink::process (xf::Cycle const& cycle)
{
	try {
		if (this->link_input && _input_changed.serial_changed())
		{
			_input_blob.insert (_input_blob.end(), this->link_input->begin(), this->link_input->end());
			auto e = _protocol->consume (_input_blob.begin(), _input_blob.end(), this, _reacquire_timer.get(), _failsafe_timer.get(), cycle.logger() + _logger);
			auto valid_bytes = std::distance (_input_blob.cbegin(), e);
			this->link_valid_bytes = this->link_valid_bytes.value_or (0) + valid_bytes;
			_input_blob.erase (_input_blob.begin(), e);
		}
	}
	catch (LinkProtocol::ParseError const&)
	{
		(cycle.logger() + _logger) << "Packet parse error. Couldn't synchronize." << std::endl;
	}
}


void
InputLink::failsafe()
{
	this->link_valid = false;
	this->link_failsafes = this->link_failsafes.value_or (0) + 1;
	_protocol->failsafe();
}


void
InputLink::reacquire()
{
	this->link_valid = true;
	this->link_reacquires = this->link_reacquires.value_or (0) + 1;
}

