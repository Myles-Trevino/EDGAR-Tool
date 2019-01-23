#include <algorithm>
#include "Utility.hpp"
#include "String.hpp"

namespace
{
   bool is_not_numeric(char c){ return !(String::is_digit(c) || c == '.'); };
   char to_uppercase(char c){ if(c < 97 || c > 122) return c; return c-32; }
}

bool String::is_space(char c){ return c == ' ' || c == '\n' || c == '\r'; }

bool String::is_digit(char c){ return c >= 48 && c <= 57; }

std::string String::make_numeric(std::string string)
{
   string.erase(std::remove_if(string.begin(), string.end(), is_not_numeric), string.end());
   return string;
}

std::string String::make_uppercase(std::string string)
{
   std::transform(string.begin(), string.end(), string.begin(), to_uppercase);
   return string;
}

std::string String::remove(std::string string, const std::string& query)
{
   size_t position{string.find(query)};
   if(position == std::string::npos) return string;
   string.erase(position, query.size());
   return string;
}

void String::replace(std::string *string, const std::string& query, const std::string& replacement)
{
   size_t index{};
   while(true)
   {
      index = string->find(query, index);
      if(index == std::string::npos) return;
      string->erase(index, query.size());
      string->insert(index, replacement);
      index += replacement.size();
   }
}

std::string String::replace(const std::string& string,
   const std::string& query, const std::string& replacement)
{
   std::string result{string};
   replace(&result, query, replacement);
   return result;
}

std::string String::trim(std::string string)
{
   string.erase(string.find_last_not_of(' ')+1);
   string.erase(0, string.find_first_not_of(' '));
   return string;
}

std::vector<std::string> String::split(const std::string& string,
   const std::string& identifier, bool cull, bool perform_trim)
{
   std::vector<std::string> results;
   size_t position, offset{};
   while((position = string.find(identifier, offset)) != std::string::npos)
   {
      std::string token{string.substr(offset, position-offset)};
      if(perform_trim) token = trim(token);
      if(token.size() || !cull) results.emplace_back(token);
      offset = position+1;
   }

   std::string token{string.substr(offset)};
   if(perform_trim) token = trim(token);
   if(token.size() || !cull) results.emplace_back(token);
   return results;
}

std::string String::extract(const std::string string, const std::string& start_indentifier,
   const std::string& end_indentifier, size_t offset)
{
   size_t start{};
   if(!start_indentifier.empty())
   {
      start = string.find(start_indentifier, offset);
      if(start == std::string::npos) throw Exception{"Extraction failed. Could"
         " not find the start identifier \""+start_indentifier+"\"."};
      start += start_indentifier.size();
   }

   size_t end{};
   if(!end_indentifier.empty())
   {
      end = string.find(end_indentifier, start);
      if(end == std::string::npos) throw Exception{"Extraction failed. Could"
         " not find the end identifier \""+end_indentifier+"\"."};
   }

   return end ? string.substr(start, end-start) : string.substr(start);
}

bool String::contains(const std::string& string, const std::string& query)
{ return string.find(query) != std::string::npos; }

std::string String::deduplicate(const std::string& string, char character)
{
   std::string result;
   std::unique_copy(string.begin(), string.end(), std::back_insert_iterator<std::string>(
      result), [character](char a, char b){ return a == character && b == character; });
   return result;
}

bool String::starts_with(const std::string& string, const std::string& query)
{ return string.substr(0, query.size()) == query; }