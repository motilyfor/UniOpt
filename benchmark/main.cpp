#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "audioeffectstore/api.h"

namespace {

struct BenchmarkCase {
    std::string              caseName;
    std::vector<std::string> moduleChain;
};

struct BenchmarkConfig {
    std::vector<BenchmarkCase> benchmarkCases = {{"rln", {"rln"}}};
    std::vector<std::size_t> frameSizes = {48, 120, 240, 480, 960};
    std::size_t              iterations = 40000;
    std::size_t              warmupIterations = 4000;
    double                   sampleRate = 48000.0;
    std::uint32_t            channels = 2;
    std::string              csvPath;
};

struct BenchmarkResult {
    std::string caseName;
    std::string moduleChain;
    std::size_t frames = 0;
    std::size_t iterations = 0;
    std::size_t warmupIterations = 0;
    double      sampleRate = 0.0;
    std::uint32_t channels = 0;
    double      elapsedMs = 0.0;
    double      nsPerFrame = 0.0;
    double      samplesPerSecond = 0.0;
    double      realtimeFactor = 0.0;
    double      checksum = 0.0;
};

std::vector<std::string> SplitString(const std::string& value, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream        stream(value);
    std::string              token;
    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string JoinModules(const std::vector<std::string>& modules, char delimiter)
{
    std::ostringstream output;
    for (std::size_t i = 0; i < modules.size(); ++i) {
        if (i != 0) {
            output << delimiter;
        }
        output << modules[i];
    }
    return output.str();
}

BenchmarkCase MakeBenchmarkCase(const std::string& rawCase)
{
    BenchmarkCase benchmarkCase;
    benchmarkCase.moduleChain = SplitString(rawCase, '+');
    if (benchmarkCase.moduleChain.empty()) {
        throw std::invalid_argument("benchmark case cannot be empty");
    }
    benchmarkCase.caseName = JoinModules(benchmarkCase.moduleChain, '+');
    return benchmarkCase;
}

std::vector<BenchmarkCase> ParseBenchmarkCases(const std::string& value)
{
    std::vector<BenchmarkCase> benchmarkCases;
    for (const std::string& rawCase : SplitString(value, ';')) {
        benchmarkCases.push_back(MakeBenchmarkCase(rawCase));
    }
    if (benchmarkCases.empty()) {
        throw std::invalid_argument("benchmark cases cannot be empty");
    }
    return benchmarkCases;
}

std::vector<std::size_t> ParseFrameSizes(const std::string& value)
{
    std::vector<std::size_t> frameSizes;
    for (const std::string& token : SplitString(value, ',')) {
        const unsigned long parsed = std::stoul(token);
        if (parsed == 0) {
            throw std::invalid_argument("frame size must be > 0");
        }
        frameSizes.push_back(static_cast<std::size_t>(parsed));
    }
    if (frameSizes.empty()) {
        throw std::invalid_argument("frame sizes cannot be empty");
    }
    return frameSizes;
}

std::vector<std::pair<std::string, std::vector<float>>> BuildParams(
    const std::vector<std::string>& modules)
{
    std::vector<std::pair<std::string, std::vector<float>>> params;
    params.reserve(modules.size());

    for (const std::string& module : modules) {
        if (module == "gain") {
            params.push_back({module, {0.95f}});
        } else if (module == "rln") {
            params.push_back({module, {1.0f}});
        } else {
            throw std::invalid_argument("unsupported module: " + module);
        }
    }

    return params;
}

std::vector<float> MakeInput(std::size_t frames, std::uint32_t channels)
{
    std::vector<float> interleaved(frames * static_cast<std::size_t>(channels), 0.0f);
    constexpr double   pi = 3.14159265358979323846;

    for (std::size_t i = 0; i < frames; ++i) {
        const double t = static_cast<double>(i);
        const float  left = static_cast<float>(0.35 * std::sin(2.0 * pi * 440.0 * t / 48000.0));
        const float  right = static_cast<float>(0.20 * std::sin(2.0 * pi * 880.0 * t / 48000.0));
        interleaved[i * channels] = left;
        if (channels > 1) {
            interleaved[i * channels + 1] = right;
        }
    }

    return interleaved;
}

double ComputeChecksum(const std::vector<float>& samples)
{
    double checksum = 0.0;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        checksum += static_cast<double>(samples[i]) * static_cast<double>((i % 13) + 1);
    }
    return checksum;
}

BenchmarkResult RunBenchmark(const BenchmarkConfig& config, const BenchmarkCase& benchmarkCase,
                             std::size_t frames)
{
    std::vector<float> input = MakeInput(frames, config.channels);
    std::vector<float> output(input.size(), 0.0f);

    const auto params = BuildParams(benchmarkCase.moduleChain);

    if (audioeffectstore::init(frames, config.channels) != audioeffectstore::Status::kOk) {
        throw std::runtime_error("audioeffectstore::init failed");
    }
    if (audioeffectstore::setParam(params) != audioeffectstore::Status::kOk) {
        throw std::runtime_error("audioeffectstore::setParam failed");
    }

    for (std::size_t i = 0; i < config.warmupIterations; ++i) {
        if (audioeffectstore::apply(input.data(), output.data(), frames, config.channels) !=
            audioeffectstore::Status::kOk) {
            throw std::runtime_error("warmup apply failed");
        }
    }

    const auto begin = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < config.iterations; ++i) {
        if (audioeffectstore::apply(input.data(), output.data(), frames, config.channels) !=
            audioeffectstore::Status::kOk) {
            throw std::runtime_error("benchmark apply failed");
        }
    }
    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::milli> elapsed = end - begin;
    const double totalFrames = static_cast<double>(frames) * static_cast<double>(config.iterations);
    const double totalSamples = totalFrames * static_cast<double>(config.channels);
    const double elapsedSeconds = elapsed.count() / 1000.0;

