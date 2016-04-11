/* ==========================================================================================================
This is a fork of Rays OpenSprinkler code thats amended to use alternative hardware:

EtherCardW5100.h and EtherCardW5100.cpp implements a minimal set of functions
as a wrapper to replace the EtherCard class libraries with the standard Arduino
Wiznet5100 Ethernet library.

Version:     Opensprinkler 2.1.6
Date:        January 2016
Repository:  https://github.com/Dave1001/OpenSprinkler-Arduino
License:     Creative Commons Attribution-ShareAlike 3.0 license

Refer to the README file for more information

========================================================================================================== */

#ifndef EtherCard_W5100_h
#define EtherCard_W5100_h
//-----------------------------------------defines foe WiFi esp8266
#ifndef ESP8266
#define ETHERNE Ethernet
#define ETHERNES EthernetServer
#define ETHERNEUDP EthernetUDP
#define ETHERNEC EthernetClient
#else


#define ETHERNE WiFi
#define ETHERNES WiFiServer
#define ETHERNEUDP WiFiUDP
#define ETHERNEC WiFiClient

#endif
#define TCP_FLAGS_FIN_V			1
#define TCP_FLAGS_ACK_V			0x10
#define WRITE_RESULT			size_t
#define WRITE_RETURN			return 1;
#define WRITEBUF				0
#define READBUF					1
#define MAX_SOCK_NUM			4
#define NTP_PACKET_SIZE			48	// NTP time stamp is in the first 48 bytes of the message
#define TCP_OFFSET				1	// offset to simulate the TCP header used in the normal EtherCard library (normally TCP_HEADER_LEN_P = 20 bytes)

#include <Arduino.h>
#ifndef ESP8266
#include <avr/pgmspace.h>

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Dns.h>

#ifdef MY_PING
#include <ICMPPing.h>

#endif
#else

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#endif
#include "Config.h"
#include "../Defines.h"

//=================================================================================

/** This class populates network send and receive buffers.
*
*   This class provides formatted printing into memory. Users can use it to write into send buffers.
*
*   Nota: PGM_P: is a pointer to a string in program space (defined in the source code)
*
*   # Format string
*
*   | Format | Parameter   | Output
*   |--------|-------------|----------
*   | $D     | uint16_t    | Decimal representation
*   | $T ¤   | double      | Decimal representation with 3 digits after decimal sign ([-]d.ddd)
*   | $H     | uint16_t    | Hexadecimal value of lsb (from 00 to ff)
*   | $L     | long        | Decimal representation
*   | $S     | const char* | Copy null terminated string from main memory
*   | $F     | PGM_P       | Copy null terminated string from program space
*   | $E     | byte*       | Copy null terminated string from EEPROM space
*   | $$     | _none_      | '$'
*
*   ¤ _Available only if FLOATEMIT is defined_
*
*   # Examples
*   ~~~~~~~~~~~~~{.c}
*     uint16_t ddd = 123;
*     double ttt = 1.23;
*     uint16_t hhh = 0xa4;
*     long lll = 123456789;
*     char * sss;
*     char fff[] PROGMEM = "MyMemory";
*
*     sss[0] = 'G';
*     sss[1] = 'P';
*     sss[2] = 'L';
*     sss[3] = 0;
*     buf.emit_p( PSTR("ddd=$D\n"), ddd );  // "ddd=123\n"
*     buf.emit_p( PSTR("ttt=$T\n"), ttt );  // "ttt=1.23\n" **TO CHECK**
*     buf.emit_p( PSTR("hhh=$H\n"), hhh );  // "hhh=a4\n"
*     buf.emit_p( PSTR("lll=$L\n"), lll );  // "lll=123456789\n"
*     buf.emit_p( PSTR("sss=$S\n"), sss );  // "sss=GPL\n"
*     buf.emit_p( PSTR("fff=$F\n"), fff );  // "fff=MyMemory\n"
*   ~~~~~~~~~~~~~
*
*/
class BufferFiller : public Print 
{
private:
	static char* wtoa(uint16_t value, char* ptr);
	uint8_t *start; //!< Pointer to start of buffer
	uint8_t *ptr; //!< Pointer to cursor position
public:
	/** @brief  Empty constructor
	*/
	BufferFiller() {}

