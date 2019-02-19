/* vim:ts=4
 *
 * Copyleft 2012…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__CRYPTO__HASH_H__INCLUDED
#define NEUTRINO__CRYPTO__HASH_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>

// Lib:
#include <mhash.h>

// Neutrino:
#include <neutrino/blob.h>
#include <neutrino/exception.h>
#include <neutrino/span.h>


namespace neutrino {

/**
 * Interface to mhash library.
 */
class Hash
{
	class FailedToSetup: public Exception
	{
	  public:
		FailedToSetup():
			Exception ("failed to setup the Hash object")
		{ }
	};

	class AlreadyFinalized: public Exception
	{
	  public:
		AlreadyFinalized():
			Exception ("Hash object already finalized")
		{ }
	};

	class UnknownAlgorithm: public Exception
	{
	  public:
		UnknownAlgorithm():
			Exception ("unknown alogrithm")
		{ }
	};

  public:
	enum Algorithm
	{
		SHA1,
	};

  public:
	// Ctor
	explicit
	Hash (Algorithm);

	/**
	 * Start the hash function and seed with initial data.
	 */
	explicit
	Hash (Algorithm, std::vector<uint8_t> const& vector);

	// Deleted ctor
	Hash (Hash const&) = delete;

	// Deleted operator=
	Hash&
	operator= (Hash const&) = delete;

	/**
	 * Start the hash function and seed with initial data.
	 */
	explicit
	Hash (Algorithm, Span<uint8_t const>);

	// Dtor
	~Hash();

	/**
	 * Update hash with new data.
	 * Slow due to allocation. Use only on big chunks.
	 */
	void
	update (std::vector<uint8_t> const& vector);

	/**
	 * Update hash with new data.
	 */
	void
	update (Span<uint8_t const>);

	/**
	 * Return hash result.
	 */
	Blob
	result();

	/**
	 * Return true if hash has been already finalized and read.
	 */
	bool
	finalized() const;

	/**
	 * Return block-size for the hash function used.
	 */
	size_t
	block_size() const;

  private:
	static constexpr hashid
	get_hashid (Algorithm);

  private:
	Algorithm			_algorithm;
	MHASH				_mhash_thread;
	std::optional<Blob>	_result;
};


inline
Hash::Hash (Algorithm algorithm)
{
	_algorithm = algorithm;
	_mhash_thread = mhash_init (get_hashid (_algorithm));

	if (_mhash_thread == MHASH_FAILED)
		throw FailedToSetup();
}


inline
Hash::Hash (Algorithm algorithm, std::vector<uint8_t> const& vector):
	Hash (algorithm)
{
	update (vector);
}


inline
Hash::Hash (Algorithm algorithm, Span<uint8_t const> vector):
	Hash (algorithm)
{
	update (vector);
}


inline
Hash::~Hash()
{
	if (!finalized())
		mhash_deinit (_mhash_thread, nullptr);
}


inline void
Hash::update (std::vector<uint8_t> const& vector)
{
	update (Span<uint8_t const> (vector));
}


inline void
Hash::update (Span<uint8_t const> vector)
{
	if (finalized())
		throw AlreadyFinalized();

	::mhash (_mhash_thread, vector.data(), vector.size());
}


inline Blob
Hash::result()
{
	if (!_result)
	{
		std::unique_ptr<uint8_t, decltype (std::free)*> ptr (static_cast<uint8_t*> (mhash_end (_mhash_thread)), std::free);
		_result = Blob (ptr.get(), ptr.get() + block_size());
	}

	return *_result;
}


inline bool
Hash::finalized() const
{
	return !!_result;
}


inline size_t
Hash::block_size() const
{
	return mhash_get_block_size (get_hashid (_algorithm));
}


constexpr hashid
Hash::get_hashid (Algorithm algorithm)
{
	switch (algorithm)
	{
		case SHA1:
			return MHASH_SHA1;

		default:
			throw UnknownAlgorithm();
	}
}


/*
 * Global functions
 */


Blob
hash (Hash::Algorithm algorithm, Span<uint8_t const> vector)
{
	return Hash (algorithm, vector).result();
}

} // namespace neutrino

#endif

