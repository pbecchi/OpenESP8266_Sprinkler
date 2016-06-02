/* ===============================================================
   This is a fork of Rays OpenSprinkler code thats amended to use alternative hardware:

   EtherCardW5100.h and EtherCardW5100.cpp implements a minimal set of functions
   as a wrapper to replace the EtherCard class libraries with the standard Arduino
   Wiznet5100 Ethernet library.

   Version:     Opensprinkler 2.1.6
   Date:        January 2016
   Repository:  https://github.com/Dave1001/OpenSprinkler-Arduino
   License:     Creative Commons Attribution-ShareAlike 3.0 license

   Refer to the README file for more information

   =============================================================== */

#include "Defines.h"
#undef DB_MASK
#define DB_MASK 4
#include <stdarg.h>
#ifndef ESP8266
#include <avr/eeprom.h>
#else
#include "EEprom_mio.h"
const char* SSID = "Vodafone-25873015";
const char* PASSWORD = "5ph87cmmjmm8cs9";
#endif

#include "EtherCardW5100.h"

//------------------------------------------------------------------
//================================================================
// Utility functions
//================================================================

/// <summary>
/// Copy four bytes to an IPAddress object
/// </summary>
/// <param name="src">Pointer to the 4 byte source</param>
/// <returns></returns>
IPAddress Byte2IP ( const byte *src )
{
    return IPAddress ( src[0], src[1], src[2], src[3] );
}

/// <summary>
/// Copy an IPAddress object to four bytes
/// </summary>
/// <param name="src">Pointer to the 4 byte source</param>
/// <returns></returns>
void IP2Byte ( IPAddress ip, byte *dest )
{
    dest[0] = ip[0];
    dest[1] = ip[1];
    dest[2] = ip[2];
    dest[3] = ip[3];
}

/// <summary>
/// ethercard.cpp - convert a two byte word to ascii
/// </summary>
/// <param name="value"></param>
/// <param name="ptr"></param>
/// <returns></returns>
char* BufferFiller::wtoa ( uint16_t value, char* ptr )
{
    if ( value > 9 )
        ptr = wtoa ( value / 10, ptr );
    *ptr = '0' + value % 10;
    *++ptr = 0;
    return ptr;
}

/// <summary>
/// webutil.cpp - convert a single hex digit character to its integer value
/// </summary>
/// <param name="c">single character</param>
/// <returns>integer value</returns>
unsigned char EtherCardW5100::h2int ( char c )
{
    if ( c >= '0' && c <= '9' )
    {
        return ( ( unsigned char ) c - '0' );
    }
    if ( c >= 'a' && c <= 'f' )
    {
        return ( ( unsigned char ) c - 'a' + 10 );
    }
    if ( c >= 'A' && c <= 'F' )
    {
        return ( ( unsigned char ) c - 'A' + 10 );
    }
    return ( 0 );
}

/// <summary>
/// webutil.cpp - convert a single character to a 2 digit hex str
/// </summary>
/// <param name="c">single char</param>
/// <param name="hstr">2 digit hex string with terminating '\0'</param>
void EtherCardW5100::int2h ( char c, char *hstr )
{
    hstr[1] = ( c & 0xf ) + '0';
    if ( ( c & 0xf ) >9 )
    {
        hstr[1] = ( c & 0xf ) - 10 + 'a';
    }
    c = ( c >> 4 ) & 0xf;
    hstr[0] = c + '0';
    if ( c > 9 )
    {
        hstr[0] = c - 10 + 'a';
    }
    hstr[2] = '\0';
}

//================================================================
// Bufferfiller object
//================================================================

