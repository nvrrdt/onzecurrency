#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <string>

#include "external_ip.hpp"

using namespace crowd;

std::string external_ip::get_external_ip(void)
{
  CURL *curl;
  CURLcode res;
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://myexternalip.com/raw");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    std::cout << res << std::endl;
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  return std::to_string(res);
}