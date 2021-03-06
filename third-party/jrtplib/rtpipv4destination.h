/*

  This file is a part of JRTPLIB
  Copyright (c) 1999-2004 Jori Liesenborgs

  Contact: jori@lumumba.luc.ac.be

  This library was developed at the "Expertisecentrum Digitale Media"
  (http://www.edm.luc.ac.be), a research center of the "Limburgs Universitair
  Centrum" (http://www.luc.ac.be). The library is based upon work done for 
  my thesis at the School for Knowledge Technology (Belgium/The Netherlands).

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef RTPIPV4DESTINATION

#define RTPIPV4DESTINATION

#include "rtpconfig.h"
#ifndef WIN32
	#include <netinet/in.h>
#else
	#include <winsock2.h>
#endif // WIN32
#ifdef RTPDEBUG
	#include <stdio.h>
	#include <string>
#endif // RTPDEBUG

class RTPIPv4Destination
{
public:
	// (nbo = network byte order, hbo = host byte order)
	
	RTPIPv4Destination(u_int32_t ip,u_int16_t rtpportbase)					{ ipaddr_hbo = ip; ipaddr_nbo = htonl(ip); rtpport_nbo = htons(rtpportbase); rtcpport_nbo = htons(rtpportbase+1); }
	u_int32_t GetIP_HBO() const								{ return ipaddr_hbo; }
	u_int32_t GetIP_NBO() const								{ return ipaddr_nbo; }
	u_int16_t GetRTPPort_NBO() const							{ return rtpport_nbo; }
	u_int16_t GetRTCPPort_NBO() const							{ return rtcpport_nbo; }
	bool operator==(const RTPIPv4Destination &src) const		{ if (src.ipaddr_nbo == ipaddr_nbo && src.rtpport_nbo == rtpport_nbo) return true; return false; } // NOTE: I only check IP and portbase
#ifdef RTPDEBUG
	std::string GetDestinationString() const;
#endif // RTPDEBUG
private:
	u_int32_t ipaddr_hbo;
	u_int32_t ipaddr_nbo;
	u_int16_t rtpport_nbo;
	u_int16_t rtcpport_nbo;
};

#ifdef RTPDEBUG
inline std::string RTPIPv4Destination::GetDestinationString() const
{
	char str[1024];
	u_int32_t ip = ipaddr_hbo;
	u_int16_t portbase = ntohs(rtpport_nbo);
	
	sprintf(str,"%d.%d.%d.%d:%d",(int)((ip>>24)&0xFF),(int)((ip>>16)&0xFF),(int)((ip>>8)&0xFF),(int)(ip&0xFF),(int)(portbase));
	return std::string(str);
}
#endif // RTPDEBUG

#endif // RTPIPV4DESTINATION
