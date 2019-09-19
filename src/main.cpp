#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "adm/common_definitions.hpp"
#include "adm/parse.hpp"
#include "adm/write.hpp"

#include "bw64/bw64.hpp"

#include "ear/ear.hpp"
#include "ear/dsp/dsp.hpp"

using namespace ear;

const unsigned int BLOCK_SIZE = 4096; // in frames

int main(int argc, char **argv) {

  // Read input BWF64/ADM file
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " [INFILE]" << std::endl;
    exit(1);
  }

  auto bw64File = bw64::readFile(argv[1]);

  // BWF64 file infos
  std::cout << std::endl << "### BWF64 file infos:" << std::endl;
  std::cout << "Format:" << std::endl;
  std::cout << " - formatTag: " << bw64File->formatTag() << std::endl;
  std::cout << " - channels: " << bw64File->channels() << std::endl;
  std::cout << " - sampleRate: " << bw64File->sampleRate() << std::endl;
  std::cout << " - bitDepth: " << bw64File->bitDepth() << std::endl;
  std::cout << " - numerOfFrames: " << bw64File->numberOfFrames() << std::endl;

  std::cout << "chunkIds:" << std::endl;
  for (auto& chunk : bw64File->chunks()) {
    std::cout << " - " << '\'' << bw64::utils::fourCCToStr(chunk.id) << '\''
              << std::endl;
  }

  if (bw64File->hasChunk(bw64::utils::fourCC("chna"))) {
    if (auto chnaChunk = bw64File->chnaChunk()) {
      std::cout << "ChnaChunk:" << std::endl;
      std::cout << " - numTracks: " << chnaChunk->numTracks() << std::endl;
      std::cout << " - numUids: " << chnaChunk->numUids() << std::endl;
      std::cout << " - audioIds:" << std::endl;
      for (auto audioId : chnaChunk->audioIds()) {
        std::cout << "   - ";
        std::cout << audioId.trackIndex() << ", " << audioId.uid() << ", "
                  << audioId.trackRef() << ", " << audioId.packRef()
                  << std::endl;
      }
    }
  }

  // Extract ADM XML from BWF64
  std::cout << std::endl << "### Extract ADM XML from BWF64:" << std::endl;
  std::stringstream axmlStringstream;
  if (bw64File->axmlChunk()) {
    bw64File->axmlChunk()->write(axmlStringstream);
    std::cout << axmlStringstream.str();
  } else {
    std::cerr << "could not find an axml chunk";
    exit(1);
  }

  // Read file ADM composition
  std::cout << std::endl << "### Read ADM composition:" << std::endl;
  auto admDocument = adm::parseXml(axmlStringstream);

  // write XML data to stdout
  std::stringstream xmlStream;
  writeXml(xmlStream, admDocument);
  std::cout << xmlStream.str();


  // TODO: extract info!

  // Calculate gains
  std::cout << std::endl << "### Calculate gains:" << std::endl;
  // make the gain calculator
  std::string outputLayout = "0+2+0"; // defined into libear/resources/2051_layouts.yaml
  Layout layout = getLayout(outputLayout);
  size_t outputNbChannels = layout.channels().size();

  // calculate gains for direct speakers
  GainCalculatorDirectSpeakers speakerGainCalculator(layout);
  DirectSpeakersTypeMetadata speakersMetadata;
  std::vector<float> gains(outputNbChannels);
  speakerGainCalculator.calculate(speakersMetadata, gains);

  // calculate gains for objects speakers
  GainCalculatorObjects objectsGainCalculator(layout);
  ObjectsTypeMetadata objectsMetadata;
  // TODO: should we extract position from input ADM?
  objectsMetadata.position = PolarPosition(0.0f, 0.0f, 1.0f); // center objects
  std::vector<float> directGains(outputNbChannels);
  std::vector<float> diffuseGains(outputNbChannels);
  objectsGainCalculator.calculate(objectsMetadata, directGains, diffuseGains);

  // print the output
  auto fmt = std::setw(10);
  std::cout << std::setprecision(4);

  std::cout << fmt << "channel"
            << fmt << "gain"
            << fmt << "direct"
            << fmt << "diffuse"  << std::endl;
  for (size_t i = 0; i < outputNbChannels; i++) {
    std::cout << fmt << layout.channels()[i].name()
              << fmt << gains[i]
              << fmt << directGains[i]
              << fmt << diffuseGains[i] << std::endl;
  }

  // Render samples with gains
  std::cout << std::endl << "### Render samples with gains:" << std::endl;

  // Interpolator
  dsp::GainInterpolator<dsp::LinearInterpVector> interpolator;
  size_t nbSamples = bw64File->numberOfFrames();
  for (int i = 0; i < nbSamples; ++i) {
    interpolator.interp_points.push_back(std::make_pair(i, gains));
  }

  // Output file
  auto outputFile = bw64::writeFile("output_bwf64.wav", outputNbChannels, bw64File->sampleRate(), bw64File->bitDepth());

  // Buffers
   // FIXME here we read every input channels, meanwhile the interpolation expect one input channel at a time!
  const size_t inputNbChannels = bw64File->channels();
  // std::vector<float> fileInputBuffer(BLOCK_SIZE * inputNbChannels);
  std::vector<float> fileOutputBuffer(BLOCK_SIZE * outputNbChannels);

  float * const in = new float[BLOCK_SIZE * inputNbChannels];
  // float **in = new float*[inputNbChannels];
  // for (int i = 0; i < inputNbChannels; ++i) {
  //   in[i] = new float[BLOCK_SIZE];
  // }

  float **out = new float*[outputNbChannels];
  for (int i = 0; i < outputNbChannels; ++i) {
    out[i] = new float[BLOCK_SIZE];
  }

  // Process interpolation and write output file
  dsp::SampleIndex blockStart = 0;
  while (!bw64File->eof()) {
    // auto readFrames = bw64File->read(&fileInputBuffer[0], BLOCK_SIZE);
    auto readFrames = bw64File->read(&in[0], BLOCK_SIZE);
    std::cout << blockStart << " | Read " << readFrames << " frames";

    // // init input buffer
    // size_t position = 0;
    // for (int f = 0; f < readFrames; ++f) {
    //   for (int c = 0; c < inputNbChannels; ++c) {
    //     in[c] = &fileInputBuffer.at(position++);
    //   }
    // }

    // for (int oc = 0; oc < outputNbChannels; ++oc) {
    //   for (int ic = 0; ic < inputNbChannels; ++ic) {
    //     interpolator.process(blockStart, readFrames, &in[ic], &out[oc]);
    //     std::cout << " | Processed";
    //   }
    // }
    // blockStart++;
    interpolator.process(blockStart++, readFrames, &in, out);

    for (int f = 0; f < readFrames; ++f) {
      for (int c = 0; c < outputNbChannels; ++c) {
        fileOutputBuffer.push_back(out[c][f]);
      }
    }

    auto wroteFrames = outputFile->write(&fileOutputBuffer[0], readFrames);
    std::cout << " | Wrote " << wroteFrames << " frames." << std::endl;
    fileOutputBuffer.clear();
  }

  delete in;
  for(int i = 0; i < outputNbChannels; ++i){
    delete[] out[i];
  }
  delete[] out;

  return 0;
}
