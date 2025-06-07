#include <generator.h>
#include <fstream>

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
    generate_struct(fout, blocks, struct_name);
}

void Generator::generate_headers(std::ofstream& fout, const std::string& file_name)
{
    std::string headers_code = "";
    headers_code += "#include \"" + file_name + "_run.h\"\n";
    headers_code += "#include <math.h>\n";
    fout << headers_code;
}

void Generator::generate_struct(std::ofstream& fout, const ParserResult& blocks, const std::string& struct_name)
{
    std::string struct_code = "static struct\n{\n";
    for (const auto& [sid, block_ptr]: blocks)
    {
        struct_code += "\tdouble " + block_ptr->name + ";\n";
    }
    struct_code += "} " + struct_name + ";\n";
    fout << struct_code;
}

}