/// <summary>
/// This class populates network send and receive buffers.
/// This class provides formatted printing into memory.Users can use it to write into send buffers.
/// </summary>
/// <param name="fmt">PGM_P : is a pointer to a string in program space(defined in the source code)</param>
void BufferFiller::emit_p ( PGM_P fmt, ... )
{
    va_list ap;
    va_start ( ap, fmt );
    for ( ;; )
    {
        char c = pgm_read_byte ( fmt++ );
        if ( c == 0 )
            break;
        if ( c != '$' )
        {
            *ptr++ = c;
            continue;
        }
        c = pgm_read_byte ( fmt++ );
        switch ( c )
        {
        case 'D':
#ifdef __AVR__
            wtoa ( va_arg ( ap, uint16_t ), ( char* ) ptr );
#else
            wtoa ( va_arg ( ap, int ), ( char* ) ptr );
#endif
            break;
#ifdef FLOATEMIT
        case 'T':
            dtostrf ( va_arg ( ap, double ), 10, 3, ( char* ) ptr );
            break;
#endif
        case 'H':
        {
#ifdef __AVR__
            char p1 = va_arg ( ap, uint16_t );
#else
            char p1 = va_arg ( ap, int );
#endif
            char p2;
            p2 = ( p1 >> 4 ) & 0x0F;
            p1 = p1 & 0x0F;
            if ( p1 > 9 ) p1 += 0x07; // adjust 0x0a-0x0f to come out 'a'-'f'
            p1 += 0x30;             // and complete
            if ( p2 > 9 ) p2 += 0x07; // adjust 0x0a-0x0f to come out 'a'-'f'
            p2 += 0x30;             // and complete
            *ptr++ = p2;
            *ptr++ = p1;
            continue;
        }
        case 'L':
            ltoa ( va_arg ( ap, long ), ( char* ) ptr, 10 );
            break;
        case 'S':
            strcpy ( ( char* ) ptr, va_arg ( ap, const char* ) );
            break;
        case 'F':
        {
            PGM_P s = va_arg ( ap, PGM_P );
            char d;
            while ( ( d = pgm_read_byte ( s++ ) ) != 0 )
                *ptr++ = d;
            continue;
        }
        case 'E':
        {
            byte* s = va_arg ( ap, byte* );
            char d;
			while ((d = eeprom_read_byte(s++)) != 0)
			{
				DEBUG_PRINT(d);
				*ptr++ = d;
			}
            continue;
        }
        default:
            *ptr++ = c;
            continue;
        }
        ptr += strlen ( ( char* ) ptr );
    }
    va_end ( ap );
}

//================================================================

EtherCardW5100 ether;

// Declare static data members (ethercard.cpp)
uint8_t EtherCardW5100::mymac[6];				///< MAC address
uint8_t EtherCardW5100::myip[4];				///< IP address
uint8_t EtherCardW5100::netmask[4];				///< Netmask
uint8_t EtherCardW5100::gwip[4];				///< Gateway
uint8_t EtherCardW5100::dhcpip[4];				///< DHCP server IP address
uint8_t EtherCardW5100::dnsip[4];				///< DNS server IP address
uint8_t EtherCardW5100::hisip[4];				///< DNS lookup result
uint16_t EtherCardW5100::hisport = 80;			///< TCP port to connect to (default 80)
bool EtherCardW5100::using_dhcp;				///< True if using DHCP
// NOT IMPLEMENTED
// uint8_t EtherCardW5100::broadcastip[4];		///< Subnet broadcast address
// bool EtherCardW5100::persist_tcp_connection; ///< False to break connections on first packet received
// uint16_t EtherCardW5100::delaycnt;			///< Counts number of cycles of packetLoop when no packet received - used to trigger periodic gateway ARP request

// Declare static data (tcpip.cp)p
ETHERNEC EtherCardW5100::outgoing_client;
enum tcpstate_t
{
	TCP_SEND = 1,
	TCP_SENT,
	TCP_ESTABLISHED,
	TCP_NOT_USED,
	TCP_CLOSING,
	TCP_CLOSED
};

static tcpstate_t outgoing_client_state;
static const char *client_urlbuf;					// Pointer to c-string path part of HTTP request URL
static const char *client_urlbuf_var;				// Pointer to c-string filename part of HTTP request URL
static const char *client_hoststr;					// Pointer to c-string hostname of current HTTP request
static const char *client_additionalheaderline;		// Pointer to c-string additional http request header info
static const char *client_postval;
static void ( *client_browser_cb ) ( uint8_t, uint16_t, uint16_t ); // Pointer to callback function to handle result of current HTTP request
static uint8_t www_fd;								// ID of current http request (only one http request at a time - one of the 8 possible concurrent TCP/IP connections)

// Declare static data members from enc28j60.h
uint16_t EtherCardW5100::bufferSize;				///< Size of data buffer

// Declare static data members for this wrapper class
IPAddress EtherCardW5100::ntpip;
#ifdef MY_PING
SOCKET EtherCardW5100::pingSocket = 0;
ICMPPing EtherCardW5100::ping ( pingSocket, 1 );
ICMPEchoReply EtherCardW5100::pingResult;				///< Result of ping request from ICMPPing library
#endif
ETHERNES EtherCardW5100::w5100server(80);
ETHERNEC EtherCardW5100::w5100client;
ETHERNEUDP EtherCardW5100::w5100udp;
#ifndef ESP8266
DNSClient EtherCardW5100::dns_client;
#endif
// External variables defined in main .ino file
extern BufferFiller bfill;

//=================================================================================
// Ethercard Wrapper Functions
//=================================================================================

uint8_t EtherCardW5100::begin ( const uint16_t size, const uint8_t* macaddr, uint8_t csPin )
{
    using_dhcp = false;
    copyMac ( mymac, macaddr );
    return 1; //0 means fail
}	


