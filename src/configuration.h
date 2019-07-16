#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>
#include <string>

#define RECURSE_DIRECTORIES false
#define ENCODING_QUALITY 5
#define OVERWRITE_EXISTING_MP3 false
#define CONVERT_ALL_FILES false

class Configuration {
  public:
    static bool parse_arguments(int argc, char* argv[]);

    static std::string   directory_path();
    static bool          recurse_directories();
    static int           encoding_quality();
    static bool          overwrite_existing_mp3();
    static bool          convert_all_files();
    static std::uint16_t number_of_threads();

  private:
    static std::string version();

  private:
    static std::string   _name;
    static std::string   _version;
    static std::string   _directory_path;
    static bool          _recurse_directories;
    static int           _encoding_quality;
    static bool          _overwrite_existing_mp3;
    static bool          _convert_all_files;
    static std::uint16_t _number_of_threads;
};

#endif  // CONFIGURATION_H
