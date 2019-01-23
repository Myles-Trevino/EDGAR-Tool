#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <Windows.h>
#include "String.hpp"
#include "Formatter.hpp"
#include "String.hpp"
#include "Filer.hpp"
#include "Parser.hpp"

namespace
{
   struct Document
   {
      std::string data, caseless_data, file,
         date, type, cik, company, //Obtained in the index
         filer_cik, filer_company, cusip, percentage, item_4, status; //Parsed
   };

   std::string label, save_buffer;
   uint32_t count, quarter_count, warning_count,
      cusips_found, percents_found, statuses_found, item_4s_found;
   std::ofstream file, log_file;
   std::mutex save_buffer_mutex, listings_mutex;
   std::vector<Document> documents;

   struct Keyword_Pair{ std::string start_identifier, end_identifier; };
   std::vector<Keyword_Pair> percentage_keywords;
   std::vector<Keyword_Pair> item_4_keywords;
   std::vector<std::string> active_status_keywords;
   std::vector<std::string> inactive_status_keywords;
   std::vector<std::string> no_status_keywords;

   //Document
   std::string extract(const Document& document, const std::string& start_indentifier,
      const std::string& end_indentifier, uint32_t maximum_length = 0, const size_t& offset = 0)
   {
      size_t start{document.caseless_data.find(start_indentifier, offset)};
      if(start == std::string::npos) return {};
      start += start_indentifier.size();

      size_t end{document.caseless_data.find(end_indentifier, start)};
      if(end == std::string::npos) return {};

      size_t length{end-start};
      if(maximum_length && length > maximum_length) return {};

      return end ? document.data.substr(start, end-start) : document.data.substr(start);
   }

   std::string get_cusip(const Document& document)
   {
      std::string cusip{extract(document, "CLASS OF SECURITIES)", "(CUSIP", 200)};
      if(cusip.empty()) return "Unknown";
      cusip = String::trim(cusip);
      if(cusip.empty()) return "None";
      return cusip;
   }

   std::string get_filer_cik(const Document& document)
   {
      std::string filer_cik{extract(document, "CENTRAL INDEX KEY:",
         ":", 100, document.caseless_data.find("ADDRESS"))};
      filer_cik = String::make_numeric(filer_cik);
      if(filer_cik.empty()) return "Unknown";
      return filer_cik;
   }

   std::string get_filer_company(const Document& document)
   {
      std::string filer_company{extract(document, "COMPANY CONFORMED NAME:",
         "CENTRAL", 100, document.caseless_data.find("ADDRESS"))};
      filer_company = String::trim(filer_company);
      if(filer_company.empty()) return "Unknown";
      return filer_company;
   }

   std::string get_percentage(const Document& document)
   {
      std::string percentage;

      //Attempt to parse the value
      for(Keyword_Pair k : percentage_keywords)
      {
         //Parse and initial formatting
         percentage = extract(document, k.start_identifier, k.end_identifier, 60);
         if(percentage.empty()) continue;
         percentage = String::remove(String::trim(String::make_uppercase(percentage)), "%");

         //Evaluate for strings
         if(String::contains(percentage, "NONE")) return "0.00%";
         if(String::contains(percentage, "ITEM 5")) return "Undisclosed";
         
         //Trim the start
         size_t start{};
         for(size_t i{}; i < percentage.size(); ++i)
         {
            char c{percentage[i]};
            if(!String::is_digit(c) && c != '.') start = i;
         }
         percentage = percentage.substr(start+1);

         //Deduplicate decimals
         percentage = String::deduplicate(percentage, '.');

         //Check that there is a number
         percentage = String::make_numeric(percentage);
         if(percentage.empty() || percentage == ".") return "Not given";

         try{ return std::to_string(std::stof(percentage))+'%'; }
         catch(...){ return percentage+'%'; }
      }

      //Otherwise return the default value
      return "Unknown";
   }