bool EtherCardW5100::staticSetup ( const uint8_t* my_ip, const uint8_t* gw_ip, const uint8_t* dns_ip, const uint8_t* mask )
{
    using_dhcp = false;
	DEBUG_PRINT("-_");
	/*
	// convert bytes to IPAddress
	IPAddress ip = Byte2IP ( my_ip );
	IPAddress gw = Byte2IP ( gw_ip );
	IPAddress subnet = Byte2IP ( mask );*/
#ifdef ESP8266
	DEBUG_PRINT("--");
	WiFiconnect(true);
#else
	// initialize the ethernet device and start listening for clients
	ETHERNE.begin(mymac, ip, gw, subnet);

#endif
	w5100server.begin();

    // save the values
    IP2Byte ( ETHERNE.localIP(), myip );
    IP2Byte ( ETHERNE.gatewayIP(), gwip );
    IP2Byte ( ETHERNE.gatewayIP(), dnsip );
    IP2Byte ( ETHERNE.subnetMask(), netmask );

    // print debug values
    printIPConfig();

    return true;
}
#ifdef ESP8266
bool EtherCardW5100::WiFiconnect(bool isStatic)
{
	//if (WiFi.status() != WL_CONNECTED)
	{  
		DEBUG_PRINTLN("Wait WIFI...");
		
		while (millis() < 40000) delay(2);
		WiFi.begin(SSID, PASSWORD);// , my_ip, dns_ip, gw_ip);
		byte netmask[4] = { 255,255,255,0 };
		byte MyIp[4] = { 192,168,1,211 };
		byte MyGate[4] = { 192,168,1,1 };
		
		if(isStatic) WiFi.config(MyIp, MyGate, netmask);
		DEBUG_PRINT("\nConnecting to "); DEBUG_PRINTLN(SSID);
		uint8_t i = 0;
		while (WiFi.status() != WL_CONNECTED && i++ < 1000) { yield(); delay(100); DEBUG_PRINT('.'); }
		if (i == 101) {
			DEBUG_PRINT("Could not connect to"); DEBUG_PRINTLN(SSID);
			return false;
		}
		
		DEBUG_PRINT("Server started IP="); DEBUG_PRINTLN(WiFi.localIP());
		return true;
	}
}
#endif

bool EtherCardW5100::dhcpSetup ( const char *hname, bool fromRam )
{
    using_dhcp = true;

    // initialize the ethernet device
    // TODO - Ignore the name for now - need to use an extension for the standard Arduino ETHERNE library to implement this
#ifndef ESP8266
	if ( ETHERNE.begin ( mymac ) == 0 )
#else
	if( WiFiconnect(false)==false)
#endif
		return false;

    // start listening for clients
    w5100server.begin();

    // save the values
    IP2Byte ( ETHERNE.localIP(), myip );
    IP2Byte ( ETHERNE.gatewayIP(), gwip );
  //  IP2Byte ( ETHERNE.dnsServerIP(), dnsip );
    IP2Byte ( ETHERNE.subnetMask(), netmask );

    // print debug values
    printIPConfig();

    return true;
    /*
    if (hname != NULL) {
    	if (fromRam) {
    		strncpy(hostname, hname, DHCP_HOSTNAME_MAX_LEN);
    	}
    	else {
    		strncpy_P(hostname, hname, DHCP_HOSTNAME_MAX_LEN);
    	}
    }
    else {
    	// Set a unique hostname, use Arduino-?? with last octet of mac address
    	hostname[8] = toAsciiHex(mymac[5] >> 4);
    	hostname[9] = toAsciiHex(mymac[5]);
    }

    dhcpState = DHCP_STATE_INIT;
    uint16_t start = millis();

    while (dhcpState != DHCP_STATE_BOUND && uint16_t(millis()) - start < 60000) {
    	if (isLinkUp()) DhcpStateMachine(packetReceive());
    }
    updateBroadcastAddress();
    delaycnt = 0;
    return dhcpState == DHCP_STATE_BOUND;
    */
}

/// <summary>
/// Print the current network configuration to the serial port
/// </summary>
void EtherCardW5100::printIPConfig()
{
#ifdef SERIAL_DEBUG
    DEBUG_PRINT ( F ( "Config:     " ) );
    DEBUG_PRINTLN ( using_dhcp ? F ( "DHCP" ) : F ( "Static IP" ) );
	DEBUG_PRINT(F("MAC:        "));
	DEBUG_PRINTLN ( F("TODO") );		// TODO

    printIp ( F ( "Local IP:   " ), myip );
    printIp ( F ( "Gateway IP: " ), gwip );
    printIp ( F ( "DNS IP:     " ), dnsip );
    printIp ( F ( "Netmask:    " ), netmask);

	DEBUG_PRINT(F("Port:       "));

	DEBUG_PRINTLN(F("TODO"));		// TODO
#endif
}

