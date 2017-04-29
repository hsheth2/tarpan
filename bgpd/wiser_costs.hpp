/*
 * wiser_costs.hpp
 *
 *  Created on: Apr 28, 2017
 *      Author: hsheth
 */

#ifndef BGPD_WISER_COSTS_HPP_
#define BGPD_WISER_COSTS_HPP_

#include <map>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "bgpd/tarpan_backpropagation.pb-c.h"

static std::mutex cost_mutex;
static std::thread wiser_cost_portal_thread;
static const int wiser_cost_portal_port = 2259;

void
wiser_costs_table_init ();
int
wiser_cost_portal_init ();

static std::unordered_map<as_t, uint32_t> advcost_sent;
static std::unordered_map<as_t, uint32_t> advcost_recv;

// initialize wiser costs table (both send and recv)
void
wiser_costs_table_init ()
{
  // technically not needed
  advcost_sent.clear ();
  advcost_recv.clear ();
}

static void
increment_map_value (std::unordered_map<as_t, uint32_t>& map, as_t key,
		     uint32_t delta)
{
  std::lock_guard<std::mutex> guard (cost_mutex);

  auto find = map.find (key);
  if (find == map.end ())
    map[key] = delta;
  else
    find->second += delta;
}

static void
update_sent_cost (as_t destination, uint32_t cost)
{
  increment_map_value (advcost_sent, destination, cost);
}

static void
update_recv_cost (as_t from, uint32_t cost)
{
  increment_map_value (advcost_recv, from, cost);
}

static void
cost_portal_handle_connection (int socket)
{
  // TODO: make more robust
  while (true)
    {
      uint32_t message_size;
      recv (socket, &message_size, sizeof(uint32_t), MSG_WAITALL);

      zlog_debug("Message size is %d", message_size);
      if (message_size > 4000000)
	{
	  zlog_warn ("Message size too large: %d", message_size);
	  break;
	}

      uint8_t * buf = (uint8_t *) malloc (sizeof(uint8_t) * message_size);
      recv (socket, buf, message_size, MSG_WAITALL);

      TarpanBackpropagation * msg;
      msg = tarpan_backpropagation__unpack (NULL, message_size, buf);

      if (msg->has_ping)
	{
	  zlog_info ("Ping received");
	  // TODO reply with pong?
	}

      if (msg->wiser_back)
	{
	  zlog_info ("wiser_back received from as %d", msg->wiser_back->as);

	  // update local cost sent
	  update_sent_cost (msg->wiser_back->as, msg->wiser_back->cost);
	}

      if (msg->has_close && msg->close)
	{
	  zlog_info ("Closing connection");
	  tarpan_backpropagation__free_unpacked (msg, NULL);
	  break;
	}

      tarpan_backpropagation__free_unpacked (msg, NULL);
      free (buf);
    }

  close (socket);
}

static int
cost_portal (int server_fd, struct sockaddr_in address)
{
  int addrlen = sizeof(address);
  int new_socket;

  zlog_debug("wiser: cost_portal open and listening for connections");

  while ((new_socket = accept (server_fd, (struct sockaddr *) &address,
			       (socklen_t*) &addrlen)))
    {
      if (new_socket < 0)
	break;

      std::thread (cost_portal_handle_connection, new_socket).detach();
    }

  close (server_fd);
  return 0;
}

int
wiser_cost_portal_init ()
{
  int server_fd;
  struct sockaddr_in address;
  int opt = 1;

  // Creating socket file descriptor
  if ((server_fd = socket (AF_INET, SOCK_STREAM, 0)) == 0)
    return 1;

  // Forcefully attaching socket to port
  if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    return 1;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(wiser_cost_portal_port);

  // Forcefully attaching socket to the port
  if (bind (server_fd, (struct sockaddr *) &address, sizeof(address)) < 0)
    return 1;
  if (listen (server_fd, 3) < 0)
    return 1;

  zlog_debug("wiser: starting cost_portal thread");
  wiser_cost_portal_thread = std::thread (cost_portal, server_fd, address);
  return 0;
}

int
wiser_contact_cost_portal (Wiser* wiser, uint32_t cost, struct bgp * self)
{
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;

  zlog_debug("wiser: attempting to contact cost portal of as %d", wiser->sender_as);

  if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    return 1;

  memset (&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(wiser_cost_portal_port);
  serv_addr.sin_addr.s_addr = wiser->sender_address->bytes;

  if (connect (sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    return 1;

  // use this socket connection
  TarpanBackpropagation tarp_back = TARPAN_BACKPROPAGATION__INIT;
  WiserBack wback = WISER_BACK__INIT;
  tarp_back.wiser_back = &wback;

  wback.as = self->as;
  wback.cost = cost; // note: not using wiser->path_cost because it may have changed

  // send the packet
  uint32_t length = tarpan_backpropagation__get_packed_size (&tarp_back);
  uint8_t* buf = (uint8_t*) malloc (length);
  tarpan_backpropagation__pack (&tarp_back, buf);
  write (sock, &length, sizeof(length));
  write (sock, buf, length);
  free (buf);

  close (sock);
  return 0;
}

// multiply the costs received from this peer by the normalization factor
static double
normalization (as_t peer)
{
  std::lock_guard<std::mutex> guard (cost_mutex);

  // TODO: come up with a better system for handling 0
  uint32_t total_send = 1 + advcost_sent[peer];
  uint32_t total_recv = 1 + advcost_recv[peer];

  return (double) total_send / (double) total_recv;
}

#endif /* BGPD_WISER_COSTS_HPP_ */
