/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <astrophoto-toolbox/data/coordinates.h>
#include <sstream>
#include <iomanip>

using namespace astrophototoolbox;


/**************************** CONSTRUCTION / DESTRUCTION *******************************/

Coordinates::Coordinates()
{
}

//-----------------------------------------------------------------------------

Coordinates::Coordinates(double ra, double dec)
{
    set(ra, dec);
}

//-----------------------------------------------------------------------------

Coordinates::Coordinates(const std::string& ra, const std::string& dec)
{
    set(ra, dec);
}


/************************************** METHODS ****************************************/

void Coordinates::set(double ra, double dec)
{
    while (ra < 0.0)
        ra += 360.0;

    while (ra >= 360.0)
        ra -= 360.0;

    this->ra = ra;
    this->dec = fmax(-90.0, fmin(90.0, dec));;
}

//-----------------------------------------------------------------------------

void Coordinates::set(const std::string& ra, const std::string& dec)
{
    static const std::regex reDeg("(-*\\d+)°(\\d+)'(\\d+[.\\d+]?)\"");
    static const std::regex reDegFull("(-*\\d+)°(\\d+)'(\\d+)\\.(\\d+)\"");
    static const std::regex reHours("(-*\\d+):(\\d+):(\\d+)");
    static const std::regex reHoursFull("(-*\\d+):(\\d+):(\\d+)\\.(\\d+)");

    std::smatch match;
    bool found = std::regex_match(ra, match, reDegFull);

    if (!found)
        found = std::regex_match(ra, match, reDeg);

    if (found)
    {
        this->ra = parseDMS(match);
    }
    else
    {
        found = std::regex_match(ra, match, reHoursFull);

        if (!found)
            found = std::regex_match(ra, match, reHours);

        if (found)
        {
            this->ra = parseHMS(match);
        }
        else
        {
            try
            {
                this->ra = std::stod(ra);
            }
            catch (std::invalid_argument)
            {
                this->ra = INFINITY;
            }
        }
    }

    if (!std::isinf(this->ra))
    {
        while (this->ra < 0.0)
            this->ra += 360.0;

        while (this->ra >= 360.0)
            this->ra -= 360.0;
    }

    found = std::regex_match(dec, match, reDegFull);

    if (!found)
        found = std::regex_match(dec, match, reDeg);

    if (!found)
        found = std::regex_match(dec, match, reHoursFull);

    if (!found)
        found = std::regex_match(dec, match, reHours);

    if (found)
    {
        this->dec = parseDMS(match);
    }
    else
    {
        try
        {
            this->dec = std::stod(dec);
        }
        catch (std::invalid_argument)
        {
            this->dec = INFINITY;
        }
    }

    if (!std::isinf(this->dec))
        this->dec = fmax(-90.0, fmin(90.0, this->dec));
}

//-----------------------------------------------------------------------------

bool Coordinates::isNull() const
{
    return std::isinf(ra) || std::isinf(dec) ;
}

//-----------------------------------------------------------------------------

std::string Coordinates::getRAasHMS() const
{
    int hours = int(floor(ra / 15.0));
    double remainder = (ra / 15.0 - double(hours)) * 60.0;

    int minutes = int(floor(remainder));
    double remainder2 = (remainder - double(minutes)) * 60.0;

    int seconds = int(floor(remainder2));
    double remainder3 = (remainder2 - double(seconds)) * 1000.0;

    int milliseconds = int(round(remainder3));
    if (milliseconds == 1000)
    {
        milliseconds = 0;
        ++seconds;
        if (seconds == 60)
        {
            seconds = 0;
            ++minutes;
            if (minutes == 60)
            {
                minutes = 0;
                ++hours;
            }
        }
    }

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << hours << ":"
           << std::setw(2) << minutes << ":" << std::setw(2) << seconds;

    if (milliseconds != 0)
        stream << "." << std::setw(3) << milliseconds; 

    return stream.str();
}

