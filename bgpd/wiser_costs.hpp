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

static std::mutex cost_mutex;
static std::thread wiser_cost_portal_thread;
static const int wiser_cost_portal_port = 2259;

void wiser_costs_table_init();
int wiser_cost_portal_init();

static std::unordered_map<as_t, uint32_t> advcost_sent;
static std::unordered_map<as_t, uint32_t> advcost_recv;

// initialize wiser costs table (both send and recv)
void wiser_costs_table_init()
{
  // technically not needed
  advcost_sent.clear();
  advcost_recv.clear();
}

static void increment_map_value(std::unordered_map<as_t, uint32_t>& map, as_t key, uint32_t delta)
{
  std::lock_guard<std::mutex> guard(cost_mutex);

  auto find = map.find(key);
  if (find == map.end())
    map[key] = delta;
  else
    find->second += delta;
}

static void update_sent_cost(as_t destination, uint32_t cost) {
  increment_map_value(advcost_sent, destination, cost);
}

static void update_recv_cost(as_t from, uint32_t cost) {
  increment_map_value(advcost_recv, from, cost);
}

static void cost_portal()
{
  // TODO
}

int wiser_cost_portal_init() {
  wiser_cost_portal_thread = std::thread(cost_portal);
  return 0;
}

// multiply the costs received from this peer by the normalization factor
static double normalization(as_t peer) {
  std::lock_guard<std::mutex> guard(cost_mutex);

  // TODO: come up with a better system for handling 0
  uint32_t total_send = 1+advcost_sent[peer];
  uint32_t total_recv = 1+advcost_recv[peer];

  return (double)total_send / (double) total_recv;
}


#endif /* BGPD_WISER_COSTS_HPP_ */
