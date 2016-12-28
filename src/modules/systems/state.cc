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
#include <algorithm>
#include <functional>

// Qt:
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/blob.h>

// Local:
#include "state.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/state", State)


namespace {

State::ConfigVariable::ConfigVariable (QDomElement const& config)
{
	if (!config.hasAttribute ("id"))
		throw xf::MissingDomAttribute (config, "id");
	_id = config.attribute ("id");

	if (!config.hasAttribute ("type"))
		throw xf::MissingDomAttribute (config, "type");
	_type = xf::PropertyType (config.attribute ("type"));

	if (!config.hasAttribute ("path"))
		throw xf::MissingDomAttribute (config, "path");
	_path = xf::PropertyPath (config.attribute ("path"));

	xf::GenericProperty::create (_path, _type);
	_property.set_path (_path);

	if (config.hasAttribute ("default"))
		_property.parse_existing (config.attribute ("default").toStdString());
}


inline QString const&
State::ConfigVariable::id() const noexcept
{
	return _id;
}


inline void
State::ConfigVariable::set_id (QString const& id)
{
	_id = id;
}


inline xf::PropertyPath const&
State::ConfigVariable::path() const noexcept
{
	return _path;
}


inline void
State::ConfigVariable::set_path (xf::PropertyPath const& path)
{
	_path = path;
}


inline xf::PropertyType const&
State::ConfigVariable::type() const noexcept
{
	return _type;
}


inline void
State::ConfigVariable::set_type (xf::PropertyType const& type)
{
	_type = type;
}


inline xf::GenericProperty&
State::ConfigVariable::property() noexcept
{
	return _property;
}


inline xf::GenericProperty const&
State::ConfigVariable::property() const noexcept
{
	return _property;
}


inline bool
State::ConfigVariable::fresh() const noexcept
{
	return _property.fresh();
}


State::State (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (QDomElement const& e: config)
	{
		if (e == "state")
		{
			for (QDomElement const& v: e)
			{
				if (v == "variable")
				{
					ConfigVariable cv (v);
					_config_variables.insert ({ cv.id(), cv });
				}
				else
					throw xf::BadDomElement (v);
			}
		}
		else if (e != "settings")
			throw xf::BadDomElement (e);
	}

	parse_settings (config, {
		{ "file", _file_name, true },
		{ "max-save-delay", _max_save_delay, false },
	});

	_save_delay_timer = std::make_unique<QTimer>();
	_save_delay_timer->setInterval (_max_save_delay.quantity<Millisecond>());
	_save_delay_timer->setSingleShot (false);
	QObject::connect (_save_delay_timer.get(), &QTimer::timeout, std::bind (&State::try_saving, this));
	_save_delay_timer->start();

	load_state();
}


State::~State()
{
	save_state();
}


void
State::data_updated()
{
	// Pass
}


void
State::try_saving()
{
	if (std::any_of (_config_variables.begin(), _config_variables.end(), [](ConfigVariables::value_type const& cv) { return cv.second.fresh(); }))
	{
		if (_save_future.valid())
		{
			if (_save_future.wait_for (std::chrono::seconds (0)) == std::future_status::ready)
			{
				try {
					_save_future.get();
				}
				catch (xf::Exception const& e)
				{
					log() << "Encountered rrror when saving state: " << e.message() << std::endl;
				}

				save_state();
			}
		}
		else
			save_state();
	}
}


void
State::load_state()
{
	try {
		QDir cwd;
		QFile file (cwd.absolutePath() + '/' + _file_name);
		QDomDocument doc;

		if (!file.exists())
			throw xf::BadConfiguration ("file '" + file.fileName() + "' not found");

		if (!file.open (QFile::ReadOnly))
			throw xf::IOError ("couldn't open '" + file.fileName() + "' for read: " + file.errorString());

		if (!doc.setContent (&file, true))
			throw xf::BadConfiguration ("config parse error: " + file.fileName());

		if (doc.documentElement() == "xefis-mod-systems-state")
		{
			for (QDomElement const& e: doc.documentElement())
			{
				if (e == "state-variable")
				{
					QString id = e.attribute ("id");

					auto cv = _config_variables.find (id);
					if (cv != _config_variables.end())
					{
						xf::PropertyType type (e.attribute ("type"));
						if (type == cv->second.type())
						{
							try {
								cv->second.property().create_and_parse (xf::parse_hex_string (e.attribute ("value")));
							}
							catch (xf::Exception const& e)
							{
								log() << "Failed to load setting '" << id.toStdString() << "': " << e.message() << std::endl;
							}
						}
						else
							log() << "Type mismatch for setting '" << id.toStdString() << "': saved: "
								  << type.string() << ", configured: " << cv->second.type().string() << std::endl;
					}
					else
						log() << "Ignoring not configured setting '" << id.toStdString() << "'" << std::endl;
				}
				else
					log() << "Unknown element <" << e.tagName().toStdString() << ">";
			}
		}
	}
	catch (xf::Exception const& e)
	{
		log() << "Error when loading state: " << e.message() << std::endl;
	}
}


void
State::save_state()
{
	try {
		QDomDocument doc;
		QDomElement root = doc.createElement ("xefis-mod-systems-state");
		doc.appendChild (root);

		for (auto const& cv: _config_variables)
		{
			QDomElement cv_element = doc.createElement ("state-variable");
			cv_element.setAttribute ("id", cv.second.id());
			cv_element.setAttribute ("type", QString::fromStdString (cv.second.type().string()));
			cv_element.setAttribute ("value", QString::fromStdString (xf::to_hex_string (cv.second.property().binarify())));
			root.appendChild (cv_element);
		}

		// Wait for previous std::async to finish:
		if (_save_future.valid())
			_save_future.wait();

		_save_future = std::async (std::launch::async, do_save_state, doc.toString(), _file_name);
	}
	catch (xf::Exception const& e)
	{
		log() << "Error when saving state: " << e.message() << std::endl;
	}
	catch (std::exception const& e)
	{
		log() << "System error: " << e.what() << std::endl;
	}
}


void
State::do_save_state (QString content, QString file_name)
{
	QDir cwd;
	QString target_file_name = cwd.absolutePath() + '/' + file_name;
	QString temp_file_name = target_file_name + '~';

	QFile file (temp_file_name);

	if (!file.open (QFile::WriteOnly))
		throw xf::IOError ("couldn't open '" + file.fileName() + "' for save: " + file.errorString());

	QTextStream ts (&file);
	ts << content;
	file.flush();
	file.close();
	if (::rename (temp_file_name.toUtf8(), target_file_name.toUtf8()) == -1)
	{
		char buf[256];
		strerror_r (errno, buf, countof (buf));
		throw xf::IOError (QString ("couldn't save settings file: %1").arg (buf));
	}
}

} // namespace

