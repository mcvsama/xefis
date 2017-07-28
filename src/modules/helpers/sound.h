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

#ifndef XEFIS__MODULES__IO__SOUND_H__INCLUDED
#define XEFIS__MODULES__IO__SOUND_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/property.h>
#include <xefis/support/ui/sound_manager.h>


/**
 * Play various sounds when corresponding boolean properties become true.
 */
class Sound:
	public QObject,
	public v1::Module
{
	Q_OBJECT

	class Alarm
	{
	  public:
		struct Compare
		{
			/**
			 * Comparison function.
			 */
			bool
			operator() (Unique<Alarm> const& a, Unique<Alarm> const& b)
			{
				return a->priority() > b->priority();
			}
		};

	  public:
		Alarm (QDomElement const&, xf::SoundManager*);

		/**
		 * Sort by priority: highest first.
		 */
		bool
		operator< (Alarm const& other) const noexcept;

		/**
		 * Return alarm priority.
		 */
		int32_t
		priority() const noexcept;

		/**
		 * Check if sound should be played.
		 * If so, start the sound and return true.
		 * Otherwise mute sound and return false.
		 */
		bool
		check();

		/**
		 * Stop the sound immediately.
		 */
		void
		stop();

	  private:
		QString							_sound_file_path;
		v1::PropertyBoolean				_property;
		Weak<xf::SoundManager::Sound>	_sound;
		int32_t							_priority;
		bool							_repeat;
		Time							_repeat_period;
		xf::SoundManager*				_sound_manager;
		bool							_was_started = false;
		std::optional<Time>				_finished_timestamp;
	};

	typedef std::set<Unique<Alarm>, Alarm::Compare> Alarms;

	class Group
	{
	  public:
		Group (QDomElement const&, xf::SoundManager*);

		/**
		 * Run test to see if any alarms need to be fired.
		 */
		void
		check();

	  private:
		Alarms	_alarms;
	};

	typedef std::set<Unique<Group>> Groups;

  public:
	// Ctor
	Sound (v1::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	Unique<QTimer>	_check_repeats_timer;
	Groups			_groups;
};


inline int32_t
Sound::Alarm::priority() const noexcept
{
	return _priority;
}

#endif
