#pragma once
#include <string>

namespace Request
{
   void initialize();
   void destroy();

   std::string get(const std::string& url, uint16_t required_response_code = 200);
}