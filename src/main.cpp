#include <parser.h>
#include <generator.h>

int main(int argc, char** argv)
{
    generator::Parser parser("/home/sklochkov/ritm-test/data/scheme.xml");
    generator::Generator code_generator(parser.parse());
    code_generator.generate_code();
    return 0;
}