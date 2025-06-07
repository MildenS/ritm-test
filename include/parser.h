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

    bool is_block_correct(const char* name, const char* sid, const char* type);
    std::shared_ptr<BaseBlock> parse_block(const std::shared_ptr<BaseBlock>& block_ptr, tinyxml2::XMLElement *block_xml, const std::string& name, 
                                           const std::string& sid, BlockType block_type);
    void add_operation_block_info(const std::shared_ptr<BaseBlock>& block_ptr, tinyxml2::XMLElement *block_xml);

    void parse_lines(const ParserResult& blocks_ptr, tinyxml2::XMLElement *block_xml);

    tinyxml2::XMLDocument doc;

};

}