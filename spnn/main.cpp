#include <iostream>
#include <fstream>
#include <thread>
#include <future>
#include <chrono>

#include "spnn.hpp"
#include "neat.hpp"

#include "tests/tests.hpp"

int main()
{
    std::ios_base::sync_with_stdio( false );

    auto f = std::async( std::launch::async, []{ std::cout << ""; } );

    {
        /*_tests::Test();
        _tests::Test2();
        _tests::Test3();
        _tests::Test4();
        _tests::Test5();
        _tests::Test6();*/
        _tests::Test7();
    }

    return 0;
}
