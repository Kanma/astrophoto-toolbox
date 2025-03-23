/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#define _USE_MATH_DEFINES

#include <astrophoto-toolbox/data/coordinatessystem.h>
#include <cmath>
#include <string.h>

#define CANVAS_ITY_IMPLEMENTATION
#include <canvas_ity.hpp>

#include "font.inc"

extern "C" {
    #include <astrometry/sip-utils.h>
}

using namespace astrophototoolbox;
using namespace canvas_ity;


std::vector<unsigned char> FONT;


/********************************** HELPER FUNCTIONS ************************************/

//----------------------------------------------------------------------------------------
/// @brief  Simple Base64 decoder
///
/// Used to decode embedded font files
//----------------------------------------------------------------------------------------
void base64_decode(char const* input, std::vector<unsigned char>& output)
{
    int index = 0;
    int data = 0;
    int held = 0;
    while ( int symbol = input[ index++ ] )
    {
        if ( symbol == '=' )
            break;
        int value = ( 'A' <= symbol && symbol <= 'Z' ? symbol - 'A' :
                      'a' <= symbol && symbol <= 'z' ? symbol - 'a' + 26 :
                      '0' <= symbol && symbol <= '9' ? symbol - '0' + 52 :
                      symbol == '+' ? 62 :
                      symbol == '/' ? 63 :
                      0 );
        data = data << 6 | value;
        held += 6;
        if ( held >= 8 )
        {
            held -= 8;
            output.push_back( static_cast< unsigned char >( ( data >> held ) & 0xff ) );
            data &= ( 1 << held ) - 1;
        }
    }
}


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

CoordinatesSystem::CoordinatesSystem()
{
}

//-----------------------------------------------------------------------------

CoordinatesSystem::CoordinatesSystem(
    const size2d_t& imageSize, const Coordinates& coords, double pixelSize,
    double raAngle, const tan_t& wcstan
)
: imageSize(imageSize), coords(coords), pixelSize(pixelSize), raAngle(raAngle)
{
    center.x = double(imageSize.width) / 2.0;
    center.y = double(imageSize.height) / 2.0;

    memcpy(&this->wcstan, &wcstan, sizeof(tan_t));

    point_t p = convert(Coordinates(coords.getRA(), 90.0));
    if (!p.isNull())
    {
        double distance = p.distance(center);
        dx = (p.x - center.x) / distance;
        dy = (p.y - center.y) / distance;
    }
    else
    {
        p = convert(Coordinates(coords.getRA(), -90.0));
        if (!p.isNull())
        {
            double distance = p.distance(center);
            dx = -(p.x - center.x) / distance;
            dy = -(p.y - center.y) / distance;
        }
        else
        {
            dx = sin(raAngle * M_PI / 180.0);
            dy = cos(raAngle * M_PI / 180.0);
        }
    }
}


/************************************** METHODS ****************************************/

point_t CoordinatesSystem::convert(const Coordinates& coords) const
{
    point_t position;

    if (!tan_radec2pixelxy(&wcstan, coords.getRA(), coords.getDEC(), &position.x, &position.y))
        return point_t();

    return position;
}

//-----------------------------------------------------------------------------

Coordinates CoordinatesSystem::convert(const point_t& position) const
{
    double ra, dec;

    tan_pixelxy2radec(&wcstan, position.x, position.y, &ra, &dec);

    return Coordinates(ra, dec);
}

//-----------------------------------------------------------------------------

bool CoordinatesSystem::isInsideImage(const Coordinates& coords) const
{
    point_t point = convert(coords);
    if (point.isNull())
        return false;

    return isInsideImage(point);
}

//-----------------------------------------------------------------------------

bool CoordinatesSystem::isInsideImage(const point_t& point) const
{
    return ((point.x >= 1) && (point.x <= imageSize.width) &&
            (point.y >= 1) && (point.y <= imageSize.height));
}

//-----------------------------------------------------------------------------

