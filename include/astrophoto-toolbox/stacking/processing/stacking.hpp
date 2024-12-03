/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <astrophoto-toolbox/stacking/processing/utils.h>

namespace astrophototoolbox {
namespace stacking {
namespace processing {


template<class BITMAP>
void FramesStacker<BITMAP>::setup(
    unsigned int nbExpectedFrames, const std::filesystem::path& tempFolder,
    unsigned long maxFileSize
)
{
    stacker.setup(nbExpectedFrames, tempFolder, maxFileSize);

    outputRect = rect_t(
        0, 0, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()
    );
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool FramesStacker<BITMAP>::addFrame(const std::string& lightFrame)
{
    Transformation transformation;

    BITMAP* bitmap = loadProcessedBitmap<BITMAP>(lightFrame, nullptr, nullptr, &transformation);
    if (!bitmap)
        return false;

    return addFrame(std::make_shared<BITMAP>(bitmap), transformation);
}

//-----------------------------------------------------------------------------

template<class BITMAP>
bool FramesStacker<BITMAP>::addFrame(
    const std::shared_ptr<BITMAP>& lightFrame, const Transformation& transformation
)
{
    rect_t transformedRect = transformation.transform(
        rect_t{ 0, 0, (int) lightFrame->width(), (int) lightFrame->height() }
    );

    if ((transformedRect.left < transformedRect.right) &&
        (transformedRect.top < transformedRect.bottom))
    {
        auto* transformed = transformation.transform(lightFrame.get());
        
        if (!stacker.addBitmap(transformed))
        {
            delete transformed;
            return false;
        }

        outputRect = outputRect.intersection(transformedRect);

        delete transformed;

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
BITMAP* FramesStacker<BITMAP>::process(const std::string& destination)
{
    BITMAP* stacked = stacker.process();
    if (!stacked)
        return nullptr;

    BITMAP* result = new BITMAP(outputRect.width(), outputRect.height());
    for (unsigned int y = 0; y < result->height(); ++y)
    {
        typename BITMAP::type_t* src = stacked->data(outputRect.left, outputRect.top + y);
        typename BITMAP::type_t* dest = result->data(y);

        memcpy(dest, src, result->bytesPerRow());
    }

    delete stacked;

    if (!destination.empty() && !io::save(destination, result, true))
    {
        delete result;
        return nullptr;
    }

    return result;
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void FramesStacker<BITMAP>::cancel()
{
    stacker.cancel();
}

//-----------------------------------------------------------------------------

template<class BITMAP>
void FramesStacker<BITMAP>::clear()
{
    stacker.clear();

    outputRect = rect_t(
        0, 0, std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()
    );
}

}
}
}
