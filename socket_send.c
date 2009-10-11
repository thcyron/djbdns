#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "byte.h"
#include "socket.h"

int socket_send4(int s,const char *buf,int len,const char ip[4],uint16 port,char srcip[4])
{
  struct sockaddr_in sa;

  byte_zero(&sa,sizeof sa);
  sa.sin_family = AF_INET;
  uint16_pack_big((char *) &sa.sin_port,port);
  byte_copy((char *) &sa.sin_addr,4,ip);

  if (srcip == NULL) {
    return sendto(s,buf,len,0,(struct sockaddr *) &sa,sizeof sa);
  } else {
    struct iovec iov;
    struct msghdr msg;
    struct {
      struct cmsghdr cm;
      struct in_pktinfo ipi;
    } cmsg = {
      .cm = {
        .cmsg_len = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo),
        .cmsg_level = SOL_IP,
        .cmsg_type = IP_PKTINFO
      },
      .ipi = {
        .ipi_ifindex = 0,
      }
    };

    byte_copy(&cmsg.ipi.ipi_spec_dst, 4, srcip);
    byte_zero(&msg, sizeof(msg));

    iov.iov_base = (char *)buf;
    iov.iov_len = len;

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    return sendmsg(s, &msg, 0);
  }
}
