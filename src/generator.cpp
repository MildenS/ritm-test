#include <generator.h>
#include <fstream>
#include <unordered_set>
#include <queue>

namespace generator
{

Generator::Generator(const ParserResult&& blocks)
{
    this->blocks = std::move(blocks);
}


void Generator::generate_code(const std::string& struct_name, const std::string& file_name)
{
    std::ofstream fout(file_name + ".c");
    generate_headers(fout, file_name);
    generate_struct(fout, struct_name);
    generate_init_method(fout, struct_name);
    generate_step_method(fout, struct_name);
    generate_ext_ports(fout, struct_name);
}

void Generator::generate_headers(std::ofstream& fout, const std::string& file_name)
{
    std::string headers_code = "";
    headers_code += "#include \"" + file_name + "_run.h\"\n";
    headers_code += "#include <math.h>\n";
    fout << headers_code;
}

void Generator::generate_struct(std::ofstream& fout, const std::string& struct_name)
{
    std::string struct_code = "\nstatic struct\n{\n";
    for (const auto& [sid, block_ptr]: blocks)
    {
        struct_code += "\tdouble " + block_ptr->name + ";\n";
    }
    struct_code += "} " + struct_name + ";\n";
    fout << struct_code;
}

void Generator::generate_init_method(std::ofstream& fout, const std::string& struct_name)
{
    std::string init_code = "\nvoid " + struct_name + "_generated_init()\n{\n";
    for (const auto& [sid, block_ptr]: blocks)
    {
        if (block_ptr->type == BlockType::UNIT_DELAY)
        {
            init_code += "\t" + struct_name + "." + block_ptr->name + " = 0;\n";
        }
    }
    init_code += "}\n";
    fout << init_code;
}

void Generator::generate_step_method(std::ofstream& fout, const std::string& struct_name)
{
    std::string method_code = "\nvoid " + struct_name + "_generated_step()\n{\n";

    //searching for start block (need inport block)
    std::shared_ptr<BaseBlock> start_block_ptr = nullptr;
    for (const auto& [sid, block_ptr]: blocks)
    {
        if (block_ptr->type == BlockType::INPORT)
        {
            start_block_ptr = block_ptr;
            break;
        }
    }

    std::unordered_set<size_t> visited_blocks; //sids of visited blocks
    std::shared_ptr<BaseBlock> current_block_ptr;
    std::queue<std::shared_ptr<BaseBlock>> blocks_queue;
    std::unordered_set<size_t> unit_delay_blocks; //sids of unit delay blocks
    blocks_queue.push(start_block_ptr);

    while (!blocks_queue.empty())
    {

        current_block_ptr = blocks_queue.front();
        blocks_queue.pop();

        if (visited_blocks.find(current_block_ptr->sid) != visited_blocks.end())
            continue;

        bool all_inputs_ready = true;
        if (is_operation(current_block_ptr->type))
        {
            std::shared_ptr<OperationBlock> current_oper_block = std::dynamic_pointer_cast<OperationBlock>(current_block_ptr);
            for (const auto &[_, input_ptr] : current_oper_block->in_ports)
            {
                auto locked_input_ptr = input_ptr.lock();
                if (visited_blocks.find(locked_input_ptr->sid) == visited_blocks.end())
                {
                    all_inputs_ready = false;
                    blocks_queue.push(locked_input_ptr);
                }
            }
        }

        if (!all_inputs_ready)
        {
            blocks_queue.push(current_block_ptr);
            continue;
        }

        if (is_operation(current_block_ptr->type))
        {
            method_code += generate_step_method_string(current_block_ptr, struct_name);
        }
        else if (current_block_ptr->type == BlockType::UNIT_DELAY)
        {
            unit_delay_blocks.insert(current_block_ptr->sid);
        }

        visited_blocks.insert(current_block_ptr->sid);
        for (const auto& next_block: current_block_ptr->next_blocks)
        {
            if (visited_blocks.find(next_block.lock()->sid) == visited_blocks.end())
            {
                blocks_queue.push(next_block.lock());
            }
        }
    }

    for (const auto &ud_block_sid : unit_delay_blocks)
    {
        std::shared_ptr<UnitDelayBlock> ud_block_ptr = std::dynamic_pointer_cast<UnitDelayBlock>(blocks[ud_block_sid]);
        method_code += "\t" + struct_name + "." + ud_block_ptr->name + " = " + struct_name + ".";
        method_code += ud_block_ptr->input.lock()->name + ";\n";
    }

    method_code += "}\n";
    fout << method_code;
}

std::string Generator::generate_step_method_string(const std::shared_ptr<BaseBlock>& block_ptr, const std::string& struct_name)
{
    std::string method_string_code = "\t" + struct_name + "." + block_ptr->name + " = ";
    std::shared_ptr<OperationBlock> current_oper_block = std::dynamic_pointer_cast<OperationBlock>(block_ptr);
    const auto &in_ports = current_oper_block->in_ports;
    if (current_oper_block->type == BlockType::SUM)
    {
        bool has_signs = current_oper_block->inputs != "";
        bool is_first = true;
        for (const auto &[port_num, input_block_ptr] : in_ports)
        {
            if (!is_first || has_signs)
            {
                if (has_signs)
                    method_string_code += " " + std::string(1, current_oper_block->inputs[port_num - 1]) + " ";
                else
                    method_string_code += " + ";
                is_first = false;
            }
            method_string_code += struct_name + "." + input_block_ptr.lock()->name;
            is_first = false;
        }
    }
    else if (current_oper_block->type == BlockType::GAIN)
    {
        bool is_first = true;
        for (const auto &[port_num, input_block_ptr] : in_ports)
        {
            method_string_code += struct_name + "." + input_block_ptr.lock()->name + " * " + std::to_string(current_oper_block->gain);
        }
    }
    method_string_code += ";\n";
    return method_string_code;
}

void Generator::generate_ext_ports(std::ofstream& fout, const std::string& struct_name)
{
    std::string ports_code = "\nstatic const " + struct_name +"_ExtPort\next_ports[] =\n{\n";
    size_t ports_count = 0;
    for (const auto& [_, block_ptr]: blocks)
    {
        if (block_ptr->is_port)
        {
            uint8_t port_code = block_ptr->type == BlockType::INPORT ? 1 : 0;
            ports_code += "\t{ \"" + block_ptr->port_name + "\", &" + struct_name + "." + block_ptr->name + ", " + 
                          std::to_string(port_code) + " },\n";
            ports_count += 1;
        }
    }
    ports_code += "\t{ ";
    for (size_t i = 0; i < ports_count; ++i)
    {
        if (i < ports_count -1)
            ports_code += "0, ";
        else
            ports_code += "0 ";
    }
    ports_code += "},\n};\n";
    ports_code += "\nconst " + struct_name + "_ExtPort * const\n" + struct_name + "_generated_ext_ports = ext_ports;\n";
    ports_code += "\nconst size_t " + struct_name + "_generated_ext_ports_size = sizeof(ext_ports);";
        
    fout << ports_code;
}


}