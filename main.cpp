#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "MeelyMachine.h"
#include "MooreMachine.h"

int main()
{
    std::string inputMealy = "/home/anton-kozlov/projects/taify/result/input.txt";

    std::string optMealyFile = "/home/anton-kozlov/projects/taify/result/out2/1.txt";
    std::string mooreFile = "/home/anton-kozlov/projects/taify/result/out2/2.txt";
    std::string optMooreFile = "/home/anton-kozlov/projects/taify/result/out2/3.txt";
    std::string optMealyFromOptMooreFile = "/home/anton-kozlov/projects/taify/result/out2/4.txt";

    try
    {
        // auto mealy = ParseMealyInput(inputMealy);
        //
        // auto optMealy = OptimazeMealy(mealy);
        // WriteMealyOutput(optMealyFile, optMealy);
        //
        // auto moore = ConvertMealyToMoore(mealy);
        // WriteMooreOutput(mooreFile, moore);
        //
        // auto optMoore = OptimazeMoore(moore);
        // WriteMooreOutput(optMooreFile, optMoore);
        //
        // auto optMealy2 = OptimazeMealy(ConvertMooreToMealy(optMoore));
        // WriteMealyOutput(optMealyFromOptMooreFile, optMealy2);

        auto moore = ParseMooreInput(inputMealy);

        auto optMoore = OptimazeMoore(moore);
        WriteMooreOutput(optMealyFile, optMoore);

        auto mealy = ConvertMooreToMealy(moore);
        WriteMealyOutput(mooreFile, mealy);

        auto optMealy = OptimazeMealy(mealy);
        WriteMealyOutput(optMooreFile, optMealy);

        auto optMoore2 = OptimazeMoore(ConvertMealyToMoore(optMealy));
        WriteMooreOutput(optMealyFromOptMooreFile, optMoore2);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
