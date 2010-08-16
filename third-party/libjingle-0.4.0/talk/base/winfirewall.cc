/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products 
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <comdef.h>
#include "netfw.h"
#include "winfirewall.h"

#define RELEASE(lpUnk) do \
	{ if ((lpUnk) != NULL) { (lpUnk)->Release(); (lpUnk) = NULL; } } while (0)

#ifndef CLSID_NetFwMgr
#define MDEF_CLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const CLSID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	MDEF_CLSID(CLSID_NetFwMgr, 0x304ce942, 0x6e39, 0x40d8, 0x94, 0x3a, 0xb9, 0x13, 0xc4, 0x0c, 0x9c, 0xd4);
	MDEF_CLSID(IID_INetFwMgr, 0xf7898af5, 0xcac4, 0x4632, 0xa2, 0xec, 0xda ,0x06, 0xe5, 0x11, 0x1a, 0xf2);
#endif

#ifndef CLSID_NetFwAuthorizedApplication
#define MDEF_CLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const CLSID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	MDEF_CLSID(CLSID_NetFwAuthorizedApplication, 0x304ce942, 0x6e39, 0x40d8, 0x94, 0x3a, 0xb9, 0x13, 0xc4, 0x0c, 0x9c, 0xd4);
	MDEF_CLSID(IID_INetFwAuthorizedApplication, 0xf7898af5, 0xcac4, 0x4632, 0xa2, 0xec, 0xda ,0x06, 0xe5, 0x11, 0x1a, 0xf2);
#endif

char *CStrFromBSTR(int codePage, BSTR bstr)
{
    UINT len = SysStringLen(bstr);
    if( len > 0 )
    {
        size_t mblen = WideCharToMultiByte(codePage,
                0, bstr, len, NULL, 0, NULL, NULL);
        if( mblen > 0 )
        {
            char *buffer = (char *)CoTaskMemAlloc(mblen+1);
            ZeroMemory(buffer, mblen+1);
            if( WideCharToMultiByte(codePage, 0, bstr, len, buffer, mblen, NULL, NULL) )
            {
                buffer[mblen] = '\0';
                return buffer;
            }
        }
    }
    return NULL;
};

BSTR BSTRFromCStr(int codePage, const char *s)
{
    int wideLen = MultiByteToWideChar(codePage, 0, s, -1, NULL, 0);
    if( wideLen )
    {
        WCHAR* wideStr = (WCHAR*)CoTaskMemAlloc(wideLen*sizeof(WCHAR));
        if( NULL != wideStr )
        {
            BSTR bstr;

            ZeroMemory(wideStr, wideLen*sizeof(WCHAR));
            MultiByteToWideChar(codePage, 0, s, -1, wideStr, wideLen);
            bstr = SysAllocString(wideStr);
            free(wideStr);

            return bstr;
        }
    }
    return NULL;
};

namespace talk_base {

//////////////////////////////////////////////////////////////////////
// WinFirewall
//////////////////////////////////////////////////////////////////////

WinFirewall::WinFirewall() : mgr_(NULL), policy_(NULL), profile_(NULL) {
}

WinFirewall::~WinFirewall() {
  Shutdown();
}

bool
WinFirewall::Initialize() {
  if (mgr_)
    return true;
//  Visual Studio
//  HRESULT hr = CoCreateInstance(__uuidof(NetFwMgr),
    HRESULT hr = CoCreateInstance(CLSID_NetFwMgr,
				0, CLSCTX_INPROC_SERVER,
//  Visual Studio
//                                __uuidof(INetFwMgr),
				IID_INetFwMgr,
                                reinterpret_cast<void **>(&mgr_));
  if (SUCCEEDED(hr) && (mgr_ != NULL))
    hr = mgr_->get_LocalPolicy(&policy_);
  if (SUCCEEDED(hr) && (policy_ != NULL))
    hr = policy_->get_CurrentProfile(&profile_);
  return SUCCEEDED(hr) && (profile_ != NULL);
}

void
WinFirewall::Shutdown() {
  RELEASE(profile_);
  RELEASE(policy_);
  RELEASE(mgr_);
}

bool
WinFirewall::Enabled() {
  if (!profile_)
    return false;

  VARIANT_BOOL fwEnabled = VARIANT_FALSE;
  profile_->get_FirewallEnabled(&fwEnabled);
  return (fwEnabled != VARIANT_FALSE);
}

bool
WinFirewall::Authorized(const char * filename, bool * known) {
  if (known) {
    *known = false;
  }

  if (!profile_)
    return false;

  VARIANT_BOOL fwEnabled = VARIANT_FALSE;
  BSTR bfilename = BSTRFromCStr(CP_ACP, filename);

  INetFwAuthorizedApplications * apps = NULL;
  HRESULT hr = profile_->get_AuthorizedApplications(&apps);
  if (SUCCEEDED(hr) && (apps != NULL)) {
    INetFwAuthorizedApplication * app = NULL;
    hr = apps->Item(bfilename, &app);
    if (SUCCEEDED(hr) && (app != NULL)) {
      hr = app->get_Enabled(&fwEnabled);
      app->Release();
      if (known) {
        *known = true;
      }
    } else if (hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
      // Unexpected error
    }
    apps->Release();
  }

  return (fwEnabled != VARIANT_FALSE);
}

bool
WinFirewall::AddApplication(const char * filename, const char * friendly_name,
                            bool authorized) {
  INetFwAuthorizedApplications * apps = NULL;
  HRESULT hr = profile_->get_AuthorizedApplications(&apps);
  if (SUCCEEDED(hr) && (apps != NULL)) {
    INetFwAuthorizedApplication * app = NULL;
//  Visual Studio
//  hr = CoCreateInstance(__uuidof(NetFwAuthorizedApplication),
    hr = CoCreateInstance(CLSID_NetFwAuthorizedApplication,
			  0, CLSCTX_INPROC_SERVER,
//  Visual Studio
//                         __uuidof(INetFwAuthorizedApplication),
			  IID_INetFwAuthorizedApplication,
                          reinterpret_cast<void **>(&app));
    if (SUCCEEDED(hr) && (app != NULL)) {
      BSTR bstr = BSTRFromCStr(CP_ACP, filename);

      hr = app->put_ProcessImageFileName(bstr);
//    FIXME!!!!!!!!!!
//    bstr = friendly_name;
      if (SUCCEEDED(hr))
        hr = app->put_Name(bstr);
      if (SUCCEEDED(hr))
        hr = app->put_Enabled(authorized ? VARIANT_TRUE : VARIANT_FALSE);
      if (SUCCEEDED(hr))
        hr = apps->Add(app);
      app->Release();
    }
    apps->Release();
  }
  return SUCCEEDED(hr);
}

} // namespace talk_base
