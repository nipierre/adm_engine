#ifndef ADM_ENGINE_WORKER
#define ADM_ENGINE_WORKER

#include <stdio.h>
#include <string.h>
#include <iostream>

#include <bw64/bw64.hpp>

#include "adm_engine/renderer.hpp"
#include "adm_engine/parser.hpp"

using namespace admengine;

void assignStringtoPointer(const std::string& str, char* pointer) {
  std::copy(str.begin(), str.end(), pointer);
  pointer[str.size()] = '\0';
}

int dumpBw64AdmFile(const std::string& path, char* output_message) {
  auto bw64File = bw64::readFile(path);
  const std::string admDocumentStr = getAdmDocumentAsString(getAdmDocument(parseAdmXmlChunk(bw64File)));
  assignStringtoPointer(admDocumentStr, output_message);
  std::cout << output_message << std::endl;
  return 0;
}

std::map<std::string, float> parseElementGains(const std::string& elementGainsStr) {
  std::map<std::string, float> elementGains;
  const size_t arrayInPos = elementGainsStr.find("[");
  if(arrayInPos ==  std::string::npos) {
    throw std::runtime_error("Invalid gains mapping string format: missing '[' as first character.");
  }
  const size_t arrayOutPos = elementGainsStr.find("]");
  if(arrayOutPos ==  std::string::npos) {
    throw std::runtime_error("Invalid gains mapping string format:  missing ']' as last character.");
  }
  const std::string gainPairsStr = elementGainsStr.substr(arrayInPos + 1, arrayOutPos - 1);

  size_t nextPosition = 0;
  std::string separator = ", ";
  while(nextPosition < gainPairsStr.size()){
    size_t separatorPos = gainPairsStr.find(separator, nextPosition);

    if(separatorPos == std::string::npos){
      separatorPos = gainPairsStr.size();
    }

    const std::string gainPair = gainPairsStr.substr(nextPosition + 1, separatorPos - (nextPosition + 2)); // "element_id=gain"
    const size_t equalPos = gainPair.find("=");
    const std::string elemId = gainPair.substr(0, equalPos); // "element_id"
    const std::string gainDbStr = gainPair.substr(equalPos + 1, gainPair.size()); // "gain"
    elementGains[elemId] = pow(10.0, std::atof(gainDbStr.c_str()) / 20.0);
    std::cout << "Gain:                  " << elementGains[elemId] << " (" << gainDbStr << " dB) applied to " << elemId << std::endl;

    nextPosition = separatorPos + separator.size();
  }

  return elementGains;
}

int renderAdmContent(const char* input,
                     const char* destination,
                     const char* elementGainsCStr,
                     const char* elementIdToRenderCStr,
                     char* output_message) {

  const std::string inputFilePath(input);
  const std::string outputDirectoryPath(destination);
  std::cout << "Input file:            " << inputFilePath << std::endl;
  std::cout << "Output directory:      " << outputDirectoryPath << std::endl;

  std::string elementIdToRender;
  if(elementIdToRenderCStr) {
    elementIdToRender = elementIdToRenderCStr;
    std::cout << "ADM element to render: " << elementIdToRender << std::endl;
  }

  std::map<std::string, float> elementGains;
  if(elementGainsCStr) {
    elementGains = parseElementGains(elementGainsCStr);
  }

  auto bw64File = bw64::readFile(inputFilePath);
  const std::string outputLayout("0+2+0"); // TODO: get it from args

  try {
    Renderer renderer(bw64File, outputLayout, outputDirectoryPath, elementGains, elementIdToRender);
    renderer.process();
  } catch(const std::exception& e) {
    std::string error(e.what());
    std::cerr << "Error: " << error << std::endl;
    assignStringtoPointer(error, output_message);
    return 1;
  }
  return 0;
}