//-----------------------------------------------------------------------------

std::string Coordinates::getRAasDMS() const
{
    return getAngleAsDMS(ra);
}

//-----------------------------------------------------------------------------

double Coordinates::getRA() const
{
    return ra;
}

//-----------------------------------------------------------------------------

std::string Coordinates::getDECasDMS() const
{
    return getAngleAsDMS(dec);
}

//-----------------------------------------------------------------------------

double Coordinates::getDEC() const
{
    return dec;
}

//-----------------------------------------------------------------------------

std::string Coordinates::getRADECasHMSDMS() const
{
    return getRAasHMS() + ", " + getDECasDMS();
}

//-----------------------------------------------------------------------------

std::string Coordinates::getRADECasDMSDMS() const
{
    return getRAasDMS() + ", " + getDECasDMS();
}

//-----------------------------------------------------------------------------

std::string Coordinates::getRADEC() const
{
    std::string str_ra = std::to_string(ra);
    std::string str_dec = std::to_string(dec);

    size_t len_ra = str_ra.size() - 1;
    size_t len_dec = str_dec.size() - 1;

    while (str_ra[len_ra] == '0') --len_ra;
    while (str_dec[len_dec] == '0') --len_dec;

    if (str_ra[len_ra] == '.') ++len_ra;
    if (str_dec[len_dec] == '.') ++len_dec;

    return str_ra.substr(0, len_ra+1) + ", " + str_dec.substr(0, len_dec+1);
}


/************************************ OPERATIONS ***************************************/

Coordinates::distance_t Coordinates::operator-(const Coordinates& other) const
{
    return distance_t(ra - other.ra, dec - other.dec);
}

//-----------------------------------------------------------------------------

Coordinates Coordinates::operator+(const distance_t& movement) const
{
    return Coordinates(ra + movement.ra, dec + movement.dec);
}

//-----------------------------------------------------------------------------

Coordinates& Coordinates::operator+=(const distance_t& movement)
{
    ra += movement.ra;
    dec += movement.dec;
    return *this;
}


/********************************* INTERNAL METHODS ************************************/

double Coordinates::parseDMS(const std::smatch& match)
{
    int deg = std::stoi(match[1].str());
    int minutes = std::stoi(match[2].str());
    double seconds = 0.0;

    if (match.size() == 5)
    {
        std::string val = match[3].str() + "." + match[4].str();
        seconds = std::stod(val);
    }
    else
    {
        seconds = std::stod(match[3]);
    }

    bool negative = deg < 0;
    deg = std::abs(deg);

    double angle = double(deg) + double(minutes) / 60.0 + seconds / 3600.0;

    if (negative)
        angle = -angle;

    return angle;
}

//-----------------------------------------------------------------------------

double Coordinates::parseHMS(const std::smatch& match)
{
    return parseDMS(match) * 15.0;
}

//-----------------------------------------------------------------------------

std::string Coordinates::getAngleAsDMS(double angle) const
{
    double absoluteAngle = std::abs(angle);

    int deg = int(std::floor(absoluteAngle));
    double remainder = (absoluteAngle - double(deg)) * 60.0;

    int minutes = int(std::floor(remainder));
    double remainder2 = (remainder - double(minutes)) * 60.0;

    int seconds = int(std::floor(remainder2));
    double remainder3 = (remainder2 - double(seconds)) * 1000.0;

    int milliseconds = int(std::round(remainder3));
    if (milliseconds == 1000)
    {
        milliseconds = 0;
        ++seconds;
        if (seconds == 60)
        {
            seconds = 0;
            ++minutes;
            if (minutes == 60)
            {
                minutes = 0;
                ++deg;
            }
        }
    }

    if (angle < 0.0)
        deg = -deg;

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << deg << "°"
           << std::setw(2) << minutes << "'" << std::setw(2) << seconds;

    if (milliseconds != 0)
        stream << "." << std::setw(3) << milliseconds; 

    stream << '"';

    return stream.str();
}
