#include "ear/ear.hpp"
#include "bw64/bw64.hpp"
#include "adm/adm.hpp"

const unsigned int BLOCK_SIZE = 4096; // in frames

class AudioObjectRenderer {

public:
  AudioObjectRenderer(const std::vector<size_t>& trackIds, const size_t& audioPackFormat, const adm::TypeDescriptor& typeDescriptor)
    : _trackIds(trackIds)
    , _audioPackFormat(audioPackFormat)
    , _typeDescriptor(typeDescriptor)
  {}

  std::vector<size_t> getTrackIds() const {
    return _trackIds;
  }

  size_t getAudioPackFormat() const {
    return _audioPackFormat;
  }

  adm::TypeDescriptor getTypeDescriptor() const {
    return _typeDescriptor;
  }

  void render(const float * in, const size_t& nbSamples, const size_t nbInputChannels, float * out) {
  }

private:
  const std::vector<size_t> _trackIds;
  const size_t _audioPackFormat;
  const adm::TypeDescriptor _typeDescriptor;

};

std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer) {
  os << "Channels: ";
  for (size_t channel : renderer.getTrackIds()) {
     os << channel << " ";
  }
  os << " - AudioPackFormat: " << renderer.getAudioPackFormat();
  os << " - TypeDesciptor: " << adm::formatTypeDefinition(renderer.getTypeDescriptor());
  return os;
}

std::vector<float> getDirectSpeakersGains(const ear::Layout& layout, const size_t& outputNbChannels) {
  // calculate gains for direct speakers
  std::cout << std::endl << "### Calculate gains:" << std::endl;
  ear::GainCalculatorDirectSpeakers speakerGainCalculator(layout);
  ear::DirectSpeakersTypeMetadata speakersMetadata;
  std::vector<float> gains(outputNbChannels);
  speakerGainCalculator.calculate(speakersMetadata, gains);

  // print the output
  auto fmt = std::setw(10);
  std::cout << std::setprecision(4);

  std::cout << fmt << "channel"
            << fmt << "gain"  << std::endl;
  for (size_t i = 0; i < outputNbChannels; i++) {
    std::cout << fmt << layout.channels()[i].name()
              << fmt << gains[i] << std::endl;
  }
}

void render(const std::unique_ptr<bw64::Bw64Reader>& bw64File,
            const ear::Layout& layout,
            const std::string programmeTitle,
            const std::vector<AudioObjectRenderer> renderers) {

  for(AudioObjectRenderer renderer: renderers) {
    std::cout << "\t- Render: " << renderer << std::endl;

    switch(renderer.getTypeDescriptor().get()) {
      case 1: // TypeDefinition::DIRECT_SPEAKERS
        break;
      case 0: // TypeDefinition::UNDEFINED
      case 2: // TypeDefinition::MATRIX
      case 3: // TypeDefinition::OBJECTS
      case 4: // TypeDefinition::HOA
      case 5: // TypeDefinition::BINAURAL
        std::cerr << "Unsupported type descriptor: " << adm::formatTypeDefinition(renderer.getTypeDescriptor()) << std::endl;
        exit(1);
    }
  }

  const size_t outputNbChannels = layout.channels().size();
  const std::vector<float> gains = getDirectSpeakersGains(layout, outputNbChannels);

  // Render samples with gains
  std::cout << std::endl << "### Render samples with gains:" << std::endl;

  // Output file
  std::stringstream outputFileName;
  outputFileName << programmeTitle << ".wav";
  auto outputFile = bw64::writeFile(outputFileName.str(), outputNbChannels, bw64File->sampleRate(), bw64File->bitDepth());

  // Buffers
  const size_t inputNbChannels = bw64File->channels();
  std::vector<float> fileInputBuffer(BLOCK_SIZE * inputNbChannels);
  std::vector<float> fileOutputBuffer(BLOCK_SIZE * outputNbChannels);

  // Read file, render with gains and write output file
  ear::dsp::SampleIndex blockStart = 0;
  while (!bw64File->eof()) {
    // Read a data block
    auto readFrames = bw64File->read(&fileInputBuffer[0], BLOCK_SIZE);
    std::cout << blockStart << " | Read " << readFrames << " frames";
    // std::cout << std::endl;

    float* ocsample = new float[outputNbChannels];
    size_t position = 0;
    while(position < readFrames) {
      for (int ic = 0; ic < inputNbChannels; ++ic) {
        if(position % inputNbChannels == ic) {
          for (int oc = 0; oc < outputNbChannels; ++oc) {
            if(position % outputNbChannels == oc) {
              ocsample[oc] += fileInputBuffer[position] * gains[oc];
              // std::cout << position << "-> " << ic << " -> " << oc << " : " << ocsample[oc] << std::endl;
            }
          }
        }
      }
      position++;
      for (int oc = 0; oc < outputNbChannels; ++oc) {
        fileOutputBuffer.push_back(ocsample[oc]); // FIXME : double sound in output ?
      }
    }

    auto wroteFrames = outputFile->write(&fileOutputBuffer[0], readFrames);
    std::cout << " | Wrote " << wroteFrames << " frames." << std::endl;
    fileOutputBuffer.clear();
    delete[] ocsample;
  }
  std::cout << "Done with: " << outputFileName.str() << std::endl;
  bw64File->seek(0);
}
