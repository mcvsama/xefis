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

#ifndef XEFIS__UTILITY__HASH_H__INCLUDED
#define XEFIS__UTILITY__HASH_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <mhash.h>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Interface to mhash library.
 */
class Hash
{
  public:
	// Ctor
	Hash();

	/**
	 * Compute and finalize hash of given vector.
	 */
	explicit
	Hash (std::vector<uint8_t> const& vector);

	/**
	 * Compute and finalize hash of given vector.
	 */
	explicit
	Hash (const void* data, std::size_t size);

	// Dtor
	~Hash();

	/**
	 * Slow due to allocation. Use only on big chunks.
	 */
	void
	update (std::vector<uint8_t> const& vector);

	void
	update (const void* data, std::size_t size);

	/**
	 * Finalize hash computation.
	 */
	void
	finalize();

	/**
	 * Return true if hash has been already finalized and read.
	 */
	bool
	finalized() const;

	/**
	 * Return hash value.
	 */
	std::vector<uint8_t>
	result() const;

	/**
	 * Return iterator to the beginning of the result.
	 */
	uint8_t*
	begin() const;

	/**
	 * Return past-the-end iterator of the result.
	 */
	uint8_t*
	end() const;

	/**
	 * Return block-size for the hash function used.
	 */
	size_t
	block_size() const;

  private:
	hashid	_algorithm;
	MHASH	_mhash_thread;
	void*	_result = nullptr;
};


inline
Hash::Hash()
{
	_algorithm = MHASH_SHA1;
	_mhash_thread = mhash_init (_algorithm);
	if (_mhash_thread == MHASH_FAILED)
		throw xf::Exception ("failed to setup Hash object");
}


inline
Hash::Hash (std::vector<uint8_t> const& vector):
	Hash()
{
	update (vector);
	finalize();
}


inline
Hash::Hash (const void* data, std::size_t size):
	Hash()
{
	update (data, size);
	finalize();
}


inline
Hash::~Hash()
{
	if (!finalized())
		finalize();
	free (_result);
}


inline void
Hash::update (std::vector<uint8_t> const& vector)
{
	update (vector.data(), vector.size());
}


inline void
Hash::update (const void* data, std::size_t size)
{
	if (finalized())
		throw xf::Exception ("Hash object already finalized");
	::mhash (_mhash_thread, data, size);
}


inline void
Hash::finalize()
{
	if (finalized())
		throw xf::Exception ("Hash object already finalized");
	_result = mhash_end (_mhash_thread);
}


inline bool
Hash::finalized() const
{
	return !!_result;
}


inline std::vector<uint8_t>
Hash::result() const
{
	std::vector<uint8_t> ret;
	ret.insert (ret.end(), begin(), end());
	return ret;
}


inline uint8_t*
Hash::begin() const
{
	return static_cast<uint8_t*> (_result);
}


inline uint8_t*
Hash::end() const
{
	return static_cast<uint8_t*> (_result) + mhash_get_block_size (_algorithm);
}


inline size_t
Hash::block_size() const
{
	return mhash_get_block_size (_algorithm);
}

} // namespace xf

#endif

