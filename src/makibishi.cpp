
#include <stdio.h>

#include <sstream>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "optparse.h"

std::string _replaceAll(std::string str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

int main(int argc, char** argv) {
  optparse_t opts("Usage:\n  makibishi"
                                    " -i <INPUT FILE>"
                                    " -c <COMMAND TO BE EXECUTED>"
                                    " [-j <NUMBER OF PROCESSES GENERATED>=1]"
                  ,
                  "j:i:c:", "ic", argc, argv);
  if(!opts.isGood()) return 0;
  
  std::ifstream ifsInput(opts.of('i').c_str(), std::ios::binary|std::ios::ate);
  size_t filesize = ifsInput.tellg();
  ifsInput.close();

  opts.defset('j', "1");
  
  int    numParallel = atoi(opts.of('j').c_str());
  size_t stride      = filesize/numParallel + 1;

  if(numParallel > 48) {
    std::cerr << "Too many sub processes." << std::endl;
    return 0;
  }
    
  for(size_t offset=0; offset<filesize; offset+=stride) {
    pid_t pid = fork();

    if(0 == pid) {              // Child.
      std::ifstream       ifsTarget(opts.of('i').c_str(), std::ios::binary);

      //
      // Seek to the head.
      ifsTarget.seekg(offset);

      std::string ln;

      // If the offset is non-zero, skip the current line.
      if(0 != offset)
        getline(ifsTarget, ln);

      // Ok, let us get the specified program started.
      std::ostringstream  ossTaskID, ossOffset; 
      std::string         command(opts.of('c'));

      ossTaskID << 1+offset/stride;
      ossOffset << ifsTarget.tellg();
      
      command = _replaceAll(command, "{}", ossTaskID.str());
      command = _replaceAll(command, "@", ossOffset.str());
      
      FILE *mystdin = popen(command.c_str(), "w");
      
      // Push each line into the sub process.
      while(ifsTarget.tellg() <= offset+stride && getline(ifsTarget, ln)) {
        fwrite(ln.c_str(), 1, ln.length(), mystdin);
        fwrite("\n", 1, 1, mystdin);
      }

      ifsTarget.close();
      pclose(mystdin);
      return 0;
    }

    // The parent just keeps going...
    
  }

  // Wait for children.
  int status;
  wait(&status);
  
  return 0;
}
