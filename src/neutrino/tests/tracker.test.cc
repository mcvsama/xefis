/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <memory>

// Neutrino:
#include <neutrino/test/test.h>
#include <neutrino/tracker.h>


namespace neutrino::test {
namespace {

class BasicInstrument
{
  public:
	BasicInstrument (char c):
		_c (c)
	{ }

	char
	character() const
	{
		return _c;
	}

	virtual void
	abstract() = 0;

  private:
	char _c;
};


template<class T>
	class TypedInstrument: public BasicInstrument
	{
	  public:
		using BasicInstrument::BasicInstrument;
	};


class Instrument_A: public TypedInstrument<int>
{
  public:
	using TypedInstrument<int>::TypedInstrument;

	void
	abstract() override
	{ };
};


class Instrument_B: public TypedInstrument<bool>
{
  public:
	using TypedInstrument<bool>::TypedInstrument;

	void
	abstract() override
	{ };
};


class CallbackCounter
{
  public:
	std::function<void (Tracker<BasicInstrument>::Disclosure& disclosure)>
	get_callback()
	{
		return [this] (Tracker<BasicInstrument>::Disclosure& disclosure) {
			++count;
			order += disclosure.value().character();
		};
	}

  public:
	size_t		count { 0 };
	std::string	order;
};


RuntimeTest t1 ("Tracker calls callbacks", []{
	// Destroy tracker first:
	{
		CallbackCounter registrations;
		CallbackCounter deregistrations;

		auto tracker = std::make_unique<Tracker<BasicInstrument>> (registrations.get_callback(), deregistrations.get_callback());
		Registrant<Instrument_A> instrument_1 ('1');
		Registrant<Instrument_B> instrument_2 ('2');
		Registrant<Instrument_A> instrument_3 ('3');

		tracker->register_object (instrument_1);
		tracker->register_object (instrument_2);
		tracker->register_object (instrument_3);

		tracker.reset();

		test_asserts::verify ("there were 3 registration callbacks", registrations.count == 3);
		test_asserts::verify ("there were 3 deregistration callbacks", deregistrations.count == 3);
		std::reverse (deregistrations.order.begin(), deregistrations.order.end());
		test_asserts::verify ("deregistration order is reversed", registrations.order == deregistrations.order);
	}

	// Destroy registrants first:
	{
		CallbackCounter registrations;
		CallbackCounter deregistrations;

		Tracker<BasicInstrument> tracker (registrations.get_callback(), deregistrations.get_callback());
		auto instrument_1 = std::make_unique<Registrant<Instrument_B>> ('1');
		auto instrument_2 = std::make_unique<Registrant<Instrument_A>> ('2');
		auto instrument_3 = std::make_unique<Registrant<Instrument_B>> ('3');

		tracker.register_object (*instrument_1);
		tracker.register_object (*instrument_2);
		tracker.register_object (*instrument_3);

		instrument_1.reset();
		instrument_2.reset();
		instrument_3.reset();

		test_asserts::verify ("there were 3 registration callbacks", registrations.count == 3);
		test_asserts::verify ("there were 3 deregistration callbacks", deregistrations.count == 3);
		test_asserts::verify ("deregistration order is as specified", registrations.order == deregistrations.order);
	}
});


RuntimeTest t2 ("Tracker traversal", []{
	Tracker<BasicInstrument> tracker;
	Registrant<Instrument_B> instrument_1 ('1');
	Registrant<Instrument_A> instrument_2 ('2');
	Registrant<Instrument_B> instrument_3 ('3');

	tracker.register_object (instrument_1);
	tracker.register_object (instrument_2);
	tracker.register_object (instrument_3);

	std::string sum;

	for (auto& disclosure: tracker)
		sum += disclosure.value().character();

	test_asserts::verify ("traversal works in specified order", sum == "123");
});


RuntimeTest t3 ("Tracker Registrants are moveable", []{
	CallbackCounter registrations;
	CallbackCounter deregistrations;

	auto tracker = std::make_unique<Tracker<BasicInstrument>> (registrations.get_callback(), deregistrations.get_callback());
	Registrant<Instrument_A> instrument_1 ('1');
	Registrant<Instrument_B> instrument_2 ('2');

	tracker->register_object (instrument_1);
	tracker->register_object (instrument_2);

	registrations.count = 0;
	deregistrations.count = 0;

	auto instrument_3 = std::move (instrument_1);
	auto instrument_4 = std::move (instrument_3);
	auto instrument_5 = std::move (instrument_4);

	test_asserts::verify ("movement does not involve registration callbacks", registrations.count == 0);
	test_asserts::verify ("movement does not involve deregistration callbacks", deregistrations.count == 0);
	tracker.reset();
	test_asserts::verify ("deregistration works properly even after movement of Registrants", deregistrations.count == 2);
});


RuntimeTest t4 ("Tracker double registration", []{
	CallbackCounter registrations;
	CallbackCounter deregistrations;

	Tracker<BasicInstrument> tracker (registrations.get_callback(), deregistrations.get_callback());
	auto instrument_1 = std::make_unique<Registrant<Instrument_B>> ('1');
	auto instrument_2 = std::make_unique<Registrant<Instrument_A>> ('2');

	tracker.register_object (*instrument_1);
	tracker.register_object (*instrument_2);

	deregistrations.count = 0;
	registrations.count = 0;

	tracker.register_object (*instrument_1); // Double-registration here.

	test_asserts::verify ("there are 2 registered objects", tracker.size() == 2);
	test_asserts::verify ("deregistration callback was called during double-registration", deregistrations.count == 1);
	test_asserts::verify ("registration callback was called during double-registration", registrations.count == 1);
});


RuntimeTest t5 ("Tracker details are properly handled", []{
	Tracker<BasicInstrument, char> tracker;
	Registrant<Instrument_A> instrument_1 ('1');
	Registrant<Instrument_B> instrument_2 ('2');

	tracker.register_object (instrument_1, '1');
	tracker.register_object (instrument_2, '2');

	for (auto& disclosure: tracker)
		test_asserts::verify ("details is correct for registered object", disclosure.value().character() == disclosure.details());
});


RuntimeTest t6 ("Tracker re-registration doesn't unregister from previous Tracker", []{
	Tracker<BasicInstrument> tracker1;
	Tracker<BasicInstrument> tracker2;

	Registrant<Instrument_A> instrument_1 ('1');
	Registrant<Instrument_B> instrument_2 ('2');

	tracker1.register_object (instrument_1);
	tracker1.register_object (instrument_2);

	tracker2.register_object (instrument_1);
	tracker2.register_object (instrument_2);

	test_asserts::verify ("there are 2 registered objects in tracker1", tracker1.size() == 2);
	test_asserts::verify ("there are 2 registered objects in tracker2", tracker2.size() == 2);
});

} // namespace
} // namespace neutrino::test

