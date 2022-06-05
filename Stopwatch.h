/*
 * Stopwatch.h
 *
 *  Created on: 29 Sep 2011
 *      Author: thomas
 *
 */

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define SEND_INTERVAL_MS 10000

#ifndef DISABLE_STOPWATCH
#define STOPWATCH(name, expression)                                                      \
  do {                                                                                   \
    const uint64_t startTime = Stopwatch::getInstance().getCurrentSystemTime();          \
    expression const uint64_t endTime = Stopwatch::getInstance().getCurrentSystemTime(); \
    Stopwatch::getInstance().addStopwatchTiming(name, endTime - startTime);              \
  } while (false)

#define TICK(name)                                                                        \
  do {                                                                                    \
    Stopwatch::getInstance().tick(name, Stopwatch::getInstance().getCurrentSystemTime()); \
  } while (false)

#define TOCK(name)                                                                        \
  do {                                                                                    \
    Stopwatch::getInstance().tock(name, Stopwatch::getInstance().getCurrentSystemTime()); \
  } while (false)
#else
#define STOPWATCH(name, expression) expression

#define TOCK(name) ((void)0)

#define TICK(name) ((void)0)

#endif

class Stopwatch {
 public:
  static Stopwatch& getInstance() {
    static Stopwatch instance;
    return instance;
  }

  void addStopwatchTiming(std::string name, uint64_t duration) {
    if (duration > 0) {
      timings[name] = (float)(duration) / 1000.0f;
    }
  }

  void setCustomSignature(uint64_t newSignature) {
    signature = newSignature;
  }

  const std::map<std::string, float>& getTimings() {
    return timings;
  }

  void printAll() {
    for (std::map<std::string, float>::const_iterator it = timings.begin(); it != timings.end();
         it++) {
      std::cout << it->first << ": " << it->second << "ms" << std::endl;
    }

    std::cout << std::endl;
  }

  void pulse(std::string name) {
    timings[name] = 1;
  }

  void sendAll() {
    gettimeofday(&clock, 0);

    if ((currentSend = (clock.tv_sec * 1000000 + clock.tv_usec)) - lastSend > SEND_INTERVAL_MS) {
      int size = 0;
      unsigned char* data = serialiseTimings(size);

      sendto(sockfd, data, size, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

      free(data);

      lastSend = currentSend;
    }
  }

  static uint64_t getCurrentSystemTime() {
    timeval tv;
    gettimeofday(&tv, 0);
    uint64_t time = (uint64_t)(tv.tv_sec * 1000000 + tv.tv_usec);
    return time;
  }

  void tick(std::string name, uint64_t start) {
    tickTimings[name] = start;
  }

  void tock(std::string name, uint64_t end) {
    float duration = (float)(end - tickTimings[name]) / 1000.0f;

    if (duration > 0) {
      timings[name] = duration;
    }
  }

 private:
  Stopwatch() {
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(45454);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    gettimeofday(&clock, 0);

    signature = clock.tv_sec * 1000000 + clock.tv_usec;

    currentSend = lastSend = clock.tv_sec * 1000000 + clock.tv_usec;
  }

  virtual ~Stopwatch() {
    close(sockfd);
  }

  unsigned char* serialiseTimings(int& packetSize) {
    packetSize = sizeof(int) + sizeof(uint64_t);

    for (std::map<std::string, float>::const_iterator it = timings.begin(); it != timings.end();
         it++) {
      packetSize += it->first.length() + 1 + sizeof(float);
    }

    int* dataPacket = (int*)calloc(packetSize, sizeof(unsigned char));

    dataPacket[0] = packetSize * sizeof(unsigned char);

    *((uint64_t*)&dataPacket[1]) = signature;

    float* valuePointer = (float*)&((uint64_t*)&dataPacket[1])[1];

    for (std::map<std::string, float>::const_iterator it = timings.begin(); it != timings.end();
         it++) {
      memcpy(valuePointer, it->first.c_str(), it->first.length() + 1);
      valuePointer = (float*)((unsigned char*)valuePointer + it->first.length() + 1);
      *valuePointer++ = it->second;
    }

    return (unsigned char*)dataPacket;
  }

  timeval clock;
  long long int currentSend, lastSend;
  uint64_t signature;
  int sockfd;
  struct sockaddr_in servaddr;
  std::map<std::string, float> timings;
  std::map<std::string, uint64_t> tickTimings;
};
