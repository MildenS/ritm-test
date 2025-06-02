#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

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
    virtual ~BaseBlock();
};

struct OperationBlock: BaseBlock
{
    std::string signs; //for add operations it may have value like "+-"; empty string means "++"
    std::unordered_map<uint8_t, std::weak_ptr<BaseBlock>> in_ports; // port-value
    double value; //for gain operation
};

using ParserResult = std::vector<std::shared_ptr<BaseBlock>>;

}