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

#ifndef XEFIS__UTILITY__QZDEVICE_H__INCLUDED
#define XEFIS__UTILITY__QZDEVICE_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <zlib.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/resource.h>

// Qt:
#include <QtCore/QFile>
#include <QtCore/QIODevice>


namespace xf {

class QZDevice: public QIODevice
{
	static constexpr std::size_t kBufferSize	= 256 * 1024;
	static constexpr uint32_t kEnableGzip		= 32;
	static constexpr uint32_t kWindowBits		= 15;

  public:
	/**
	 * File is a gzip file.
	 */
	explicit
	QZDevice (QFile* gz_file, QObject* parent = nullptr);

	// QIODevice API
	bool
	atEnd() const override;

	// QIODevice API
	void
	close() override;

	// QIODevice API
	bool
	isSequential() const override;

	// QIODevice API
	bool
	open (OpenMode mode) override;

  protected:
	// QIODevice API
	qint64
	readData (char* data, qint64 max_size) override;

	// QIODevice API
	qint64
	writeData (const char* data, qint64 max_size) override;

  private:
	/**
	 * Fill in input buffer.
	 * Buffer sizes are set appropriately.
	 */
	void
	pull();

	/**
	 * Decompress input buffer to decompressed buffer.
	 * Buffer sizes are set appropriately.
	 */
	void
	decompress();

  private:
	QFile*					_input;
	std::vector<uint8_t>	_input_buffer;
	std::vector<uint8_t>	_decompressed_buffer;
	std::size_t				_decompressed_avail	= 0;
	::z_stream				_ctx;
	Resource				_ctx_resource;
	bool					_z_at_eof			= false;
	bool					_need_pull			= true;
};

} // namespace xf

#endif

