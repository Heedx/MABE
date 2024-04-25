//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "DigitWorld.h"

// this is how you setup a parameter in MABE, the function Parameters::register_parameter()takes the
// name of the parameter (catagory-name), default value (which must conform with the type), a the useage message
shared_ptr<ParameterLink<int>> DigitWorld::evaluationsPerGenerationPL =
    Parameters::register_parameter("WORLD_Digit-evaluationsPerGeneration", 10,
    "how many numbers should organism look at each gen?");
shared_ptr<ParameterLink<int>> DigitWorld::defaultWorldUpdatesPL = 
    Parameters::register_parameter("WORLD_Digit-WorldUpdates", 50, 
                                "number of world updates brain has to evaluate each value");
shared_ptr<ParameterLink<int>> DigitWorld::defaultRetinaTypePL = 
Parameters::register_parameter("WORLD_Digit-retinaType", 2, 
                            "1 = center only, 2 = 2x2, 3 = 3x3, 4 = 4x4");
shared_ptr<ParameterLink<string>> DigitWorld::numeralDataFileNamePL = 
Parameters::register_parameter("WORLD_Digit-dataFileName", 
(string) "./ternaryMiniDigits.txt", "name of file with numeral data");

shared_ptr<ParameterLink<bool>> DigitWorld::allowDoneBitPL = 
Parameters::register_parameter("WORLD_Digit-allowDoneBit", true, "allow the brain to decide when its done");

// the constructor gets called once when MABE starts up. use this to set things up
DigitWorld::DigitWorld(shared_ptr<ParametersTable> PT) : AbstractWorld(PT) {
    //localize a parameter value for faster access
    evaluationsPerGeneration = evaluationsPerGenerationPL->get(PT);
    worldUpdates = defaultWorldUpdatesPL->get(PT); 
    retinaType = defaultRetinaTypePL->get(PT); 
    numeralDataFileName = numeralDataFileNamePL->get(PT);
    switch (retinaType) {
	case 1:// center only
		retinaSensors = 1;
		stepSize = 1;
		break;
	case 2:// 2x2 sensor, position starts in top left.
		retinaSensors = 4;
		stepSize = 1;
		break;
    case 3:// 3x3 sensor
        retinaSensors = 9;
        stepSize = 1;
        break;
    case 4:// 4x4 sensor
        retinaSensors = 16;
        stepSize = 1;
        break;
	default:
		std::cout << "\nIn DigitWorld constructor, undefined retinaType" << retinaType << "\nExiting!\n" << std::endl;
		exit(1);
	}
    allowDoneBit = allowDoneBitPL->get(PT);
    
    // Load in 8x8 numbers
    worldSize = 8;
    numeralData.resize(10);
  
    exploredCells.resize(worldSize);
    for (int i = 0; i < worldSize; i++) {
        exploredCells[i].resize(worldSize);
    }

	std::string fileName = numeralDataFileName;
	std::ifstream FILE(fileName);
	std::string rawLine;
    int readInt;
	bool readBit;

    std::cout << "reading file: "<< fileName << std::endl;
    if (FILE.is_open()) {
        while (std::getline(FILE, rawLine)) {
            std::stringstream ss(rawLine);
            ss >> readInt;  // pull number from line (identifies what number is in image)
            for (int i = 0; i < worldSize; i++){
                std::getline(FILE, rawLine);  // get next line
                for (char c : rawLine) {
                    if (isdigit(c)) {
                        numeralData[readInt].push_back(c - '0'); // Convert char to int
                    }
                }
            }
            std::getline(FILE, rawLine); //skip blank line
        }
    } else {
        std::cout << "\n ERROR: DigitWorld constructor, unable to open file: " << fileName << std::endl;
        exit(1);
    }
    // End load

    inputNodesCount = retinaSensors;
    // Organism requires 14 nodes for output; turnLeft(1), turnRight(1), moveForward(1), 0-9(10), done(1)
    outputNodesCount = 14;

    // popFileColumns tell MABE what data should be saved to pop.csv files
	popFileColumns.clear();
    popFileColumns.push_back("score");
    for (int i = 0; i < 10; i++) {
		popFileColumns.push_back(to_string(i) + "-correct");
		popFileColumns.push_back(to_string(i) + "-incorrect");
	}
	popFileColumns.push_back("totalCorrect");
	popFileColumns.push_back("totalIncorrect");
    popFileColumns.push_back("R");
    popFileColumns.push_back("earlyR25");
    popFileColumns.push_back("lateR25");
    popFileColumns.push_back("rawR");
    popFileColumns.push_back("earlyRawR25");
    popFileColumns.push_back("lateRawR25");
}

