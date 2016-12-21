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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "sound_manager.h"


namespace xf {

SoundManager::Sound::Sound (QString const& wav_file_name)
{
	auto exit = [&](int, QProcess::ExitStatus) {
		_finished = true;
	};

	_play_process = std::make_unique<QProcess>();
	QObject::connect (_play_process.get(),
					  static_cast<void (QProcess::*)(int, QProcess::ExitStatus)> (&QProcess::finished),
					  exit);
	_play_process->start ("aplay", { wav_file_name });
}


SoundManager::Sound::~Sound()
{
	stop();
}


void
SoundManager::Sound::stop()
{
	_play_process->terminate();
}


SoundManager::SoundManager()
{
	_logger.set_prefix ("<sound manager>");
	_logger << "Creating SoundManager" << std::endl;
}


SoundManager::~SoundManager()
{
	_logger << "Destroying SoundManager" << std::endl;
}


Shared<SoundManager::Sound>
SoundManager::play (QString const& wav_file_name)
{
	cleanup();
	return *_sounds.insert (std::make_shared<Sound> (wav_file_name)).first;
}


void
SoundManager::cleanup()
{
	for (auto s = _sounds.begin(); s != _sounds.end(); )
		if ((*s)->finished())
			_sounds.erase (s++);
		else
			++s;
}

} // namespace xf

