/*
   This is a modified version of OpenSprinkler Generation 2 that
 has been adapted to run on a standard Arduino W5100 Ethernet Shield, 
 and a Freetronics LCD Shield.
 
 The code implements a minimal set of functionality to
 replace the EtherCard class libraries and all underlying
 ENC28J60 code files used by Ray in the OpenSprinkler2 library.
 
 Creative Commons Attribution-ShareAlike 3.0 license
 
 Dave Skinner - Aug 2013
 */

#include "EtherCard_W5100.h"
#include <stdarg.h>
#include <avr/eeprom.h>

//================================================================

void BufferFiller::emit_p(PGM_P fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  for (;;) {
    char c = pgm_read_byte(fmt++);
    if (c == 0)
      break;
    if (c != '$') {
      *ptr++ = c;
      continue;
    }
    c = pgm_read_byte(fmt++);
    switch (c) {
    case 'D':
      //wtoa(va_arg(ap, word), (char*) ptr);  //ray
      itoa(va_arg(ap, word), (char*) ptr, 10);
      break;
    case 'L':
      ultoa(va_arg(ap, long), (char*) ptr, 10);
      break;
    case 'S':
      strcpy((char*) ptr, va_arg(ap, const char*));
      break;
    case 'F': 
      {
        PGM_P s = va_arg(ap, PGM_P);
        char d;
        while ((d = pgm_read_byte(s++)) != 0)
          *ptr++ = d;
        continue;
      }
    case 'E': 
      {
        byte* s = va_arg(ap, byte*);
        char d;
        while ((d = eeprom_read_byte(s++)) != 0)
          *ptr++ = d;
        continue;
      }
    default:
      *ptr++ = c;
      continue;
    }
    ptr += strlen((char*) ptr);
  }
  va_end(ap);
}

//============================================================================================
// Declare static data members
uint8_t EtherCard::mymac[6];  // my MAC address
uint8_t EtherCard::myip[4];   // my ip address
uint8_t EtherCard::mymask[4]; // my net mask
uint8_t EtherCard::gwip[4];   // gateway
uint8_t EtherCard::dhcpip[4]; // dhcp server
uint8_t EtherCard::dnsip[4];  // dns server
uint8_t EtherCard::hisip[4];  // dns result
IPAddress EtherCard::ntpip;   // ntp time server
uint16_t EtherCard::hisport = 80; // tcp port to browse to

EtherCard ether;

// External variables defined in main pde file
extern uint8_t ntpclientportL;
extern byte ntpip[];
extern BufferFiller bfill;
extern char tmp_buffer[];
extern EthernetServer server;
extern EthernetUDP udp;
extern ICMPPing ping;

uint8_t EtherCard::begin (const uint16_t size,
const uint8_t* macaddr,
uint8_t csPin) {
  copyMac(mymac, macaddr);
  return 1; //0 means fail
}

bool EtherCard::staticSetup (const uint8_t* my_ip,
const uint8_t* gw_ip,
const uint8_t* dns_ip) {

  // initialize the ethernet device
  Ethernet.begin(mymac, my_ip, dns_ip, gw_ip);

  // start listening for clients
  server.begin();
  for (int i = 0; i < 4; i++)
  {
    myip[i] = Ethernet.localIP()[i];
    gwip[i] = Ethernet.gatewayIP()[i];
    dnsip[i] = Ethernet.dnsServerIP()[i];
    mymask[i] = Ethernet.subnetMask()[i];
  }
  return true;
}

// Initialise DHCP with a particular name.
bool EtherCard::dhcpSetup (const char *name) 
{
  // initialize the ethernet device
  if (Ethernet.begin(mymac) == 0)
    return false;

  // start listening for clients
  server.begin();  
  for (int i = 0; i < 4; i++)
  {
    myip[i] = Ethernet.localIP()[i];
    gwip[i] = Ethernet.gatewayIP()[i];
    dnsip[i] = Ethernet.dnsServerIP()[i];
    mymask[i] = Ethernet.subnetMask()[i];
  }

  return true; 
}

//=================================================================================

word EtherCard::packetReceive() {

  // do nothing - handle everything in packetloop 
  return 0;
}

EthernetClient client;

word EtherCard::packetLoop (word plen) 
{  
  // listen for incoming clients
  client = server.available();
  int i = 0;

  if (client) 
  {
    // set all bytes in the buffer to 0 - add a 
    // byte to simulate space for the TCP header
    memset(buffer, 0, ETHER_BUFFER_SIZE);
    memset(buffer, ' ', TCP_OFFSET);
    i = TCP_OFFSET; // add a space for TCP offset

    while (client.connected() && (i < ETHER_BUFFER_SIZE)) 
    {
      if (client.available()) 
        buffer[i] = client.read();

      i++;
    }
    return TCP_OFFSET;
  }
  return 0;
}

void EtherCard::httpServerReply (word dlen) {

  // ignore dlen - just add a null termination 
  // to the buffer and print it out to the client
  buffer[bfill.position() + TCP_OFFSET] = '\0';
  client.print((char*) bfill.buffer());

  // close the connection:   
  delay(1);       // give the web browser time to receive the data 
  client.stop();
}

void EtherCard::ntpRequest (byte *ntp_ip, byte srcport) {

  udp.begin(srcport);

  // set all bytes in the buffer to 0
  memset(buffer, 0, ETHER_BUFFER_SIZE); 
  // Initialize values needed to form NTP request
  buffer[0] = 0b11100011;    // LI, Version, Mode
  buffer[1] = 0;             // Stratum, or type of clock
  buffer[2] = 6;             // Polling Interval
  buffer[3] = 0xEC;          // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  buffer[12]  = 49; 
  buffer[13]  = 0x4E;
  buffer[14]  = 49;
  buffer[15]  = 52;

  ntpip = IPAddress(ntp_ip[0], ntp_ip[1], ntp_ip[2], ntp_ip[3]);

  // all NTP fields have been given values, now you can send a packet requesting a timestamp: 		   
  udp.beginPacket(ntpip, 123); //NTP requests are to port 123
  udp.write(buffer, NTP_PACKET_SIZE);
  udp.endPacket(); 
}

