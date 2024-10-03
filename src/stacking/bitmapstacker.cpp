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

#include <astrophoto-toolbox/stacking/bitmapstacker.h>
#include <astrophoto-toolbox/algorithms/math.h>
#include <sstream>

using namespace astrophototoolbox;
using namespace stacking;


BitmapStacker::~BitmapStacker()
{
    clear();
}

//-----------------------------------------------------------------------------

void BitmapStacker::setup(
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

bool BitmapStacker::addBitmap(DoubleColorBitmap* bitmap)
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

DoubleColorBitmap* BitmapStacker::process() const
{
    DoubleColorBitmap* output = new DoubleColorBitmap(width, height, range);

    const unsigned int nbRowElements = width * 3;
    std::vector<double_t> buffer;

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

        fread(buffer.data(), sizeof(double_t), bufferSize, f);
        fclose(f);

        stack(part.startRow, part.endRow, nbRowElements, buffer.data(), output);
    }

    return output;
}

//-----------------------------------------------------------------------------

void BitmapStacker::clear()
{
    for (const auto& part : partFiles)
        std::filesystem::remove(part.filename);

    partFiles.clear();
    nbAddedBitmaps = 0;
}

//-----------------------------------------------------------------------------

void BitmapStacker::initPartFiles()
{
    // Make files of maximum 50MB
    const int lineSize = 3 * sizeof(double) * width;

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

void BitmapStacker::stack(
    unsigned int startRow, unsigned int endRow, unsigned int nbRowElements, double* buffer,
    DoubleColorBitmap* output
) const
{
    const int nbRows = endRow - startRow + 1;

    std::vector<double*> srcRows(nbBitmaps, nullptr);

    for (unsigned int row = startRow; row <= endRow; ++row)
    {
        for (size_t k = 0, offset = (row - startRow) * nbRowElements; k < nbBitmaps; ++k)
        {
            srcRows[k] = buffer + offset;
            offset += nbRows * nbRowElements;
        }

        combine(row, srcRows, output);
    }
}

//-----------------------------------------------------------------------------

void BitmapStacker::combine(
    unsigned int row, const std::vector<double*>& srcRows, DoubleColorBitmap* output
) const
{
    const double maxValue = (range == RANGE_BYTE ? 255.0
                             : (range == RANGE_USHORT ? 65535.0
                                : (range == RANGE_UINT ? double(2^32-1)
                                : 1.0
                               )
                             )
                            );

    std::vector<double_t> redValues;
    std::vector<double_t> greenValues;
    std::vector<double_t> blueValues;

    redValues.reserve(srcRows.size());
    greenValues.reserve(srcRows.size());
    blueValues.reserve(srcRows.size());

    double* dest = output->data(row);

    for (unsigned int i = 0; i < width; ++i)
    {
        redValues.resize(0);
        greenValues.resize(0);
        blueValues.resize(0);

        for (const double* ptr : srcRows)
        {
            const double* p = ptr + i * 3;
            redValues.push_back(p[0]);
            greenValues.push_back(p[1]);
            blueValues.push_back(p[2]);
        }

        // Median method
        dest[0] = computeMedian(redValues, maxValue);
        dest[1] = computeMedian(greenValues, maxValue);
        dest[2] = computeMedian(blueValues, maxValue);

        dest += 3;
    }
}
