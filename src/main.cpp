#include <parser.h>

int main(int argc, char** argv)
{
    generator::Parser parser("/home/sklochkov/ritm-test/data/scheme.xml");
    parser.parse();
    return 0;
}