   void get_item_4_and_status(Document *document)
   {
      //Attempt to parse the values
      for(Keyword_Pair k : item_4_keywords)
      {
         //Parse
         std::string item_4{extract(*document, k.start_identifier, k.end_identifier, 100000)};
         if(item_4.empty()) continue;
         std::string item_4_case_insensitive{String::make_uppercase(item_4)};

         //Trim the start
         size_t start{};
         for(size_t i{}; i < item_4.size(); ++i)
         {
            char c{item_4[i]};
            if(!String::is_space(c) && c != 's' && c != '.' && c != ':'){ start = i; break; }
         }

         //Check for content
         item_4 = item_4.substr(start);
         if(item_4.empty() || !start){ document->item_4 = "None"; document->status = "N/A"; return; }

         //Set item 4
         document->item_4 = String::trim(item_4.substr(0, 4096));

         //Get the status
         for(const std::string& a : active_status_keywords)
            if(String::contains(item_4_case_insensitive, a)){ document->status = "Active"; return; }

         for(const std::string& i : inactive_status_keywords)
            if(String::contains(item_4_case_insensitive, i)){ document->status = "Inactive"; return; }

         for(const std::string& a : no_status_keywords)
            if(String::contains(item_4_case_insensitive, a)){ document->status = "N/A"; return; }

         document->status = "Unknown";
         return;
      }

      //Otherwise use the default values
      document->item_4 = document->type == "SC 13D/A" ? "None" : "Unknown";
      document->status = document->type == "SC 13D/A" ? "N/A" : "Unknown";
   }

   //Print
   void print_percentage(const std::string& prefix, uint32_t count)
   {
      float percentage{100};
      if(quarter_count) percentage = count/static_cast<float>(quarter_count)*100;
      Utility::print(prefix+std::to_string(percentage)+"% ("+std::to_string(count)+
         "/"+std::to_string(quarter_count)+").\n", &log_file);
   }

   void print_status(){ Utility::mutex_print(label+": "+std::to_string(quarter_count)+" of "+
      std::to_string(documents.size())+". Total: "+std::to_string(count)+".\r"); }

   void print_thread_function(bool *active){ while(*active){ print_status(); Sleep(100); } }

   //Parse
   void parser_thread_function(std::vector<uint32_t> indices)
   {
      Utility::set_thread_priority();
      for(const uint32_t& i : indices)
      {
         //Atomic, no need for a mutex
         ++count;
         ++quarter_count;
         Document *document{&documents[i]};

         try
         {
            //Format
            document->data = Filer::decompress(document->data);
            document->caseless_data = String::make_uppercase(document->data);

            //Parse
            document->cusip = get_cusip(*document);
            document->filer_cik = get_filer_cik(*document);
            document->filer_company = get_filer_company(*document);
            document->percentage = get_percentage(*document);
            get_item_4_and_status(document);

            //Update counters
            if(document->cusip != "Unknown") ++cusips_found;
            if(document->percentage != "Unknown") ++percents_found;
            if(document->item_4 != "Unknown") ++item_4s_found;
            if(document->status != "Unknown") ++statuses_found;

            //Save
            std::lock_guard<std::mutex> lock_guard{save_buffer_mutex};
            save_buffer +=
               document->date+','+
               document->type+','+
               document->cik+",\""+
               String::replace(document->company, "\"", "\"\"")+"\","+
               document->filer_cik+",\""+
               String::replace(document->filer_company, "\"", "\"\"")+"\","+
               document->cusip+','+
               document->percentage+",\""+
               String::replace(document->item_4, "\"", "\"\"")+"\","+
               document->status+','+
               document->file+'\n';
         }
         catch(const Exception& e){ Utility::mutex_print(label+": Warning: Encountered an "
            "error parsing: "+document->file+": "+e.message+".\n"); ++warning_count; }
         catch(const std::exception& e){ Utility::mutex_print(label+": Warning: Encountered an external "
            "error parsing: "+document->file+": "+e.what()+".\n"); ++warning_count; }
      }
   }

   //Keywords
   std::vector<Keyword_Pair> get_keyword_pairs(const std::string& file)
   {
      std::vector<std::string> lines{Filer::load_lines("Keywords/"+file)};
      std::vector<Keyword_Pair> results;
      for(std::string& l : lines)
      {
         if(l.size() < 3) continue; //Ignore empty/insufficient lines
         if(l.substr(0, 2) == "//") continue; //Ignore comments

         std::vector<std::string> tokens{String::split(l, "|", true)};
         if(tokens.size() != 2) continue; //Ignore invalid formatting

         results.push_back({tokens[0], tokens[1]});
      }
      return results;
   }

   std::vector<std::string> get_keywords(const std::string& file)
   {
      std::vector<std::string> lines{Filer::load_lines("Keywords/"+file)};
      std::vector<std::string> results;
      for(std::string& l : lines)
      {
         if(l.empty()) continue; //Ignore empty/insufficient lines
         if(l.size() >= 2 && l.substr(0, 2) == "//") continue; //Ignore comments
         results.emplace_back(l);
      }
      return results;
   }

