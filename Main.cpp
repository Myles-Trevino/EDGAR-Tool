#include <iostream>
#include <Windows.h>
#include "Utility.hpp"
#include "Downloader.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Extractor.hpp"
#include "String.hpp"

bool valid_quarter(uint8_t quarter){ return quarter >= 1 && quarter <= 4; }

int main()
{
   //Initialize
   SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
   Utility::initialize();
   Request::initialize();
   Utility::set_thread_priority();

   struct Foo{int bar;} foo;

   foo.bar = 1;

   try
   {
      //Print the readme
      Utility::print("EDGAR Tool - Myles Trevino\n\n"
         "Command format: <action> <start year> <start quarter> <end year> <end quarter>\n"
         "Actions: download, extract, parse\n"
         "Example: parse 1993 1 2018 3\n\n");

      Utility::print(
         "Download: Downloads the database from the EDGAR site so that it can be \n"
         "   accessed quickly in the future. Downloading the entire database will take \n"
         "   multiple hours. This is because the EDGAR site limits the rate of access \n"
         "   to prevent abuse of their system and this limit cannot be bypassed legally.\n"
         "Extract: Extracts a human-readable (formatted) version and a parser-optimized \n"
         "   version from the downloaded EDGAR database. On an i7 5820k and 850 Pro \n"
         "   SSD, it takes about three minutes to perform an extraction on the entire \n"
         "   database.\n"
         "Parse: Parses the extracted optimized database and stores the results in a \n"
         "   .csv file. On an i7 5820k and 850 Pro SSD, it takes about ten seconds to \n"
         "   parse the entire database.\n\n");

      Utility::print("Usage: Download the database, perform an extraction, and then \n"
         "   parse it. Open the resulting .csv file and check for undesirable results. \n"
         "   Note the file name of the unsatisfactory result, and look in the formatted \n"
         "   database for that file. Open that file and determine what went wrong. \n"
         "   Modify the appropriate  keywords file in the keywords folder to resolve \n"
         "   the issue, then reparse.\n---\n");

      while(true)
      {
         //Get the command
         std::string line;
         Utility::print("> ");
         std::getline(std::cin, line);
         Utility::print("\n");

         //Extract the command's tokens
         std::vector<std::string> tokens{String::split(line, " ")};
         if(tokens.size() != 5){ Utility::print("Invalid format.\n\n"); continue; }

         //Parse the start and end dates
         Date date;
         try
         {
            //Set
            date.year = std::stoi(tokens[1]);
            date.quarter = std::stoi(tokens[2]);
            date.end_year = std::stoi(tokens[3]);
            date.end_quarter = std::stoi(tokens[4]);

            //Validate
            if(date.year < 1993 || !valid_quarter(date.quarter) ||
               !valid_quarter(date.end_quarter) || date.end_year < date.year ||
               (date.end_year == date.year && date.end_quarter < date.quarter))
               throw Exception{};
         }
         catch(...){ Utility::print("Invalid date.\n\n"); continue; }

         //Execute the command
         if(tokens[0] == "download") Downloader::download(date);
         else if(tokens[0] == "extract") Extractor::extract(date);
         else if(tokens[0] == "parse") Parser::parse(date);
         else{ Utility::print("Invalid action.\n\n"); continue; }

         Utility::print("\n");
      }
   }
   catch(const Exception& e){ Utility::print("Main-level error: "+e.message+".\n"); }
	catch(...){ Utility::print("Main-level external error.\n"); }

   Request::destroy();
   system("pause");
}