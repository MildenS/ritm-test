#pragma once

#include <parser_types.h>

#include <tinyxml2.h>
#include <string>

namespace generator
{
class Parser
{
public:

    Parser(const std::string file_path);

    ParserResult parse();


private:

    tinyxml2::XMLDocument doc;

};

}