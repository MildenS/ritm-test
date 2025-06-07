#pragma once

#include <parser_types.h>


namespace generator
{

class Generator
{

public:

    Generator(const ParserResult&& blocks);
    void generate_code(const std::string& struct_name = "nwocg", const std::string& file_name = "nwocg");

private:

    void generate_headers(std::ofstream& fout, const std::string& file_name);
    void generate_struct(std::ofstream& fout, const ParserResult& blocks, const std::string& struct_name);

    ParserResult blocks;

};

}