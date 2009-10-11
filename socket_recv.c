#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"

int socket_recv4(int s,char *buf,int len,char ip[4],uint16 *port,char dstip[4])
{
  struct sockaddr_in sa;
  int dummy = sizeof sa;
  int r;

  if (dstip == NULL) {
    r = recvfrom(s,buf,len,0,(struct sockaddr *) &sa,&dummy);
    if (r == -1) return -1;
  } else {
    struct iovec iov;
    struct cmsghdr cmsg, *cmsgptr;
    struct msghdr msg;
    char cbuf[64];

    byte_zero(&cmsg, sizeof(cmsg));
    byte_zero(&msg, sizeof(msg));

    iov.iov_base = buf;
    iov.iov_len = len;

    msg.msg_name = &sa;
    msg.msg_namelen = dummy;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cbuf;
    msg.msg_controllen = sizeof(cbuf);

    r = recvmsg(s, &msg, 0);

    if (r == -1)
      return (-1);

    for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
      if (cmsgptr->cmsg_level == SOL_IP && cmsgptr->cmsg_type == IP_PKTINFO) {
        struct in_pktinfo *pki = (struct in_pktinfo *)CMSG_DATA(cmsgptr);
        byte_copy(dstip, 4, (char *)&(pki->ipi_addr));
      }
    }
  }

  byte_copy(ip,4,(char *) &sa.sin_addr);
  uint16_unpack_big((char *) &sa.sin_port,port);

  return r;
}