	/** @brief  Constructor
	*   @param  buf Pointer to the ethernet data buffer
	*/
	BufferFiller(uint8_t* buf) : start(buf), ptr(buf) {}

	/** @brief  Add formatted text to buffer
	*   @param  fmt Format string (see Class description)
	*   @param  ... parameters for format string
	*/
	void emit_p(PGM_P fmt, ...);

	/** @brief  Add data to buffer from main memory
	*   @param  s Pointer to data
	*   @param  n Number of characters to copy
	*/
	void emit_raw(const char* s, uint16_t n) { memcpy(ptr, s, n); ptr += n; }

	/** @brief  Add data to buffer from program space string
	*   @param  p Program space string pointer
	*   @param  n Number of characters to copy
	*/
	void emit_raw_p(PGM_P p, uint16_t n) { memcpy_P(ptr, p, n); ptr += n; }

	/** @brief  Get pointer to start of buffer
	*   @return <i>uint8_t*</i> Pointer to start of buffer
	*/
	uint8_t* buffer() const { return start; }

	/** @brief  Get cursor position
	*   @return <i>uint16_t</i> Cursor postion
	*/
	uint16_t position() const { return ptr - start; }

	/** @brief  Write one byte to buffer
	*   @param  v Byte to add to buffer
	*/
	virtual WRITE_RESULT write(uint8_t v) { *ptr++ = v; WRITE_RETURN }
};

//=================================================================================
/** Wrapper class for Wiznet W5100 to appear (partially) as an Ethercard
@note   Functions that are commented out have not been implemented
@note   All TCP/IP client (outgoing) connections are made from source port in range 2816-3071. Do not use these source ports for other purposes.
*/
class EtherCardW5100
{
private:
	// Definitions specific to this wrapper class
	static IPAddress ntpip;							///< ntp time server ip
#ifdef MY_PING
	static SOCKET pingSocket;						///< socket object used by ICMPPing
	static ICMPPing ping;							///< ping object used by ICMPPing
	static ICMPEchoReply pingResult;				///< result of ping request from ICMPPing 
#endif
	static ETHERNES w5100server;				///< server object
	static ETHERNEC w5100client;				///< client object
	static ETHERNEUDP	  w5100udp;					///< server object
	static ETHERNEC outgoing_client;           //outgoing client
											   // Enum used by tcp_client_state
	

#ifndef ESP8266
	static DNSClient dns_client;                    ///DNSLookUp client
#endif	
													// Helper functions
	static unsigned char h2int(char c);				///< convert a single hex digit character to its integer value
	static void int2h(char c, char *hstr);			///< convert a single character to a 2 digit hex str
	static void printIPConfig();					///< print the current network configuration

public:
	// Definitions from ethercard.h
    static uint8_t mymac[6];			///< MAC address
    static uint8_t myip[4];				///< IP address
    static uint8_t netmask[4];			///< Netmask
    static uint8_t gwip[4];				///< Gateway
    static uint8_t dhcpip[4];			///< DHCP server IP address
    static uint8_t dnsip[4];			///< DNS server IP address
    static uint8_t hisip[4];			///< DNS lookup 

    static uint16_t hisport;			///< TCP port to connect to (default 80)
    static bool using_dhcp;				///< True if using DHCP
	// NOT IMPLEMENTED
	// static uint8_t broadcastip[4];		///< Subnet broadcast address
	// static bool persist_tcp_connection;	///< False to break connections on first packet received
	// static uint16_t delaycnt;				///< Counts number of cycles of packetLoop when no packet received - used to trigger periodic gateway ARP request

	// Definitions from enc28j60.h
	static uint8_t buffer[];				///< Data buffer (shared by recieve and transmit)
	static uint16_t bufferSize;				///< Size of data buffer
	//
	// ESP8266-------------------------------------------------------------------------
	//
	static  bool WiFiconnect(bool is);          // Connect ESP8266 to network and start server
	//

