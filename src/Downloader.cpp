#include <algorithm>
#include "Request.hpp"
#include "Filer.hpp"
#include "String.hpp"
#include "Downloader.hpp"

namespace
{
   std::string label;
   uint32_t quarter_count{}, count{};
   std::vector<Listing> listings;

   void print_status(){ Utility::print(label+": "+std::to_string(quarter_count)+" of "+
      std::to_string(listings.size())+". Total: "+std::to_string(count)+".\r"); }
}

void Downloader::download(Date date)
{
   try
   {
      uint32_t warnings{};
      std::ofstream log_file{"Downloader Log.txt"};
      Utility::print("Downloading the EDGAR database.\n---\n", &log_file);

      while(true)
      {
         //Initialize the quarter
         quarter_count = 0;
         label = Utility::get_label(date);
         std::string directory{Utility::get_directory(date)};
         Filer::create_directory(directory);

         //Retrieve and the index file
         Utility::print(label+": Retrieving the index.\n", &log_file);
         std::string data{Request::get("https://www.sec.gov/Archives/edgar/full-index/"+
            std::to_string(date.year)+"/QTR"+std::to_string(date.quarter)+"/master.idx")};

         //Save the index file
         std::ofstream file{directory+"\\Index.dat"};
         file<<data;
         file.close();
         Utility::print(label+": Retrieved the index.\n", &log_file);

         //Retrieve the SC 13D and SC 13D/A files
         Utility::print(label+": Retrieving the documents.\n", &log_file);
         listings = Utility::get_listings(String::split(data, "\n", true), true);

         for(const Listing& l : listings)
         {
            //Send the request
            ++quarter_count, ++count;
            std::string url{"https://www.sec.gov/Archives/"+l.url};
            try{ data = Request::get(url); }
            catch(...){ Utility::print(label+": Warning: Failed to retrieve "
               "the document from "+url+".\n", &log_file); ++warnings; continue; }

            //Create the file
            std::string id{String::extract(l.url, "data/", ".")};
            std::replace(id.begin(), id.end(), '/', '-');
            std::string file_name{l.date+'-'+id};
            std::ofstream file{directory+"\\"+file_name+".dat"};
            if(!file) throw Exception{"Could not create a document file."};
            file<<data;
            file.close();

            //Print the status
            print_status();
         }

         //Print an end message for each quarter
         print_status();
         Utility::print('\n'+label+": Retrieved the documents.\n", &log_file);
         if(!Utility::is_last_quarter(date)) Utility::print("\n");

         //Print a separator for each year
         Date next_date{Utility::get_next_date(date)};
         if(next_date.year != date.year) Utility::print("---\n", &log_file);

         if(!next_date.year) break; //Break if past the end year
         date = next_date;
      }

      Utility::print("Warnings: "+std::to_string(warnings)+".\n", &log_file);
      Utility::print("Finished downloading the EDGAR database.\n", &log_file);
   }
   catch(const Exception& e){ Utility::print("Downloader error: "+e.message+".\n"); }
   catch(...){ Utility::print("Downloader external error.\n"); }
}