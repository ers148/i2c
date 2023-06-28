//
// AbstractFile.hpp
//
//  Created on: Aug 15, 2018
//      Author: Alexander
//

#ifndef DRONEDEVICE_INTERNALDEVICE_ABSTRACTFILE_HPP_
#define DRONEDEVICE_INTERNALDEVICE_ABSTRACTFILE_HPP_

#include <DroneDevice/CoreTypes.hpp>

namespace Device {

//!
//! \brief The AbstractFile class
//!
class AbstractFile {
public:
	virtual ~AbstractFile() = default;

	//!
	//! \brief returns a checksum, can cache values
	//! \return crc of file content
	//!
	virtual uint32_t getChecksum() const = 0;
	virtual FileFlags getFlags() const = 0;
	//!
	//! \brief get file size
	//! \return
	//!
	virtual uint32_t getSize() const = 0;

	//!
	//! \brief read Read data by chunck
	//! \param aOffset The byte offset in file.
	//! \param aBuffer Pointer to an array where the extracted chunck are stored.
	//! \param aLength Size of buffer to read.
	//! \return The total number of elements successfully read is returned.
	//! If this number differs from the count parameter, either a reading error
	//! occurred or the end-of-file was reached while reading
	//!
	virtual bool readChunk(uint32_t aOffset, void *aBuffer, size_t aLength) const = 0;

	//!
	//! \brief write Write data chunck
	//! \param aOffset The byte offset in file.
	//! \param aBuffer Pointer to an array of at least size characters.
	//! \param aLength Size of buffer to write.
	//! \return The total number of elements successfully write is returned.
	//! If this number differs from the count parameter, either a writing error
	//! occurred or the end-of-file was reached while writting
	//!
	virtual bool writeChunk(uint32_t aOffset, const void *aBuffer, size_t aLength) = 0;

	//!
	//! \brief restartWrite
	//! \return
	//!
	virtual bool restartWrite() = 0;

	//!
	//! \brief finalizeWrite
	//! \param total
	//! \return
	//!
	virtual bool finalizeWrite(uint32_t aTotal) = 0;

	//!
	//! \brief isFinalized
	//! \return
	//!
	virtual bool isFinalized() const = 0;
};

} // namespace Device

#endif // DRONEDEVICE_INTERNALDEVICE_ABSTRACTFILE_HPP_