uint8_t* CoordinatesSystem::drawAxes(
    unsigned int width, unsigned int height, unsigned int length
) const
{
    if (length == 0)
        length = std::min(width, height) / 5;

    double centerX = double(width) * 0.5;
    double centerY = double(height) * 0.5;

    double ra_x = centerX + length * dy;
    double ra_y = centerY - length * dx;

    double dec_x = centerX + length * dx;
    double dec_y = centerY + length * dy;

    canvas dest(width, height);

    dest.set_line_width(2.0f);

    dest.set_color(brush_type::stroke_style, 1.0f, 0.0f, 0.0f, 1.0f);
    dest.begin_path();
    dest.move_to(centerX, centerY);
    dest.line_to(ra_x, ra_y);
    dest.stroke();

    dest.set_color(brush_type::stroke_style, 0.0f, 1.0f, 0.0f, 1.0f);
    dest.begin_path();
    dest.move_to(centerX, centerY);
    dest.line_to(dec_x, dec_y);
    dest.stroke();

    uint8_t* layer = new uint8_t[height * width * 4];
    dest.get_image_data(layer, width, height, width * 4 * sizeof(uint8_t), 0, 0);
    return layer;
}

//-----------------------------------------------------------------------------

uint8_t* CoordinatesSystem::drawGrid(unsigned int width, unsigned int height, float fontSize) const
{
    double ramin, ramax, decmin, decmax;

    sip_t* sip = sip_create();
    sip_wrap_tan(&wcstan, sip);
    sip_get_radec_bounds(sip, 50, &ramin, &ramax, &decmin, &decmax);
    sip_free(sip);

    double ramin2, ramax2, rastep;
    computeGridParameters(ramin, ramax, ramin2, ramax2, rastep);

    double decmin2, decmax2, decstep;
    computeGridParameters(decmin, decmax, decmin2, decmax2, decstep);

    double scaleX = double(width) / imageSize.width;
    double scaleY = double(height) / imageSize.height;

    canvas dest(width, height);

    if (fontSize <= 0.0f)
        fontSize = float(std::min(width, height)) / 60.0f;

    if (FONT.empty())
        base64_decode(BASE64_FONT, FONT);

    dest.set_font(FONT.data(), FONT.size(), fontSize);

    dest.set_line_width(2.0f);
    dest.set_color(brush_type::stroke_style, 0.0f, 0.2f, 0.0f, 1.0f);
    dest.set_color(brush_type::fill_style, 1.0f, 0.0f, 0.0f, 1.0f);

    for (double ra = ramin2; ra <= ramax2; ra += rastep)
    {
        dest.begin_path();

        for (double dec = decmin; dec <= decmax; dec += decstep / 10.0)
        {
            point_t p = convert(Coordinates(ra, dec));
            dest.line_to(p.x * scaleX, p.y * scaleY);
        }

        dest.stroke();

        double dec;
        if (findRALabelLocation(ra, decmin, decmax, dec))
        {
            Coordinates coords(ra, dec);
            point_t point = convert(coords);
            std::string label = coords.getRAasHMS();

            point.x *= scaleX;
            point.y *= scaleY;

            float textWidth = dest.measure_text(label.c_str());

            if (point.x > width - textWidth)
                point.x -= textWidth;

            if (point.y < fontSize)
                point.y += fontSize;
            else if (point.y > height - 5)
                point.y -= 5;

            dest.fill_text(label.c_str(), point.x, point.y);
        }
    }

    dest.set_color(brush_type::stroke_style, 0.3f, 0.0f, 0.0f, 1.0f);
    dest.set_color(brush_type::fill_style, 0.0f, 1.0f, 0.0f, 1.0f);

    for (double dec = decmin2; dec <= decmax2; dec += decstep)
    {
        dest.begin_path();

        for (double ra = ramin; ra <= ramax; ra += rastep / 10.0)
        {
            point_t p = convert(Coordinates(ra, dec));
            dest.line_to(p.x * scaleX, p.y * scaleY);
        }

        dest.stroke();

        double ra;
        if (findDECLabelLocation(dec, ramin, ramax, ra))
        {
            Coordinates coords(ra, dec);
            point_t point = convert(coords);
            std::string label = coords.getDECasDMS();

            point.x *= scaleX;
            point.y *= scaleY;

            float textWidth = dest.measure_text(label.c_str());

            if (point.x > width - textWidth)
                point.x -= textWidth;

            if (point.y < fontSize)
                point.y += fontSize;
            else if (point.y > height - 5)
                point.y -= 5;

            dest.fill_text(label.c_str(), point.x, point.y);
        }
    }

    uint8_t* layer = new uint8_t[height * width * 4];
    dest.get_image_data(layer, width, height, width * 4 * sizeof(uint8_t), 0, 0);
    return layer;
}

