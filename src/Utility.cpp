#include <iostream>
#include <mutex>
#include <chrono>
#include <Windows.h>
#include "String.hpp"
#include "Utility.hpp"

namespace
{
   uint8_t compressor_thread_count, loader_thread_count, parser_thread_count;
   std::mutex log_mutex;
}

void Utility::initialize()
{
   uint32_t thread_count{std::thread::hardware_concurrency()};
   if(!thread_count)
   {
      Utility::print("Warning: Could not determine the number of "
         "concurrent threads your CPU supports. Using a default of four.\n\n");
      thread_count = 4;
   }
   compressor_thread_count = thread_count;
   loader_thread_count = 2;
   parser_thread_count = thread_count;
}

double Utility::get_time(){ return std::chrono::system_clock::now().time_since_epoch().count()/1000000000.; }

void Utility::set_thread_priority()
{ SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST); }

//Printing
void Utility::print(const std::string& message, std::ofstream *file)
{
   std::cout<<message;

   if(!file) return;
   *file<<message;
   file->flush();
}

void Utility::mutex_print(const std::string& string, std::ofstream *file)
{
   std::lock_guard<std::mutex> lock_guard{log_mutex};
   print(string, file);
}

//Database
std::vector<Listing> Utility::get_listings(std::vector<std::string> lines, bool trim)
{
   std::vector<Listing> listings;
   if(trim) lines.erase(lines.begin(), lines.begin()+11); //Trim raw index files
   size_t count{};
   for(const std::string& l : lines)
   {
      count++;
      if(l.empty()) continue;
      std::vector<std::string> tokens{String::split(l, "|")};
      if(tokens.size() != 5) throw Exception{"Tokenization failed. Line: \""+l+'\"'};
      if(tokens[2] != "SC 13D" && tokens[2] != "SC 13D/A") continue;
      listings.push_back({tokens[3], tokens[2], tokens[0], tokens[1], tokens[4]});
   }
   return listings;
}

//Date
std::string Utility::get_label(const Date& date)
{ return std::to_string(date.year)+" Q"+std::to_string(date.quarter); }

std::string Utility::get_directory(const Date& date, bool compressed)
{
   return std::string{compressed ? "Compressed " : ""}+"Database\\"+
      std::to_string(date.year)+"\\Q"+std::to_string(date.quarter);
}

bool Utility::is_last_quarter(const Date& date)
{ return (date.year == date.end_year) ?  date.quarter == date.end_quarter : date.quarter == 4; }

Date Utility::get_next_date(Date date)
{
   uint8_t maximum_quarter{static_cast<uint8_t>(
      (date.year == date.end_year) ? date.end_quarter : 4)};
   if(++date.quarter > maximum_quarter)
   {
      date.quarter = 1;
      if(++date.year > date.end_year) return {};
      else return date;
   }
   return date;
}

//Threading
uint8_t Utility::get_compressor_thread_count(){ return compressor_thread_count; }
uint8_t Utility::get_parser_thread_count(){ return parser_thread_count; }
uint8_t Utility::get_loader_thread_count(){ return loader_thread_count; }

std::vector<std::thread> Utility::launch_threads(uint32_t task_count,
   void (*thread_function)(std::vector<uint32_t> indices), uint8_t thread_count)
{
   std::vector<std::thread> threads;
   if(thread_count == 1 || task_count < thread_count)
   {
      std::vector<uint32_t> indices;
      for(uint32_t i{}; i < task_count; ++i) indices.emplace_back(i);
      thread_function(indices);
   }
   else
   {
      //Create the index groups
      std::vector<std::vector<uint32_t>> index_groups;
      index_groups.resize(thread_count);
      for(uint32_t i{}; i < task_count; ++i) index_groups[i%thread_count].emplace_back(i);

      //Launch the threads
      for(size_t i{}; i < thread_count; ++i)
         threads.emplace_back(thread_function, index_groups[i]);
   }
   return threads;
}