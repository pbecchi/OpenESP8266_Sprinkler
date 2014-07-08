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

// ==================
// Class Definitions
// ==================

#ifndef EtherCard_W5100_h
#define EtherCard_W5100_h

#if ARDUINO >= 100
#include <Arduino.h> // Arduino 1.0
#define WRITE_RESULT size_t
#define WRITE_RETURN return 1;
#else
#include <WProgram.h> // Arduino 0022
#define WRITE_RESULT void
#define WRITE_RETURN
#endif

#include <avr/pgmspace.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ICMPPing.h>

#define MAX_SOCK_NUM        4
#define ETHER_BUFFER_SIZE   1100  // if buffer size is increased, you must check the total RAM consumption
// otherwise it may cause the program to crash
//#define WEBPREFIX           ""    //By specifying a prefix of "", all pages will be at the root of the server.
#define NTP_PACKET_SIZE     48    // NTP time stamp is in the first 48 bytes of the message
#define TCP_OFFSET          1

class BufferFiller : 
public Print 
{
  uint8_t *start, *ptr;
public:
  BufferFiller () {
  }
  BufferFiller (uint8_t* buf) : 
  start (buf), ptr (buf) {
  }

  void emit_p (PGM_P fmt, ...);

  void emit_raw (const char* s, uint16_t n) { 
    memcpy(ptr, s, n); 
    ptr += n; 
  }

  void emit_raw_p (PGM_P p, uint16_t n) { 
    memcpy_P(ptr, p, n); 
    ptr += n; 
  }

  uint8_t* buffer () const { 
    return start; 
  }

  uint16_t position () const { 
    return ptr - start; 
  }

  virtual WRITE_RESULT write (uint8_t v) { 
    *ptr++ = v; 
    WRITE_RETURN         }
};

//=====================================================

//class EtherCard : public Ethernet {
class EtherCard {
public:
  static uint8_t mymac[6];  // my MAC address
  static uint8_t myip[4];   // my ip address
  static uint8_t mymask[4]; // my net mask
  static uint8_t gwip[4];   // gateway
  static uint8_t dhcpip[4]; // dhcp server
  static uint8_t dnsip[4];  // dns server
  static uint8_t hisip[4];  // dns result
  static IPAddress ntpip;   // ntp time server
  static uint16_t hisport;  // tcp port to connect to (default 80)
  static byte buffer[ETHER_BUFFER_SIZE];

  // EtherCard.cpp
  static uint8_t begin (const uint16_t size, const uint8_t* macaddr, uint8_t csPin =8); 
  static bool staticSetup (const uint8_t* my_ip =0, const uint8_t* gw_ip =0, const uint8_t* dns_ip =0);
  static uint16_t packetLoop (uint16_t plen);
  static void httpServerReply (uint16_t dlen);
  static void ntpRequest (uint8_t *ntpip,uint8_t srcport);
  static uint8_t ntpProcessAnswer (uint32_t *time, uint8_t dstport_l);
  static bool dhcpSetup (const char *);

  // webutil.cpp
  static void copyIp (uint8_t *dst, const uint8_t *src);
  static void copyMac (uint8_t *dst, const uint8_t *src);
  static uint8_t findKeyVal(const char *str,char *strbuf, uint8_t maxlen, const char *key);
  static void urlDecode(char *urlbuf);
  static  void urlEncode(char *str,char *urlbuf);
  static uint8_t parseIp(uint8_t *bytestr,char *str);
  static void makeNetStr(char *rs,uint8_t *bs, uint8_t len, char sep, uint8_t base);

  // enc28j60.h
  static uint16_t packetReceive ();  
  static uint8_t* tcpOffset () { 
    return buffer + TCP_OFFSET; 
  }

  //======================= NOT IMPLEMENTED ==============================
  /*
  // ethercard.cpp
  static uint8_t packetLoopIcmpCheckReply (const uint8_t *ip_mh);
  static void clientIcmpRequest (const uint8_t *destip);
  
  // tcpip.cpp
   static void initIp (uint8_t *myip,uint16_t wwwp);
   static void makeUdpReply (char *data,uint8_t len, uint16_t port);
   static void setGwIp (const uint8_t *gwipaddr);
   static uint8_t clientWaitingGw ();
   static uint8_t clientTcpReq (uint8_t (*r)(uint8_t,uint8_t,uint16_t,uint16_t),
   uint16_t (*d)(uint8_t),uint16_t port);
   static void browseUrl (prog_char *urlbuf, const char *urlbuf_varpart,
   prog_char *hoststr,
   void (*cb)(uint8_t,uint16_t,uint16_t));
   static void httpPost (prog_char *urlbuf, prog_char *hoststr,
   prog_char *header, const char *postval,
   void (*cb)(uint8_t,uint16_t,uint16_t));
   static void udpPrepare (uint16_t sport, uint8_t *dip, uint16_t dport);
   static void udpTransmit (uint16_t len);
   static void sendUdp (char *data,uint8_t len,uint16_t sport,
   uint8_t *dip, uint16_t dport);
   static void registerPingCallback (void (*cb)(uint8_t*));
   static void sendWol (uint8_t *wolmac);
   // new stash-based API
   static uint8_t tcpSend ();
   static const char* tcpReply (byte fd);
   
   // dhcp.cpp
   static uint32_t dhcpStartTime ();
   static uint32_t dhcpLeaseTime ();
   static byte dhcpFSM ();
   static bool dhcpValid ();
   static bool dhcpLease ();
   static bool dhcpSetup ();
   
   // dns.cpp
   static bool dnsLookup (prog_char* name, bool fromRam =false);
   
   // webutil.cpp
   static void printIp (const byte *buf);
   static void printIp (const char* msg, const uint8_t *buf);
   static void printIp (const __FlashStringHelper *ifsh, const uint8_t *buf);
   */
};

extern EtherCard ether;

#endif




