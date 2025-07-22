#include <dns_proxy_request.h>
#include <string.h>

static int dns_proxy_parse_qname(dns_packet_t *packet, int offset,
                                 char *domain) {
  int i = 0;
  int length = packet[offset];
  int pos = 0;

  while (length > 0) {
    offset++;
    for (i = 0; i < length; i++) {
      domain[pos++] = packet[offset++];
    }
    domain[pos++] = '.';
    length = packet[offset];
  }
  if (pos > 0) {
    domain[pos - 1] = '\0'; // Remove the trailing dot and null-terminate
  } else {
    domain[0] = '\0';
  }
  return offset + 1; // Return new offset (skip the zero length byte)
}

void dns_proxy_parse_packet(dns_packet_t *packet,
                            dns_proxy_request_t *request) {
  memcpy(&request->header, packet, sizeof(dns_proxy_header_t));

  // Convert fields to host byte order
  request->header.id = ntohs(request->header.id);
  request->header.flags = ntohs(request->header.flags);
  request->header.qdcount = ntohs(request->header.qdcount);
  request->header.ancount = ntohs(request->header.ancount);
  request->header.nscount = ntohs(request->header.nscount);
  request->header.arcount = ntohs(request->header.arcount);

  // Parse QNAME
  int offset = sizeof(dns_proxy_header_t);
  offset = dns_proxy_parse_qname(packet, offset, request->question.qname);
  request->question.qname[offset - 1] = '\0';

  // Parse QTYPE and QCLASS
  memcpy(&request->question.qtype, packet + offset, sizeof(uint16_t));
  offset += sizeof(uint16_t);
  memcpy(&request->question.qclass, packet + offset, sizeof(uint16_t));

  request->question.qtype = ntohs(request->question.qtype);
  request->question.qclass = ntohs(request->question.qclass);
}