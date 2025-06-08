#include "parser.h"
#include <iostream>
#include <algorithm>

namespace generator
{
    namespace
    {
        namespace xml = tinyxml2;
    }

    Parser::Parser(const std::string file_path)
    {
        if (!doc.LoadFile(file_path.c_str()) == xml::XMLError::XML_SUCCESS)
        {
            std::cerr << "Read xml file error\n";
            std::abort();
        }
    }

    ParserResult Parser::parse()
    {
        xml::XMLElement *root = doc.FirstChildElement("System");
        if (!root)
        {
            std::cerr << "Parser: no <System> root found." << std::endl;
            return ParserResult();
        }

        ParserResult parser_res = parse_blocks(root);

        parse_lines(parser_res, root);
        
        return parser_res;
    }

    ParserResult Parser::parse_blocks(tinyxml2::XMLElement *root_xml)
    {
        ParserResult parser_res;
        for (xml::XMLElement *block_xml = root_xml->FirstChildElement("Block"); block_xml != nullptr; block_xml = block_xml->NextSiblingElement("Block"))
        {
            std::shared_ptr<BaseBlock> block_ptr;
            BlockType block_type;
            const char *name = block_xml->Attribute("Name");
            const char *sid = block_xml->Attribute("SID");
            const char *block_type_str = block_xml->Attribute("BlockType");
            if (!is_block_correct(name, sid, block_type_str))
            {
                throw std::invalid_argument("Parser: invalid block (no params)");
            }
            try
            {
                block_type = get_block_type(block_type_str);
            }
            catch(const std::invalid_argument& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }

            if (is_operation(block_type))
            {
                block_ptr = std::make_shared<OperationBlock>();
            }
            else if (block_type == BlockType::UNIT_DELAY)
            {
                block_ptr = std::make_shared<UnitDelayBlock>();
            }
            else
            {
                block_ptr = std::make_shared<BaseBlock>();
            }

            block_ptr = parse_block(block_ptr, block_xml, name, sid, block_type);
            parser_res.insert({block_ptr->sid, block_ptr});
        }
        return parser_res;
    }

    bool Parser::is_block_correct(const char* name, const char* sid, const char* type)
    {
        if (name && sid && type)
            return true;
        return false;
    }

    std::shared_ptr<BaseBlock> Parser::parse_block(const std::shared_ptr<BaseBlock>& block_ptr, tinyxml2::XMLElement *block_xml, const std::string& name, 
                                           const std::string& sid, BlockType block_type)
    {
        if (!block_ptr)
        {
            throw std::logic_error("Empty block pointer in parse_block Parser's method");
        }

        std::string name_no_spaces = name;
        name_no_spaces.erase(std::remove_if(name_no_spaces.begin(), name_no_spaces.end(), [](unsigned char c)
                                 { return std::isspace(c); }), name_no_spaces.end());
        block_ptr->name = name_no_spaces;
        block_ptr->sid = std::stoi(sid);
        block_ptr->type = block_type;
        block_ptr->is_port = false;

        for (xml::XMLElement *port = block_xml->FirstChildElement("Port"); port != nullptr; port = port->NextSiblingElement("Port"))
        {
            for (xml::XMLElement *param = port->FirstChildElement("P"); param != nullptr; param = param->NextSiblingElement("P"))
            {
                const char *param_name = param->Attribute("Name");
                const char *param_value = param->GetText();
                if (param_name && param_value)
                {
                    std::string param_name_str = param_name;
                    std::string param_value_str = param_value;
                    block_ptr->is_port = true;
                    if (param_name_str == "Name")
                        block_ptr->port_name = param_value_str;
                }
            }
        }

        if (is_operation(block_type))
            add_operation_block_info(block_ptr, block_xml);
        
        return block_ptr;
    } 
    
    void Parser::add_operation_block_info(const std::shared_ptr<BaseBlock>& block_ptr, tinyxml2::XMLElement *block_xml)
    {
        if (!block_ptr)
        {
            throw std::logic_error("Empty block pointer in add_operation_block_info Parser's method");
        }

        std::shared_ptr<OperationBlock> oper_block_ptr = std::dynamic_pointer_cast<OperationBlock>(block_ptr);
        oper_block_ptr->inputs = "";

        for (xml::XMLElement *param = block_xml->FirstChildElement("P"); param != nullptr; param = param->NextSiblingElement("P"))
        {
            const char *param_name = param->Attribute("Name");
            const char *param_value = param->GetText();
            if (param_name && param_value)
            {
                auto param_name_str = std::string(param_name);
                auto param_value_str = std::string(param_value);
                if (param_name_str == "Inputs")
                {
                    oper_block_ptr->inputs = param_value_str;
                }
                else if (param_name_str == "Gain")
                {
                    oper_block_ptr->gain = std::stod(param_value_str);
                }
            }
        }
    }

