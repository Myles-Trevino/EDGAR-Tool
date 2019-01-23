#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>
#include <Windows.h>
#include "String.hpp"
#include "Downloader.hpp"
#include "Filer.hpp"
#include "Extractor.hpp"
#include "Formatter.hpp"

namespace
{
   std::string label, source_directory, formatted_directory;
   uint32_t count, warning_count;
   size_t uncompressed_size;
   std::ofstream log_file;
   std::mutex buffer_mutex, log_mutex;
   std::vector<Listing> listings;
   std::string buffer;

   std::string get_directory(const Date& date)
   { return std::to_string(date.year)+"\\Q"+std::to_string(date.quarter); }

   void print_status(){ std::cout<<label<<": "<<count<<" of "<<listings.size()<<".\r"; }
   []
   //Thread
   void print_thread_function(bool *active)
   {
      while(*active) //Atomic operations, no need for a mutex
      {{ std::lock_guard<std::mutex> lock_guard{log_mutex}; print_status(); } Sleep(100); }
   }

   void mutex_log(const std::string& string)
   {
      std::lock_guard<std::mutex> lock_guard{log_mutex};
      Utility::print(string, &log_file);
      log_file<<string;
   }

   void extractor_thread_function(std::vector<uint32_t> indices)
   {
      Utility::set_thread_priority();
      for(const uint32_t& i : indices)
      {
         ++count; //Atomic, no need for a mutex
         const Listing& listing{listings[i]};

         std::string id{String::extract(listing.url, "data/", ".")};
         std::replace(id.begin(), id.end(), '/', '-');
         std::string file_name{listing.date+'-'+id};

         try
         {
            //Load the data
            std::string data{Filer::load(source_directory+'\\'+file_name+".dat")};
            uncompressed_size += data.size();

            //Format and save
            data = Formatter::format(data.substr(0, data.find("</DOCUMENT>")));
            Filer::save(data, formatted_directory+'\\'+file_name+".txt");

            //Compress
            data = "<DOCUMENT>\n"+listing.cik+'|'+listing.company+'|'+listing.type+
               '|'+listing.date+'|'+listing.url+'\n'+Filer::compress(data)+"\n</DOCUMENT>\n";

            //Write the data to the buffer
            std::lock_guard<std::mutex> lock_guard{buffer_mutex};
            buffer += data;
         }
         catch(const Exception& e){ mutex_log(label+": Warning: Encountered an "
            "error extracting from: "+file_name+": "+e.message+".\n"); ++warning_count; }
         catch(const std::exception& e){ mutex_log(label+": Warning: Encountered an external "
            "error extracting from: "+file_name+": "+e.what()+".\n"); ++warning_count; }
      }
   }

   void extract()
   {
      std::vector<std::thread> compressor_threads{Utility::launch_threads(static_cast<uint32_t>(
         listings.size()), extractor_thread_function, Utility::get_compressor_thread_count())};

      //Launch the print thread
      bool print_thread_active{true};
      std::thread print_thread{print_thread_function, &print_thread_active};

      //Wait for the compressor threads to finish
      for(std::thread& t : compressor_threads) t.join();

      //Wait for the print thread to finish
      print_thread_active = false;
      print_thread.join();
   }
}

void Extractor::extract(Date date)
{
   try
   {
      //Initialize
      warning_count = 0;

      log_file.open("Extractor Log.txt");
      Utility::print("Performing an extraction of the EDGAR database.\n---\n", &log_file);

      while(true)
      {
         //Initialize the quarter
         uncompressed_size = 0, count = 0;
         buffer.clear();
         label = Utility::get_label(date);
         source_directory = "Database\\"+get_directory(date);
         formatted_directory = "Formatted "+source_directory;

         //Create the directory
         Filer::create_directory(formatted_directory);
         Filer::create_directory("Optimized Database");

         //Load the listings
         Utility::print(label+": Loading the index.\n", &log_file);
         listings = Utility::get_listings(Filer::load_lines(
            source_directory+"\\Index.dat"), true);
         Utility::print(label+": Loaded the index.\n", &log_file);

         //Multithreaded extraction
         Utility::print(label+": Extracting.\n", &log_file);
         ::extract();
         Filer::save(buffer, "Optimized Database\\"+label+".lz4");

         //Print the statistics for the quarter
         print_status();
         Utility::print("\n"+label+": Extracting.\n");
         float ratio{buffer.size() ? uncompressed_size/static_cast<float>(buffer.size()) : 1};
         Utility::print(label+": Compression ratio: "+std::to_string(ratio)+"x ("+ std::to_string(
            uncompressed_size/1000000.)+" MB to "+std::to_string(buffer.size()/1000000.)+" MB).\n");
         if(!Utility::is_last_quarter(date)) Utility::print("\n", &log_file);

         //Print a separator for each year
         Date next_date{Utility::get_next_date(date)};
         if(next_date.year != date.year) Utility::print("---\n", &log_file);

         if(!next_date.year) break; //Break if past the end year
         date = next_date;
      }

      //Print the final statistics
      Utility::print("Warnings: "+std::to_string(warning_count)+".\n", &log_file);
      Utility::print("Finished the extraction of the EDGAR database.\n", &log_file);
      log_file.close();
   }
   catch(const Exception& e){ Utility::print("Extractor error: "+e.message+".\n"); }
   catch(...){ Utility::print("Extractor external error.\n"); }
}