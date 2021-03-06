/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Fri Nov 21 13:10:08 2003
 */
/* Compiler settings for F:\CYJ\INFOCARD\IPReceiveSvr\IPRecSvr.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

extern "C" const IID IID_IBufPacket = {0x9E99F19B,0x1B5D,0x46b0,{0x8B,0x01,0x7A,0x03,0xCE,0x5A,0xEC,0x5F}};


extern "C" const IID IID_IFileObject = {0xC60599AC,0x3C99,0x4ef8,{0xB5,0xB6,0x90,0xC4,0x9B,0xB0,0x4C,0x06}};


extern "C" const IID IID_IDVBFileReceiver = {0x77035EDB,0x54B1,0x46EC,{0x89,0xB8,0x24,0xA0,0x78,0x70,0x80,0xB9}};


extern "C" const IID IID_IIPFileMendHelper = {0xDA4BCA9C,0x66B3,0x42EC,{0xAE,0x8D,0x3B,0x36,0xC0,0x77,0xC4,0xEA}};


extern "C" const IID LIBID_IPRECSVRLib = {0x087C147B,0xBB1C,0x4667,{0x9F,0x6B,0xC1,0xC3,0x04,0xF2,0x5B,0xF2}};


extern "C" const CLSID CLSID_BufPacket = {0x402FADBF,0x3EB7,0x4a31,{0x93,0x3E,0x37,0x36,0x75,0x99,0x52,0xCA}};


extern "C" const CLSID CLSID_FileObject = {0x94FA790D,0x2B3B,0x4df3,{0x92,0xE4,0x8E,0x37,0xB8,0xE9,0x97,0x67}};


extern "C" const IID DIID__IDVBFileReceiverEvents = {0xF167E830,0x8CD2,0x4B85,{0x8A,0xED,0x70,0x89,0x45,0x95,0x9F,0x1D}};


extern "C" const CLSID CLSID_DVBFileReceiver = {0x64A5878C,0x9EA2,0x4898,{0xAB,0x46,0xFF,0xD3,0x18,0x33,0x5F,0x55}};

extern "C" const CLSID CLSID_IPFileMendHelper = {0x0F5598B4,0x62F5,0x4C67,{0xA3,0x61,0x1B,0xE9,0x65,0xE0,0x31,0x88}};
