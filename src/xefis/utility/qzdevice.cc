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

// Lib:
#include <zlib.h>

// Xefis:
#include <xefis/core/stdexcept.h>

// Local:
#include "qzdevice.h"


namespace xf {

QZDevice::QZDevice (QFile* gzip_file, QObject* parent):
	QIODevice (parent),
	_input (gzip_file)
{
	_ctx.zalloc = Z_NULL;
	_ctx.zfree = Z_NULL;
	_ctx.opaque = Z_NULL;
	_ctx.avail_in = 0;
	_ctx.next_in = Z_NULL;
}


bool
QZDevice::atEnd() const
{
	return _z_at_eof;
}


void
QZDevice::close()
{
	_ctx_resource.destroy();
	QIODevice::close();
}


bool
QZDevice::isSequential() const
{
	return true;
}


bool
QZDevice::open (OpenMode mode)
{
	if (mode != QIODevice::ReadOnly)
		return false;

	if (inflateInit2 (&_ctx, kWindowBits + kEnableGzip) != Z_OK)
		return false;

	QIODevice::open (mode);

	_input_buffer.resize (kBufferSize);

	// Ensure the z-state is closed on error, or when this is deleted:
	_ctx_resource = [&] { ::inflateEnd (&_ctx); };

	return true;
}


qint64
QZDevice::readData (char* char_output_buffer, qint64 max_size)
{
	uint8_t* output_buffer = reinterpret_cast<uint8_t*> (char_output_buffer);
	qint64 have_read = 0;

	while (max_size - have_read > 0)
	{
		// Return as many available bytes as possible:
		qint64 to_copy = std::min<qint64> (_decompressed_avail, max_size - have_read);
		auto begin = _decompressed_buffer.data() + _decompressed_buffer.size() - _decompressed_avail;
		std::copy (begin, begin + to_copy, output_buffer + have_read);
		_decompressed_avail -= to_copy;
		have_read += to_copy;

		if (_z_at_eof)
			break;

		// If there's still something to add:
		if (max_size - have_read > 0)
			decompress();
	}

	if (have_read == 0 && _z_at_eof)
		return -1;
	else
		return have_read;
}


qint64
QZDevice::writeData (const char*, qint64)
{
	return -1;
}


void
QZDevice::pull()
{
	_input_buffer.resize (kBufferSize);

	auto n = _input->read (reinterpret_cast<char*> (_input_buffer.data()), _input_buffer.size());

	if (n == -1 && !_input->atEnd())
		throw IOError ("failed to read from input file");

	_input_buffer.resize (n);
	_ctx.avail_in = _input_buffer.size();
	_ctx.next_in = _input_buffer.data();
}


void
QZDevice::decompress()
{
	if (_need_pull)
	{
		pull();
		_need_pull = false;
	}

	_decompressed_buffer.resize (kBufferSize);

	_ctx.avail_out = _decompressed_buffer.size();
	_ctx.next_out = _decompressed_buffer.data();

	switch (::inflate (&_ctx, Z_SYNC_FLUSH))
	{
		case Z_NEED_DICT:
			throw xf::IOError ("failed to decompress input file: Z_NEED_DICT");

		case Z_DATA_ERROR:
			throw xf::IOError ("failed to decompress input file: Z_DATA_ERROR");

		case Z_MEM_ERROR:
			throw xf::IOError ("failed to decompress input file: Z_MEM_ERROR");

		case Z_STREAM_END:
			_z_at_eof = true;
			[[fallthrough]];

		case Z_OK:
			if (_ctx.avail_out != 0 || _ctx.avail_in == 0)
				_need_pull = true;

			_decompressed_buffer.resize (kBufferSize - _ctx.avail_out);
			_decompressed_avail = _decompressed_buffer.size();
			break;
	}
}

} // namespace xf

