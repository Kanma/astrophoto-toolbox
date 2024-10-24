/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is essentially a reimplementation of parts of 'DeepSkyStacker',
 * which is released under a BSD 3-Clause license,
 * Copyright (c) 2006-2019, LucCoiffier
 * Copyright (c) 2018-2023,
 *      David C. Partridge, Tony Cook, Mat Draper,
 *      Simon C. Smith, Vitali Pelenjow, Michal Schulz, Martin Toeltsch
*/

#pragma once

#include <astrophoto-toolbox/algorithms/math.h>
#include <sstream>

namespace astrophototoolbox {
namespace stacking {


template<class BITMAP>
BitmapStacker<BITMAP>::~BitmapStacker()
{
    clear();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::setup(
    unsigned int nbBitmaps, const std::filesystem::path& tempFolder,
    unsigned long maxFileSize
)
{
    clear();

    this->nbBitmaps = nbBitmaps;
    this->nbAddedBitmaps = 0;
    this->width = 0;
    this->height = 0;
    this->maxFileSize = maxFileSize;
    this->tempFolder = tempFolder;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool BitmapStacker<BITMAP>::addBitmap(BITMAP* bitmap)
{
    if (partFiles.empty())
    {
        width = bitmap->width();
        height = bitmap->height();
        range = bitmap->range();
        initPartFiles();
    }

    const size_t rowSize = bitmap->bytesPerRow();

    for (const auto& part : partFiles)
    {
        FILE* f = std::fopen(part.filename.c_str(), "ab");
        if (!f)
            return false;

        for (int y = part.startRow; y <= part.endRow; ++y)
        {
            if (fwrite(bitmap->data(y), rowSize, 1, f) != 1)
                return false;
        }

        fclose(f);
    }

    ++nbAddedBitmaps;

    return true;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* BitmapStacker<BITMAP>::process() const
{
    bool cancelled = false;

    BITMAP* output = new BITMAP(width, height, range);

    const unsigned int nbRowElements = width * BITMAP::Channels;
    std::vector<typename BITMAP::type_t> buffer;

    for (const auto& part : partFiles)
    {
        const size_t bufferSize = nbRowElements * nbAddedBitmaps * (part.endRow - part.startRow + 1);
        if (bufferSize > buffer.size())
            buffer.resize(bufferSize);

        FILE* f = std::fopen(part.filename.c_str(), "rb");
        if (!f)
        {
            delete output;
            return nullptr;
        }

        size_t nb = fread(buffer.data(), BITMAP::ChannelSize, bufferSize, f);
        fclose(f);

        stack(part.startRow, part.endRow, nbRowElements, buffer.data(), output);

        if (cancelled)
        {
            delete output;
            return nullptr;
        }
    }

    return output;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::clear()
{
    for (const auto& part : partFiles)
        std::filesystem::remove(part.filename);

    partFiles.clear();
    nbBitmaps = 0;
    nbAddedBitmaps = 0;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::initPartFiles()
{
    // Make files of maximum 50MB
    const int lineSize = BITMAP::Channels * BITMAP::ChannelSize * width;

    const int nbLinesPerFile = maxFileSize / lineSize;
    int nbLines = nbLinesPerFile / nbBitmaps;
    int nbRemainingLines = nbLinesPerFile % nbBitmaps;

    if (nbLines == 0)
        nbLines = 1;

    partFiles.clear();

    std::filesystem::create_directories(tempFolder);

    unsigned int startRow = 0;
    unsigned int endRow = 0;

    unsigned int part = 0;
    while (endRow < height - 1)
    {
        endRow = startRow + nbLines - 1;
        if (nbRemainingLines != 0)
        {
            ++endRow;
            --nbRemainingLines;
        }

        endRow = std::min(endRow, height - 1);

        std::stringstream stream;
        stream << "part" << std::setfill('0') << std::setw(4) << part << ".dat";

        partFiles.push_back(part_file_t{ (tempFolder / stream.str()).c_str(), startRow, endRow });

        startRow = endRow + 1;
        ++part;
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::stack(
    unsigned int startRow, unsigned int endRow, unsigned int nbRowElements,
    typename BITMAP::type_t* buffer, BITMAP* output
) const
{
    const int nbRows = endRow - startRow + 1;

    std::vector<typename BITMAP::type_t*> srcRows(nbAddedBitmaps, nullptr);

    for (unsigned int row = startRow; row <= endRow; ++row)
    {
        for (size_t k = 0, offset = (row - startRow) * nbRowElements; k < nbAddedBitmaps; ++k)
        {
            srcRows[k] = buffer + offset;
            offset += nbRows * nbRowElements;
        }

        combine(row, srcRows, output);

        if (cancelled)
            return;
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::combine(
    unsigned int row, const std::vector<typename BITMAP::type_t*>& srcRows,
    BITMAP* output
) const requires(BITMAP::Channels == 3)
{
    const typename BITMAP::type_t maxValue = output->maxRangeValue();

    std::vector<typename BITMAP::type_t> redValues;
    std::vector<typename BITMAP::type_t> greenValues;
    std::vector<typename BITMAP::type_t> blueValues;

    redValues.reserve(srcRows.size());
    greenValues.reserve(srcRows.size());
    blueValues.reserve(srcRows.size());

    typename BITMAP::type_t* dest = output->data(row);

    sparse_histogram_t histogram;

    for (unsigned int i = 0; i < output->width(); ++i)
    {
        redValues.resize(0);
        greenValues.resize(0);
        blueValues.resize(0);

        for (const typename BITMAP::type_t* ptr : srcRows)
        {
            const typename BITMAP::type_t* p = ptr + i * 3;

            if (p[0])
                redValues.push_back(p[0]);

            if (p[1])
                greenValues.push_back(p[1]);

            if (p[2])
                blueValues.push_back(p[2]);
        }

        // Median method
        dest[0] = computeMedian(redValues, histogram, maxValue);
        dest[1] = computeMedian(greenValues, histogram, maxValue);
        dest[2] = computeMedian(blueValues, histogram, maxValue);

        dest += 3;
    }
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void BitmapStacker<BITMAP>::combine(
    unsigned int row, const std::vector<typename BITMAP::type_t*>& srcRows,
    BITMAP* output
) const requires(BITMAP::Channels == 1)
{
    const typename BITMAP::type_t maxValue = output->maxRangeValue();

    std::vector<typename BITMAP::type_t> values;

    values.reserve(srcRows.size());

    typename BITMAP::type_t* dest = output->data(row);

    sparse_histogram_t histogram;

    for (unsigned int i = 0; i < output->width(); ++i)
    {
        values.resize(0);

        for (const typename BITMAP::type_t* ptr : srcRows)
        {
            const typename BITMAP::type_t* p = ptr + i;

            if (*p)
                values.push_back(*p);
        }

        // Median method
        *dest = computeMedian(values, histogram, maxValue);

        ++dest;
    }
}

}
}
