/***************************************************
 * A program to process wavedump output files
 *
 * Author 
 *  Gary Smith
 *  https://github.com/gsmith23
 *  20 11 18
 *
 * Adapted from
 * SPE_Gen.cpp 
 *  Tomi Akindele
 *  https://github.com/akindele
 *  04 10 2018
 *
 * Purpose
 *  DAQ reads out to binary file.
 *  This program de-codes the file
 *  and reads the pulse data in 
 *  to a root TTree.
 *
 * How to build
 *  $ make BinToRoot
 *
 * How to run
 *  $ ./BinToRoot  
 *
 * Dependencies
 *  root.cern
 *  Makefile
 *
 */ 

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "TFile.h"
#include "TTree.h"

using namespace std;

int  getNSamples(char digitiser){

  if     ( digitiser == 'V' )
    return 110;
  else if( digitiser == 'D' )
    return 1024;
  else{
    cerr << "Error: Unknown digitiser " << endl;
    return 0;
  }
  
}

bool isCorrectDigitiser(int header, char digitiser){
  
  if( ( header == 244  &&  digitiser == 'V' ) || 
      ( header == 4120 &&  digitiser == 'D' ) )
    return true;
  else{
    cerr << " Error: digitiser choice does not match header info " << endl;
    return false;
  }
}

int ProcessBinaryFile(string fileName,
		      char digitiser = 'D',
		      int  verbosity = 0
		      ){
  
  ifstream fileStream(fileName);
  
  int   nEvents = 0,  fileHeader  = 0, sample = 0;
  float VDC     = 0., eventHeader = 0.;    

  float minVDC = 0., maxVDC = 0.;  
  
  // read in data from streamer object
  // until the end of the file
  while ( fileStream.is_open() && 
	  fileStream.good()    && 
	  !fileStream.eof()       ){
    
    nEvents++;
    
    //  if ( nEvents%10000 == 0)
     //    printf("Waveform Progress: %d \n", nEvents);
    
    // data to be recorded for each pulse
    sample = 0;
    VDC    = 0.;    
    
    eventHeader = 0.;    
    
    // range will be used to check for 
    // zero crossing
    minVDC = 100000.;
    maxVDC = -100000.;  
    
    // read in header info which comes 
    // as six char sized chunks
    for (int iHeader = 0 ; iHeader < 6 ; iHeader++ ){
      
      fileHeader = 0;
      
      // read in 4 bit (char) segment
      fileStream.read( (char*)&fileHeader,
		       sizeof(int)); 
      
      // check first header value matches expectations
      // NB other values may be acceptable so modify
      // isCorrectDigitiser() as appropriate
      if ( iHeader == 0 && nEvents == 1 &&
	   !isCorrectDigitiser(fileHeader,
			       digitiser)  ) {
	
	return -1;
      }
    } // end: for (int i = 0 ; i < intsPerHeader
    
    unsigned short shortResult = 0.;
    
    // read in pulse which comes 
    // in 2 (VME) or 4 (Desktop) bit chunks
    for (int sample = 0; sample < getNSamples(digitiser); sample++){
      
      VDC = 0;
      
      if     ( digitiser == 'V' ){
	shortResult = 0;    
	
	fileStream.read((char*)&shortResult,2); //read 2 bits
	VDC = (float)shortResult;
      }
      
      else if( digitiser == 'D' ){
	
	fileStream.read((char*)&VDC,sizeof(float));// read 4 bits
	
      }
      else {
	
	cerr << " Error:  incorrect digitiser " << endl;
	return -1;
	
      }
      
      if     ( VDC < minVDC ){
	minVDC = VDC;
      }
      else if( VDC > maxVDC ) {
	maxVDC = VDC;
      }
      
      //--------------------------------
      // Write sample by sample data here
      
      if(verbosity > 1)
	cout << " VDC(" << sample << ") = " << VDC << endl;
      
    }
    
    if( minVDC < 0 && maxVDC > 0 )
      cout << " Warning: pulse is zero crossing " << endl;
    

    //--------------------------------
    // Write event by event data here
    
    if(verbosity > 0){
      cout << endl;
      cout << " minVDC(" << nEvents << ") = " << minVDC << endl;
      cout << " maxVDC(" << nEvents << ") = " << maxVDC << endl;
      
      if(verbosity > 2)
	cout << endl;
    }
  
  } // end: while loop

  // close wavedump file
  fileStream.close();	

  //--------------------------------
  // Write file info here
  
  
  return nEvents;
}


int main(int argc, char **argv)
{
  
  // 'D' - Desktop
  // 'V' - VME
  char   digitiser = 'V';
  string fileName = "../../Data/wave_0_VME.dat";
  
  // 0 - silence, 1 - event-by-event, 2 - sample-by-sample
  int  verbosity   = 0;
  
  if( digitiser == 'D' )
    fileName = "../../Data/wave_0.dat";
  
  cout << " The binary file is called  " << fileName << endl;
  
  int nEvents = ProcessBinaryFile(fileName,
				  digitiser,
				  verbosity);
  
  cout << " This file contains " << nEvents << " events " << endl; 
}
