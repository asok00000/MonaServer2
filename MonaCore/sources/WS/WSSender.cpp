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

#include "Mona/WS/WSSender.h"
#include "Mona/Session.h"

using namespace std;

namespace Mona {

WSSender::WSSender(const shared<Socket>& pSocket, WS::Type type, const Packet& packet) : _pSocket(pSocket), _packet(move(packet)), type(!type ? WS::TYPE_TEXT : type), Runner("WSSender"),
	writer(!type ? (DataWriter&)*new JSONWriter(*new Buffer(10)) : (DataWriter&)*new StringWriter<>(*new Buffer(10))) { // 10 => expect place for header!
	_pBuffer.reset(&writer->buffer());
}

bool WSSender::run(Exception&) {

	if (Media::Data::ToType(writer) == Media::Data::TYPE_JSON && _packet) {
		// two JSON part, concatenate (replace last ']' of first part by ',', and remove first '[' of second part)
		*(STR _pBuffer->data() + _pBuffer->size() - 1) = ',';
		++_packet;
	}

	UInt32 size(writer->size()+ _packet.size());
	UInt8 headerSize(size < 126 ? 2 : (size < 65536 ? 4 : 10));

	_pBuffer->clip(10 - headerSize); // += offset

	// Write header
	BinaryWriter writer(_pBuffer->data(), headerSize);
	writer.write8(type | 0x80);
	if (headerSize == 2)
		writer.write8(size);
	else if (headerSize == 4)
		writer.write8(126).write16(size);
	else
		writer.write8(127).write64(size);

	if (!send(Packet(_pBuffer)))
		return true;

	if (_packet)
		send(_packet);

	return true;
}

bool WSSender::send(const Packet& packet) {
	Exception ex;
	DUMP_RESPONSE(_pSocket->isSecure() ? "WSS" : "WS", packet.data(), packet.size(), _pSocket->peerAddress());
	int result = _pSocket->write(ex, packet);
	if (ex || result<0)
		WARN(ex);
	return result >= 0;
}


bool WSDataSender::run(Exception& ex) {
	if (_packetType != Media::Data::TYPE_JSON) {
		unique_ptr<DataReader> pReader(Media::Data::NewReader(_packetType, _packet));
		if (pReader)
			pReader->read(writer); // Convert to JSON
		else
			writer.writeBytes(_packet.data(), _packet.size()); // Write Raw
		_packet = nullptr;
	}
	return WSSender::run(ex);
}


} // namespace Mona
