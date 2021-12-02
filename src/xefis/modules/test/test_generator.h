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

#ifndef XEFIS__MODULES__TEST__TEST_GENERATOR_H__INCLUDED
#define XEFIS__MODULES__TEST__TEST_GENERATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

// Neutrino:
#include <neutrino/range.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/sockets/module_socket.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


template<class Value>
	concept UsefulWithRange = requires (Value const& v) {
		xf::Range<Value> (v, v);
		v / 1_s;
	};


class TestNilCondition
{
  public:
	si::Time	nil		{ 0_s };
	si::Time	not_nil	{ 1_s };
};


class TestGeneratorIO: public xf::ModuleIO
{
  public:
	template<class Value>
		using RateOfChange = decltype (std::declval<Value>() / 1_s);

	template<class Value>
		using EnumTuple = std::tuple<std::variant<Value, xf::Nil>, si::Time>;

	enum class BorderCondition
	{
		Reset,
		Periodic,
		Mirroring,
	};

	class SocketGenerator
	{
	  public:
		// Ctor
		explicit
		SocketGenerator (TestNilCondition const nil_condition):
			_nil_condition (nil_condition)
		{ }

		// Dtor
		virtual
		~SocketGenerator() = default;

		virtual void
		update (si::Time update_dt) = 0;

		void
		perhaps_set_to_nil (xf::BasicAssignableSocket& socket, si::Time const dt);

	  private:
		TestNilCondition const	_nil_condition;
		si::Time				_time_left		{ _nil_condition.not_nil };
		bool					_is_nil_now		{ false };
	};

  public:
	/**
	 * Create and manage new output socket for sockets that can be used
	 * with xf::Range<>.
	 */
	template<UsefulWithRange Value>
		xf::ModuleOut<Value>&
		create_socket (std::string_view const& identifier,
					   Value initial_value,
					   xf::Range<Value> value_range,
					   RateOfChange<Value> rate_of_change,
					   BorderCondition border_condition = BorderCondition::Mirroring,
					   TestNilCondition const nil_condition = {});

	/**
	 * Create a socket that enumerates all listed values for some period of time.
	 */
	template<class Value>
		xf::ModuleOut<Value>&
		create_enum_socket (std::string_view const& identifier,
							std::vector<EnumTuple<Value>> const& values_and_intervals,
							TestNilCondition const nil_condition = {});

	/**
	 * Update all generators.
	 */
	void
	update_all (si::Time update_dt);

  private:
	std::vector<std::unique_ptr<SocketGenerator>> _generators;
};


class TestGenerator: public xf::Module<TestGeneratorIO>
{
  public:
	// Ctor
	explicit
	TestGenerator (std::unique_ptr<TestGeneratorIO>, std::string_view const& instance = {});

  protected:
	// Module API
	void
	process (xf::Cycle const&) override;
};


inline void
TestGeneratorIO::SocketGenerator::perhaps_set_to_nil (xf::BasicAssignableSocket& socket, si::Time const dt)
{
	_time_left -= dt;

	if (_time_left < 0_s)
	{
		_is_nil_now = !_is_nil_now;
		_time_left = _is_nil_now ? _nil_condition.nil : _nil_condition.not_nil;

		if (_time_left == 0_s)
		{
			_is_nil_now = !_is_nil_now;
			_time_left = _is_nil_now ? _nil_condition.nil : _nil_condition.not_nil;
		}
	}

	if (_is_nil_now)
		socket = xf::nil;
}


template<UsefulWithRange Value>
	inline xf::ModuleOut<Value>&
	TestGeneratorIO::create_socket (std::string_view const& identifier,
									Value const initial_value,
									xf::Range<Value> const value_range,
									RateOfChange<Value> const rate_of_change,
									BorderCondition border_condition,
									TestNilCondition const nil_condition)
	{
		class RangeGenerator: public SocketGenerator
		{
		  public:
			xf::ModuleOut<Value> socket;

		  public:
			// Ctor
			RangeGenerator (TestGeneratorIO* io,
							std::string_view const& identifier,
							Value const initial_value,
							xf::Range<Value> const value_range,
							RateOfChange<Value> const rate_of_change,
							BorderCondition const border_condition,
							TestNilCondition const nil_condition):
				SocketGenerator (nil_condition),
				socket (io, identifier),
				_initial_value (initial_value),
				_current_value (initial_value),
				_value_range (value_range),
				_rate_of_change (rate_of_change),
				_border_condition (border_condition)
			{ }

		  public:
			void
			update (si::Time const update_dt) override
			{
				auto new_value = _current_value + update_dt * _rate_of_change;

				if (!_value_range.includes (new_value))
				{
					switch (_border_condition)
					{
						case BorderCondition::Reset:
							new_value = _initial_value;
							break;

						case BorderCondition::Periodic:
							if (new_value > _value_range.max())
								new_value = _value_range.min() + (new_value - _value_range.max());
							else
								new_value = _value_range.max() - (_value_range.min() - new_value);
							break;

						case BorderCondition::Mirroring:
							if (new_value > _value_range.max())
								new_value = 2.0 * _value_range.max() - new_value;
							else
								new_value = 2.0 * _value_range.min() - new_value;

							_rate_of_change = -_rate_of_change;
							break;
					}
				}

				_current_value = new_value;
				this->socket = _current_value;
				perhaps_set_to_nil (this->socket, update_dt);
			}

		  private:
			Value const				_initial_value;
			Value					_current_value;
			xf::Range<Value> const	_value_range;
			RateOfChange<Value>		_rate_of_change;
			BorderCondition const	_border_condition;
		};

		_generators.emplace_back (std::make_unique<RangeGenerator> (this, identifier, initial_value, value_range, rate_of_change, border_condition, nil_condition));
		return static_cast<RangeGenerator&> (*_generators.back()).socket;
	}


template<class Value>
	inline xf::ModuleOut<Value>&
	TestGeneratorIO::create_enum_socket (std::string_view const& identifier,
										 std::vector<EnumTuple<Value>> const& values_and_intervals,
										 TestNilCondition const nil_condition)
	{
		class EnumGenerator: public SocketGenerator
		{
		  public:
			xf::ModuleOut<Value> socket;

		  public:
			// Ctor
			EnumGenerator (TestGeneratorIO* io,
						   std::string_view const& identifier,
						   std::vector<EnumTuple<Value>> const& values_and_intervals,
						   TestNilCondition const nil_condition):
				SocketGenerator (nil_condition),
				socket (io, identifier),
				_values_and_intervals (values_and_intervals)
			{ }

		  public:
			void
			update (si::Time const update_dt) override
			{
				_last_change_timestamp += update_dt;

				if (_last_change_timestamp > std::get<1> (_values_and_intervals[_current_index]))
				{
					_current_index = (_current_index + 1) % _values_and_intervals.size();
					_last_change_timestamp = 0_s;
				}

				auto const& variant = std::get<0> (_values_and_intervals[_current_index]);

				std::visit (xf::overload {
					[&] (Value const& value) {
						this->socket = value;
					},
					[&] (xf::Nil) {
						this->socket = xf::nil;
					},
				}, variant);

				perhaps_set_to_nil (this->socket, update_dt);
			}

		  private:
			si::Time							_last_change_timestamp	{ 0_s };
			std::size_t							_current_index			{ 0 };
			std::vector<EnumTuple<Value>> const	_values_and_intervals;
		};

		_generators.emplace_back (std::make_unique<EnumGenerator> (this, identifier, values_and_intervals, nil_condition));
		return static_cast<EnumGenerator&> (*_generators.back()).socket;
	}

#endif