//-----------------------------------------------------------------------------

bool CoordinatesSystem::isNull() const
{
    return coords.isNull();
}


/********************************* INTERNAL METHODS ************************************/

void CoordinatesSystem::computeGridParameters(
    double inMin, double inMax, double& outMin, double& outMax, double& step
) const
{
    step = (inMax - inMin) / 10.0;

    if (step < 1.0)
    {
        double multiplier = 10.0;
        while (step * multiplier < 1.0)
            multiplier *= 10.0;
        step = int(ceil(step * multiplier)) / multiplier;
    }
    else if (step < 10.0)
    {
        step = int(ceil(step));
    }
    else
    {
        double multiplier = 0.1;
        while (step * multiplier > 10.0)
            multiplier *= 0.01;
        step = int(ceil(step * multiplier)) / multiplier;
    }

    outMin = step * floor(inMin / step);
    outMax = step * ceil(inMax / step);
}

//-----------------------------------------------------------------------------

bool CoordinatesSystem::findRALabelLocation(double ra, double decmin, double decmax, double& dec) const
{
    double out;
    double in = coords.getDEC();
    int dirs[2] = {1, -1};

    // Find coordinates outside the image
    bool found = false;

    for (int j = 0; j < 2; ++j)
    {
        int dir = dirs[j];

        for (int i = 1;; ++i)
        {
            // Take 10-deg steps
            out = in + i * dir * 10.0;
            if ((out >= 100.0) || (out <= -100))
                break;

            out = std::min(90.0, std::max(-90.0, out));

            if (!isInsideImage(Coordinates(ra, out)))
            {
                found = true;
                break;
            }
        }

        if (found)
            break;
    }
    if (!found)
        return false;

    // We've got a declination inside the image (in) and a declination outside the image (out).
    // Now find the boundary.
    int i = 0;
    int N = 10;
    while (!isInsideImage(Coordinates(ra, in)))
    {
        if (i == N)
            break;

        in = decmin + double(i) / double(N-1) * (decmax - decmin);
        ++i;
    }

    if (!isInsideImage(Coordinates(ra, in)))
        return false;

    while (fabs(out - in) > 1e-6)
    {
        double half = (out + in) / 2.0;

        if (isInsideImage(Coordinates(ra, half)))
            in = half;
        else
            out = half;
    }

    dec = in;

    return true;
}

//-----------------------------------------------------------------------------

bool CoordinatesSystem::findDECLabelLocation(double dec, double ramin, double ramax, double& ra) const
{
    double out;
    double in = coords.getRA();
    int dirs[2] = {1, -1};

    // Find coordinates outside the image
    bool found = false;

    for (int j = 0; j < 2; ++j)
    {
        int dir = dirs[j];

        for (int i = 1;; ++i)
        {
            // Take 10-deg steps
            out = in + i * dir * 10.0;
            if ((out >= 370.0) || (out <= -10))
                break;

            out = std::min(360.0, std::max(0.0, out));

            if (!isInsideImage(Coordinates(out, dec)))
            {
                found = true;
                break;
            }
        }

        if (found)
            break;
    }
    if (!found)
        return false;

    // We've got a right ascension inside the image (in) and one outside the image (out).
    // Now find the boundary.
    int i = 0;
    int N = 10;
    while (!isInsideImage(Coordinates(in, dec)))
    {
        if (i == N)
            break;

        in = ramin + double(i) / double(N-1) * (ramax - ramin);
        ++i;
    }

    if (!isInsideImage(Coordinates(in, dec)))
        return false;

    while (fabs(out - in) > 1e-6)
    {
        double half = (out + in) / 2.0;

        if (isInsideImage(Coordinates(half, dec)))
            in = half;
        else
            out = half;
    }

    ra = in;

    return true;
}