double DigitWorld::evaluateOrganism(std::shared_ptr<Organism> org, int analyze, int visualize, int debug, bool isFirst, bool calcRepresentation) {
    auto brain = org->brains[brainName]; 
    // Only record brain activity if needed to calculate R.
    brain->setRecordActivity(calcRepresentation);

    TS::intTimeSeries worldStateSet;

    double score = 0;
    int currentX, currentY; //for a 2x2 eye, top-left of sensor window is current position
    int movementX, movementY;

    int countCellsExplored = 0;
    double explorationBonus = 0;

    int numeralPick; // number being tested
    int whichNumeral; // which particular number from set is being tested

    std::vector<int> correct;  // for each number (0 to 9) number of times organism is correct (at most # of evaluations)
    std::vector<int> incorrect;  // for each number (0 to 9) number of times organism is incorrect (at most 9 * # evaluations)
    std::vector<int> counts;  // how many times each number shows up.

    correct.resize(10);
    incorrect.resize(10);
    counts.resize(10);
    
    // clear the brain - resets brain state including memory
    brain->resetBrain();
    // evaluate this organism some number of times based on evaluationsPerGeneration
    for (int t = 0; t < evaluationsPerGeneration; t++) {        
        numeralPick = t % 10;//Random::getInt(9); // Select a number between 0-9
        whichNumeral = t/10;//Random::getIndex(numeralData[numeralPick].size() / (8 * 8));
        brain->resetBrain();
        counts[numeralPick]++;

        //place the organism in the top left of the world
        currentX = 0;
        currentY = 0;

        //orient the organism to face to the right.
        movementX = 1;
        movementY = 0;

        countCellsExplored = 0;

        // Marks every cell as unexplored.
        for (int i = 0; i < worldSize; i++) {
            for (int j = 0; j < worldSize; j++) {
                exploredCells[i][j] = false;
            }
        }

        for (int worldUpdate = 0; worldUpdate < worldUpdates; worldUpdate++) {
            // load in inputs.
            int nodeAssignmentCounter = 0;
            for(int s = 0; s < retinaSensors; s++){
                int checkX = currentX + retinalOffsets[s].first;
                int checkY = currentY + retinalOffsets[s].second;
                if (checkX >= 0 && checkX < worldSize && checkY >= 0 && checkY < worldSize){
                    int checkInt = numeralData[numeralPick][(whichNumeral * worldSize * worldSize) + (checkY * worldSize) + (checkX)];
                    if(checkInt == 0) checkInt = -1;
                    else if(checkInt == 1) checkInt = 0;
                    else if(checkInt == 2) checkInt = 1;
                    brain->setInput(nodeAssignmentCounter++, checkInt);
                } else {
                    brain->setInput(nodeAssignmentCounter++, -1);
                }
            }

            brain->update(); // Update the outputs using the inputs.

            worldStateSet.push_back({ numeralPick });

            // Get the direction to turn.
            int turnLeft = brain->readOutput(0) > 0.5;
            int turnRight = brain->readOutput(1) > 0.5;
            
            // If turning right...
            if (turnLeft && !turnRight) {
                // ...and if facing down...
                if (movementY == 1) {
                    // ...turn to face left.
                    movementX = -1;
                    movementY = 0;
                }
                // ...else if facing left...
                else if (movementX == -1) {
                    // ...turn to face up.
                    movementX = 0;
                    movementY = -1;
                }
                // ...else if facing up.
                else if (movementY == -1) {
                    // ...turn to face right.
                    movementX = 1;
                    movementY = 0;
                }
                // ...else facing right.
                else {
                    // ...turn to face down.
                    movementX = 0;
                    movementY = 1;
                }
            }
            // If turning left...
            if (turnRight && !turnLeft) {
                // ...and if facing down.
                if (movementY == 1) {
                    // ...turn to face right.
                    movementX = 1;
                    movementY = 0;
                }
                // ...else if facing right.
                else if (movementX == 1) {
                    // ...turn to face up.
                    movementX = 0;
                    movementY = -1;
                }
                // ...else if facing up.
                else if (movementY == -1) {
                    // ...turn to face left.
                    movementX = -1;
                    movementY = 0;
                }
                // ...else facing left.
                else {
                    // ...turn to face down.
                    movementX = 0;
                    movementY = 1;
                }
            }

            // move the organism
            int moveForward = brain->readOutput(2) > 0.5;
            if (moveForward) {
                currentX += movementX * stepSize;
                currentY += movementY * stepSize;

                // prevent organism from walking out of the boundaries of the world.
                if(currentX < 0) currentX = 0;
                if(currentX >= worldSize) currentX = worldSize-1;
                if(currentY < 0) currentY = 0;
                if(currentY >= worldSize) currentY = worldSize-1;

                if (exploredCells[currentY][currentX] == false) {
                    explorationBonus += 2 / pow(2, countCellsExplored);
                    countCellsExplored++;

                    exploredCells[currentY][currentX] = true;
                }
            }

            if (allowDoneBit && brain->readOutput(13) > 0.5) {
                //break; // if organism decides that it is done, then we stop.
            }

            // Output the world to debug.
            // Only output first organism, and their last evaluation
            if (debug && isFirst && t == evaluationsPerGeneration-1) {
                //display world state
                std::vector<std::vector<int>>image;
                image.resize(worldSize);
                for (int x = 0; x < worldSize; x++) {
                    image[x].resize(worldSize);
                }
                std::cout << "Number in image: " << numeralPick << std::endl;
                int index = whichNumeral * worldSize * worldSize;
                for(int r = 0; r < worldSize; r++){
                    for(int c = 0; c < worldSize; c++) {
                        image[r][c] = numeralData[numeralPick][index + (r * worldSize) + c];
                    }
                    // std::cout << std::endl;
                }

                for(int sensor = 0; sensor < retinaSensors; sensor++){
                    int checkX = currentX + retinalOffsets[sensor].first;
                    int checkY = currentY + retinalOffsets[sensor].second;
                    if (checkX >= 0 && checkX < worldSize && checkY >= 0 && checkY < worldSize) {  // if we are on the image
                        image[checkY][checkX] = 9;
                    } 
                }

                for (int r = 0; r < worldSize; r++) {  
                    for (int c = 0; c < worldSize; c++) {
                        std::cout << image[r][c];
                    }
                    std::cout << std::endl;
                }

                //output agent output states
                for(int out = 0; out < outputNodesCount; out++){
                    std::cout << brain->readOutput(out) << " ";
                }
                std::cout << std::endl;
            }

            // if (debug) {
            //     std::cout << "\n----------------------------\n";
            //     std::cout << "\ngeneration update: " << Global::update << "  world update: " << worldUpdate << "\n";
            //     std::cout << "currentLocation: " << currentX << "," << currentY << "\n";
            //     std::cout << "outNodes: ";
            //     for(int i = 0; i < outputNodesCount; i++){
            //         std::cout << brain->readOutput(i) << " ";
            //     }
            //     std::cout << "\n -- brain update -- \n";
            // }
            
        } // end of world loop

        bool guessedCorrectly = false; // Defaults to false.
        int guessesMade = 0;

        // Determine if organism got it right.
        for (int i =0; i < 10; i++){
            if (numeralPick == i && brain->readOutput(3 + i) > 0.5) {
                correct[i]++;
                guessedCorrectly = true;
                guessesMade++;
            }
            if (numeralPick != i && brain->readOutput(3 + i) > 0.5) {
                incorrect[i]++;
                guessesMade++;
            }
        }

        // If it gets it right, the base score is 10.
        // But the actual score is divided by the number of guesses it made.
        if (guessesMade > 0 && guessedCorrectly) {
            score += 10 / guessesMade;
        }    

    } //end of evaluation loop

    // // Score the organism
    // for (int i = 0; i < 10; i++) {
    //     score += 10 * correct[i];
    //     score -= 10 * incorrect[i] * (1 / 9);
    // }

    score += explorationBonus;

    // add score to organisms data
    // it can be expensive to access dataMap too often. also, here we want score to be the sum of the correct answers
    // Save to pop file
    int total_correct = 0;
    int total_incorrect = 0;
    string temp_name;
    double val;

    if (calcRepresentation) {
        auto remapRule = TS::RemapRules::TRIT;
        auto lifeTimes = brain->getLifeTimes();
        auto inputStateSet = TS::remapToIntTimeSeries(brain->getInputStates(), remapRule);
        auto hiddenStates = brain->getHiddenStates();
        auto brainHiddenStateTimeSeries = TS::remapToIntTimeSeries(brain->getHiddenStates(), remapRule);
        auto brainAfterStateSet = TS::trimTimeSeries(brainHiddenStateTimeSeries, TS::Position::FIRST, lifeTimes);

        // int bufferSize = 0;
        // for (int i = 0; i < worldStateSet.size(); i++) {
        //     auto state = worldStateSet[i];
        //     for (int j = 0; j < state.size(); j++) {
        //         bufferSize++;
        //         auto statePart = state[j];
        //         // std::cout << statePart << " ";
        //     }
        // }
        // for (int i = 0; i < brainAfterStateSet.size(); i++) {
        //     auto state = brainAfterStateSet[i];
        //     for (int j = 0; j < state.size(); j++) {
        //         bufferSize++;
        //         auto statePart = state[j];
        //         std::cout << statePart << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // for (int i = 0; i < inputStateSet.size(); i++) {
        //     auto state = inputStateSet[i];
        //     for (int j = 0; j < state.size(); j++) {
        //         bufferSize++;
        //         auto statePart = state[j];
        //         std::cout << statePart << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << bufferSize << std::endl;

        double R = ENT::ConditionalMutualEntropy(worldStateSet, brainAfterStateSet, inputStateSet);
        double rawR = ENT::MutualEntropy(worldStateSet, brainAfterStateSet);

        auto earlyWorldStateSet25 = TS::trimTimeSeries(worldStateSet, {0, .25}, lifeTimes);
        auto earlyBrainAfterStateSet25 = TS::trimTimeSeries(brainAfterStateSet, {0, 0.25}, lifeTimes);
        auto earlyInputStateSet25 = TS::trimTimeSeries(inputStateSet, {0, 0.25}, lifeTimes);

        double earlyR25 = ENT::ConditionalMutualEntropy(earlyWorldStateSet25, earlyBrainAfterStateSet25, earlyInputStateSet25);
        double earlyRawR25 = ENT::MutualEntropy(earlyWorldStateSet25, earlyBrainAfterStateSet25);

        auto lateWorldStateSet25 = TS::trimTimeSeries(worldStateSet, {0.75, 1}, lifeTimes);
        auto lateBrainAfterStateSet25 = TS::trimTimeSeries(brainAfterStateSet, {0.75, 1}, lifeTimes);
        auto lateInputStateSet25 = TS::trimTimeSeries(inputStateSet, {0.75, 1}, lifeTimes);

        double lateR25 = ENT::ConditionalMutualEntropy(lateWorldStateSet25, lateBrainAfterStateSet25, lateInputStateSet25);
        double lateRawR25 = ENT::MutualEntropy(lateWorldStateSet25, lateBrainAfterStateSet25);

        org->dataMap.set("R", R);
        org->dataMap.set("earlyR25", earlyR25);
        org->dataMap.set("lateR25", lateR25);
        org->dataMap.set("rawR", rawR);
        org->dataMap.set("earlyRawR25", earlyRawR25);
        org->dataMap.set("lateRawR25", lateRawR25);
    } else {
        for (int i = 0; i < 10; i++) {
            total_correct += correct[i];
            total_incorrect += incorrect[i];

            temp_name = to_string(i) + "-correct";
            (counts[i] > 0) ? val = (double) correct[i] / (double) counts[i] : val = 0;
            org->dataMap.append(temp_name, val);
            org->dataMap.setOutputBehavior(temp_name, DataMap::AVE);

            temp_name = to_string(i) + "-incorrect";  
            (counts[i] < evaluationsPerGeneration) ? val = (double) incorrect[i] / ((double) evaluationsPerGeneration - counts[i]) : val = 0;
            org->dataMap.append(temp_name, val);
            org->dataMap.setOutputBehavior(temp_name, DataMap::AVE);
        }


        org->dataMap.append("totalCorrect", total_correct);  
        org->dataMap.append("totalIncorrect", total_incorrect);
        org->dataMap.append("score", score);
        org->dataMap.append("R", 0.0);
        org->dataMap.append("earlyR25", 0.0);
        org->dataMap.append("lateR25", 0.0);
        org->dataMap.append("rawR", 0.0);
        org->dataMap.append("earlyRawR25", 0.0);
        org->dataMap.append("lateRawR25", 0.0);
    }

    return score;
}