    BenchmarkResult result;
    result.caseName = benchmarkCase.caseName;
    result.moduleChain = JoinModules(benchmarkCase.moduleChain, '+');
    result.frames = frames;
    result.iterations = config.iterations;
    result.warmupIterations = config.warmupIterations;
    result.sampleRate = config.sampleRate;
    result.channels = config.channels;
    result.elapsedMs = elapsed.count();
    result.nsPerFrame = elapsedSeconds * 1e9 / totalFrames;
    result.samplesPerSecond = totalSamples / elapsedSeconds;
    result.realtimeFactor = (totalFrames / config.sampleRate) / elapsedSeconds;
    result.checksum = ComputeChecksum(output);
    return result;
}

void PrintUsage(const char* argv0)
{
    std::cout
        << "Usage: " << argv0 << " [options]\n"
    << "  --module gain|rln|gain,rln    Single module chain to benchmark\n"
    << "  --cases gain;rln;gain+rln     Multiple benchmark cases for grouped comparison\n"
        << "  --frames 48,120,240,480,960   Frame sizes per apply call\n"
        << "  --iterations 40000            Measured iterations\n"
        << "  --warmup 4000                 Warmup iterations\n"
        << "  --sample-rate 48000           Used for realtime_x calculation\n"
    << "  --csv benchmark.csv           Write CSV to file\n"
        << "  --help                        Show this message\n";
}

BenchmarkConfig ParseArgs(int argc, char** argv)
{
    BenchmarkConfig config;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if (arg == "--help") {
            PrintUsage(argv[0]);
            std::exit(0);
        }

        if (i + 1 >= argc) {
            throw std::invalid_argument("missing value for argument: " + std::string(arg));
        }

        const std::string value(argv[++i]);
        if (arg == "--module") {
            BenchmarkCase benchmarkCase;
            benchmarkCase.moduleChain = SplitString(value, ',');
            if (benchmarkCase.moduleChain.empty()) {
                throw std::invalid_argument("module chain cannot be empty");
            }
            benchmarkCase.caseName = JoinModules(benchmarkCase.moduleChain, '+');
            config.benchmarkCases = {benchmarkCase};
        } else if (arg == "--cases") {
            config.benchmarkCases = ParseBenchmarkCases(value);
        } else if (arg == "--frames") {
            config.frameSizes = ParseFrameSizes(value);
        } else if (arg == "--iterations") {
            config.iterations = std::stoull(value);
        } else if (arg == "--warmup") {
            config.warmupIterations = std::stoull(value);
        } else if (arg == "--sample-rate") {
            config.sampleRate = std::stod(value);
            if (config.sampleRate <= 0.0) {
                throw std::invalid_argument("sample rate must be > 0");
            }
        } else if (arg == "--csv") {
            config.csvPath = value;
        } else {
            throw std::invalid_argument("unknown argument: " + std::string(arg));
        }
    }

    if (config.iterations == 0) {
        throw std::invalid_argument("iterations must be > 0");
    }
    return config;
}

std::string BuildCsv(const std::vector<BenchmarkResult>& results)
{
    std::ostringstream output;
    output << std::fixed << std::setprecision(3);
    output << "case_name,module_chain,frames,iterations,warmup,sample_rate,channels,elapsed_ms,ns_per_frame,samples_per_sec,realtime_x,checksum\n";
    for (const BenchmarkResult& result : results) {
        output << result.caseName << ','
               << result.moduleChain << ','
               << result.frames << ','
               << result.iterations << ','
               << result.warmupIterations << ','
               << result.sampleRate << ','
               << result.channels << ','
               << result.elapsedMs << ','
               << result.nsPerFrame << ','
               << result.samplesPerSecond << ','
               << result.realtimeFactor << ','
               << result.checksum << '\n';
    }
    return output.str();
}

void WriteCsvFile(const std::string& path, const std::string& content)
{
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("failed to open csv file: " + path);
    }
    output << content;
    if (!output.good()) {
        throw std::runtime_error("failed to write csv file: " + path);
    }
}

}   // namespace

int main(int argc, char** argv)
{
    try {
        const BenchmarkConfig config = ParseArgs(argc, argv);
        std::vector<BenchmarkResult> results;
        results.reserve(config.benchmarkCases.size() * config.frameSizes.size());

        for (const BenchmarkCase& benchmarkCase : config.benchmarkCases) {
            for (std::size_t frames : config.frameSizes) {
                results.push_back(RunBenchmark(config, benchmarkCase, frames));
            }
        }

        const std::string csv = BuildCsv(results);
        std::cout << csv;
        if (!config.csvPath.empty()) {
            WriteCsvFile(config.csvPath, csv);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "benchmark error: " << error.what() << '\n';
        return 1;
    }
}
