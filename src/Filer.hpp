#pragma once
#include <Windows.h>
#include <string>
#include <vector>

namespace Filer
{
   void create_directory(const std::string& directory);
   std::string compress(const std::string& data);
   std::string decompress(const std::string& data);
   std::vector<std::string> load_lines(const std::string& path);
   std::string load(const std::string& path);
   void save(const std::string& data, const std::string& file);
}