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
    void generate_struct(std::ofstream& fout, const std::string& struct_name);
    void generate_init_method(std::ofstream& fout, const std::string& struct_name);
    void generate_step_method(std::ofstream& fout, const std::string& struct_name);
    void generate_ext_ports(std::ofstream& fout, const std::string& struct_name);

    std::string generate_step_method_string(const std::shared_ptr<BaseBlock>& block_ptr, const std::string& struct_name);

    ParserResult blocks;

};

}