uint16_t EtherCardW5100::packetLoop ( uint16_t plen )
{
    // listen for incoming clients
    /*ETHERNEC*/ w5100client = w5100server.available();
    int i = 0;

    if ( w5100client )
    {
        // set all bytes in the buffer to 0 - add a
        // byte to simulate space for the TCP header
        memset ( buffer, 0, ETHER_BUFFER_SIZE );
        memset ( buffer, ' ', TCP_OFFSET );
        i = TCP_OFFSET; // add a space for TCP offset

        while ( w5100client.connected() && ( i < ETHER_BUFFER_SIZE ) )
        {
            if ( w5100client.available() )
				buffer[i] = w5100client.read();
            i++;
        } 
		DEBUG_PRINTLN(i); DEBUG_PRINTLN("http req");
    //	w5100client.print("wait....\0");
        return TCP_OFFSET;
    }
    else
        return 0;
}

void EtherCardW5100::ntpRequest ( uint8_t *nt_pip, uint8_t srcport )
{
    // set all bytes in the buffer to 0
    memset ( buffer, 0, ETHER_BUFFER_SIZE );

    // Initialize values needed to form NTP request
    buffer[0] = 0b11100011;    // LI, Version, Mode
    buffer[1] = 0;             // Stratum, or type of clock
    buffer[2] = 6;             // Polling Interval
    buffer[3] = 0xEC;          // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    buffer[12] = 49;
    buffer[13] = 0x4E;
    buffer[14] = 49;
    buffer[15] = 52;

    ntpip = IPAddress ( nt_pip[0], nt_pip[1], nt_pip[2], nt_pip[3] );

    // all NTP fields have been given values, now you can send a packet requesting a timestamp:
    w5100udp.begin ( srcport );			// TODO - should this be in begin()?
    w5100udp.beginPacket ( ntpip, 123 );	// NTP requests are to port 123
    w5100udp.write ( buffer, NTP_PACKET_SIZE );
    w5100udp.endPacket();
}

///  TODO START HERE #########################################################

/// <summary>
/// Ethercard.cpp - Process network time protocol response
/// </summary>
/// <param name="time">Pointer to integer to hold result</param>
/// <param name="dstport_l">Destination port to expect response. Set to zero to accept on any port</param>
/// <returns>True (1) on success</returns>
byte EtherCardW5100::ntpProcessAnswer ( uint32_t *time, byte dstport_l )
{
    int packetSize = w5100udp.parsePacket();
    if ( packetSize )
    {
        // check the packet is from the correct timeserver IP and port
        if ( w5100udp.remotePort() != 123 || w5100udp.remoteIP() != ntpip )
            return 0;

        //the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long.
        w5100udp.read ( buffer, packetSize );
        ( ( byte* ) time ) [3] = buffer[40];
        ( ( byte* ) time ) [2] = buffer[41];
        ( ( byte* ) time ) [1] = buffer[42];
        ( ( byte* ) time ) [0] = buffer[43];

        return 1;
    }
    return 0;
}

//=================================================================================
// Webutil.cpp
//=================================================================================

/// <summary>
/// webutil.cpp - copies an IP address
/// There is no check of source or destination size. Ensure both are 4 bytes
/// </summary>
/// <param name="dst">dst Pointer to the 4 byte destination</param>
/// <param name="src">src Pointer to the 4 byte source</param>
void EtherCardW5100::copyIp ( byte *dst, const byte *src )
{
    memcpy ( dst, src, 4 );
}

/// <summary>
/// webutil.cpp - copies a hardware address
/// There is no check of source or destination size. Ensure both are 6 bytes
/// </summary>
/// <param name="dst">dst Pointer to the 6 byte destination</param>
/// <param name="src">src Pointer to the 6 byte destination</param>
void EtherCardW5100::copyMac ( byte *dst, const byte *src )
{
    memcpy ( dst, src, 6 );
}

void EtherCardW5100::printIp ( const char* msg, const uint8_t *buf )
{
    Serial.print ( msg );
    EtherCardW5100::printIp ( buf );
    Serial.println();
}

void EtherCardW5100::printIp ( const __FlashStringHelper *ifsh, const uint8_t *buf )
{
    Serial.print ( ifsh );
    EtherCardW5100::printIp ( buf );
    Serial.println();
}

void EtherCardW5100::printIp ( const uint8_t *buf )
{
    for ( uint8_t i = 0; i < 4; ++i )
    {
        Serial.print ( buf[i], DEC );
        if ( i < 3 )
            Serial.print ( '.' );
    }
}

