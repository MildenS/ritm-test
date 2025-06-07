#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>

namespace generator
{

enum BlockType
{
    INPORT = 0,
    OUTPORT,
    SUM,
    GAIN,
    UNIT_DELAY
};

struct BaseBlock
{
    std::string name;
    BlockType type;
    size_t sid;
    std::weak_ptr<BaseBlock> next_block;
    bool is_port;
    virtual ~BaseBlock() {};
};

inline BlockType get_block_type(const std::string& type_str)
{
    if (type_str == "Inport")
        return BlockType::INPORT;
    else if (type_str == "Outport")
        return BlockType::OUTPORT;
    else if (type_str == "Gain")
        return BlockType::GAIN;
    else if (type_str == "Sum")
        return BlockType::SUM;
    else if (type_str == "UnitDelay")
        return BlockType::UNIT_DELAY;
    else
    {
        throw std::invalid_argument("Parser: invalid block type string " + type_str);
    }
}

inline bool is_operation(const BlockType& type)
{
    return type == BlockType::GAIN || type == BlockType::SUM;
}

struct OperationBlock: BaseBlock
{
    std::string inputs; //for add operations it may have value like "+-"; empty string means "++"
    std::unordered_map<uint8_t, std::weak_ptr<BaseBlock>> in_ports; // in_port-value
    double gain; //for gain operation
};

using ParserResult = std::unordered_map<size_t, std::shared_ptr<BaseBlock>>; //sid - block pointer

}