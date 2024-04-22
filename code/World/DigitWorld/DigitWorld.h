//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#pragma once    // directive to insure that this .h file is only included one time

#include <World/AbstractWorld.h> // AbstractWorld defines all the basic function templates for worlds
#include <string>
#include <memory> // shared_ptr
#include <map>

#include "../../Analyze/brainTools.h"

using std::shared_ptr;
using std::string;
using std::map;
using std::unordered_map;
using std::unordered_set;
using std::to_string;

class DigitWorld : public AbstractWorld {
private:
    int outputNodesCount, inputNodesCount;
public:
    // parameters for group and brain namespaces
    static shared_ptr<ParameterLink<int>> evaluationsPerGenerationPL;
    static shared_ptr<ParameterLink<int>> defaultWorldUpdatesPL;
	static shared_ptr<ParameterLink<int>> defaultRetinaTypePL;
	static shared_ptr<ParameterLink<string>> numeralDataFileNamePL;
    static shared_ptr<ParameterLink<bool>> allowDoneBitPL;
    
    // a local variable used for faster access to the ParameterLink value
    int evaluationsPerGeneration;
    
    std::string groupName = "root::";
    std::string brainName = "root::";
    
    int worldUpdates;
    int worldSize; //width and height because square.
    int retinaType;
    string numeralDataFileName;
    bool allowDoneBit;
    
	std::vector<std::vector<int>>numeralData; // [number][2D matrix stored in a vector]

    std::vector<std::vector<bool>>exploredCells;

    std::vector<std::pair<int, int>> retinalOffsets = {{0, 0},{1, 0},{0, 1},{1, 1}, //2x2
                                                        {0, 2},{1, 2},{2, 2},{2,1},{2,0}, //3x3
                                                        {0,3},{1, 3},{2, 3},{3, 3},{3,2},{3,1},{3,0} //4x4
                                                    };
	// retina is a list of offsets defining input sensor array to brain
	//  0   1    8   15
    //  2   3    7   14
    //  4   5    6   13
    //  9  10   11   12
    int retinaSensors, stepSize;

    DigitWorld(shared_ptr<ParametersTable> PT);
    virtual ~DigitWorld() = default;

    virtual double evaluateOrganism(std::shared_ptr<Organism> /*org*/, int /*analyze*/, int /*visualize*/, int /*debug*/, bool /*isFirst*/, bool /*calcRepresentation*/);
    virtual auto evaluate(map<string, shared_ptr<Group>>& /*groups*/, int /*analyze*/, int /*visualize*/, int /*debug*/) -> void override;

    virtual auto requiredGroups() -> unordered_map<string,unordered_set<string>> override;
};

