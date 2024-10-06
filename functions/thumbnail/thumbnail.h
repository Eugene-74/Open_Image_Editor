#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <exiv2/exiv2.hpp>
#include <fstream> 

#include <filesystem>  
void createThumbnails(const std::vector<std::string>& imagePaths, const std::string& outputDir);

