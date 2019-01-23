#pragma once
#include <vector>
#include <string>

struct HTML_Entity{ std::string name, character; };
extern std::vector<HTML_Entity> html_entities;