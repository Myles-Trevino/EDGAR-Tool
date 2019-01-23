#pragma once
#include <string>
#include <vector>

namespace String
{
   bool is_space(char c);
   bool is_digit(char c);
   std::string make_numeric(std::string string);
   std::string make_uppercase(std::string string);
   std::string remove(std::string string, const std::string& query);
   void replace(std::string *string, const std::string& query, const std::string& replacement);
   std::string replace(const std::string& string, const std::string& query, const std::string& replacement);
   std::string trim(std::string string);
   std::vector<std::string> split(const std::string& string,
      const std::string& identifier, bool cull = false, bool perform_trim = false);
   std::string extract(const std::string string, const std::string& start_indentifier,
      const std::string& end_indentifier, size_t offset = 0);
   bool contains(const std::string& string, const std::string& query);
   std::string deduplicate(const std::string& string, char character);
   bool starts_with(const std::string& string, const std::string& query);
}