#pragma once

#include "../Types.hpp"

#include "ChunkHeader.hpp"
#include "StreamReader.hpp"

#include "UcfbChunk.hpp"
#include "LocalizationChunk.hpp"

namespace SWBF2
{
    using ChunkProcessingFunction = std::function<void(StreamReader &streamReader)>;

    class ChunkProcessor {
    public:
        static inline const std::unordered_map<uint32_t, ChunkProcessingFunction> m_functions
        {
            { "ucfb"_m, UcfbChunk::ProcessChunk },
        };

        static void ProcessChunk(StreamReader &streamReader, StreamReader &parentReader);
    };

}