/// <summary>
/// webutil.cpp - search for a string of the form key=value in a string that looks like q?xyz=abc&uvw=defgh HTTP/1.1\\r\\n
/// Ensure strbuf has memory allocated of at least maxlen + 1 (to accomodate result plus terminating null)
/// </summary>
/// <param name="str">Pointer to the null terminated string to search</param>
/// <param name="strbuf">Pointer to buffer to hold null terminated result string</param>
/// <param name="maxlen">Maximum length of result</param>
/// <param name="key">Pointer to null terminated string holding the key to search for</param>
/// <returns>Length of the value. 0 if not found</returns>
byte EtherCardW5100::findKeyVal ( const char *str, char *strbuf, byte maxlen, const char *key )
{
    byte found = 0;
    byte i = 0;
    const char *kp;
    kp = key;
    while ( *str &&  *str != ' ' && *str != '\n' && found == 0 )
    {
        if ( *str == *kp )
        {
            kp++;
            if ( *kp == '\0' )
            {
                str++;
                kp = key;
                if ( *str == '=' )
                {
                    found = 1;
                }
            }
        }
        else
        {
            kp = key;
        }
        str++;
    }
    if ( found == 1 )
    {
        // copy the value to a buffer and terminate it with '\0'
        while ( *str &&  *str != ' ' && *str != '\n' && *str != '&' && i<maxlen - 1 )
        {
            *strbuf = *str;
            i++;
            str++;
            strbuf++;
        }
        *strbuf = '\0';
    }
    // return the length of the value
    return ( i );
}

/// <summary>
/// webutil.cpp - decode a URL string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
/// </summary>
/// <param name="urlbuf">Pointer to the null terminated URL (urlbuf is modified)</param>
void EtherCardW5100::urlDecode ( char *urlbuf )
{
    char c;
    char *dst = urlbuf;
    while ( ( c = *urlbuf ) != 0 )
    {
        if ( c == '+' ) c = ' ';
        if ( c == '%' )
        {
            c = *++urlbuf;
            c = ( h2int ( c ) << 4 ) | h2int ( *++urlbuf );
        }
        *dst++ = c;
        urlbuf++;
    }
    *dst = '\0';
}

/// <summary>
/// webutil.cpp - encode a URL, replacing illegal charaters like ' '.
/// There must be enough space in urlbuf. In the worst case that is 3 times the length of str
/// </summary>
/// <param name="str">str Pointer to the null terminated string to encode</param>
/// <param name="urlbuf">urlbuf Pointer to a buffer to contain the null terminated encoded URL</param>
void EtherCardW5100::urlEncode ( char *str, char *urlbuf )
{
    char c;
    while ( ( c = *str ) != 0 )
    {
        if ( c == ' ' || isalnum ( c ) )
        {
            if ( c == ' ' )
            {
                c = '+';
            }
            *urlbuf = c;
            str++;
            urlbuf++;
            continue;
        }
        *urlbuf = '%';
        urlbuf++;
        int2h ( c, urlbuf );
        urlbuf++;
        urlbuf++;
        str++;
    }
    *urlbuf = '\0';
}

/// <summary>
/// webutil.cpp - Convert an IP address from dotted decimal formated string to 4 bytes
/// </summary>
/// <param name="bytestr">Pointer to the 4 byte array to store IP address</param>
/// <param name="str">Pointer to string to parse</param>
/// <returns>0 on success</returns>
byte EtherCardW5100::parseIp ( byte *bytestr, char *str )
{
    char *sptr;
    byte i = 0;
    sptr = NULL;
    while ( i<4 )
    {
        bytestr[i] = 0;
        i++;
    }
    i = 0;
    while ( *str && i<4 )
    {
        // if a number then start
        if ( sptr == NULL && isdigit ( *str ) )
        {
            sptr = str;
        }
        if ( *str == '.' )
        {
            *str = '\0';
            bytestr[i] = ( atoi ( sptr ) & 0xff );
            i++;
            sptr = NULL;
        }
        str++;
    }
    *str = '\0';
    if ( i == 3 )
    {
        bytestr[i] = ( atoi ( sptr ) & 0xff );
        return ( 0 );
    }
    return ( 1 );
}

/// <summary>
/// webutil.cpp - take a byte string and convert it to a human readable display string
/// </summary>
/// <param name="resultstr">Pointer to a buffer to hold the resulting null terminated string</param>
/// <param name="bytestr">Pointer to the byte array containing the address to convert</param>
/// <param name="len">Length of the array (4 for IP address, 6 for hardware (MAC) address)</param>
/// <param name="separator">Delimiter character (typically '.' for IP address and ':' for hardware (MAC) address)</param>
/// <param name="base">Base for numerical representation (typically 10 for IP address and 16 for hardware (MAC) address</param>
void EtherCardW5100::makeNetStr ( char *resultstr, byte *bytestr, byte len, char separator, byte base )
{
    byte i = 0;
    byte j = 0;
    while ( i<len )
    {
        itoa ( ( int ) bytestr[i], &resultstr[j], base );
        // search end of str:
        while ( resultstr[j] )
        {
            j++;
        }
        resultstr[j] = separator;
        j++;
        i++;
    }
    j--;
    resultstr[j] = '\0';
}

