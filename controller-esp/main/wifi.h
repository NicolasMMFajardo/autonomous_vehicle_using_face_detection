#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>

#define WIFI_SSID   "FAJARDO"
#define WIFI_PASS   "10Piratas123"
#define PORT 3333

extern float posX;
extern float posY;
extern float posZ;

void wifi_init_sta(void);
void tcp_server_task(void *pvParameters);

#endif