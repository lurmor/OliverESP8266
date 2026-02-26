#include "findmDNS.h"

WiFiUDP mDNSudp;

void sendMDNSQuery(const char *name, IPAddress localIP)
{
  // Пример: "_services._dns-sd._udp.local"
  byte packet[512];
  int len = 0;

  // DNS Header (12 bytes)
  packet[len++] = 0x00;
  packet[len++] = 0x00; // ID
  packet[len++] = 0x00;
  packet[len++] = 0x00; // Flags
  packet[len++] = 0x00;
  packet[len++] = 0x01; // QDCOUNT = 1
  packet[len++] = 0x00;
  packet[len++] = 0x00; // ANCOUNT = 0
  packet[len++] = 0x00;
  packet[len++] = 0x00; // NSCOUNT = 0
  packet[len++] = 0x00;
  packet[len++] = 0x00; // ARCOUNT = 0

  // QNAME (разбить name на labels)
  String qname(name);
  int start = 0;
  while (true)
  {
    int dot = qname.indexOf('.', start);
    if (dot == -1)
      dot = qname.length();
    int labellen = dot - start;
    packet[len++] = labellen;
    for (int i = 0; i < labellen; i++)
    {
      packet[len++] = qname[start + i];
    }
    start = dot + 1;
    if (dot >= qname.length())
      break;
  }
  packet[len++] = 0x00; // end of QNAME

  // QTYPE = PTR (0x000C)
  packet[len++] = 0x00;
  packet[len++] = 0x0C;

  // QCLASS = IN (0x0001), multicast = 0x8001
  packet[len++] = 0x80;
  packet[len++] = 0x01;

  // Отправка
  mDNSudp.beginPacketMulticast(IPAddress(224, 0, 0, 251), 5353, localIP, 255);
  mDNSudp.write(packet, len);
  mDNSudp.endPacket();

  Serial.println("mDNS query sent!");
}

String readName(uint8_t *buf, int &pos, int size)
{
  String name = "";
  bool first = true;
  while (pos < size)
  {
    uint8_t len = buf[pos++];
    if (len == 0)
      break;
    if ((len & 0xC0) == 0xC0)
    { // сжатие
      int offset = ((len & 0x3F) << 8) | buf[pos++];
      int dummy = offset;
      if (!first)
        name += ".";
      name += readName(buf, dummy, size);
      break;
    }
    else
    {
      if (!first)
        name += ".";
      for (int i = 0; i < len; i++)
      {
        name += (char)buf[pos++];
      }
      first = false;
    }
  }
  return name;
}

bool parseMDNS(uint8_t *buf, int size,
               String &serviceName, String &targetHost,
               String &ipAddr, String &name, uint16_t &port)
{
  if (size < 12)
    return false;

  int qdcount = (buf[4] << 8) | buf[5];
  int ancount = (buf[6] << 8) | buf[7];
  int nscount = (buf[8] << 8) | buf[9];
  int arcount = (buf[10] << 8) | buf[11];

  int pos = 12;

  // пропускаем вопросы
  for (int i = 0; i < qdcount; i++)
  {
    readName(buf, pos, size);
    pos += 4;
  }

  int total = ancount + nscount + arcount;

  for (int i = 0; i < total; i++)
  {
    String name = readName(buf, pos, size);
    if (pos + 10 > size)
      return false;

    uint16_t type = (buf[pos] << 8) | buf[pos + 1];
    pos += 2;
    uint16_t clas = (buf[pos] << 8) | buf[pos + 1];
    pos += 2;
    uint32_t ttl = (buf[pos] << 24) | (buf[pos + 1] << 16) | (buf[pos + 2] << 8) | buf[pos + 3];
    pos += 4;
    uint16_t rdlen = (buf[pos] << 8) | buf[pos + 1];
    pos += 2;

    if (pos + rdlen > size)
      return false;

    if (type == 12)
    { // PTR
      int p = pos;
      String target = readName(buf, p, size);
      serviceName = target;
    }
    else if (type == 33)
    { // SRV
      uint16_t priority = (buf[pos] << 8) | buf[pos + 1];
      uint16_t weight = (buf[pos + 2] << 8) | buf[pos + 3];
      port = (buf[pos + 4] << 8) | buf[pos + 5];
      int p = pos + 6;
      targetHost = readName(buf, p, size);
    }
    else if (type == 1)
    { // A
      char ip[16];
      snprintf(ip, sizeof(ip), "%d.%d.%d.%d",
               buf[pos], buf[pos + 1], buf[pos + 2], buf[pos + 3]);
      ipAddr = ip;
    }
    // TXT и другие можно добавить аналогично

    pos += rdlen;
  }

  return true;
}

int findmDNS(IPAddress localIP, String target, IPAddress &ip, uint16_t &port)
{
  unsigned long start = millis();
  mDNSudp.beginMulticast(localIP, IPAddress(224, 0, 0, 251), 5353);

  sendMDNSQuery("_rtp._tcp.local", localIP);

  while (millis() - start < 10000)
  {
    int size = mDNSudp.parsePacket();
    if (size)
    {
      uint8_t buf[512];
      mDNSudp.read(buf, sizeof(buf));
      String service, host, tmpip, name;
      if (parseMDNS(buf, size, service, host, tmpip, name, port))
      {
        if (target == service)
        {
          ip.fromString(tmpip);
          mDNSudp.stop();
          return 0;
        }
      }
    }
  }
  mDNSudp.stop();
  return -1;
}