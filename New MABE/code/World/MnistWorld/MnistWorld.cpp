//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "MnistWorld.h"

// this is how you setup a parameter in MABE, the function Parameters::register_parameter()takes the
// name of the parameter (catagory-name), default value (which must conform with the type), a the useage message
shared_ptr<ParameterLink<int>> MnistWorld::evaluationsPerGenerationPL =
    Parameters::register_parameter("WORLD_Mnist-evaluationsPerGeneration", 30,
    "how many times should each organism be tested in each generation?");

shared_ptr<ParameterLink<bool>> MnistWorld::infiniteWorldPL =
    Parameters::register_parameter("WORLD_Mnist-infiniteWorld", false,
    "Whether the organism can wander off the image.");

// the constructor gets called once when MABE starts up. use this to set things up
MnistWorld::MnistWorld(shared_ptr<ParametersTable> PT) : AbstractWorld(PT) {
    
    //localize a parameter value for faster access
    evaluationsPerGeneration = evaluationsPerGenerationPL->get(PT);
    infiniteWorld = infiniteWorldPL->get(PT);

    if (evaluationsPerGeneration % 10 != 0) {
        std::cout << "evaluationsPerGeneration must be divisible by 10." << std::endl;
        exit(1);
    }

    int instancesPerClass = evaluationsPerGeneration / 10;

    std::string rawLine;
    int digitClass;

    digitClasses.resize(evaluationsPerGeneration);
    images.resize(evaluationsPerGeneration);
    for (int i = 0; i < evaluationsPerGeneration; i++) {
        // Sets the height of the image to the world size.
        images[i].resize(worldSize);
        for (int j = 0; j < worldSize; j++) {
            // Sets the width of the image to the world size.
            images[i][j].resize(worldSize);
        }
    }

    int digitClassCounts[10];
    // Sets the count of each class to zero.
    for (int i = 0; i < 10; i++) {
        digitClassCounts[i] = 0;
    }

    int fileIndex = 0;
    int imageIndex = 0;
    bool hasFailed = false;
    while (imageIndex < evaluationsPerGeneration && !hasFailed) {
        // Produces the filename of the current file.
        std::string fileName;
        fileName.append(numeralFileFolder).append("/").append(std::string() + ((char)(fileIndex + '0'))).append(".txt");
        std::cout << imageIndex << " images read." << std::endl;
        std::cout << fileName << std::endl;

        // Opens the file.
        std::ifstream FILE(fileName);

        if (FILE.is_open()) {
            while (std::getline(FILE, rawLine) && imageIndex < evaluationsPerGeneration) {
                std::stringstream ss(rawLine);
                ss >> digitClass;  // Pull number from line (identifies what number is in image)
                //std::cout << digitClass << ";" << rawLine << std::endl;
                // Skip if there are already enough images of this class.
                if (digitClassCounts[digitClass] >= instancesPerClass) {
                    // Read to the end of the image.
                    for (int i = 0; i < worldSize; i++) {
                        std::getline(FILE, rawLine);
                    }
                    // Read the blank line after the image.
                    std::getline(FILE, rawLine);
                    // Skip over this iteration.
                    continue;
                }
                digitClassCounts[digitClass]++;
                digitClasses[imageIndex] = digitClass;
                for (int i = 0; i < worldSize; i++) {
                    std::getline(FILE, rawLine); // Get next line.
                    int j = 0;
                    for (char c : rawLine) {
                        if (isdigit(c)) {
                            images[imageIndex][i][j] = c - '0'; // Convert char to int.
                        }
                        else {
                            std::cout << "\n Non-numeric data inside of image data." << std::endl;
                            hasFailed = true;
                            break;
                        }
                        j++;
                    }
                    if (hasFailed) {
                        break;
                    }
                }
                if (hasFailed) {
                    break;
                }
                std::getline(FILE, rawLine); // Skip blank line.
                imageIndex++;
            }
        }
        else {
            std::cout << "\n ERROR: DigitWorld constructor, unable to open file: " << fileName << std::endl;
            hasFailed = true;
        }

        fileIndex++;
    }

    if (hasFailed) {
        exit(1);
    }

    std::cout << imageIndex << " images read." << std::endl;

    // Sets the count of each class to zero.
    for (int i = 0; i < 10; i++) {
        digitClassCounts[i] = 0;
    }
    // Counts the number of instances of each class.
    for (int i = 0; i < evaluationsPerGeneration; i++) {
        digitClassCounts[digitClasses[i]]++;
    }
    // Prints the count of each class.
    std::cout << "Digit Counts" << std::endl;
    std::cout << "============" << std::endl;
    for (int i = 0; i < 10; i++) {
        std::cout << i << ": " << digitClassCounts[i] << std::endl;
    }
    std::cout << "============" << std::endl;

    // The input consists of a square of direct input sensors plus 8 raycast sensors.
    inputNodesCount = (retinaSize * retinaSize) + 8;

    // The output consists of 14 nodes: 0-9(10), turnLeft(1), turnRight(1), moveForward(1), done(1)
    outputNodesCount = 14;

    // popFileColumns tell MABE what data should be saved to pop.csv files
	popFileColumns.clear();
    popFileColumns.push_back("score");
    for (int i = 0; i < 10; i++) {
		popFileColumns.push_back(to_string(i) + "-truePositiveRate");
		popFileColumns.push_back(to_string(i) + "-falsePositiveRate");
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

/// @brief 
/// @param imageIndex 
/// @param startX 
/// @param startY 
/// @param scanDirection 0 = right, 1 = down, 2 = left, 3 = up
/// @return 
bool MnistWorld::scanImage(int imageIndex, int startX, int startY, int scanDirection) {
    int raycastX = startX;
    int raycastY = startY;
    if (scanDirection < 0 || scanDirection > 3) {
        std::cout << "Invalid scan direction: " << scanDirection << std::endl;
        exit(1);
    }
    bool firstIteration = true;
    // The boundary condition to check depends on which direction to scan.
    // WARNING: This can loop infinitely if scan direction is not 0, 1, 2, or 3
    while ((scanDirection != 0 || raycastX < worldSize) // If scanning right, stop at right boundary.
        && (scanDirection != 1 || raycastY < worldSize) // If scanning down, stop at bottom boundary.
        && (scanDirection != 2 || raycastX >= 0) // If scanning left, stop at left boundary.
        && (scanDirection != 3 || raycastY >= 0)) { // If scanning up, stop at top boundary.
            bool rayJumpedIntoBounds = false;
            if (firstIteration) {
                // If scanning horizontally, breaks if organism is vertically out of bounds. (No need to scan.)
                if ((scanDirection == 0 || scanDirection == 2) && (raycastY < 0 || raycastY >= worldSize)) {
                    break;
                }
                // If scanning vertically, breaks if organism is horizontally out of bounds. (No need to scan.)
                if ((scanDirection == 1 || scanDirection == 3) && (raycastX < 0 || raycastX >= worldSize)) {
                    break;
                }
                // If scanning right and located left of bounds, skip into bounds.
                if (scanDirection == 0 && raycastX < 0) {
                    raycastX = 0;
                    rayJumpedIntoBounds = true;
                }
                // If scanning down and located above bounds, skip into bounds.
                else if (scanDirection == 1 && raycastY < 0) {
                    raycastY = 0;
                    rayJumpedIntoBounds = true;
                }
                // If scanning left and located right of bounds, skip into bounds.
                else if (scanDirection == 2 && raycastX >= worldSize) {
                    raycastX = worldSize - 1;
                    rayJumpedIntoBounds = true;
                }
                // If scanning up and located below bounds, skip into bounds.
                else if (scanDirection == 3 && raycastY >= worldSize) {
                    raycastY = worldSize - 1;
                    rayJumpedIntoBounds = true;
                }
            }
            // Doesn't read the pixel on the first iteration (unless ray jumped into bounds.)
            // This prevents detecting light at the point of origin.
            if (!firstIteration || rayJumpedIntoBounds) {
                // Only tries to read the data if it is within bounds.
                if (0 <= raycastX && raycastX < worldSize && 0 <= raycastY && raycastY < worldSize) {
                    // If gray or white is encountered, stop scanning.
                    if (images[imageIndex][raycastY][raycastX] > 0) {
                        return true;
                    }
                }
            }
            firstIteration = false;
            // If scanning right, move raycast position right.
            if (scanDirection == 0) {
                raycastX++;
            }
            // If scanning down, move raycast position down.
            else if (scanDirection == 1) {
                raycastY++;
            }
            // If scanning left, move raycast position left.
            else if (scanDirection == 2) {
                raycastX--;
            }
            // If scanning up, move raycast position up.
            else {
                raycastY--;
            }
    }
    // Gray or white not encountered.
    return false;
}

double MnistWorld::evaluateOrganism(std::shared_ptr<Organism> org, int analyze, int visualize, int debug, bool isFirst, bool calcRepresentation) {
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

    std::vector<std::vector<bool>> exploredCells;
    exploredCells.resize(worldSize);
    
    for (int i = 0; i < worldSize; i++) {
        exploredCells[i].resize(worldSize);
    }

    // evaluate this organism some number of times based on evaluationsPerGeneration
    for (int imageIndex = 0; imageIndex < evaluationsPerGeneration; imageIndex++) {      
        // clear the brain - resets brain state including memory
        brain->resetBrain();        

        numeralPick = digitClasses[imageIndex];
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
            int nodeAssignmentCounter = 0;
            
            // Loads in direct inputs.
            for (int yReadIndex = 0; yReadIndex < retinaSize; yReadIndex++) {
                for (int xReadIndex = 0; xReadIndex < retinaSize; xReadIndex++) {
                    int sensorX = currentX + xReadIndex;
                    int sensorY = currentY + yReadIndex;

                    // Verifies that the sensor is within bounds.
                    if (0 <= sensorX && sensorX < worldSize && 0 <= sensorY && sensorY < worldSize) {
                        // Reads the pixel (the values in the file are offset by one, e.g., 2 represents 1).
                        brain->setInput(nodeAssignmentCounter++, images[imageIndex][sensorY][sensorX] - 1);
                    }
                    // Otherwise, reads black.
                    else {
                        brain->setInput(nodeAssignmentCounter++, -1);
                    }
                }
            }

            // Initializes the positions to cast rays from.
            int topRightX = currentX + retinaSize - 1;
            int topRightY = currentY;
            int bottomRightX = currentX + retinaSize - 1;
            int bottomRightY = currentY + retinaSize - 1;
            int bottomLeftX = currentX;
            int bottomLeftY = currentY + retinaSize - 1;
            int topLeftX = currentX;
            int topLeftY = currentY;

            // Sets the raycast inputs.
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, topRightX, topRightY, 0) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, bottomRightX, bottomRightY, 0) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, bottomRightX, bottomRightY, 1) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, bottomLeftX, bottomLeftY, 1) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, bottomLeftX, bottomLeftY, 2) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, topLeftX, topLeftY, 2) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, topLeftX, topLeftY, 3) ? 1 : 0);
            brain->setInput(nodeAssignmentCounter++, scanImage(imageIndex, topRightX, topRightY, 3) ? 1 : 0);

            // Updates the outputs using the inputs.
            brain->update();

            worldStateSet.push_back({ numeralPick });

            // Gets the direction to turn.
            int turnLeft = brain->readOutput(10) > 0.5;
            int turnRight = brain->readOutput(11) > 0.5;
            
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

            // Moves the organism.
            int moveForward = brain->readOutput(12) > 0.5;
            if (moveForward) {
                currentX += movementX * stepSize;
                currentY += movementY * stepSize;

                // Calculates the index of the center of the retina.
                int centerX = currentX + (retinaSize / 2);
                int centerY = currentY + (retinaSize / 2);

                // If the center is within bounds.
                if (0 <= centerX && centerX < worldSize && 0 <= centerY && centerY < worldSize) {
                    // If this cell is unexplored ////and is gray or white.
                    if (exploredCells[centerY][centerX] == false /*&& images[imageIndex][centerY][centerX] > 0*/) {
                        // Award an exploration bonus.
                        explorationBonus += 2 / pow(2, countCellsExplored);
                        
                        // Increment the number of cells explored.
                        countCellsExplored++;

                        // Mark the cell as explored.
                        exploredCells[centerY][centerX] = true;
                    }
                }
                // If the center is out of bounds and infinite world is turned off.
                else if (!infiniteWorld) {
                    // Move back.
                    currentX -= movementX * stepSize;
                    currentY -= movementY * stepSize;
                }
            }

            if (allowDoneBit && brain->readOutput(13) > 0.5) {
                break; // if organism decides that it is done, then we stop.
            }

            // If debugging and this is the first organism on the last image,
            // outputs (part of) the world.
            if (debug && isFirst && worldUpdate % 10 == 0 && imageIndex == evaluationsPerGeneration - 1) {
                std::vector<std::vector<int>> worldSummary;
                worldSummary.resize(worldSize);
                for (int i = 0; i < worldSize; i++) {
                    worldSummary[i].resize(worldSize);
                }
                std::cout << "Number in image: " << numeralPick << std::endl;

                // Writes the image to the 2D vector.
                for (int y = 0; y < worldSize; y++) {
                    for (int x = 0; x < worldSize; x++) {
                        worldSummary[y][x] = images[imageIndex][y][x];
                    }
                }

                // Overwrites the pixels where the organism is at.
                for (int yReadIndex = 0; yReadIndex < retinaSize; yReadIndex++) {
                    for (int xReadIndex = 0; xReadIndex < retinaSize; xReadIndex++) {
                        // Calculates the position of the sensor.
                        int sensorX = currentX + xReadIndex;
                        int sensorY = currentY + yReadIndex;

                        // Verifies that the sensor is within bounds.
                        if (0 <= sensorX && sensorX < worldSize && 0 <= sensorY && sensorY < worldSize) {
                            worldSummary[sensorY][sensorX] = 9;
                        }
                    }
                }

                // Prints out the summary of the world.
                for (int y = 0; y < worldSize; y++) {
                    for (int x = 0; x < worldSize; x++) {
                        if (worldSummary[y][x] == 9) {
                            std::cout << "_";
                        }
                        else {
                            std::cout << worldSummary[y][x];
                        }
                    }
                    std::cout << std::endl;
                }

                // Prints the organism's outputs.
                for (int outIndex = 0; outIndex < outputNodesCount; outIndex++) {
                    std::cout << brain->readOutput(outIndex) << " ";
                }
                std::cout << std::endl;
            }
        }

        bool guessedCorrectly = false; // Defaults to false.
        int guessesMade = 0;

        // Determines if the organism got it right.
        for (int i = 0; i < 10; i++) {
            bool guessedDigit = brain->readOutput(i) > 0.5;
            if (numeralPick == i && guessedDigit) {
                correct[i]++;
                guessedCorrectly = true;
                guessesMade++;
            }
            else if (numeralPick != i && guessedDigit) {
                incorrect[i]++;
                guessesMade++;
            }
        }

        // If it gets it right, the base score is 10.
        // But the actual score is divided by the number of guesses it made.
        if (guessesMade > 0 && guessedCorrectly) {
            score += 10 / guessesMade;
        }
    }

    // Adds the overall exploration bonus to the score.
    score += explorationBonus;

    org->dataMap.append("score", score);

    int totalCorrect = 0;
    int totalIncorrect = 0;
    std::string tempName;

    // Records the accuracy of organism for each digit.
    for (int i = 0; i < 10; i++) {
        totalCorrect += correct[i];
        totalIncorrect += incorrect[i];
        
        tempName = to_string(i) + "-truePositiveRate";
        
        double truePositiveRate = 0;
        if (counts[i] > 0) {
            // Calculates how often the organism correctly guessed the digit when it was present.
            truePositiveRate = ((double)correct[i]) / ((double)counts[i]);
        }
        org->dataMap.append(tempName, truePositiveRate);
        org->dataMap.setOutputBehavior(tempName, DataMap::AVE);

        tempName = to_string(i) + "-falsePositiveRate";
        
        // Calculates how many times the digit DIDN'T show up.
        int absentCount = evaluationsPerGeneration - counts[i];

        double falsePositiveRate = 0;        
        if (absentCount > 0) {
            // Calculates how often the organism incorrectly guessed the digit when it was absent.
            falsePositiveRate = ((double)incorrect[i]) / ((double)absentCount);
        }
        org->dataMap.append(tempName, falsePositiveRate);
        org->dataMap.setOutputBehavior(tempName, DataMap::AVE);
    }

    // Records the total number of correct and incorrect guesses.
    org->dataMap.append("totalCorrect", totalCorrect);  
    org->dataMap.append("totalIncorrect", totalIncorrect);

    if (calcRepresentation) {
        auto remapRule = TS::RemapRules::TRIT;
        auto lifeTimes = brain->getLifeTimes();
        auto inputStateSet = TS::remapToIntTimeSeries(brain->getInputStates(), remapRule);
        auto hiddenStates = brain->getHiddenStates();
        auto brainHiddenStateTimeSeries = TS::remapToIntTimeSeries(brain->getHiddenStates(), remapRule);
        auto brainAfterStateSet = TS::trimTimeSeries(brainHiddenStateTimeSeries, TS::Position::FIRST, lifeTimes);

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
    }
    else {
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
auto MnistWorld::evaluate(map<string, shared_ptr<Group>>& groups, int analyze, int visualize, int debug) -> void {

    int popSize = groups[groupName]->population.size(); 
    double bestScore = -1;
    auto bestOrganismIndex = 0;

    // Overrides the debug variable so that debugging is only performed on certain updates.
    int debugOverride = debug && (Global::update > 0) && (Global::update % 50 == 0);

    // In this world, organisms do not interact, so we can just iterate over the population.
    for (int i = 0; i < popSize; i++) {
        // Create a shortcut to access the organism and the organism's brain.
        auto org = groups[groupName]->population[i];
        // Evaluate the organism.
        double score = evaluateOrganism(org, analyze, visualize, debugOverride, i==0, false);

        if (score > bestScore) {
            bestScore = score;
            bestOrganismIndex = i;
        }
    } // end of population loop

    // Every ten generations, re-run and calculate R on the best organism. (Does not debug.)
    if (Global::update % 10 == 0) {
        auto bestOrg = groups[groupName]->population[bestOrganismIndex];
        evaluateOrganism(bestOrg, analyze, visualize, false, false, true);
    }
}

// the requiredGroups function lets MABE know how to set up populations of organisms that this world needs
auto MnistWorld::requiredGroups() -> unordered_map<string,unordered_set<string>> {
	return { { groupName, { "B:"+brainName+"," + to_string(inputNodesCount) + "," + to_string(outputNodesCount) } } };
        
        // this tells MABE to make a group called "root::" with a brain called "root::" that takes 2 inputs and has 1 output
        // "root::" here also indicates the namespace for the parameters used to define these elements.
        // "root::" is the default namespace, so parameters defined without a namespace are "root::" parameters
}
