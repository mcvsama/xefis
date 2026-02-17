/* vim:ts=4
 *
 * Copyleft 2023  Michał Gawron
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
#include "link_decoder.h"


using namespace nu::si::literals;


LinkDecoder::LinkDecoder (xf::ProcessingLoop& loop, std::unique_ptr<LinkProtocol> protocol, Parameters const& params, nu::Logger const& logger, std::string_view const instance):
	Module (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
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
LinkDecoder::process (xf::Cycle const& cycle)
{
	try {
		if (this->encoded_input && _input_changed.serial_changed())
		{
			_input_blob.append (this->encoded_input->begin(), this->encoded_input->end());
			auto const consume_result = _protocol->consume (_input_blob.begin(), _input_blob.end(), cycle.logger() + _logger);

			auto consumed_bytes = std::distance (_input_blob.cbegin(), consume_result.parsing_end);
			this->link_received_bytes = this->link_received_bytes.value_or (0) + consumed_bytes;
			this->link_valid_bytes = this->link_valid_bytes.value_or (0) + consume_result.valid_bytes;
			this->link_valid_envelopes = this->link_valid_envelopes.value_or (0) + consume_result.valid_envelopes;
			this->link_error_bytes = this->link_error_bytes.value_or (0) + consume_result.error_bytes;

			if (this->link_valid.is_nil() && consume_result.valid_bytes > 0)
				this->link_valid = true;

			if (_reacquire_timer)
			{
				if (consume_result.error_bytes > 0)
					_reacquire_timer->stop();

				if (consume_result.valid_bytes > 0)
					if (!this->link_valid.value_or (false) && !_reacquire_timer->isActive())
						_reacquire_timer->start();
			}

			if (_failsafe_timer && consume_result.valid_envelopes > 0)
				_failsafe_timer->start();

			_input_blob.erase (_input_blob.begin(), consume_result.parsing_end);
		}
	}
	catch (LinkProtocol::ParseError const&)
	{
		(cycle.logger() + _logger) << "Packet parse error. Couldn't synchronize." << std::endl;
		_input_blob.clear();
	}
}


void
LinkDecoder::failsafe()
{
	this->link_valid = false;
	this->link_failsafes = this->link_failsafes.value_or (0) + 1;
	_protocol->failsafe();
}


void
LinkDecoder::reacquire()
{
	this->link_valid = true;
	this->link_reacquires = this->link_reacquires.value_or (0) + 1;
}