   void load_keywords()
   {
      try
      {
         percentage_keywords = get_keyword_pairs("Percentage Keywords.txt");
         item_4_keywords = get_keyword_pairs("Item 4 Keywords.txt");
         active_status_keywords = get_keywords("Active Status Keywords.txt");
         inactive_status_keywords = get_keywords("Inactive Status Keywords.txt");
      }
      catch(...){ throw Exception{"Could not load the keywords"}; }
   }

   //Load
   std::vector<Document> loader_documents;
   void load(const Date& date)
   {
      loader_documents.clear();
      std::string file_name{Utility::get_label(date)},
         full_file_name{"Optimized Database\\"+file_name+".lz4"};

      std::string data;
      try{ data = Filer::load(full_file_name); }
      catch(...){ return; }

      try
      {
         size_t index{};
         while(true)
         {
            index = data.find("<DOCUMENT>", index);
            if(index == std::string::npos) break;

            //Get the raw document
            size_t end{data.find("</DOCUMENT>", index)};
            std::string raw_document{data.data()+index+11, (end-12)-index};

            //Extract the index line tokens
            size_t start{raw_document.find('\n')};
            std::string index_line{raw_document.substr(0, start)};
            std::vector<std::string> tokens{String::split(index_line, "|")};
            if(tokens.size() != 5) throw Exception{"Invalid index line"};

            //Create the document
            Document document;
            std::string id{String::extract(tokens[4], "data/", ".")};
            std::replace(id.begin(), id.end(), '/', '-');
            document.file = tokens[3]+'-'+id;

            document.date = tokens[3];
            document.type = tokens[2];
            document.cik = tokens[0];
            document.company = tokens[1];
            document.data = raw_document.substr(start+1);

            //Add the document
            loader_documents.emplace_back(document);
            index = end;
         }
      }
      catch(const Exception& e){ throw Exception{"Encountered an "
         "error loading: "+file_name+": "+e.message}; }
      catch(const std::exception& e){ throw Exception{"Encountered "
         "an external error loading: "+file_name+": "+e.what()}; }
   }
}

void Parser::parse(Date date)
{
   try
   {
      //Initialize
      warning_count = 0;

      log_file.open("Parser Log.txt");
      Utility::print("Parsing the EDGAR database.\n---\n", &log_file);

      load_keywords();

      file.open("EDGAR.csv");
      file<<"Date,Type,CIK,Company,Filer CIK,Filer Company,CUSIP,Percentage,Item 4,Status,File\n";

      load(date);
      while(true)
      {
         //Initialize the quarter
         quarter_count = 0, cusips_found = 0, percents_found = 0,
            statuses_found = 0, item_4s_found = 0;
         label = Utility::get_label(date);

         //Launch the parser threads
         documents = loader_documents;
         Utility::print(label+": Parsing the documents.\n");
         std::vector<std::thread> parser_threads{Utility::launch_threads(static_cast<uint32_t>(
            documents.size()), parser_thread_function, Utility::get_parser_thread_count())};

         //Launch the print thread
         bool print_thread_active{true};
         std::thread print_thread{print_thread_function, &print_thread_active};

         //Load
         Date next_date{Utility::get_next_date(date)};
         if(next_date.year) load(next_date);

         //Wait for the parser threads to finish
         for(std::thread& t : parser_threads) t.join();

         //Wait for the print thread to finish
         print_thread_active = false;
         print_thread.join();

         //Write the output buffer to the CSV file
         file<<save_buffer;
         save_buffer.clear();

         //Print the statistics for the quarter
         print_status();
         Utility::print('\n'+label+": Parsed the documents.\n");
         print_percentage(label+": Documents with parsable CUSIPs: ", cusips_found);
         print_percentage(label+": Documents with parsable percentages: ", percents_found);
         print_percentage(label+": Documents with parsable item 4s: ", item_4s_found);
         print_percentage(label+": Documents with determinable statuses: ", statuses_found);
         if(!Utility::is_last_quarter(date)) Utility::print("\n", &log_file);

         //Print a separator for each year
         if(next_date.year != date.year) Utility::print("---\n", &log_file);

         if(!next_date.year) break; //Break if past the end year
         date = next_date;
      }

      //Print the final statistics
      Utility::print("Warnings: "+std::to_string(warning_count)+".\n", &log_file);
      Utility::print("---\nFinished parsing the EDGAR database.\n", &log_file);
      file.close();
      log_file.close();
   }
   catch(const Exception& e){ Utility::print("Parser error: "+e.message+".\n"); }
   catch(...){ Utility::print("Parser external error.\n"); }
}