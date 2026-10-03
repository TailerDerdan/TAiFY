#pragma once
#include <cstdint>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Trim.h"
#include "types.h"

inline MooreMachine ConvertMealyToMoore(const MealyMachine& mealy)
{
	MooreMachine moore;
	std::map<std::string, std::set<std::string>> stateOutputs;

	for (const auto& tr : mealy.transitions)
	{
		stateOutputs[tr.dst].insert(tr.output);
	}

	std::map<std::string, std::vector<std::string>> mooreSubstates;
	for (const auto& [state, outputs] : stateOutputs)
	{
		for (const auto& out : outputs)
		{
			std::string mooreId = state + "_" + out;
			moore.states.push_back({mooreId, state, out});
			mooreSubstates[state].push_back(mooreId);
		}
	}

	for (const auto& tr : mealy.transitions)
	{
		std::string targetMooreId = tr.dst + "_" + tr.output;

		for (const auto& srcMooreId : mooreSubstates[tr.src])
		{
			moore.transitions.push_back({srcMooreId, targetMooreId, tr.input});
		}
	}

	if (!mealy.start.empty())
	{
		auto start = mealy.start;
		auto it = std::find_if(moore.states.begin(), moore.states.end(), [start](const MooreState& mr)
		{
			if (start == mr.origId) return true;
			return false;
		});
		if (it != moore.states.end())
		{
			moore.start = it->id;
		}
	}

	return moore;
}

inline MealyMachine ParseMealyInput(const std::string& filename)
{
	MealyMachine mealy;
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open input file");
	}
	std::string line;
	bool inTransitions = false;

	while (std::getline(file, line))
	{
		if (line.rfind("start:", 0) == 0)
		{
			mealy.start = line.substr(6);
			Trim(mealy.start);
		}
		else if (line == "transitions:")
		{
			inTransitions = true;
		}
		else if (inTransitions)
		{
			std::stringstream ss(line);
			std::string src, dst, inputLabel, slash, outputLabel;

			ss >> src >> dst >> inputLabel >> slash >> outputLabel;
			mealy.transitions.push_back({src, dst, inputLabel, outputLabel});
		}
	}
	return mealy;
}

inline void WriteMealyOutput(const std::string& filename, const MealyMachine& mealy)
{
	std::ofstream file(filename);

	file << "type: mealy" << std::endl;
	file << "start: " << mealy.start << std::endl << std::endl;

	file << "transitions:" << std::endl;
	for (const auto& tr : mealy.transitions)
	{
		file << tr.src << " " << tr.dst << " " << tr.input << " / " << tr.output << std::endl;
	}
}

inline MealyMachine OptimazeMealy(const MealyMachine& mealy)
{
	std::set<std::string> states;
	std::set<std::string> inputs;
	std::map<std::pair<std::string, std::string>, std::pair<std::string, std::string>> transMap;

	for (const auto& tr : mealy.transitions)
	{
		states.insert(tr.src);
		states.insert(tr.dst);
		inputs.insert(tr.input);
		transMap[{tr.src, tr.input}] = {tr.dst, tr.output};
	}

	size_t classEql = 0;
	std::map<std::string, size_t> idToClass;
	std::map<std::vector<std::string>, size_t> outputsToClass;
	for (const auto& st : states)
	{
		std::vector<std::string> outputs;
		for (const auto& in : inputs)
		{
			auto it = transMap.find({st, in});
			if (it != transMap.end())
			{
				outputs.push_back(it->second.second);
			}
		}

		if (!outputsToClass.contains(outputs))
		{
			outputsToClass[outputs] = classEql++;
		}
		idToClass[st] = outputsToClass[outputs];
	}

    bool changed = true;
    while (changed)
    {
    	std::map<std::pair<size_t, std::vector<size_t>>, size_t> newOutputsToClass;
    	std::map<std::string, size_t> nextIdToClass;

    	size_t newClassEql = 0;
    	for (const auto& st : states)
    	{
    		size_t curClass = idToClass[st];
    		std::vector<size_t> curOutputsClasses;

    		for (const auto& in : inputs)
    		{
    			auto it = transMap.find({st, in});
    			if (it != transMap.end())
    			{
    				curOutputsClasses.push_back(idToClass[it->second.first]);
    			}
    		}

    		auto signature = std::make_pair(curClass, curOutputsClasses);
    		if (!newOutputsToClass.contains(signature))
    		{
    			newOutputsToClass[signature] = newClassEql++;
    		}
    		nextIdToClass[st] = newOutputsToClass[signature];
    	}

    	if (nextIdToClass == idToClass)
    	{
    		changed = false;
    	}
    	else
    	{
    		idToClass = std::move(nextIdToClass);
    		classEql = newClassEql;
    	}
    }

	std::map<size_t, std::string> classToName;
	for (const auto& [st, cls] : idToClass)
	{
		if (!classToName.contains(cls))
		{
			classToName[cls] = "Q" + std::to_string(cls);
		}
	}

	MealyMachine optMealy;
	std::set<std::tuple<std::string, std::string, std::string, std::string>> addedTransitions;

	for (const auto& tr : mealy.transitions)
	{
		std::string newSrc = classToName[idToClass[tr.src]];
		std::string newDst = classToName[idToClass[tr.dst]];

		auto key = std::make_tuple(newSrc, newDst, tr.input, tr.output);
		if (!addedTransitions.contains(key))
		{
			addedTransitions.insert(key);
			optMealy.transitions.push_back({newSrc, newDst, tr.input, tr.output});
		}
	}

	if (!mealy.start.empty())
	{
		optMealy.start = classToName[idToClass[mealy.start]];
	}

    return optMealy;
}