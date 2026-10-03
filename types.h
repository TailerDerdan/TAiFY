#pragma once
#include <string>
#include <vector>

struct MooreState
{
	std::string id;
	std::string origId;
	std::string output;
};

struct MooreTransition
{
	std::string src;
	std::string dst;
	std::string input;
};

struct MooreMachine
{
	std::string start;
	std::vector<MooreState> states;
	std::vector<MooreTransition> transitions;
};

struct MealyTransition
{
	std::string src;
	std::string dst;
	std::string input;
	std::string output;
};

struct MealyMachine
{
	std::string start;
	std::vector<MealyTransition> transitions;
};