    // EtherCard.cpp
    /**   @brief  Initialise the network interface
    *     @param  size Size of data buffer
    *     @param  macaddr Hardware address to assign to the network interface (6 bytes)
    *     @param  csPin Arduino pin number connected to chip select. Default = 8
    *     @return <i>uint8_t</i> Firmware version or zero on failure.
    */
    static uint8_t begin ( const uint16_t size, const uint8_t* macaddr, uint8_t csPin = 8 );

    /**   @brief  Configure network interface with static IP
    *     @param  my_ip IP address (4 bytes). 0 for no change.
    *     @param  gw_ip Gateway address (4 bytes). 0 for no change. Default = 0
    *     @param  dns_ip DNS address (4 bytes). 0 for no change. Default = 0
    *     @param  mask Subnet mask (4 bytes). 0 for no change. Default = 0
    *     @return <i>bool</i> Returns true on success - actually always true
    */
    static bool staticSetup ( const uint8_t* my_ip,
                              const uint8_t* gw_ip = 0,
                              const uint8_t* dns_ip = 0,
                              const uint8_t* mask = 0 );

    // tcpip.cpp
    /**   @brief  Sends a UDP packet to the IP address of last processed received packet
    *     @param  data Pointer to data payload
    *     @param  len Size of data payload (max 220)
    *     @param  port Source IP port
    */
    // NOT IMPLEMENTED
	// static void makeUdpReply ( const char *data, uint8_t len, uint16_t port );

    /**   @brief  Parse received data
    *     @param  plen Size of data to parse (e.g. return value of packetReceive()).
    *     @return <i>uint16_t</i> Offset of TCP payload data in data buffer or zero if packet processed
    *     @note   Data buffer is shared by receive and transmit functions
    *     @note   Only handles ARP and IP
    */
    static uint16_t packetLoop ( uint16_t plen );

    /**   @brief  Accept a TCP/IP connection
    *     @param  port IP port to accept on - do nothing if wrong port
    *     @param  plen Number of bytes in packet
    *     @return <i>uint16_t</i> Offset within packet of TCP payload. Zero for no data.
    */
	// NOT IMPLEMENTED
    // static uint16_t accept ( uint16_t port, uint16_t plen );

    /**   @brief  Send a response to a HTTP request
    *     @param  dlen Size of the HTTP (TCP) payload
    */
    static void httpServerReply ( uint16_t dlen );

    /**   @brief  Send a response to a HTTP request
    *     @param  dlen Size of the HTTP (TCP) payload
    *     @param  flags TCP flags
    */
    static void httpServerReply_with_flags ( uint16_t dlen, uint8_t flags );

    /**   @brief  Acknowledge TCP message
    *     @todo   Is this / should this be private?
    */
    static void httpServerReplyAck();

    /**   @brief  Set the gateway address
    *     @param  gwipaddr Gateway address (4 bytes)
    */
    static void setGwIp ( const uint8_t *gwipaddr );

    /**   @brief  Updates the broadcast address based on current IP address and subnet mask
    */
	// NOT IMPLEMENTED
	// static void updateBroadcastAddress();

    /**   @brief  Check if got gateway hardware address (ARP lookup)
    *     @return <i>unit8_t</i> True if gateway found
    */
	// NOT IMPLEMENTED
    // static uint8_t clientWaitingGw();

    /**   @brief  Check if got gateway DNS address (ARP lookup)
    *     @return <i>unit8_t</i> True if DNS found
    */
	// NOT IMPLEMENTED
    // static uint8_t clientWaitingDns();

    /**   @brief  Prepare a TCP request
    *     @param  result_cb Pointer to callback function that handles TCP result
    *     @param  datafill_cb Pointer to callback function that handles TCP data payload
    *     @param  port Remote TCP/IP port to connect to
    *     @return <i>unit8_t</i> ID of TCP/IP session (0-7)
    *     @note   Return value provides id of the request to allow up to 7 concurrent requests
    */
	// NOT IMPLEMENTED
    // static uint8_t clientTcpReq ( uint8_t ( *result_cb ) ( uint8_t, uint8_t, uint16_t, uint16_t ), uint16_t ( *datafill_cb ) ( uint8_t ), uint16_t port );

