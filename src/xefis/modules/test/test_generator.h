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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/utility/range.h>


class TestGeneratorIO: public xf::ModuleIO
{
  public:
	template<class Value>
		using RateOfChange = decltype(std::declval<Value>() / 1_s);

	template<class Value>
		using EnumTuple = std::tuple<std::variant<Value, xf::Nil>, si::Time>;

	enum class BorderCondition
	{
		Reset,
		Periodic,
		Mirroring,
	};

	class PropertyGenerator
	{
	  public:
		// Dtor
		virtual
		~PropertyGenerator() = default;

		virtual void
		update (si::Time update_dt) = 0;
	};

  public:
	/**
	 * Create and manage new output property for properties that can be used
	 * with xf::Range<>.
	 */
	template<class Value,
			 class = std::void_t<decltype (xf::Range<Value>()),
								 RateOfChange<Value>>>
		xf::PropertyOut<Value>&
		create_property (std::string_view const& identifier,
						 Value initial_value,
						 xf::Range<Value> value_range,
						 RateOfChange<Value> rate_of_change,
						 BorderCondition border_condition = BorderCondition::Mirroring);

	/**
	 * Create a property that enumerates all listed values for some period of time.
	 */
	template<class Value>
		xf::PropertyOut<Value>&
		create_enum_property (std::string_view const& identifier,
							  std::vector<EnumTuple<Value>> const& values_and_intervals);

	/**
	 * Update all generators.
	 */
	void
	update_all (si::Time update_dt);

  private:
	std::vector<std::unique_ptr<PropertyGenerator>> _generators;
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


template<class Value, class>
	inline xf::PropertyOut<Value>&
	TestGeneratorIO::create_property (std::string_view const& identifier,
									  Value const initial_value,
									  xf::Range<Value> const value_range,
									  RateOfChange<Value> const rate_of_change,
									  BorderCondition border_condition)
	{
		class RangeGenerator: public PropertyGenerator
		{
		  public:
			xf::PropertyOut<Value> property;

		  public:
			// Ctor
			RangeGenerator (TestGeneratorIO* io,
							std::string_view const& identifier,
							Value const initial_value,
							xf::Range<Value> const value_range,
							RateOfChange<Value> const rate_of_change,
							BorderCondition const border_condition):
				property (io, identifier),
				_initial_value (initial_value),
				_value_range (value_range),
				_rate_of_change (rate_of_change),
				_border_condition (border_condition)
			{ }

		  public:
			void
			update (si::Time const update_dt) override
			{
				auto new_value = this->property.value_or (_initial_value) + update_dt * _rate_of_change;

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

				this->property = new_value;
			}

		  private:
			Value const					_initial_value;
			xf::Range<Value> const		_value_range;
			RateOfChange<Value>			_rate_of_change;
			BorderCondition const		_border_condition;
		};

		_generators.emplace_back (std::make_unique<RangeGenerator> (this, identifier, initial_value, value_range, rate_of_change, border_condition));
		return static_cast<RangeGenerator&> (*_generators.back()).property;
	}


template<class Value>
	inline xf::PropertyOut<Value>&
	TestGeneratorIO::create_enum_property (std::string_view const& identifier,
										   std::vector<EnumTuple<Value>> const& values_and_intervals)
	{
		class EnumGenerator: public PropertyGenerator
		{
		  public:
			xf::PropertyOut<Value> property;

		  public:
			// Ctor
			EnumGenerator (TestGeneratorIO* io,
						   std::string_view const& identifier,
						   std::vector<EnumTuple<Value>> const& values_and_intervals):
				property (io, identifier),
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
						this->property = value;
					},
					[&] (xf::Nil) {
						this->property = xf::nil;
					},
				}, variant);
			}

		  private:
			si::Time							_last_change_timestamp	{ 0_s };
			std::size_t							_current_index			{ 0 };
			std::vector<EnumTuple<Value>> const	_values_and_intervals;
		};

		_generators.emplace_back (std::make_unique<EnumGenerator> (this, identifier, values_and_intervals));
		return static_cast<EnumGenerator&> (*_generators.back()).property;
	}

#endif