//=================================================================================
// enc28j60.cpp
//=================================================================================

/// <summary>
/// enc28j60.cpp - copy recieved packets to data buffer
/// Data buffer is shared by recieve and transmit functions
/// </summary>
/// <returns>Size of recieved data</returns>
uint16_t EtherCardW5100::packetReceive()
{
    // do nothing - handle everything in packetloop in this wrapper class
    return 0;
}

//=================================================================================
// tcpip.cpp
//=================================================================================

/// <summary>
/// tcpip.cpp - send a response to a HTTP request
/// </summary>
/// <param name="dlen">Size of the HTTP (TCP) payload</param>
 void EtherCardW5100::httpServerReply ( uint16_t dlen )
{
    // ignore dlen - just add a null termination
    // to the buffer and print it out to the client
    buffer[bfill.position() + TCP_OFFSET] = '\0';
    w5100client.print ( ( char* ) bfill.buffer() );
	//w5100client.print("prova1.............\0");

	DEBUG_PRINT(">>>"); DEBUG_PRINTLN((int)bfill.position());
    // close the connection:
    delay ( 1 ); // give the web browser time to receive the data
    w5100client.stop();
}

/// <summary>
/// tcpip.cpp - send a response to a HTTP request
/// </summary>
/// <param name="dlen">Size of the HTTP (TCP) payload</param>
/// <param name="flags">TCP flags</param>
void EtherCardW5100::httpServerReply_with_flags ( uint16_t dlen, uint8_t flags )
{
    // Need functionality to handle:
    //     - TCP_FLAGS_ACK_V
    //     - TCP_FLAGS_FIN_V
    //     - TCP_FLAGS_ACK_V|TCP_FLAGS_FIN_V

    // Same as above - ignore dlen & just add a null termination and print it out to the client
    buffer[bfill.position() + TCP_OFFSET] = '\0';
	
	for (int j = 0; j <= bfill.position() + TCP_OFFSET; j++)DEBUG_PRINT( (char)*(bfill.buffer() + j));
	
   w5100client.print ( ( char* ) bfill.buffer() );
	DEBUG_PRINT(">>"); DEBUG_PRINTLN(bfill.position());
  delay(10); // give the web browser time to receive the data

    if ( flags&TCP_FLAGS_FIN_V != 0 ) // final packet in the stream
    {
        // close the connection:
        w5100client.stop();
    }

    /* // Original Code from tcpip.cpp
    for(byte i=0;i<=dup;i++)
    {
    set_seq();
    gPB[TCP_FLAGS_P] = flags; // final packet
    make_tcp_ack_with_data_noflags(dlen); // send data
    }
    //if(keepseq) {}
    //else {SEQ=SEQ+dlen;}
    SEQ=SEQ+dlen;
    */
}

/// <summary>
/// tcpip.cpp - acknowledge TCP message
/// </summary>
void EtherCardW5100::httpServerReplyAck()
{
    /*
    make_tcp_ack_from_any(info_data_len,0); // send ack for http get
    get_seq(); //get the sequence number of packets after an ack from GET
    */
}

/// <summary>
/// tcpip.cpp - prepare HTTP request
/// Request sent in main packetloop
/// </summary>
/// <param name="urlbuf">Pointer to c-string URL folder</param>
/// <param name="urlbuf_varpart">Pointer to c-string URL file</param>
/// <param name="hoststr">Pointer to c-string hostname</param>
/// <param name="callback">Pointer to callback function to handle response</param>
void EtherCardW5100::browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) )
{
    browseUrl ( urlbuf, urlbuf_varpart, hoststr, PSTR ( "Accept: text/html" ), callback );
}

