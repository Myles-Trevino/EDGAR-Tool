#include <cURL/curl.h>
#include "Utility.hpp"
#include "Request.hpp"

namespace
{
   CURL *curl_handle;
   uint8_t maximum_attempts{3};

   size_t read_callback(void *buffer, size_t element_size,
      size_t element_count, std::string *string)
   {
      const size_t max_size{element_size*element_count};
      const size_t size{max_size > string->size()+1 ? string->size()+1 : max_size};
      std::memcpy(buffer, string->data(), size);
      return size;
   }

   size_t write_callback(void *buffer, size_t element_size,
      size_t element_count, std::string *string)
   {
      const size_t buffer_size{element_size*element_count}, previous_size{string->size()};
      string->resize(previous_size+buffer_size);
      std::copy(static_cast<char *>(buffer), static_cast<char *>(buffer)+
         buffer_size, string->begin()+previous_size);
      return buffer_size;
   }

   std::string request(const std::string& url, uint16_t required_response_code)
   {
      //Set up
      std::string response;
      char error_buffer[CURL_ERROR_SIZE];
      curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
      curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
      curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, false);
      curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
      curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");
      curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "");
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
      curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
      curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);

      //Loop until the request was successful or the maximum number of attempts is exceeded
      uint16_t attempt{1};
      while(true)
      {
         try
         {
            //Submit the request
            CURLcode curl_error{curl_easy_perform(curl_handle)};

            //Check for a cURL error
            if(curl_error) throw Exception{"cURL could not complete a request."
               " cURL Error: \""+std::string{error_buffer}+"\"."};

            //Check the response code
            int32_t response_code;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code != required_response_code) throw Exception{"Recieved an "
               "invalid response code of "+std::to_string(response_code)+" when "+
               std::to_string(required_response_code)+" was required."};

            break; //Successful request - exit the loop
         }
         catch(const Exception& e){ Utility::print("Request-level error: "+
            e.message+" Attempt: "+std::to_string(attempt)+".\n"); }
		   catch(...){ Utility::print("Request-level external error. "
            "Attempt: "+std::to_string(attempt)+".\n"); }

         //Failed request - exit the loop if the maximum number of attempts was exceeded
         if(attempt >= maximum_attempts) throw Exception{"The request "
            "was aborted after too many failed attempts"};

         ++attempt;
      }

      //Destroy
      curl_easy_reset(curl_handle);

      return response;
   }
}

void Request::initialize()
{
   curl_global_init(CURL_GLOBAL_ALL);
   curl_handle = curl_easy_init();
}

void Request::destroy(){ if(curl_handle) curl_easy_cleanup(curl_handle); }

std::string Request::get(const std::string& url, uint16_t required_response_code)
{ return request(url, required_response_code); }