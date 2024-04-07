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
                            "1 = center only, 2 = 2x2");
shared_ptr<ParameterLink<string>> DigitWorld::numeralDataFileNamePL = 
Parameters::register_parameter("WORLD_Digit-dataFileName", 
(string) "./ternaryMiniDigits.txt", "name of file with numeral data");

// the constructor gets called once when MABE starts up. use this to set things up
DigitWorld::DigitWorld(shared_ptr<ParametersTable> PT) : AbstractWorld(PT) {
    //localize a parameter value for faster access
    evaluationsPerGeneration = evaluationsPerGenerationPL->get(PT);
    worldUpdates = defaultWorldUpdatesPL->get(PT); 
    retinaType = defaultRetinaTypePL->get(PT); // "1 = center only, 2 = 2x2"
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
	default:
		std::cout << "\nIn DigitWorld constructor, undefined retinaType" << retinaType << "\nExiting!\n" << std::endl;
		exit(1);
	}
    
    // Load in 8x8 numbers
    worldSize = 8;
    numeralData.resize(10);
    

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
    // Organism requires 13 nodes for output; moveX(1), moveY(1), 0-9(10), done(1)
    // 0-9 outputs, though will only have 2 classes to recognize (4, 7)
    outputNodesCount = 13;

    // popFileColumns tell MABE what data should be saved to pop.csv files
	popFileColumns.clear();
    popFileColumns.push_back("score");
    for (int i = 0; i < 10; i++) {
		popFileColumns.push_back(to_string(i) + "-correct");
		popFileColumns.push_back(to_string(i) + "-incorrect");
	}
	popFileColumns.push_back("totalCorrect");
	popFileColumns.push_back("totalIncorrect");
}

// the evaluate function gets called every generation. evaluate should set values on organisms datamaps
// that will be used by other parts of MABE for things like reproduction and archiving
auto DigitWorld::evaluate(map<string, shared_ptr<Group>>& groups, int analyze, int visualize, int debug) -> void {

    int popSize = groups[groupName]->population.size(); 
    
     // in this world, organisms do not interact, so we can just iterate over the population
    for (int i = 0; i < popSize; i++) {
        // create a shortcut to access the organism and organisms brain
        auto org = groups[groupName]->population[i];
        auto brain = org->brains[brainName]; 

        double score = 0;
        int currentX, currentY; //for a 2x2 eye, top-left of sensor window is current position

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
            bool goodNumber = false;
            while (!goodNumber) {
                goodNumber = true;
                numeralPick = Random::getIndex(10);  // pick a number
                for (int check = 0; check < 10; check++) {
                    if (counts[check] < counts[numeralPick]) {
                        goodNumber = false;
                    }
                }
            }
            // numeralPick = Random::getInt(9); // Select a number between 0-9
            whichNumeral = Random::getIndex(numeralData[numeralPick].size() / (8 * 8));

            counts[numeralPick]++;

            //place the organism in the top left of the world
            currentX = 0;
            currentY = 0;

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

                // move the organism
                currentX += Trit(brain->readOutput(0)) * stepSize; // left and right
                currentY += Trit(brain->readOutput(1)) * stepSize; // up and down

                // prevent organism from walking out of the boundaries of the world.
                if(currentX < 0) currentX = 0;
                if(currentX >= worldSize) currentX = worldSize-1;
                if(currentY < 0) currentY = 0;
                if(currentY >= worldSize) currentY = worldSize-1;

                if (brain->readOutput(12)) {
                    break; // if organism decides that it is done, then we stop.
                }

                // Output the world to debug.
                // Only output first organism, and their last evaluation
                if (debug && i == 0 && t == evaluationsPerGeneration-1) {
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

            // Determine if organism got it right.
            for (int i =0; i < 10; i++){
                if (numeralPick == i && brain->readOutput(2 + i) > 0) {
                    correct[i]++;
                }
                if (numeralPick != i && brain->readOutput(2 + i) > 0) {
                    incorrect[i]++;
                }
            }

            // Score the organism
            for (int i = 0; i < 10; i++) {
                double c = (counts[i] == 0) ? 1.0 : (double)counts[i];
                
                score += pow(   (((double) correct[i])/c) - (((double)incorrect[i]) / ((double)evaluationsPerGeneration - c))  ,   2   );
                //score -= ((double) incorrect[i]) / 10.0;
            }

            if (score < 0.0) {
                score = 0.0;
            }

            
        } //end of evaluation loop

        // add score to organisms data
        // it can be expensive to access dataMap too often. also, here we want score to be the sum of the correct answers
        // Save to pop file
        int total_correct = 0;
        int total_incorrect = 0;
        string temp_name;
        double val;

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

        
    } // end of population loop
}

// the requiredGroups function lets MABE know how to set up populations of organisms that this world needs
auto DigitWorld::requiredGroups() -> unordered_map<string,unordered_set<string>> {
	return { { groupName, { "B:"+brainName+","+to_string(inputNodesCount) + "," + to_string(outputNodesCount) } } };
        
        // this tells MABE to make a group called "root::" with a brain called "root::" that takes 2 inputs and has 1 output
        // "root::" here also indicates the namespace for the parameters used to define these elements.
        // "root::" is the default namespace, so parameters defined without a namespace are "root::" parameters
}
