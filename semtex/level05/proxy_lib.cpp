#include "proxy_lib.h"

/*+----+----------+----------+
  |VER | NMETHODS | METHODS  |
  +----+----------+----------+
  | 1  |    1     | 1 to 255 |
  +----+----------+----------+*/
struct openRequest {
  u_char ver;
  u_char numMethods;
  u_char methods[255];
};


/* +----+--------+
   |VER | METHOD |
   +----+--------+
   | 1  |   1    |
   +----+--------+*/
struct openResponse {
  u_char ver;
  u_char method;
};


/*+----+-----+-------+------+----------+----------+
  |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
  +----+-----+-------+------+----------+----------+
  | 1  |  1  | X'00' |  1   | Variable |    2     |
  +----+-----+-------+------+----------+----------+*/
struct socksRequest {
  u_char ver;
  u_char command;
  u_char reserved;
  u_char attyp;
  u_char dst_addr[256];
  u_short dst_port;
};


/*+----+-----+-------+------+----------+----------+
  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
  +----+-----+-------+------+----------+----------+
  | 1  |  1  | X'00' |  1   | Variable |    2     |
  +----+-----+-------+------+----------+----------+*/
struct socksResponse {
  u_char ver;
  u_char reply;
  u_char reserved;
  u_char attyp;
  u_char bnd_addr[16];
  u_short bnd_port;
};

struct authUserPass {
  u_char ver;
  u_char usernameLength;
  u_char username[31];
  u_char passwordLength;
  u_char password[31];
};

struct authUserPassResponse {
  u_char ver;
  u_char reply;
};

CProxyOut::CProxyOut() {
  Reset();
}

CProxyOut::CProxyOut(char* host,unsigned int port,unsigned int socks) {
  Reset();
  SetProxy(host,port,socks);
}

CProxyOut::~CProxyOut() {
  close(m_iSocket);
}

void CProxyOut::SetProxy(const char* host,unsigned int port,unsigned int socks) {
  memset(m_sProxyHost,'\0',1024);
  strncpy(m_sProxyHost,host,1023);
  m_iProxyPort=port;
  m_iSocksVersion=socks;
}

