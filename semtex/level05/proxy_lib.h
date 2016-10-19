/*************************************************
 * Simple socks5 (Maybe soon to be http/socks4?) 
 * proxy library, written by Chris Stufflebeam.
 ************************************************/
#ifndef PROXY_LIB_H
#define PROXY_LIB_H

#include <exception>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <cstdarg>
#include <fstream>


//*************Macro Definitions****************************/
#define THROW_EXCEPTION( except )		\
  throw new except(__FILE__,__LINE__);

#define RETURN_FORMATTED_ERROR( msg )                       \
  char* cret=new char[strlen(m_sFile)+strlen(msg)+20];      \
  sprintf(cret,"%s:%d: %s",m_sFile,m_iLine, msg);           \
  std::string ret=cret;                                     \
  delete[] cret;                                            \
  return ret.c_str();

//**********************************************************/

#define SOCKS_CMD_CONNECT                        0x01
#define SOCKS_CMD_BIND                           0x02
#define SOCKS_CMD_UDP_ASSOCIATE                  0x03

#define SOCKS_ATTYP_IPV4                         0x01
#define SOCKS_ATTYP_DOMAIN                       0x03
#define SOCKS_ATTYP_IPV6                         0x04

#define SOCKS_METHOD_NOAUTH                      0x00
#define SOCKS_METHOD_GSSAPI                      0x01
#define SOCKS_METHOD_USERNAME_PASSSWORD          0x02
#define SOCKS_METHOD_NO_ACCEPTABLE_METHODS       0xFF

#define SOCKS_REPLY_SUCCEED                      0x00
#define SOCKS_REPLY_GENERAL_FAIL                 0x01
#define SOCKS_REPLY_DENIED                       0x02
#define SOCKS_REPLY_NETWORK_UNREACHABLE          0x03
#define SOCKS_REPLY_HOST_UNREACHABLE             0x04
#define SOCKS_REPLY_CONNECTION_REFUSED           0x05
#define SOCKS_REPLY_TTL_EXPIRED                  0x06
#define SOCKS_REPLY_COMMAND_NOT_SUPPORTED        0x07
#define SOCKS_REPLY_ADDRESS_TYPE_NOT_SUPPORTED   0x08
#define SOCKS_REPLY_UNASSIGNED                   0x09//-0xff

#define SOCKS_REPLY_VALID_PASS                   0x00

class CProxyOut {
 public:
  CProxyOut();
  CProxyOut(char*,unsigned int,unsigned int=0x05);
  ~CProxyOut();
  void SetProxy(const char*,unsigned int,unsigned int=0x05);
  void ConnectToHost(char*, unsigned int);
  int Read(char*,unsigned int);
  int Send(char*,unsigned int);
  void SetUserPass(char*,char*);
 private:
  void Reset();
  void info(const char* m,...);

  sockaddr_in m_cProxyAddr;
  hostent* m_cProxyEnt;
  int m_iSocket;

  char m_sDestinationHost[1024];
  unsigned int m_iDestinationPort;

  char m_sProxyHost[1024];
  unsigned int m_iProxyPort;
  
  char m_sUsername[31];
  char m_sPassword[31];
  unsigned int m_iSocksVersion;
}; 


class CProxyException: public std::exception {
 public:
  CProxyException(const char* file, unsigned line);
  CProxyException();

 protected:
  const char* m_sFile;
  unsigned m_iLine;
};


class ErrorInvalidProxy: public CProxyException {
 public:
  ErrorInvalidProxy(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Invalid Proxy Specified.");}
};

class ErrorBadSocket: public CProxyException {
 public:
  ErrorBadSocket(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Socket creation failed or invalid socket.");}
};


class ErrorConnectionToProxyFailed: public CProxyException {
 public:
  ErrorConnectionToProxyFailed(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Connection to proxy server failed.");}
};

class ErrorBadProxyResponse : public CProxyException {
 public:
  ErrorBadProxyResponse(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Proxy responsed with invalid data.");}
};

class ErrorProxyNotSupported : public CProxyException {
 public:
  ErrorProxyNotSupported(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Proxy responded with 0xff - methods not supported.");}
};

class ErrorIO : public CProxyException {
 public:
  ErrorIO(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Input/output error on call to Recv/send.");}
};

class ErrorConnectionReset : public CProxyException {
 public:
  ErrorConnectionReset(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Connection was reset.");}
};

class ErrorInvalidUserPass : public CProxyException {
 public:
  ErrorInvalidUserPass(const char*f,unsigned l) : CProxyException(f,l) { }
  virtual const char* what() const throw() {RETURN_FORMATTED_ERROR("Username or password rejected by proxy.");}
};


#endif  /* PROXY_LIB_H */
 
