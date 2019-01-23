#include <cctype>
#include "HTML Entities.hpp"
#include "Utility.hpp"
#include "String.hpp"
#include "Formatter.hpp"

namespace
{
   bool is_space_or_dash(char c){ return c == ' ' || c == '-' || c == '_'; }

   void add_to_result(std::string *result, size_t *index, char character)
   {
      if(character == ' ' && (*result)[*index-1] == ' ') return; //Do not allow duplicate spaces
      (*result)[(*index)++] = character;
   }

   void add_to_result(std::string *result, size_t *index, const std::string& string)
   {
      for(char c : string)
      {
         add_to_result(result, index, c);
         if(*index >= result->size()) return;
      }
   }
}

std::string Formatter::format(const std::string& document)
{
   std::string result;
   result.resize(document.size());
   size_t index{1};

   for(size_t i{}; i < document.size();)
   {
      char c{document[i]};
      if(index >= result.size()) break;

      //Ignore HTML tags
      if(c == '<') i = document.find('>', i+1)+1;

      //HTML entitiies
      else if(c == '&')
      {
         //Decimal entity
         if(document[i+1] == '#')
         {
            //Get the length
            size_t length{};
            for(size_t j{i+2}; j < document.size(); ++j)
            {
               char c{document[j]};
               if(!String::is_digit(c) && !String::is_space(c)) break;
               ++length;
            }

            //Get the number
            if(length)
            {
               std::string number_string{document.substr(i+2, length)};
               int32_t number{std::stoi(number_string)};
               if(number >= 160 && number <= 255)
                  add_to_result(&result, &index, html_entities[number-160].character);
            }

            i += length+((document[i+length+2] == ';') ? 3 : 2);
         }

         //Named entity
         else
         {
            HTML_Entity entity;
            for(const HTML_Entity& e : html_entities)
               if(document.substr(i+1, e.name.size()) == e.name)
               { add_to_result(&result, &index, e.character); entity = e; break; }

            if(!entity.name.empty()) i += entity.name.size()+
               ((document[i+entity.name.size()+1] == ';') ? 2 : 1);
            else{ add_to_result(&result, &index, c); ++i; } //Regular ampersand
         }
      }

      //Replace newlines and tabs
      else if(c == '\n' || c == '\r' || c == '\t'){ add_to_result(&result, &index, ' '); ++i; }

      //Discard repeated dashes and underscores
      else if((!i || i+1 >= document.size() || is_space_or_dash(document[i-1]) ||
         is_space_or_dash(document[i+1])) && (c == '-' || c == '_')) ++i;

      //Discard non-ASCII characters
      else if(c > 126) ++i;

      //Default
      else{ add_to_result(&result, &index, c); ++i; }
   }

   return result.substr(1, index-1);
}