#pragma once
#include <string>
#include <vector>
#include <thread>
#include <fstream>

struct Exception{ std::string message; };

struct Listing{ std::string date, type, cik, company, url; };

struct Date
{
   uint16_t year;
   uint8_t quarter;
   uint16_t end_year;
   uint8_t end_quarter;
};

namespace Utility
{
   void initialize();
   void set_thread_priority();
   double get_time();

   //Printing
   void print(const std::string& message, std::ofstream *file = nullptr);
   void mutex_print(const std::string& message, std::ofstream *file = nullptr);

   //Database
   std::vector<Listing> get_listings(std::vector<std::string> lines, bool trim);

   //Date
   std::string get_label(const Date& date);
   std::string get_directory(const Date& date, bool compressed = false);
   bool is_last_quarter(const Date& date);
   Date get_next_date(Date date);

   //Threading
   uint8_t get_compressor_thread_count();
   uint8_t get_parser_thread_count();
   uint8_t get_loader_thread_count();
   std::vector<std::thread> launch_threads(uint32_t task_count,
      void (*thread_function)(std::vector<uint32_t> indices), uint8_t thread_count);
}