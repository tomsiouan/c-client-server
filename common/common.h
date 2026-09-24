#ifndef COMMON_H
#define COMMON_H

// shared between client and server so both sides agree on the protocol
#define PORT 8081
#define MAX_MESSAGE_SIZE 256

// the client broadcasts DISCOVERY_REQUEST on DISCOVERY_PORT (UDP) and the
// server answers with DISCOVERY_RESPONSE, which tells the client its address
#define DISCOVERY_PORT 8082
#define DISCOVERY_REQUEST "C_CLIENT_SERVER_DISCOVER"
#define DISCOVERY_RESPONSE "C_CLIENT_SERVER_HERE"

#endif