    void Parser::parse_lines(ParserResult& blocks_ptr, xml::XMLElement *root_xml)
    {
        if (blocks_ptr.size() == 0)
            throw std::logic_error("Empty blocks map in parse_lines Parser's method");
        for (xml::XMLElement *line_xml = root_xml->FirstChildElement("Line"); line_xml != nullptr; line_xml = line_xml->NextSiblingElement("Line"))
        {
            parse_line(blocks_ptr, line_xml);  
        }
    }

    void Parser::parse_line(ParserResult& blocks_ptr, tinyxml2::XMLElement *line_xml)
    {
        size_t src_sid;
        std::vector<std::pair<uint8_t, size_t>> dsts; //dst_in_port - dst_sid
        for (xml::XMLElement *param = line_xml->FirstChildElement("P"); param != nullptr; param = param->NextSiblingElement("P"))
        {
            const char *param_name = param->Attribute("Name");
            const char *param_value = param->GetText();
            if (param_name && param_value)
            {
                auto param_name_str = std::string(param_name);
                auto param_value_str = std::string(param_value);
                if (param_name_str == "Src")
                {
                    src_sid = std::stoi(param_value_str);
                    if (blocks_ptr.find(src_sid) == blocks_ptr.end())
                        throw std::out_of_range("Parser: line has block with sid = " + std::to_string(src_sid) + "which doesn't parsed");
                }
                else if (param_name_str == "Dst")
                {
                    size_t dst_sid = std::stoi(param_value_str);
                    if (blocks_ptr.find(dst_sid) == blocks_ptr.end())
                        throw std::out_of_range("Parser: line has block with sid = " + std::to_string(dst_sid) + "which doesn't parsed");
                    size_t colon_pos = param_value_str.find(':');
                    size_t dst_port = 0;
                    if (colon_pos != std::string::npos)
                    {
                        dst_port = std::stoi(param_value_str.substr(colon_pos + 1));
                    }
                    dsts.push_back({dst_port, dst_sid});
                }
            }
        }
        parse_branch(dsts, blocks_ptr, line_xml);

        for (const auto& [dst_port, dst_sid]: dsts)
        {
            blocks_ptr[src_sid]->next_blocks.push_back(blocks_ptr[dst_sid]);

            if (is_operation(blocks_ptr[dst_sid]->type))
            {
               std::shared_ptr<OperationBlock> oper_block_ptr = std::dynamic_pointer_cast<OperationBlock>(blocks_ptr[dst_sid]);
               oper_block_ptr->in_ports.insert({dst_port, blocks_ptr[src_sid]});
            }

            if (blocks_ptr[dst_sid]->type == BlockType::UNIT_DELAY)
            {
                std::shared_ptr<UnitDelayBlock> ud_block_ptr = std::dynamic_pointer_cast<UnitDelayBlock>(blocks_ptr[dst_sid]);
                ud_block_ptr->input = blocks_ptr[src_sid];
            }
        }
    }

    void Parser::parse_branch(std::vector<std::pair<uint8_t, size_t>>& dsts, const ParserResult& blocks_ptr, tinyxml2::XMLElement *line_xml)
    {
        for (xml::XMLElement *branch = line_xml->FirstChildElement("Branch"); branch != nullptr; branch = branch->NextSiblingElement("Branch"))
        {
            for (xml::XMLElement *param = branch->FirstChildElement("P"); param != nullptr; param = param->NextSiblingElement("P"))
            {
                const char *param_name = param->Attribute("Name");
                const char *param_value = param->GetText();
                if (param_name && param_value)
                {
                    auto param_name_str = std::string(param_name);
                    auto param_value_str = std::string(param_value);
                    if (param_name_str == "Dst")
                    {
                        size_t dst_sid = std::stoi(param_value_str);
                        if (blocks_ptr.find(dst_sid) == blocks_ptr.end())
                            throw std::out_of_range("Parser: line has block with sid = " + std::to_string(dst_sid) + "which doesn't parsed");
                        size_t colon_pos = param_value_str.find(':');
                        size_t dst_port = 0;
                        if (colon_pos != std::string::npos)
                        {
                            dst_port = std::stoi(param_value_str.substr(colon_pos + 1));
                        }
                        dsts.push_back({dst_port, dst_sid});
                    }
                }
            }
        }
    }
}