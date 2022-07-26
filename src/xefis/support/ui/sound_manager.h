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

#ifndef XEFIS__SUPPORT__UI__SOUND_MANAGER_H__INCLUDED
#define XEFIS__SUPPORT__UI__SOUND_MANAGER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QtCore/QProcess>

// Standard:
#include <cstddef>
#include <set>


namespace xf {

class SoundManager
{
  public:
	/**
	 * Represents sound process.
	 */
	class Sound
	{
	  public:
		// Ctor
		explicit
		Sound (QString const& wav_file_name);

		// Dtor
		~Sound();

		/**
		 * Return true if sound has finished playing.
		 */
		bool
		finished() const noexcept;

		/**
		 * Stop the sound.
		 */
		void
		stop();

	  private:
		std::unique_ptr<QProcess>	_play_process;
		bool						_finished = false;
	};

  public:
	// Ctor
	SoundManager (Logger const&);

	// Dtor
	~SoundManager();

	/**
	 * Play sound.
	 */
	std::shared_ptr<Sound>
	play (QString const& wav_file_name);

	/**
	 * Clean old finished sounds.
	 */
	void
	cleanup();

  private:
	Logger								_logger;
	std::set<std::shared_ptr<Sound>>	_sounds;
};


inline bool
SoundManager::Sound::finished() const noexcept
{
	return _finished;
}

} // namespace xf

#endif