void CProxyOut::ConnectToHost(char* host, unsigned int port) {
  memset(m_sDestinationHost,'\0',1024);
  strncpy(m_sDestinationHost,host,1023);
  m_iDestinationPort=port;
  u_char buffer[1024];

  memset(buffer,'\0',1024);
  if((m_cProxyEnt=gethostbyname(m_sProxyHost))==NULL)
    THROW_EXCEPTION(ErrorInvalidProxy);
  m_cProxyAddr.sin_family=AF_INET;
  m_cProxyAddr.sin_port=htons(m_iProxyPort);
  bcopy((char*)m_cProxyEnt->h_addr,
	(char*)&m_cProxyAddr.sin_addr.s_addr,
	m_cProxyEnt->h_length);
  m_iSocket=socket(PF_INET,SOCK_STREAM,0);
  if(!m_iSocket)
    THROW_EXCEPTION(ErrorBadSocket);
  if(connect(m_iSocket,(struct sockaddr*)&m_cProxyAddr, sizeof(m_cProxyAddr))<0)
    THROW_EXCEPTION(ErrorConnectionToProxyFailed);
  info("Connection Established (%s:%d). Negotiating connection to %s:%d.",m_sProxyHost,m_iProxyPort,m_sDestinationHost,m_iDestinationPort);

  openRequest* orequest=(openRequest*)buffer;
  orequest->ver=0x05;
  orequest->numMethods=0x02;
  orequest->methods[0]=SOCKS_METHOD_NOAUTH;
  orequest->methods[1]=SOCKS_METHOD_USERNAME_PASSSWORD;
  int bytesSent=send(m_iSocket,buffer,2+orequest->numMethods,0);

  memset(buffer,'\0',1024);
  int bytesRecv=recv(m_iSocket,buffer,8,0);//check if it's socks4
  if(bytesRecv!=2)
    THROW_EXCEPTION(ErrorBadProxyResponse);
  openResponse* oresponse=(openResponse*)buffer;
  if(oresponse->ver!=0x05)
    THROW_EXCEPTION(ErrorProxyNotSupported);
  switch(oresponse->method) {
  case SOCKS_METHOD_NOAUTH:
    //nothing to be done here. Proceed!
    break;
  case SOCKS_METHOD_USERNAME_PASSSWORD: {
    if(strlen(m_sUsername)==0 || strlen(m_sPassword)==0) 
      THROW_EXCEPTION(ErrorInvalidUserPass);
    memset(buffer,'\0',1024);
    authUserPass* auth=(authUserPass*)buffer;
    auth->ver=m_iSocksVersion;
    auth->usernameLength=31;//tmp - strlen(m_sUsername);
    auth->passwordLength=31;//tmp - strlen(m_sPassword);
    memcpy(auth->username,m_sUsername,31);
    memcpy(auth->password,m_sPassword,31);
    send(m_iSocket,buffer,sizeof(authUserPass),0);

    memset(buffer,'\0',1024);
    int br=recv(m_iSocket,buffer,1024,0);
    authUserPassResponse* authResponse=(authUserPassResponse*)buffer;
    if(br==-1)
      THROW_EXCEPTION(ErrorConnectionReset);
    if(authResponse->ver!=m_iSocksVersion || br!=2)
      THROW_EXCEPTION(ErrorProxyNotSupported);
    if(authResponse->reply!=SOCKS_REPLY_VALID_PASS)
      THROW_EXCEPTION(ErrorInvalidUserPass);
    
    break;
    }
  default:
    THROW_EXCEPTION(ErrorProxyNotSupported);
    break;
  }
  memset(buffer,'\0',1024);
  socksRequest* request=(socksRequest*)buffer;
  request->ver=m_iSocksVersion;
  request->command=SOCKS_CMD_CONNECT;//Lazy - only supporting connect for now. Not bind, or udp
  request->reserved=0x00;
  request->attyp=SOCKS_ATTYP_DOMAIN;//This might be an issue. In my tests, I was able to connect via an ip address
                                    // with 0x03 here (should have to be 0x01). Might break on some proxies
  request->dst_addr[0]=255;//bytes following
  memcpy(request->dst_addr+1,m_sDestinationHost,strlen(m_sDestinationHost));
  request->dst_port=htons(m_iDestinationPort);
  bytesSent=send(m_iSocket,buffer,sizeof(socksRequest),0);
  if(bytesSent==-1)
    THROW_EXCEPTION(ErrorConnectionReset);
  memset(buffer,'\0',1024);
  bytesRecv=recv(m_iSocket,buffer,1024,0);
  if(bytesRecv==-1)
    THROW_EXCEPTION(ErrorConnectionReset);
  socksResponse* response=(socksResponse*)buffer;
  if(response->reply!=SOCKS_REPLY_SUCCEED) {
    errno=response->reply;
    THROW_EXCEPTION(ErrorBadProxyResponse);
  }
  info("Connection through proxy server (%s:%d) complete.",m_sProxyHost,m_iProxyPort);
}

int CProxyOut::Read(char* buf,unsigned int len) {
  int ret=recv(m_iSocket,buf,len,0);
  if(ret==-1)
    THROW_EXCEPTION(ErrorIO);
  return ret;
}

int CProxyOut::Send(char* buf,unsigned int len) {
  int ret=send(m_iSocket,buf,len,0);
  if(ret==-1)
    THROW_EXCEPTION(ErrorIO);
  return ret;

}

void CProxyOut::Reset() {
  memset(m_sDestinationHost,'\0',1024);
  memset(m_sProxyHost,'\0',1024);
  m_iDestinationPort=0;
  m_iProxyPort=0;
}



void CProxyOut::SetUserPass(char* user,char* pass) {
  strncpy(m_sUsername,user,31);
  strncpy(m_sPassword,pass,31);
}

void CProxyOut::info(const char* m,...) {
  char* tmp=new char[strlen(m)+10];
  sprintf(tmp,"[+] %s\n",m);
  va_list ap;
  va_start(ap,m);
  vprintf(tmp,ap);
  va_end(ap);
  delete[] tmp;
}

CProxyException::CProxyException() {
  
}

CProxyException::CProxyException(const char* file,unsigned line) : m_sFile(file), m_iLine(line) {
  
}
