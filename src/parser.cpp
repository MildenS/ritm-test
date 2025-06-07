#include "parser.h"
#include <iostream>

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
            std::cerr << "No <System> root found." << std::endl;
            return ParserResult();
        }

        ParserResult parser_res;

        for (xml::XMLElement *block_xml = root->FirstChildElement("Block"); block_xml != nullptr; block_xml = block_xml->NextSiblingElement("Block"))
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

        block_ptr->name = name;
        block_ptr->sid = std::stoi(sid);
        block_ptr->type = block_type;
        block_ptr->is_port = false;

        for (xml::XMLElement *port = block_xml->FirstChildElement("Port"); port != nullptr; port = port->NextSiblingElement("Port"))
        {
            for (xml::XMLElement *param = port->FirstChildElement("P"); param != nullptr; param = param->NextSiblingElement("P"))
            {
                const char *paramName = param->Attribute("Name");
                const char *paramValue = param->GetText();
                if (paramName && paramValue)
                {
                    block_ptr->is_port = true;
                    break;
                }
            }
            if (block_ptr->is_port)
                break;
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

    void Parser::parse_lines(const ParserResult& blocks_ptr, xml::XMLElement *block_xml)
    {
        
    }
}