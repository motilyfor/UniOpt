#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "src/common/algomemcpy.hpp"

namespace {

enum class DataType {
    kInt8,
    kInt16,
    kInt32,
    kFloat,
};

struct Config {
    std::vector<DataType>    types = {DataType::kFloat};
    std::vector<std::size_t> counts = {256, 1024, 4096, 16384, 65536};
    std::size_t              warmupIterations = 5000;
    std::size_t              iterations = 50000;
    std::size_t              samples = 20;
    std::size_t              workingSetBytes = 8 * 1024 * 1024;
    std::string              csvPath;
};

struct Row {
    std::string type;
    std::string method;
    std::size_t count = 0;
    std::size_t bytes = 0;
    std::size_t alignmentBytes = 0;
    std::size_t warmup = 0;
    std::size_t iterations = 0;
    std::size_t samples = 0;
    std::size_t workingSetBytes = 0;
    std::size_t bufferSlots = 0;
    double      elapsedMsMean = 0.0;
    double      elapsedMsMin = 0.0;
    double      elapsedMsMax = 0.0;
    double      gbpsMean = 0.0;
    double      gbpsMin = 0.0;
    double      gbpsMax = 0.0;
    double      checksum = 0.0;
};

struct Stats {
    double mean = 0.0;
    double min = 0.0;
    double max = 0.0;
};

template <typename T>
class AlignedBuffer {
public:
    AlignedBuffer(std::size_t count, std::size_t alignment) : ptr_(nullptr), count_(count)
    {
        const std::size_t sizeBytes = count * sizeof(T);
        void*             memory = nullptr;
#if defined(_MSC_VER)
        memory = _aligned_malloc(sizeBytes, alignment);
        if (memory == nullptr) {
            throw std::bad_alloc();
        }
#else
        if (posix_memalign(&memory, alignment, sizeBytes) != 0) {
            throw std::bad_alloc();
        }
#endif
        ptr_ = static_cast<T*>(memory);
    }

    ~AlignedBuffer()
    {
#if defined(_MSC_VER)
        _aligned_free(ptr_);
#else
        std::free(ptr_);
#endif
    }

    T* data() { return ptr_; }
    const T* data() const { return ptr_; }
    std::size_t size() const { return count_; }

private:
    T*          ptr_;
    std::size_t count_;
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

std::vector<std::size_t> ParseCounts(const std::string& value)
{
    std::vector<std::size_t> counts;
    for (const std::string& token : SplitString(value, ',')) {
        const std::size_t count = std::stoull(token);
        if (count == 0) {
            throw std::invalid_argument("count must be > 0");
        }
        counts.push_back(count);
    }
    if (counts.empty()) {
        throw std::invalid_argument("counts cannot be empty");
    }
    return counts;
}

DataType ParseType(const std::string& token)
{
    if (token == "int8") {
        return DataType::kInt8;
    }
    if (token == "int16") {
        return DataType::kInt16;
    }
    if (token == "int32") {
        return DataType::kInt32;
    }
    if (token == "float") {
        return DataType::kFloat;
    }
    throw std::invalid_argument("unsupported type: " + token);
}

std::vector<DataType> ParseTypes(const std::string& value)
{
    std::vector<DataType> types;
    for (const std::string& token : SplitString(value, ',')) {
        types.push_back(ParseType(token));
    }
    if (types.empty()) {
        throw std::invalid_argument("types cannot be empty");
    }
    return types;
}

const char* TypeName(DataType type)
{
    switch (type) {
        case DataType::kInt8:
            return "int8";
        case DataType::kInt16:
            return "int16";
        case DataType::kInt32:
            return "int32";
        case DataType::kFloat:
            return "float";
    }
    return "unknown";
}

void PrintUsage(const char* argv0)
{
    std::cout
        << "Usage: " << argv0 << " [options]\n"
        << "  --types float,int8,int16,int32  Data types to benchmark\n"
        << "  --counts 256,1024,4096           Element count per copy\n"
        << "  --warmup 5000                    Warmup iterations\n"
        << "  --iterations 50000               Measured iterations\n"
        << "  --samples 20                     Measured sample runs\n"
        << "  --working-set-kb 8192            Total rotating src+dst working set in KB\n"
        << "  --csv memcpy_compare.csv         Write CSV to file\n"
        << "  --help                           Show this message\n";
}

Config ParseArgs(int argc, char** argv)
{
    Config config;
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
        if (arg == "--types") {
            config.types = ParseTypes(value);
        } else if (arg == "--counts") {
            config.counts = ParseCounts(value);
        } else if (arg == "--warmup") {
            config.warmupIterations = std::stoull(value);
        } else if (arg == "--iterations") {
            config.iterations = std::stoull(value);
        } else if (arg == "--samples") {
            config.samples = std::stoull(value);
        } else if (arg == "--working-set-kb") {
            config.workingSetBytes = std::stoull(value) * 1024;
        } else if (arg == "--csv") {
            config.csvPath = value;
        } else {
            throw std::invalid_argument("unknown argument: " + std::string(arg));
        }
    }

