/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

*/

#pragma once

#include "Mona/Mona.h"
#include "Mona/AMF.h"
#include "Mona/ReferableReader.h"
#include <vector>

namespace Mona {


struct AMFReader : ReferableReader, virtual Object {
	AMFReader(const UInt8* data, UInt32 size);

	enum {
		OBJECT =	OTHER,
		ARRAY =		OTHER+1,
		MAP =		OTHER+2,
		AMF0_REF =	OTHER+3
	};


	void			startReferencing() { _referencing = true; }
	void			stopReferencing() { _referencing = false; }

	void			reset();

private:

	UInt8			followingType();

	bool			readOne(UInt8 type, DataWriter& writer);
	bool			writeOne(UInt8 type, DataWriter& writer);

	const char*		readText(UInt32& size);

	std::vector<UInt32>		_stringReferences;
	std::vector<UInt32>		_classDefReferences;
	std::vector<UInt32>		_references;
	std::vector<UInt32>		_amf0References;

	UInt8					_amf3;
	bool					_referencing;
	std::string				_buffer;
};



} // namespace Mona
