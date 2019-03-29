#ifndef TESTS_HPP_INCLUDED
#define TESTS_HPP_INCLUDED

namespace _tests
{
    void SetProcessPriority_low();
    void SetProcessPriority_lowest();

    void Test(); // two neurons
    void Test2(); // massive blob of neurons
    void Test3(); // single neuron ramp-up/down
    void Test4(); // population, speciation, mutation, simple fitness testing
    void Test5(); // splicing
    void Test6(); // population, speciation, mutation, simple fitness testing, the whole shebang!
    void Test7(); // population, speciation, mutation, simple fitness testing, the whole shebang! but with a more difficult fitness function
}

#endif // TESTS_HPP_INCLUDED