    if (config.iterations == 0) {
        throw std::invalid_argument("iterations must be > 0");
    }
    if (config.samples == 0) {
        throw std::invalid_argument("samples must be > 0");
    }
    if (config.workingSetBytes == 0) {
        throw std::invalid_argument("working set must be > 0");
    }
    return config;
}

std::size_t ComputeBufferSlots(const Config& config, std::size_t sizeBytes)
{
    const std::size_t bytesPerSlot = sizeBytes * 2;
    std::size_t       slots = config.workingSetBytes / bytesPerSlot;
    if (slots == 0) {
        slots = 1;
    }
    return slots;
}

Stats ComputeStats(const std::vector<double>& values)
{
    Stats stats;
    if (values.empty()) {
        return stats;
    }

    double sum = 0.0;
    stats.min = std::numeric_limits<double>::max();
    stats.max = std::numeric_limits<double>::lowest();
    for (double v : values) {
        sum += v;
        if (v < stats.min) {
            stats.min = v;
        }
        if (v > stats.max) {
            stats.max = v;
        }
    }
    stats.mean = sum / static_cast<double>(values.size());
    return stats;
}

template <typename T>
void FillInput(T* src, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        src[i] = static_cast<T>((i * 17 + 31) & 0x7f);
    }
}

template <>
void FillInput<float>(float* src, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        src[i] = static_cast<float>((i % 1024) * 0.001f);
    }
}

template <typename T>
double Checksum(const T* dst, std::size_t count)
{
    double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        sum += static_cast<double>(dst[i]) * static_cast<double>((i % 13) + 1);
    }
    return sum;
}

