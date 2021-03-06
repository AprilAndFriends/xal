/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Represents a virtual entry of audio data in the sound system.

#ifndef XAL_SOUND_H
#define XAL_SOUND_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "AudioManager.h"
#include "xalExport.h"

namespace xal
{
	class Player;
	class Buffer;
	class Category;

	/// @brief Provides audio data definition.
	class xalExport Sound
	{
	public:
		friend class Player;

		/// @brief Constructor.
		/// @param[in] filename Filename of the Sound.
		/// @param[in] category The Category where to register this Sound.
		/// @param[in] prefix Used to differentiate between Sounds that have the same filename (e.g. by using a directory path as prefix).
		Sound(chstr filename, Category* category, chstr prefix = "");
		/// @brief Constructor.
		/// @param[in] name Filename of the Sound.
		/// @param[in] category The Category where to register this Sound.
		/// @param[in] data Raw audio data.
		/// @param[in] size Raw audio data byte size.
		/// @param[in] channels Number of channels.
		/// @param[in] samplingRate Sampling rate.
		/// @param[in] bitsPerSample Bits per sample.
		Sound(chstr name, Category* category, unsigned char* data, int size, int channels, int samplingRate, int bitsPerSample);
		/// @brief Destructor.
		~Sound();

		/// @brief Gets name.
		HL_DEFINE_GET(hstr, name, Name);
		/// @brief Gets filename.
		HL_DEFINE_GET(hstr, filename, Filename);
		/// @brief Gets the Category.
		HL_DEFINE_GET(Category*, category, Category);

		/// @return Byte-size of the audio data.
		int getSize();
		/// @return Source byte size.
		int getSourceSize();
		/// @return Number of channels in the audio data.
		int getChannels();
		/// @return Sampling rate of the audio data.
		int getSamplingRate();
		/// @return Number of bits per sample in the audio data.
		int getBitsPerSample();
		/// @return Length of the audio data in seconds.
		float getDuration();
		/// @return File format of the underlying audio file.
		Format getFormat() const;
		/// @return Gets Buffer's idle time
		float getBufferIdleTime();
		/// @return True if the Sounds's Buffer accesses streamed data.
		bool isStreamed() const;
		/// @return True if the Sounds's Buffer is loaded.
		bool isLoaded();

		/// @brief Reads the raw PCM data from the buffer.
		/// @param[out] output The data stream where to store the PCM data.
		/// @note If the underlying Source does not provide data as PCM, it will always be converted to PCM.
		void readPcmData(hstream& output);

	protected:
		/// @brief Name of the Sound.
		hstr name;
		/// @brief Logical filename of the Sound.
		hstr filename;
		/// @brief Category to which the Sound is assigned.
		Category* category;
		/// @brief Buffer instance that handles decoded data.
		Buffer* buffer;

	};

}

#endif
