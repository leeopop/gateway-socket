/*
 * gateway.hh
 *
 *  Created on: 2014. 1. 12.
 *      Author: leeopop
 */

#ifndef GATEWAY_HH_
#define GATEWAY_HH_

#include "socket.hh"
#include "arp.hh"
#include "packet.hh"
#include <sys/time.h>
#include <pthread.h>
#include <boost/unordered_map.hpp>
#include <queue>

#define MY_MTU ETH_DATA_LEN
#define MY_PACKET_LEN (ETH_FRAME_LEN)
#define IO_BURST 32
#define DHCP_TIMEOUT 7200

struct userInfo
{
	struct in_addr ip;
	struct ether_addr user_mac;
	uint64_t last_access;
	uint64_t timeout;
};

class Database;

class Gateway
{
private:
	Device *inDev;
	Device *outDev;
	Database* db;

	struct event_base* evbase;

	int termEventFD;
	int addStaticIPEventFD;
	int delStaticIPEventFD;

	int addUserEventFD;
	int delUserEventFD;

	int sendPacketFD;

	pthread_mutex_t addStaticIPLock;
	pthread_mutex_t delStaticIPLock;
	pthread_mutex_t addUserLock;
	pthread_mutex_t delUserLock;
	pthread_mutex_t sendPacketLock;


	std::queue< std::pair<struct in_addr, struct ether_addr> > staticIPAddRequest;
	std::queue<struct in_addr> staticIPDelRequest;

	std::queue< struct userInfo* > userAddRequest;
	std::queue< struct in_addr > userDelRequest;

	std::queue<Packet*> sendPacketRequestQueue;

	boost::unordered_map<uint32_t, struct ether_addr> staticIPMap;

	boost::unordered_map<uint32_t, struct userInfo*> userMap;

	static void add_static_ip(int fd, short what, void *arg);
	static void del_static_ip(int fd, short what, void *arg);
	static void add_user(int fd, short what, void *arg);
	static void del_user(int fd, short what, void *arg);
	static void icmp_mtu(Device* dev, Packet* packet, uint16_t MTU);

	static void send_packet(int fd, short what, void *arg);

public:
	Gateway(const char* inDev, const char* outDev, Database* db);
	~Gateway();

	void serve(void);

	void terminate();

	void addStaticIP(struct in_addr ip, struct ether_addr mac);
	void delStaticIP(struct in_addr ip);

	void addUserInfo(struct userInfo info);
	void delUserInfo(struct in_addr ip);

	void sendPacketRequest(Packet* packet);
};


#endif /* GATEWAY_HH_ */