void displayUsage() {
  std::cout << "Parameters:      TYPE                           DESCRIPTION" << std::endl;
  std::cout << std::endl;
  std::cout << "  input          (string)                       BW64/ADM audio file path" << std::endl;
  std::cout << "  output         (string) (optional)            Destination directory" << std::endl;
  std::cout << "  element_id     (string) (optional)            Select the AudioProgramme or AudioObject to be renderer by `element_id`" << std::endl;
  std::cout << "  gain_mapping   (array_of_strings) (optional)  Array of `ELEMENT_ID=GAIN` strings, where `GAIN` is the gain value (in dB) to apply to ADM element defined by its `ELEMENT_ID`" << std::endl;
  std::cout << std::endl;
  std::cout << "  If no `output` argument is specified, this program dumps the input BW64/ADM file information." << std::endl;
  std::cout << "  Otherwise, it enables ADM rendering to BW64/ADM file into destination directory." << std::endl;
  std::cout << std::endl;
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get worker name
 */
char* get_name() {
	return (char*)"ADM Engine Worker";
}

/**
 * Get worker short description
 */
char* get_short_description() {
	return (char*)"Processes BW64/ADM audio file rendering.";
}

/**
 * Get worker long description
 */
char* get_description() {
	return (char*)"This worker processes ADM rendering from specified BW64/ADM audio file.";
}

/**
 * Get worker version
 */
char* get_version() {
	return (char*)"1.0.0";
}

/**
 * Worker parameter type
 */
typedef struct Parameter {
    char* identifier;
    char* label;
    unsigned int kind_size;
    char** kind;
    int required;
} Parameter;

// Worker parameters
char* string_kind[1] = { (char*)"string" };
char* array_of_strings_kind[1] = { (char*)"array_of_strings" };

Parameter worker_parameters[4] = {
    {
        .identifier = (char*)"input",
        .label = (char*)"BW64/ADM audio file path",
        .kind_size = 1,
        .kind = string_kind,
        .required = 1
    },
    {
        .identifier = (char*)"output",
        .label = (char*)"Destination directory",
        .kind_size = 1,
        .kind = string_kind,
        .required = 0
    },
    {
        .identifier = (char*)"element_id",
        .label = (char*)"Select the AudioProgramme or AudioObject to be renderer by `element_id`",
        .kind_size = 1,
        .kind = string_kind,
        .required = 0
    },
    {
        .identifier = (char*)"gain_mapping",
        .label = (char*)"Array of `ELEMENT_ID=GAIN` strings, where `GAIN` is the gain value (in dB) to apply to ADM element defined by its `ELEMENT_ID`",
        .kind_size = 1,
        .kind = array_of_strings_kind,
        .required = 0
    }
};

/**
 * Get number of worker parameters
 */
unsigned int get_parameters_size() {
    return sizeof(worker_parameters) / sizeof(Parameter);
}

/**
 * Retrieve worker parameters
 * @param parameters    Output parameters array pointer
 */
void get_parameters(Parameter* parameters) {
    memcpy(parameters, worker_parameters, sizeof(worker_parameters));
}

typedef void* JobParameters;
typedef char* (*GetParameterValueCallback)(JobParameters, const char*);
typedef void* (*Logger)(const char*);
typedef int* (*CheckError)();

/**
 * Worker main process function
 * @param job                      Job parameters handler
 * @param parametersValueGetter    Get job parameter value callback
 * @param checkError               Check error callback
 * @param logger                   Rust logger callback
 */
int process(JobParameters job, GetParameterValueCallback parametersValueGetter, CheckError checkError, Logger logger, char* output_message) {
    // Print message through the Rust internal logger
    logger("Start C Worker process...");

    // Retrieve job parameter value
    char* inputFilePath = parametersValueGetter(job, "input");
    if(inputFilePath == NULL) {
    	checkError();
    	displayUsage();
    	return 1;
    }
    logger(inputFilePath);

    char* outputDirectoryPath = parametersValueGetter(job, "output");
    char* elementGainsStr = parametersValueGetter(job, "gain_mapping");
    char* elementIdToRender = parametersValueGetter(job, "element_id");

    if(outputDirectoryPath == NULL) {
    	return dumpBw64AdmFile(inputFilePath, output_message);
    } else {
    	return renderAdmContent(inputFilePath, outputDirectoryPath, elementGainsStr, elementIdToRender, output_message);
    }
}

#ifdef __cplusplus
}
#endif

#endif
