#include <stdint.h>

// String macAddrList = "";

//UDP udp;

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  // udp.begin(/*localPort=*/6969);
  Serial.begin(9600);
  Serial.println(WiFi.localIP().toString());

}

#define MSGTYPE_SET_COLOR 102
#define MSGTYPE_SET_POWER 117

// ARM IS LITTLE ENDIAN!!!!!!111
#pragma pack(push, 1)
struct PacketHeader {
  // Frame
  uint16_t size;
  uint16_t protocol   :12;
  uint8_t addressable :1;

  // For discovery using Device::GetService,
  //   the tagged field should be set to one (1)
  //   and the target should be all zeroes.
  // In all other messages, the tagged field
  //   should be set to zero (0)
  //   and the target field should contain
  //   the device MAC address.
  // CONFLICTING DOC:
  //   =1 when any receiving device should process this packet.
  uint8_t tagged      :1;
  uint8_t origin      :2;  // must be 0
  uint32_t source;  // unique value set by client (us)

  // Frame Address
  uint8_t target[8];  // 6 byte target MAC addr, last 2 bytes 0s
  uint8_t reserved_1[6];  // must all be 0s
  bool res_required : 1;
  bool ack_required : 1;
  uint8_t :6;  // must all be 0s
  uint8_t sequence;  // wrap around msg seq number

  // Protocol Header
  uint64_t :64;
  uint16_t type;
  uint16_t :16;

  // Payload follows
};

struct SetPowerPacket {
  PacketHeader header;
  uint16_t level;  // must be 0 or 65535
  uint32_t duration;  // transition time in ms
};

struct HSBK {
  uint16_t hue;
  uint16_t saturation;
  uint16_t brightness;
  uint16_t kelvin;
};

struct SetColorPacket {
  PacketHeader header;
  uint8_t :8;
  HSBK color;
  uint32_t duration; // transition time in ms
};

#pragma pack(pop)

#define MAX_PACKET_SIZE_BYTES 128
#define FRAME_SIZE_BYTES 8

bool ON = false;

void print_binary(uint8_t* data, size_t size) {
  for (uint32_t i = 0; i < size; i++) {
    char rep[3];
    sprintf(rep, "%.2X ", data[i]);
    Serial.print(rep);
  }
  Serial.println();
  Serial.println();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  SetColorPacket packet;
  memset(&packet, 0, sizeof(packet));
  
  packet.header.protocol = 1024;
  packet.header.addressable = 1;
  packet.header.tagged = 1;
  packet.header.origin = 0;
  packet.header.source = 0x69696969;
  packet.header.type = MSGTYPE_SET_COLOR;

  packet.color.hue = 21845;
  packet.color.saturation = 65535;
  packet.color.brightness = 65535;
  packet.color.kelvin = 3500;
  packet.duration = 1024;
  ON = !ON;
  // packet.level = ON ? 65535 : 0;

  packet.header.size = sizeof(packet);

  print_binary((uint8_t*) &packet, sizeof(packet));

  delay(4000);
  
}