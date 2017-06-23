#include <cr/common/assert.h>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    try
    {
        int a = 1;
        std::string b("2");
        CR_ASSERT(a != 1)(a)(b);
    }
    catch (cr::AssertError& e)
    {
        std::cerr << e.what() << std::endl;
    }
}