// the evaluate function gets called every generation. evaluate should set values on organisms datamaps
// that will be used by other parts of MABE for things like reproduction and archiving
auto DigitWorld::evaluate(map<string, shared_ptr<Group>>& groups, int analyze, int visualize, int debug) -> void {

    int popSize = groups[groupName]->population.size(); 
    double bestScore = -1;
    auto bestOrganismIndex = 0;

     // in this world, organisms do not interact, so we can just iterate over the population
    for (int i = 0; i < popSize; i++) {
        // create a shortcut to access the organism and organisms brain
        auto org = groups[groupName]->population[i];
        // Evaluate the organism.
        double score = evaluateOrganism(org, analyze, visualize, debug, i==0, false);
        
        if (score > bestScore) {
            bestScore = score;
            bestOrganismIndex = i;
        }
    } // end of population loop

    // Every ten generations, calculate R on the best organism.
    if (Global::update % 10 == 0) {
        auto bestOrg = groups[groupName]->population[bestOrganismIndex];
        evaluateOrganism(bestOrg, analyze, visualize, debug, false, true);
    }
}

// the requiredGroups function lets MABE know how to set up populations of organisms that this world needs
auto DigitWorld::requiredGroups() -> unordered_map<string,unordered_set<string>> {
	return { { groupName, { "B:"+brainName+","+to_string(inputNodesCount) + "," + to_string(outputNodesCount) } } };
        
        // this tells MABE to make a group called "root::" with a brain called "root::" that takes 2 inputs and has 1 output
        // "root::" here also indicates the namespace for the parameters used to define these elements.
        // "root::" is the default namespace, so parameters defined without a namespace are "root::" parameters
}
