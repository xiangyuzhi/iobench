#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

void generateRandomFile(const std::string &filename, std::size_t fileSize) {
  std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);

  if (!outFile.is_open()) {
    std::cerr << "Error opening file for writing: " << filename << std::endl;
    return;
  }

  // Seed the random number generator with the current time
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  // Generate and write random data to the file
  for (std::size_t i = 0; i < fileSize; ++i) {
    char randomByte = static_cast<char>(std::rand() % 256);
    outFile.write(&randomByte, sizeof(char));
  }

  outFile.close();
}

int main(int argc, char *argv[]) {

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << "<filename> <file_size_in_MB>"
              << std::endl;
  }

  const std::string filename = argv[1];
  const std::size_t fileSize = std::stoull(argv[2]) * 1024 * 1024;

  generateRandomFile(filename, fileSize);

  std::cout << "Random file generated: " << filename << " (" << fileSize
            << " bytes)" << std::endl;

  return 0;
}
