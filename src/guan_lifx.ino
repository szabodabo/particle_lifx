#include <stdint.h>

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

void print_binary(uint8_t* data, size_t size) {
  for (uint32_t i = 0; i < size; i++) {
    char rep[3];
    sprintf(rep, "%.2X ", data[i]);
    Serial.print(rep);
  }
  Serial.println();
  Serial.println();
}

void PopulateHeader(PacketHeader* header, size_t size) {
  memset(header, 0, sizeof(*header));
  header->size = size;
  header->protocol = 1024;
  header->addressable = 1;
  header->tagged = 1;
  header->origin = 0;
  header->source = 0x69696969;
}

UDP udp;
bool ON = false;

#define MSGTYPE_SET_COLOR 102
#define MSGTYPE_SET_POWER 117

#define LIFX_PORT 56700
#define LOCAL_PORT LIFX_PORT
#define REMOTE_PORT LIFX_PORT

void SetPower(bool on) {
  SetPowerPacket packet;
  memset(&packet, 0, sizeof(packet));
  PopulateHeader(&packet.header, sizeof(packet));
  packet.header.type = MSGTYPE_SET_POWER;
  packet.level = on ? UINT16_MAX : 0;
  packet.duration = 3000;
  SendPacket((uint8_t*) &packet, sizeof(packet));
}

void SetColor(uint16_t hue, uint16_t sat, uint16_t brt, uint16_t kelvin) {
  SetColorPacket packet;
  memset(&packet, 0, sizeof(packet));
  PopulateHeader(&packet.header, sizeof(packet));
  packet.header.type = MSGTYPE_SET_COLOR;

  packet.color.hue = hue;
  packet.color.saturation = sat;
  packet.color.brightness = brt;
  packet.color.kelvin = kelvin;
  packet.duration = 1024;
  SendPacket((uint8_t*) &packet, sizeof(packet));
}

void SendPacket(uint8_t* packet, size_t size) {
  Serial.print("Sending Packet from ");
  Serial.print(WiFi.localIP().toString());
  Serial.print(":");
  Serial.print(LIFX_PORT);
  
  Serial.print(" to ");
  Serial.print(BroadcastIP().toString());
  Serial.print(":");
  Serial.println(REMOTE_PORT);
  print_binary(packet, size);

  int result = udp.sendPacket(packet, size, BroadcastIP(), REMOTE_PORT);
  if (result < 0) {
    Particle.publish("UDP Send Error", PRIVATE);
  }
}

IPAddress BroadcastIP() {
  const uint32_t mask = ~WiFi.subnetMask().raw().ipv4;
  const uint32_t bcast = WiFi.localIP().raw().ipv4 | mask;
  return IPAddress(bcast);
}

void setup() {
  udp.begin(/*localPort=*/LOCAL_PORT);
  Serial.begin(9600);
}

void loop() {
  //SetColor(21845, UINT16_MAX, UINT16_MAX, 3500);
  ON = !ON;
  SetPower(ON);

  delay(3000);
  
}