/// <summary>
/// tcpip.cpp - prepare HTTP request
/// Request sent in main packetloop
/// </summary>
/// <param name="urlbuf">Pointer to c-string URL folder</param>
/// <param name="urlbuf_varpart">Pointer to c-string URL file</param>
/// <param name="hoststr">Pointer to c-string hostname</param>
/// <param name="additionalheaderline">additionalheaderline Pointer to c-string with additional HTTP header info</param>
/// <param name="callback">callback Pointer to callback function to handle response</param>
void EtherCardW5100::browseUrl ( const char *urlbuf, const char *urlbuf_varpart, const char *hoststr, const char *additionalheaderline, void ( *callback ) ( uint8_t, uint16_t, uint16_t ) )
{
    client_urlbuf = urlbuf;
    client_urlbuf_var = urlbuf_varpart;
    client_hoststr = hoststr;
    client_additionalheaderline = additionalheaderline;
    client_postval = 0;
    client_browser_cb = callback;
	DEBUG_PRINT("hoststring=");
	for (int i = 0; i < strlen(hoststr); i++)DEBUG_PRINT(hoststr[i]);

	//    client_tcp_datafill_cb = &www_client_internal_datafill_cb;

	// check cb pointer is 'real' (non zero)
	
	if (!client_browser_cb)
		return;

	// fill the buffer
	uint16_t len;// = (*www_client_internal_datafill_cb) (www_fd);
	buffer[bfill.position() + TCP_OFFSET] = '\0';
	
	DEBUG_PRINT(F("Browse URL: ")); DEBUG_PRINTLN(bfill.position());
	/*
	for (uint16_t c = TCP_OFFSET; c < bfill.position() + TCP_OFFSET; c++)
	{
		// print readable ascii characters
		if (buffer[c] >= 0x08 && buffer[c] <= 0x0D)
		{
			DEBUG_PRINT((char)buffer[c]); // DEBUG_PRINT ( F ( ", " ) );		// substitute a space so less rows in serial output
		}
		else if (buffer[c] > 0x1F)
		{
			DEBUG_PRINT((char)buffer[c]);
		}
	}*/
	


	// close any connection before send a new request, to free the socket
	w5100client.stop();
	
	delay(500);
	// send the request
	if (outgoing_client.connect(*hoststr, hisport))
	{
		DEBUG_PRINTLN(hisport);
		// send the HTTP GET request:
		outgoing_client.print((char*)bfill.buffer());
		outgoing_client.println();
		DEBUG_PRINT(F("Browse URL: sent to "));
		DEBUG_PRINT(hoststr);
		DEBUG_PRINT(F(" port "));
		DEBUG_PRINT(hisport);
		DEBUG_PRINTLN(F("(OK)"));
		outgoing_client_state = TCP_ESTABLISHED;
	}
	else
	{
		DEBUG_PRINTLN(F("Browse URL: failed (could not connect)"));
		outgoing_client_state = TCP_CLOSED;
	}
	DEBUG_PRINTLN("Browse URL");
    /*
    // if there's a successful connection:
    char server[] = "www.arduino.cc";
    if ( client.connect (client_hoststr, 80 ) )
    {
    Serial.println ( "connecting..." );
    // send the HTTP PUT request:
    client.println ( "GET /latest.txt HTTP/1.1" );
    client.println ( "Host: www.arduino.cc" );
    client.println ( "User-Agent: arduino-ethernet" );
    client.println ( "Connection: close" );
    client.println();
    }
    else
    {
    // if you couldn't make a connection:
    Serial.println ( "connection failed" );
    }
    */
}

/// <summary>
/// tcp.cpp - send ping
/// </summary>
/// <param name="destip">Pointer to 4 byte destination IP address</param>
void EtherCardW5100::clientIcmpRequest ( const uint8_t *destip )
{
#ifdef MY_PING
    IPAddress pingAddr ( destip[0], destip[1], destip[2], destip[3] ); // ip address to ping

    // note - asynchStart will return false if we couldn't even send a ping
    if ( !ping.asyncStart ( pingAddr, 3, pingResult ) )
    {
        DEBUG_PRINT ( F ( "Failed to send ping request - status=" ) );
        DEBUG_PRINTLN ( ( int ) pingResult.status );
    }
#endif
}