template <typename T>
Row RunStdMemcpy(const Config& config, std::size_t count, const T* src)
{
    const std::size_t sizeBytes = count * sizeof(T);
    const std::size_t bufferSlots = ComputeBufferSlots(config, sizeBytes);
    AlignedBuffer<T> srcBuffers(count * bufferSlots,
                                audioeffectstore::internal::kAlgoMemcpyRequiredAlignment);
    AlignedBuffer<T> dstBuffers(count * bufferSlots,
                                audioeffectstore::internal::kAlgoMemcpyRequiredAlignment);

    for (std::size_t slot = 0; slot < bufferSlots; ++slot) {
        std::memcpy(srcBuffers.data() + slot * count, src, sizeBytes);
    }

    for (std::size_t i = 0; i < config.warmupIterations; ++i) {
        const std::size_t slot = i % bufferSlots;
        std::memcpy(dstBuffers.data() + slot * count, srcBuffers.data() + slot * count,
                    sizeBytes);
    }

    std::vector<double> elapsedMsSamples;
    std::vector<double> gbpsSamples;
    elapsedMsSamples.reserve(config.samples);
    gbpsSamples.reserve(config.samples);

    for (std::size_t s = 0; s < config.samples; ++s) {
        const auto begin = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < config.iterations; ++i) {
            const std::size_t slot = i % bufferSlots;
            std::memcpy(dstBuffers.data() + slot * count, srcBuffers.data() + slot * count,
                        sizeBytes);
        }
        const auto end = std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsed = end - begin;
        const double seconds = elapsed.count() / 1000.0;
        const double totalBytes = static_cast<double>(sizeBytes) * config.iterations;
        elapsedMsSamples.push_back(elapsed.count());
        gbpsSamples.push_back(totalBytes / seconds / 1e9);
    }

    const Stats elapsedStats = ComputeStats(elapsedMsSamples);
    const Stats gbpsStats = ComputeStats(gbpsSamples);

    Row row;
    row.type = "";
    row.method = "std_memcpy";
    row.count = count;
    row.bytes = count * sizeof(T);
    row.alignmentBytes = audioeffectstore::internal::kAlgoMemcpyRequiredAlignment;
    row.warmup = config.warmupIterations;
    row.iterations = config.iterations;
    row.samples = config.samples;
    row.workingSetBytes = bufferSlots * sizeBytes * 2;
    row.bufferSlots = bufferSlots;
    row.elapsedMsMean = elapsedStats.mean;
    row.elapsedMsMin = elapsedStats.min;
    row.elapsedMsMax = elapsedStats.max;
    row.gbpsMean = gbpsStats.mean;
    row.gbpsMin = gbpsStats.min;
    row.gbpsMax = gbpsStats.max;
    row.checksum = Checksum(dstBuffers.data() + (bufferSlots - 1) * count, count);
    return row;
}

template <typename T>
Row RunAlgoMemcpy(const Config& config, std::size_t count, const T* src)
{
    const std::size_t sizeBytes = count * sizeof(T);
    const std::size_t bufferSlots = ComputeBufferSlots(config, sizeBytes);
    AlignedBuffer<T> srcBuffers(count * bufferSlots,
                                audioeffectstore::internal::kAlgoMemcpyRequiredAlignment);
    AlignedBuffer<T> dstBuffers(count * bufferSlots,
                                audioeffectstore::internal::kAlgoMemcpyRequiredAlignment);

    for (std::size_t slot = 0; slot < bufferSlots; ++slot) {
        std::memcpy(srcBuffers.data() + slot * count, src, sizeBytes);
    }

    for (std::size_t i = 0; i < config.warmupIterations; ++i) {
        const std::size_t slot = i % bufferSlots;
        audioeffectstore::internal::algomemcpy(dstBuffers.data() + slot * count,
                                               srcBuffers.data() + slot * count, sizeBytes);
    }

    std::vector<double> elapsedMsSamples;
    std::vector<double> gbpsSamples;
    elapsedMsSamples.reserve(config.samples);
    gbpsSamples.reserve(config.samples);

    for (std::size_t s = 0; s < config.samples; ++s) {
        const auto begin = std::chrono::steady_clock::now();
        for (std::size_t i = 0; i < config.iterations; ++i) {
            const std::size_t slot = i % bufferSlots;
            audioeffectstore::internal::algomemcpy(dstBuffers.data() + slot * count,
                                                   srcBuffers.data() + slot * count, sizeBytes);
        }
        const auto end = std::chrono::steady_clock::now();

        const std::chrono::duration<double, std::milli> elapsed = end - begin;
        const double seconds = elapsed.count() / 1000.0;
        const double totalBytes = static_cast<double>(count) * sizeof(T) * config.iterations;
        elapsedMsSamples.push_back(elapsed.count());
        gbpsSamples.push_back(totalBytes / seconds / 1e9);
    }

    const Stats elapsedStats = ComputeStats(elapsedMsSamples);
    const Stats gbpsStats = ComputeStats(gbpsSamples);

    Row row;
    row.type = "";
    row.method = "algomemcpy";
    row.count = count;
    row.bytes = count * sizeof(T);
    row.alignmentBytes = audioeffectstore::internal::kAlgoMemcpyRequiredAlignment;
    row.warmup = config.warmupIterations;
    row.iterations = config.iterations;
    row.samples = config.samples;
    row.workingSetBytes = bufferSlots * sizeBytes * 2;
    row.bufferSlots = bufferSlots;
    row.elapsedMsMean = elapsedStats.mean;
    row.elapsedMsMin = elapsedStats.min;
    row.elapsedMsMax = elapsedStats.max;
    row.gbpsMean = gbpsStats.mean;
    row.gbpsMin = gbpsStats.min;
    row.gbpsMax = gbpsStats.max;
    row.checksum = Checksum(dstBuffers.data() + (bufferSlots - 1) * count, count);
    return row;
}