    /**   @brief  Prepare HTTP request
    *     @param  urlbuf Pointer to c-string URL folder
    *     @param  urlbuf_varpart Pointer to c-string URL file
    *     @param  hoststr Pointer to c-string hostname
    *     @param  additionalheaderline Pointer to c-string with additional HTTP header info
    *     @param  callback Pointer to callback function to handle response
    *     @note   Request sent in main packetloop
    */
    static void browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, const char *additionalheaderline, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) );

    /**   @brief  Prepare HTTP request
    *     @param  urlbuf Pointer to c-string URL folder
    *     @param  urlbuf_varpart Pointer to c-string URL file
    *     @param  hoststr Pointer to c-string hostname
    *     @param  callback Pointer to callback function to handle response
    *     @note   Request sent in main packetloop
    */
    static void browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) );

    /**   @brief  Prepare HTTP post message
    *     @param  urlbuf Pointer to c-string URL folder
    *     @param  hoststr Pointer to c-string hostname
    *     @param  additionalheaderline Pointer to c-string with additional HTTP header info
    *     @param  postval Pointer to c-string HTML Post value
    *     @param  callback Pointer to callback function to handle response
    *     @note   Request sent in main packetloop
    */
	// NOT IMPLEMENTED
    // static void httpPost ( const char *urlbuf, const char *hoststr, const char *additionalheaderline, const char *postval, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) );

    /**   @brief  Send NTP request
    *     @param  ntpip IP address of NTP server
    *     @param  srcport IP port to send from
    */
    static void ntpRequest ( uint8_t *ntpip, uint8_t srcport );

    /**   @brief  Process network time protocol response
    *     @param  time Pointer to integer to hold result
    *     @param  dstport_l Destination port to expect response. Set to zero to accept on any port
    *     @return <i>uint8_t</i> True (1) on success
    */
    static uint8_t ntpProcessAnswer ( uint32_t *time, uint8_t dstport_l );

    /**   @brief  Prepare a UDP message for transmission
    *     @param  sport Source port
    *     @param  dip Pointer to 4 byte destination IP address
    *     @param  dport Destination port
    */
	// NOT IMPLEMENTED
    // static void udpPrepare ( uint16_t sport, const uint8_t *dip, uint16_t dport );

    /**   @brief  Transmit UDP packet
    *     @param  len Size of payload
    */
	// NOT IMPLEMENTED
    // static void udpTransmit ( uint16_t len );

    /**   @brief  Sends a UDP packet
    *     @param  data Pointer to data
    *     @param  len Size of payload (maximum 220 octets / bytes)
    *     @param  sport Source port
    *     @param  dip Pointer to 4 byte destination IP address
    *     @param  dport Destination port
    */
	// NOT IMPLEMENTED
    // static void sendUdp ( const char *data, uint8_t len, uint16_t sport, const uint8_t *dip, uint16_t dport );

    /**   @brief  Resister the function to handle ping events
    *     @param  cb Pointer to function
    */
	// NOT IMPLEMENTED
	// static void registerPingCallback ( void ( *cb ) ( uint8_t* ) );

    /**   @brief  Send ping
    *     @param  destip Ponter to 4 byte destination IP address
    */
    static void clientIcmpRequest ( const uint8_t *destip );

    /**   @brief  Check for ping response
    *     @param  ip_monitoredhost Pointer to 4 byte IP address of host to check
    *     @return <i>uint8_t</i> True (1) if ping response from specified host
    */
    static uint8_t packetLoopIcmpCheckReply ( const uint8_t *ip_monitoredhost );

    /**   @brief  Send a wake on lan message
    *     @param  wolmac Pointer to 6 byte hardware (MAC) address of host to send message to
    */
	// NOT IMPLEMENTED
	// static void sendWol ( uint8_t *wolmac );

    // new stash-based API
    /**   @brief  Send TCP request
    */
	// NOT IMPLEMENTED
    // static uint8_t tcpSend();

    /**   @brief  Get TCP reply
    *     @return <i>char*</i> Pointer to TCP reply payload. NULL if no data.
    */
	// NOT IMPLEMENTED
    // static const char* tcpReply ( uint8_t fd );

    /**   @brief  Configure TCP connections to be persistent or not
    *     @param  persist True to maintain TCP connection. False to finish TCP connection after first packet.
    */
	// NOT IMPLEMENTED
    // static void persistTcpConnection ( bool persist );

    //udpserver.cpp
    /**   @brief  Register function to handle incomint UDP events
    *     @param  callback Function to handle event
    *     @param  port Port to listen on
    */
    // NOT IMPLEMENTED
	// static void udpServerListenOnPort ( UdpServerCallback callback, uint16_t port );

    /**   @brief  Pause listing on UDP port
    *     @brief  port Port to pause
    */
	// NOT IMPLEMENTED
    // static void udpServerPauseListenOnPort ( uint16_t port );

    /**   @brief  Resume listing on UDP port
    *     @brief  port Port to pause
    */
	// NOT IMPLEMENTED
    // static void udpServerResumeListenOnPort ( uint16_t port );

    /**   @brief  Check if UDP server is listening on any ports
    *     @return <i>bool</i> True if listening on any ports
    */
	// NOT IMPLEMENTED
    // static bool udpServerListening();                        //called by tcpip, in packetLoop

    /**   @brief  Passes packet to UDP Server
    *     @param  len Not used
    *     @return <i>bool</i> True if packet processed
    */
	// NOT IMPLEMENTED
	// static bool udpServerHasProcessedPacket ( uint16_t len ); //called by tcpip, in packetLoop

    // dhcp.cpp
    /**   @brief  Update DHCP state
    *     @param  len Length of received data packet
    */
	// NOT IMPLEMENTED
	// static void DhcpStateMachine ( uint16_t len );

    /**   @brief Not implemented
    *     @todo Implement dhcpStartTime or remove declaration
    */
	// NOT IMPLEMENTED
    // static uint32_t dhcpStartTime();

    /**   @brief Not implemented
    *     @todo Implement dhcpLeaseTime or remove declaration
    */
	// NOT IMPLEMENTED
    // static uint32_t dhcpLeaseTime();

    /**   @brief Not implemented
    *     @todo Implement dhcpLease or remove declaration
    */
	// NOT IMPLEMENTED
    // static bool dhcpLease();

    /**   @brief  Configure network interface with DHCP
    *     @return <i>bool</i> True if DHCP successful
    *     @note   Blocks until DHCP complete or timeout after 60 seconds
    */
    static bool dhcpSetup ( const char *hname = NULL, bool fromRam = false );

    /**   @brief  Register a callback for a specific DHCP option number
    *     @param  option The option number to request from the DHCP server
    *     @param  callback The function to be call when the option is received
    */
	// NOT IMPLEMENTED
    // static void dhcpAddOptionCallback ( uint8_t option, DhcpOptionCallback callback );

    // dns.cpp
    /**   @brief  Perform DNS lookup
    *     @param  name Host name to lookup
    *     @param  fromRam Set true to look up cached name. Default = false
    *     @return <i>bool</i> True on success.
    *     @note   Result is stored in <i>hisip</i> member
    */
    static bool dnsLookup ( const char* name, bool fromRam = false );

    // webutil.cpp
    /**   @brief  Copies an IP address
    *     @param  dst Pointer to the 4 byte destination
    *     @param  src Pointer to the 4 byte source
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void copyIp ( uint8_t *dst, const uint8_t *src );

    /**   @brief  Copies a hardware address
    *     @param  dst Pointer to the 6 byte destination
    *     @param  src Pointer to the 6 byte destination
    *     @note   There is no check of source or destination size. Ensure both are 6 bytes
    */
    static void copyMac ( uint8_t *dst, const uint8_t *src );

    /**   @brief  Output to serial port in dotted decimal IP format
    *     @param  buf Pointer to 4 byte IP address
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void printIp ( const uint8_t *buf );

    /**   @brief  Output message and IP address to serial port in dotted decimal IP format
    *     @param  msg Pointer to null terminated string
    *     @param  buf Pointer to 4 byte IP address
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void printIp ( const char* msg, const uint8_t *buf );

    /**   @brief  Output Flash String Helper and IP address to serial port in dotted decimal IP format
    *     @param  ifsh Pointer to Flash String Helper
    *     @param  buf Pointer to 4 byte IP address
    *     @note   There is no check of source or destination size. Ensure both are 4 bytes
    */
    static void printIp ( const __FlashStringHelper *ifsh, const uint8_t *buf );

    /**   @brief  Search for a string of the form key=value in a string that looks like q?xyz=abc&uvw=defgh HTTP/1.1\\r\\n
    *     @param  str Pointer to the null terminated string to search
    *     @param  strbuf Pointer to buffer to hold null terminated result string
    *     @param  maxlen Maximum length of result
    *     @param  key Pointer to null terminated string holding the key to search for
    *     @return <i>unit_t</i> Length of the value. 0 if not found
    *     @note   Ensure strbuf has memory allocated of at least maxlen + 1 (to accomodate result plus terminating null)
    */
    static uint8_t findKeyVal ( const char *str, char *strbuf, uint8_t maxlen, const char *key );

    /**   @brief  Decode a URL string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
    *     @param  urlbuf Pointer to the null terminated URL
    *     @note   urlbuf is modified
    */
    static void urlDecode ( char *urlbuf );

    /**   @brief  Encode a URL, replacing illegal charaters like ' '
    *     @param  str Pointer to the null terminated string to encode
    *     @param  urlbuf Pointer to a buffer to contain the null terminated encoded URL
    *     @note   There must be enough space in urlbuf. In the worst case that is 3 times the length of str
    */
    static  void urlEncode ( char *str, char *urlbuf );

    /**   @brief  Convert an IP address from dotted decimal formated string to 4 bytes
    *     @param  bytestr Pointer to the 4 byte array to store IP address
    *     @param  str Pointer to string to parse
    *     @return <i>uint8_t</i> 0 on success
    */
    static uint8_t parseIp ( uint8_t *bytestr, char *str );

    /**   @brief  Convert a byte array to a human readable display string
    *     @param  resultstr Pointer to a buffer to hold the resulting null terminated string
    *     @param  bytestr Pointer to the byte array containing the address to convert
    *     @param  len Length of the array (4 for IP address, 6 for hardware (MAC) address)
    *     @param  separator Delimiter character (typically '.' for IP address and ':' for hardware (MAC) address)
    *     @param  base Base for numerical representation (typically 10 for IP address and 16 for hardware (MAC) address
    */
    static void makeNetStr ( char *resultstr, uint8_t *bytestr, uint8_t len, char separator, uint8_t base );

    /**   @brief  Return the sequence number of the current TCP package
    */
	// NOT IMPLEMENTED
    // static uint32_t getSequenceNumber();

    /**   @brief  Return the payload length of the current Tcp package
    */
	// NOT IMPLEMENTED
    // static uint16_t getTcpPayloadLength();

	// ==========================================
	// Inherited from enc28j60.cpp

	/**   @brief  Copy recieved packets to data buffer
	*     @return <i>uint16_t</i> Size of recieved data
	*     @note   Data buffer is shared by recieve and transmit functions
	*/
	static uint16_t packetReceive();

	// static uint8_t* tcpOffset() { return buffer + 0x36; } //!< Pointer to the start of TCP payload
	static uint8_t* tcpOffset() { return buffer + TCP_OFFSET; } //!< Pointer to the start of TCP payload
};

extern EtherCardW5100 ether; //!< Global presentation of EtherCard class

#endif