/// <summary>
/// tcp.cpp - check for ping response
/// </summary>
/// <param name="ip_monitoredhost">Pointer to 4 byte IP address of host to check</param>
/// <returns>True (1) if ping response from specified host</returns>
uint8_t EtherCardW5100::packetLoopIcmpCheckReply ( const uint8_t *ip_monitoredhost )
{
#ifdef MY_PING
    if ( ping.asyncComplete ( pingResult ) )
    {
        if ( pingResult.status != SUCCESS )
        {
            // failure... but whyyyy?
            DEBUG_PRINT ( F ( "Ping failed - status=" ) );
            DEBUG_PRINTLN ( pingResult.status );
            return 0;
        }
        else
        {
            // huzzah
            DEBUG_PRINT ( F ( "Ping succeeded - reply " ) );
            DEBUG_PRINT ( pingResult.data.seq );
            DEBUG_PRINT ( F ( " from " ) );
            DEBUG_PRINT ( pingResult.addr[0] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( pingResult.addr[1] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( pingResult.addr[2] );
            DEBUG_PRINT ( F ( "." ) );
            DEBUG_PRINT ( pingResult.addr[3] );
            DEBUG_PRINT ( F ( " bytes=" ) );
            DEBUG_PRINT ( REQ_DATASIZE );
            DEBUG_PRINT ( F ( " time=" ) );
            DEBUG_PRINT ( millis() - pingResult.data.time );
            DEBUG_PRINT ( F ( " TTL=" ) );
            DEBUG_PRINT ( pingResult.ttl );

            // check the address
            if (	pingResult.addr[0] == ip_monitoredhost[0] &&
                    pingResult.addr[1] == ip_monitoredhost[1] &&
                    pingResult.addr[2] == ip_monitoredhost[2] &&
                    pingResult.addr[3] == ip_monitoredhost[3] )
                return 1;
            else
            {
                DEBUG_PRINT ( F ( " (received reply from wrong host)" ) );
                return 0;
            }
            DEBUG_PRINTLN ( F ( "" ) );
        }
    }
#else
	return 0;
#endif
}

/*
/// <summary>
/// get the sequence number of packets after an ack from GET
/// </summary>
static void get_seq()
{
    SEQ = ( ( ( unsigned long ) gPB[TCP_SEQ_H_P] * 256 + gPB[TCP_SEQ_H_P + 1] ) * 256 + gPB[TCP_SEQ_H_P + 2] ) * 256 + gPB[TCP_SEQ_H_P + 3];
} //thanks to mstuetz for the missing (unsigned long)

/// <summary>
/// Set the correct sequence number and calculate the next with the length of current packet
/// </summary>
static void set_seq()
{
    gPB[TCP_SEQ_H_P] = ( SEQ & 0xff000000 ) >> 24;
    gPB[TCP_SEQ_H_P + 1] = ( SEQ & 0xff0000 ) >> 16;
    gPB[TCP_SEQ_H_P + 2] = ( SEQ & 0xff00 ) >> 8;
    gPB[TCP_SEQ_H_P + 3] = ( SEQ & 0xff );
}
*/
//=================================================================================
// dns.cpp
//=================================================================================

/// /// <summary>
/// dns.cpp - perform DNS lookup.
/// Use during setup, as this discards all incoming requests until it returns.
/// Result is stored in hisip member.
/// </summary>
/// <param name="name">Host name to lookup</param>
/// <param name="fromRam">Set true to look up cached name. Default = false</param>
/// <returns>True on success. </returns>
bool EtherCardW5100::dnsLookup ( const char* name, bool fromRam )
{
#ifdef ESP8266  //use WiFi.hostByName
	IPAddress serverIP(0, 0, 0, 0);
	DEBUG_PRINTLN("Starting LOOKUP");
#ifndef ESP8266 //for ethernet shield

	dns_client.begin(ETHERNE.gatewayIP()); //..................for dns.h
	int result = dns_client.HostByName(name, serverIP);

#endif

	dns_client.begin(ETHERNE.gatewayIP());
	int result = ETHERNE.hostByName(name, serverIP);

	DEBUG_PRINT(F("DNS lookup "));
	DEBUG_PRINT(name);
	DEBUG_PRINT(F(" is "));
	DEBUG_PRINT(serverIP[0]);
	DEBUG_PRINT(F("."));
	DEBUG_PRINT(serverIP[1]);
	DEBUG_PRINT(F("."));
	DEBUG_PRINT(serverIP[2]);
	DEBUG_PRINT(F("."));
	DEBUG_PRINT(serverIP[3]);

	for (uint8_t i = 0; i < 4; i++)
		hisip[i] = serverIP[i];

	if (result == 1)
	{
		DEBUG_PRINTLN(F(" (OK)"));
		return true;
	}
	else
	{
		DEBUG_PRINTLN(F(" (failed)"));
		return false;
	}
#else //original routine
    uint16_t start = millis();

    while (!isLinkUp())
    {
    	if (uint16_t(millis()) - start >= 30000)
    		return false; //timeout waiting for link
    }
    while (clientWaitingDns())
    {
    	packetLoop(packetReceive());
    	if (uint16_t(millis()) - start >= 30000)
    		return false; //timeout waiting for gateway ARP
    }

    memset(hisip, 0, 4);
    dnsRequest(name, fromRam);

    start = millis();
    while (hisip[0] == 0) {
    	if (uint16_t(millis()) - start >= 30000)
    		return false; //timout waiting for dns response
    	word len = packetReceive();
    	if (len > 0 && packetLoop(len) == 0) //packet not handled by tcp/ip packet loop
    		if (checkForDnsAnswer(len))
    			return false; //DNS response recieved with error
    }
#endif
    return true;
}









