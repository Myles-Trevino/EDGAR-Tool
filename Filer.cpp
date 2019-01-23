#include <Shlobj.h>
#include <LZ4/lz4hc.h>
#include "Utility.hpp"
#include "Filer.hpp"

namespace
{
   struct File_Map{ HANDLE file, map; char *data; DWORD size; };

   File_Map map_file(std::string path, uint32_t size = 0)
   {
      File_Map file_map;
      file_map.file = CreateFile(path.c_str(), !size ? GENERIC_READ : GENERIC_READ|GENERIC_WRITE,
         FILE_SHARE_READ, NULL, !size ? OPEN_EXISTING : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if(file_map.file == INVALID_HANDLE_VALUE) throw Exception{"Could not load the file"};

      if(!size)
      {
         file_map.size = GetFileSize(file_map.file, NULL);
         if(!file_map.size) return {}; //Empty files cannot be mapped
      }
      else file_map.size = size;

      file_map.map = CreateFileMapping(file_map.file, NULL, !size ?
         PAGE_READONLY : PAGE_READWRITE, NULL, file_map.size, NULL);
      if(!file_map.map) throw Exception{"Could not map the file. Error: "+std::to_string(GetLastError())};

      file_map.data = reinterpret_cast<char *>(MapViewOfFile(file_map.map,
         !size ? FILE_MAP_READ : FILE_MAP_WRITE, NULL, NULL, file_map.size));
      if(!file_map.data) throw Exception{"Could not create a view "
         "of the file. Error: "+std::to_string(GetLastError())};

      return file_map;
   }

   void unmap_file(const File_Map& file_map)
   {
      UnmapViewOfFile(file_map.data);
      CloseHandle(file_map.map);
      CloseHandle(file_map.file);
   }

   std::string get_working_directory()
   {
      char directory[MAX_PATH];
      GetCurrentDirectory(MAX_PATH, directory);
      return directory;
   }
}

void Filer::create_directory(const std::string& directory)
{
   std::string test{(get_working_directory()+'\\'+directory)};
   if(SHCreateDirectoryExA(NULL, test.c_str(), NULL) != ERROR_SUCCESS && GetLastError() !=
      ERROR_FILE_EXISTS && GetLastError() != ERROR_ALREADY_EXISTS) throw Exception{"Could not "
      "create a directory for the database. Error: "+std::to_string(GetLastError())};
}

std::string Filer::compress(const std::string& data)
{
   //Compress
   int32_t maximum_compressed_size{LZ4_compressBound(static_cast<int32_t>(data.size()))};
   char *compressed_data{new char[maximum_compressed_size]};
   int32_t compressed_size{LZ4_compress_default(data.c_str(), compressed_data,
      static_cast<int32_t>(data.size()), maximum_compressed_size)};
   if(compressed_size <= 0) throw Exception{"Compression failed"};

   //Return
   std::string result{compressed_data, static_cast<size_t>(compressed_size)};
   delete[] compressed_data;
   return std::to_string(data.size())+' '+result;
}

std::string Filer::decompress(const std::string& data)
{
   //Get the decompressed size
   size_t index{};
   std::string decompressed_size_string;
   while(true)
   {
      char c{data[index++]};
      if(c == ' ' || index >= data.size()) break;
      decompressed_size_string += c;
   }
   uint32_t decompressed_size{static_cast<uint32_t>(
      std::stoi(decompressed_size_string))};
   if(decompressed_size <= 0) return {};

   //Decompress
   char *decompressed_data{new char[decompressed_size]};
   int32_t compressed_size{LZ4_decompress_fast(data.data()+index,
      decompressed_data, decompressed_size)};
   if(compressed_size <= 0)
      throw Exception{"Decompression failed. Error: "+std::to_string(compressed_size)};

   std::string result{decompressed_data, decompressed_size};
   delete[] decompressed_data;
   return result;
}

std::vector<std::string> Filer::load_lines(const std::string& path)
{
   File_Map file_map{map_file(path)};

   std::vector<std::string> result;
   result.emplace_back();
   for(size_t i{}; i < file_map.size; ++i)
   {
      char c{file_map.data[i]};

      if(c == '\0') break;
      if(c == '\r') continue;
      if(c == '\n') result.emplace_back();
      else result.back() += c;
   }

   unmap_file(file_map);
   return result;
}

std::string Filer::load(const std::string& path)
{
   File_Map file_map{map_file(path)};
   std::string result{file_map.data, file_map.size};
   unmap_file(file_map);
   return result;
}

void Filer::save(const std::string& data, const std::string& file)
{
   if(data.empty()) return;
   File_Map file_map{map_file(file, static_cast<uint32_t>(data.size()))};
   CopyMemory(file_map.data, data.c_str(), data.size());
   unmap_file(file_map);
}