template <typename T>
void RunType(const Config& config, const char* typeName, std::vector<Row>& rows)
{
    for (std::size_t count : config.counts) {
        AlignedBuffer<T> src(count, audioeffectstore::internal::kAlgoMemcpyRequiredAlignment);
        FillInput(src.data(), count);

        Row stdRow = RunStdMemcpy<T>(config, count, src.data());
        stdRow.type = typeName;
        rows.push_back(stdRow);

        Row algoRow = RunAlgoMemcpy<T>(config, count, src.data());
        algoRow.type = typeName;
        rows.push_back(algoRow);
    }
}

std::string BuildCsv(const std::vector<Row>& rows)
{
    std::map<std::tuple<std::string, std::size_t>, double> stdGbpsMeanByCase;
    for (const Row& row : rows) {
        if (row.method == "std_memcpy") {
            stdGbpsMeanByCase[{row.type, row.count}] = row.gbpsMean;
        }
    }

    std::ostringstream output;
    output << std::fixed << std::setprecision(3);
    output << "type,method,count,bytes,alignment_bytes,warmup,iterations,samples,working_set_bytes,buffer_slots,elapsed_ms_mean,elapsed_ms_min,elapsed_ms_max,gbps_mean,gbps_min,gbps_max,ratio_mean,checksum\n";
    for (const Row& row : rows) {
        double ratioMean = 1.0;
        const auto it = stdGbpsMeanByCase.find({row.type, row.count});
        if (it != stdGbpsMeanByCase.end() && it->second > 0.0) {
            ratioMean = row.gbpsMean / it->second;
        }

        output << row.type << ',' << row.method << ',' << row.count << ',' << row.bytes << ','
               << row.alignmentBytes << ',' << row.warmup << ',' << row.iterations << ','
               << row.samples << ',' << row.workingSetBytes << ',' << row.bufferSlots << ','
               << row.elapsedMsMean << ',' << row.elapsedMsMin << ','
               << row.elapsedMsMax << ',' << row.gbpsMean << ',' << row.gbpsMin << ','
               << row.gbpsMax << ',' << ratioMean << ',' << row.checksum << '\n';
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

}  // namespace

int main(int argc, char** argv)
{
    try {
        const Config config = ParseArgs(argc, argv);
        std::vector<Row> rows;
        rows.reserve(config.types.size() * config.counts.size() * 2);

        for (DataType type : config.types) {
            switch (type) {
                case DataType::kInt8:
                    RunType<std::int8_t>(config, "int8", rows);
                    break;
                case DataType::kInt16:
                    RunType<std::int16_t>(config, "int16", rows);
                    break;
                case DataType::kInt32:
                    RunType<std::int32_t>(config, "int32", rows);
                    break;
                case DataType::kFloat:
                    RunType<float>(config, "float", rows);
                    break;
            }
        }

        const std::string csv = BuildCsv(rows);
        std::cout << csv;
        if (!config.csvPath.empty()) {
            WriteCsvFile(config.csvPath, csv);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "memcpy_benchmark error: " << error.what() << '\n';
        return 1;
    }
}