byte EtherCard::ntpProcessAnswer (uint32_t *time, byte dstport_l) {

  int packetSize = udp.parsePacket();
  if(packetSize)
  {
    // check the packet is from the correct timeserver IP and port
    if ( udp.remotePort() != 123 || udp.remoteIP() != ntpip)
      return 0;

    //the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long. 
    udp.read(buffer, packetSize);
    ((byte*) time)[3] = buffer[40];
    ((byte*) time)[2] = buffer[41];
    ((byte*) time)[1] = buffer[42];
    ((byte*) time)[0] = buffer[43];

    return 1;
  }  
  return 0;
}

//=================================================================================
// Some common utilities needed for IP and web applications
// Author: Guido Socher
// Copyright: GPL V2
//
// 2010-05-20 <jc@wippler.nl>

void EtherCard::copyIp (byte *dst, const byte *src) {
  memcpy(dst, src, 4);
}

void EtherCard::copyMac (byte *dst, const byte *src) {
  memcpy(dst, src, 6);
}

// search for a string of the form key=value in
// a string that looks like q?xyz=abc&uvw=defgh HTTP/1.1\r\n
//
// The returned value is stored in strbuf. You must allocate
// enough storage for strbuf, maxlen is the size of strbuf.
// I.e the value it is declated with: strbuf[5]-> maxlen=5
byte EtherCard::findKeyVal (const char *str,char *strbuf, byte maxlen,const char *key)
{
  byte found=0;
  byte i=0;
  const char *kp;
  kp=key;
  while(*str &&  *str!=' ' && *str!='\n' && found==0){
    if (*str == *kp){
      kp++;
      if (*kp == '\0'){
        str++;
        kp=key;
        if (*str == '='){
          found=1;
        }
      }
    }
    else{
      kp=key;
    }
    str++;
  }
  if (found==1){
    // copy the value to a buffer and terminate it with '\0'
    while(*str &&  *str!=' ' && *str!='\n' && *str!='&' && i<maxlen-1){
      *strbuf=*str;
      i++;
      str++;
      strbuf++;
    }
    *strbuf='\0';
  }
  // return the length of the value
  return(i);
}

// convert a single hex digit character to its integer value
unsigned char h2int(char c)
{
  if (c >= '0' && c <='9'){
    return((unsigned char)c - '0');
  }
  if (c >= 'a' && c <='f'){
    return((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <='F'){
    return((unsigned char)c - 'A' + 10);
  }
  return(0);
}

// decode a url string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
void EtherCard::urlDecode (char *urlbuf)
{
  char c;
  char *dst = urlbuf;
  while ((c = *urlbuf) != 0) {
    if (c == '+') c = ' ';
    if (c == '%') {
      c = *++urlbuf;
      c = (h2int(c) << 4) | h2int(*++urlbuf);
    }
    *dst++ = c;
    urlbuf++;
  }
  *dst = '\0';
}

// convert a single character to a 2 digit hex str
// a terminating '\0' is added
void int2h(char c, char *hstr)
{
  hstr[1]=(c & 0xf)+'0';
  if ((c & 0xf) >9){
    hstr[1]=(c & 0xf) - 10 + 'a';
  }
  c=(c>>4)&0xf;
  hstr[0]=c+'0';
  if (c > 9){
    hstr[0]=c - 10 + 'a';
  }
  hstr[2]='\0';
}

// there must be enoug space in urlbuf. In the worst case that is
// 3 times the length of str
void EtherCard::urlEncode (char *str,char *urlbuf)
{
  char c;
  while ((c = *str) != 0) {
    if (c == ' '||isalnum(c)){ 
      if (c == ' '){ 
        c = '+';
      }
      *urlbuf=c;
      str++;
      urlbuf++;
      continue;
    }
    *urlbuf='%';
    urlbuf++;
    int2h(c,urlbuf);
    urlbuf++;
    urlbuf++;
    str++;
  }
  *urlbuf='\0';
}

// parse a string an extract the IP to bytestr
byte EtherCard::parseIp (byte *bytestr,char *str)
{
  char *sptr;
  byte i=0;
  sptr=NULL;
  while(i<4){
    bytestr[i]=0;
    i++;
  }
  i=0;
  while(*str && i<4){
    // if a number then start
    if (sptr==NULL && isdigit(*str)){
      sptr=str;
    }
    if (*str == '.'){
      *str ='\0';
      bytestr[i]=(atoi(sptr)&0xff);
      i++;
      sptr=NULL;
    }
    str++;
  }
  *str ='\0';
  if (i==3){
    bytestr[i]=(atoi(sptr)&0xff);
    return(0);
  }
  return(1);
}

// take a byte string and convert it to a human readable display string  (base is 10 for ip and 16 for mac addr), len is 4 for IP addr and 6 for mac.
void EtherCard::makeNetStr (char *resultstr,byte *bytestr,byte len,char separator,byte base)
{
  byte i=0;
  byte j=0;
  while(i<len){
    itoa((int)bytestr[i],&resultstr[j],base);
    // search end of str:
    while(resultstr[j]){
      j++;
    }
    resultstr[j]=separator;
    j++;
    i++;
  }
  j--;
  resultstr[j]='\0';
}

// end of webutil.c






