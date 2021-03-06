/////////////////////////////////////////////////////////////////////////////////////////////////////
//Center for Satellite Applications and Research (STAR)
//NOAA Center for Weather and Climate Prediction (NCWCP)
//5830 University Research Court
//College Park, MD 20740
//Purpose: Parse a STAR JSON format file
// see star_json.html for a description of the format
// Dependencies: 
// Jansson library
// HDF5 library
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <stdio.h>
#include "star_json.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "usage : ./star_json <JSON file in ./data>" << std::endl;
    exit(1);
  }
  star_json_t star(argv[1]);
  star.process();

  return 0;
}


