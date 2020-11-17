#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

#include "SoCKalman.h"


void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<int> > > dataset){
    // Make a CSV file with one or more columns of integer values
    // Each column of data is represented by the pair <column name, column data>
    //   as std::pair<std::string, std::vector<int>>
    // The dataset is represented as a vector of these columns
    // Note that all columns should be the same size
    
    // Create an output filestream object
    std::ofstream myFile(filename);
    
    // Send column names to the stream
    for(int j = 0; j < dataset.size(); ++j)
    {
        myFile << dataset.at(j).first;
        if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
    }
    myFile << "\n";
    
    // Send data to the stream
    for(int i = 0; i < dataset.at(0).second.size(); ++i)
    {
        for(int j = 0; j < dataset.size(); ++j)
        {
            myFile << dataset.at(j).second.at(i);
            if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
        }
        myFile << "\n";
    }
    
    // Close the file
    myFile.close();
}

std::vector<std::pair<std::string, std::vector<int> > > process_csv(std::string filename){
    // Reads a CSV file into a vector of <string, vector<int>> pairs where
    // each pair represents <column name, column values>

    // Instantiate kalman filter and initial values
    SoCKalman kalman;

    uint32_t batteryEff = 85000;
    uint32_t initialSoC = 0xFFFFFFFF;
    uint32_t batteryCapacity = 1200;

    // Create a vector of <string, int vector> pairs to store the result
    std::vector<std::pair<std::string, std::vector<int> > > result;

    // Create an input filestream
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    // Helper vars
    std::string line, colname;
    int val;

    // Read the column names
    if(myFile.good())
    {
        // Extract the first line in the file
        std::getline(myFile, line);

        // Create a stringstream from line
        std::stringstream ss(line);

        // Extract each column name
        while(std::getline(ss, colname, ',')){
            
            // Initialize and add <colname, int vector> pairs to result
            std::pair<std::string, std::vector<int> > column;
            column.first = colname;
            result.push_back(column);
        }
    }

    int lineIdx = 0;
    // Read data, line by line
    while(std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);
        
        // Keep track of the current column index
        int colIdx = 0;
        
        // Extract each integer
        while(ss >> val){
            // Add the current integer to the 'colIdx' column's values vector
            result.at(colIdx).second.push_back(val);
            
            // If the next token is a comma, ignore it and move on
            if(ss.peek() == ',') ss.ignore();
            
            // Increment the column index
            colIdx++;
        }
        // TODO: parse line values and use for kalman sample
        if (lineIdx == 0) {
            // use battery voltage to initialize kalman filter
            uint32_t batteryVoltage = result.at(3).second.back();
            kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
        } else {
            // use sensor data to do a sample with the kalman filter
            bool isBatteryInFloat = (result.at(2).second.back() == 3);
            int32_t batteryMilliAmps = result.at(0).second.back();
            uint32_t batteryVoltage = result.at(3).second.back();
            int32_t batteryMilliWatts = result.at(1).second.back();
            uint32_t samplePeriodMilliSec = result.at(4).second.back();
            kalman.sample(isBatteryInFloat, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
        }
        lineIdx++;
    }

    // Close file
    myFile.close();

    return result;
}

int main() {

    // Read and process sensor data using kalman filter
    std::vector<std::pair<std::string, std::vector<int> > > sensor_data = process_csv("../../data/sensor_data.csv");

    printf("Finished reading.\n");

    // Write to another file to check that this was successful
    write_csv("../../data/raw_sensor_data.csv", sensor_data);

    printf("Finished writing.\n");
    
    return 0;
}