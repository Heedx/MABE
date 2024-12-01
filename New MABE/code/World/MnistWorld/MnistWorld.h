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

class MnistWorld : public AbstractWorld {

private:
    int inputNodesCount, outputNodesCount;

public:
    // parameters for group and brain namespaces
    static shared_ptr<ParameterLink<int>> evaluationsPerGenerationPL;
    static shared_ptr<ParameterLink<bool>> infiniteWorldPL; 
    static shared_ptr<ParameterLink<int>> worldUpdatesPL;
    static shared_ptr<ParameterLink<int>> imagesPerDigitPL;
    
    // a local variable used for faster access to the ParameterLink value
    int evaluationsPerGeneration;
    bool infiniteWorld;
    
    std::string groupName = "root::";
    std::string brainName = "root::";
    
    //int worldSize = 28;
    int worldSize = 8;
    int retinaSize = 3;
    int stepSize = 1;

    int worldUpdates;
    //std::string numeralFileFolder = "../code/World/MnistWorld/Ternary MNIST";
    std::string numeralFileFolder = "../code/World/MnistWorld/Small Ternary Data";
    bool allowDoneBit = false;

    std::vector<int> digitClasses;
    std::vector<std::vector<std::vector<int>>> images;

    MnistWorld(shared_ptr<ParametersTable> PT);
    virtual ~MnistWorld() = default;

    virtual bool scanImage(int /*imageIndex*/, int /*startX*/, int /*startY*/, int /*scanDirection*/);
    virtual double evaluateOrganism(std::shared_ptr<Organism> /*org*/, int /*analyze*/, int /*visualize*/, int /*debug*/, bool /*isFirst*/, bool /*calcRepresentation*/);
    virtual auto evaluate(map<string, shared_ptr<Group>>& /*groups*/, int /*analyze*/, int /*visualize*/, int /*debug*/) -> void override;

    virtual auto requiredGroups() -> unordered_map<string,unordered_set<string>> override;
};

