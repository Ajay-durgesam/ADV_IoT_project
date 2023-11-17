#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest.h"
#include "sys/clock.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(_VA_ARGS_)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

char temp[100];

static uint16_t total_requests = 0;
static uint16_t successful_responses = 0;
static clock_time_t total_delay = 0;

RESOURCE(pir, METHOD_GET, "pir");
void
pir_handler(REQUEST* request, RESPONSE* response)
{
  total_requests++;

  char pir_value_str[10];
  int success = 1;

  // Extract PIR value from the query parameters
  if (rest_get_query_variable(request, "pir_value", pir_value_str, sizeof(pir_value_str))) {
    // Convert the string to an integer
    int pir_value = atoi(pir_value_str);

    snprintf(temp, sizeof(temp), "Received PIR Value: %d", pir_value);

    // Simulate end-to-end delay
    clock_time_t start_time = clock_time();
    clock_delay(100); // Adjust the delay value as needed
    total_delay += clock_time() - start_time;

    rest_set_header_content_type(response, TEXT_PLAIN);
    rest_set_response_payload(response, (uint8_t*)temp, strlen(temp));  // Explicit cast to uint8_t*

    // Simulate a successful response
    successful_responses++;
    PRINTF("Received PIR Value: %d\n", pir_value);
  } else {
    // Query parameter "pir_value" not found

    snprintf(temp, sizeof(temp), "Query parameter 'pir_value' not found in the URI");

    rest_set_header_content_type(response, TEXT_PLAIN);
    rest_set_response_payload(response, (uint8_t*)temp, strlen(temp));  // Explicit cast to uint8_t*

    PRINTF("Query parameter 'pir_value' not found in the URI\n");
  }
}


PROCESS(rest_server_example, "Cloud Sever");
AUTOSTART_PROCESSES(&rest_server_example);

PROCESS_THREAD(rest_server_example, ev, data)
{
  PROCESS_BEGIN();

#ifdef WITH_COAP
  PRINTF("COAP Server\n");
#else
  PRINTF("HTTP Server\n");
#endif

  rest_init();

  rest_activate_resource(&resource_pir);

  while (1) {
    PROCESS_WAIT_EVENT();

    if (ev == PROCESS_EVENT_POLL) {
      PRINTF("Metrics - Total Requests: %u, Successful Responses: %u, PDR: %u%%, Avg. End-to-End Delay: %lu ms, Power Consumption: %lu\n",
             total_requests, successful_responses,
             (successful_responses * 100) / total_requests,
             (total_requests > 0) ? (total_delay / total_requests) * CLOCK_SECOND * 1000 / CLOCK_SECOND : 0,
             0 /* Placeholder for power consumption */
      );
    }
  }

  PROCESS_END();
}