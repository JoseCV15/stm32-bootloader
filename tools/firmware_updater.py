import serial
import struct
import zlib

PORT = "COM3"
BR = 115200

sequence = 0x01
cmd = 0x01
payload = b""

header = struct.pack("<BBH", sequence, cmd, len(payload))
crc = zlib.crc32(header + payload) & 0xFFFFFFFF

packet = (b"\xAA\x55" + header + payload + struct.pack("<I", crc))

with serial.Serial(PORT, BR, timeout=2) as ser:
    ser.reset_input_buffer()
    print("TX:", packet.hex(" ").upper())

    ser.write(packet)

    response = ser.read(11)

    print("RX:", response.hex(" ").upper())
    