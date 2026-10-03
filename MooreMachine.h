#pragma once
#include <fstream>
#include <limits>
#include <map>
#include <string>

#include "Trim.h"
#include "types.h"

inline void WriteMooreOutput(const std::string& filename, const MooreMachine& moore)
{
    std::ofstream file(filename);

    file << "type: moore" << std::endl;
    file << "start: " << moore.start << std::endl << std::endl;

    file << "states:" << std::endl;
    for (const auto& state : moore.states)
    {
        file << state.id << " | " << state.origId << " / " << state.output << std::endl;
    }

    file << std::endl << "transitions:" << std::endl;
    for (const auto& tr : moore.transitions)
    {
        file << tr.src << " " << tr.dst << " " << tr.input << std::endl;
    }
}

inline MealyMachine ConvertMooreToMealy(const MooreMachine& moore)
{
    struct MooreStateInfo
    {
        std::string origId;
        std::string output;
    };

    std::map<std::string, MooreStateInfo> stateInfos;
    for (const auto& state : moore.states)
    {
        stateInfos[state.id] = {state.origId, state.output};
    }

    MealyMachine mealy;
    for (const auto& transition : moore.transitions)
    {
        MealyTransition mealyTransition = {
            .src = stateInfos[transition.src].origId,
            .dst = stateInfos[transition.dst].origId,
            .input = transition.input,
            .output = stateInfos[transition.dst].output,
        };
        auto it = std::find_if(
            mealy.transitions.begin(),
            mealy.transitions.end(),
            [&mealyTransition](const MealyTransition& t) {
            return t.src == mealyTransition.src &&
                   t.dst == mealyTransition.dst &&
                   t.input == mealyTransition.input &&
                   t.output == mealyTransition.output;
        });
        if (it == mealy.transitions.end())
        {
            mealy.transitions.push_back(mealyTransition);
        }
    }
    mealy.start = stateInfos[moore.start].origId;
    return mealy;
}

inline MooreMachine ParseMooreInput(const std::string& filename)
{
    MooreMachine moore;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open input file");
    }
    std::string line;
    bool inTransitions = false;
    bool isStates = false;

    while (std::getline(file, line))
    {
        if (line.rfind("start:", 0) == 0)
        {
            moore.start = line.substr(6);
            Trim(moore.start);
        }
        else if (line == "states:")
        {
            inTransitions = false;
            isStates = true;
        }
        else if (line == "transitions:")
        {
            isStates = false;
            inTransitions = true;
        }
        else if (isStates && !line.empty())
        {
            std::stringstream ss(line);
            std::string id, vertSlash, origId, slash, outputLabel;

            ss >> id >> vertSlash >> origId >> slash >> outputLabel;
            moore.states.push_back({id, origId, outputLabel});
        }
        else if (inTransitions)
        {
            std::stringstream ss(line);
            std::string src, dst, input;

            ss >> src >> dst >> input;
            moore.transitions.push_back({src, dst, input});
        }
    }
    return moore;
}

inline MooreMachine OptimazeMoore(const MooreMachine& moore)
{
    std::set<std::string> uniqInputs;
    for (const auto& transition : moore.transitions)
    {
        uniqInputs.insert(transition.input);
    }

    std::map<std::string, std::map<std::string, std::string>> transitions;
    for (const auto& transition : moore.transitions)
    {
        transitions[transition.src][transition.input] = transition.dst;
    }

    std::map<std::string, std::vector<std::string>> outputGroups;
    for (const auto& state : moore.states)
    {
        outputGroups[state.output].push_back(state.id);
    }

    size_t classEql = 0;
    std::map<std::string, size_t> idToClass;
    for (const auto& [_, ids] : outputGroups)
    {
        for (const auto& id : ids)
        {
            idToClass[id] = classEql;
        }
        classEql++;
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        std::map<std::string, size_t> nextIdToClass;
        std::map<std::pair<size_t, std::vector<size_t>>, std::vector<std::string>> groupByClassEql;

        for (const auto& state : moore.states)
        {
            auto currentClass = idToClass[state.id];
            std::vector<size_t> classOfState;

            for (const auto& input : uniqInputs)
            {
                if (transitions[state.id].contains(input))
                {
                    classOfState.push_back(idToClass[transitions[state.id][input]]);
                }
                else
                {
                    classOfState.push_back(std::numeric_limits<size_t>::max());
                }
            }

            groupByClassEql[{currentClass, classOfState}].push_back(state.id);
        }

        size_t nextClassEql = 0;
        for (const auto& [_, ids] : groupByClassEql)
        {
            for (const auto& id : ids)
            {
                nextIdToClass[id] = nextClassEql;
            }
            nextClassEql++;
        }

        if (nextClassEql != classEql)
        {
            idToClass = nextIdToClass;
            classEql = nextClassEql;
            changed = true;
        }
    }

    MooreMachine optMoore;
    std::map<size_t, std::string> addedId;

    for (const auto& state : moore.states)
    {
        auto clsEql = idToClass[state.id];
        if (!addedId.contains(clsEql))
        {
            auto newNameForId = "Q" + std::to_string(clsEql);
            addedId[clsEql] = newNameForId;

            MooreState newState;
            newState.id = newNameForId;
            newState.output = state.output;
            newState.origId = state.id;
            optMoore.states.push_back(newState);
        }
    }

    std::set<std::pair<std::pair<std::string, std::string>, std::string>> uniqueTransitions;
    for (const auto& transition : moore.transitions)
    {
        auto newSrc = addedId[idToClass[transition.src]];
        auto newDst = addedId[idToClass[transition.dst]];

        if (!uniqueTransitions.contains({{newSrc, newDst}, transition.input}))
        {
            uniqueTransitions.insert({{newSrc, newDst}, transition.input});
            optMoore.transitions.push_back({newSrc, newDst, transition.input});
        }
    }

    if (!moore.start.empty() && idToClass.contains(moore.start))
    {
        optMoore.start = addedId[idToClass[moore.start]];
    }

    return optMoore;
}