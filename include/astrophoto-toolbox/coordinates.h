#pragma once

#include <string>
#include <regex>
#include <cmath>


namespace astrophototoolbox {

    //------------------------------------------------------------------------------------
    /// @brief  Represents astronomical coordinates (right ascension and declination)
    ///
    /// The right ascension part can be set/retrieved either as:
    ///   - an angle in degrees
    ///   - HH:MM:SS.sss (1 hour = 15 degrees)
    ///   - DD°MM'SS.sss"
    /// 
    /// The declination part can be set/retrieved either as:
    ///   - an angle in degrees
    ///   - DD°MM'SS.sss"
    ///   - DD:MM:SS.sss (for convenience, but cannot be retrieved in that form)
    //------------------------------------------------------------------------------------
    class Coordinates
    {
        //_____ Internal types __________
    public:
        //--------------------------------------------------------------------------------
        /// @brief  Represents the distance between two astronomical coordinates
        //--------------------------------------------------------------------------------
        struct distance_t
        {
            distance_t(double ra, double dec)
            : ra(ra), dec(dec)
            {}

            double ra;
            double dec;
        };


        //_____ Construction / Destruction __________
    public:
        Coordinates();
        Coordinates(double ra, double dec);
        Coordinates(const std::string& ra, const std::string& dec);


        //_____ Methods __________
    public:
        void set(double ra, double dec);
        void set(const std::string& ra, const std::string& dec);

        bool isNull() const;

        std::string getRAasHMS() const;
        std::string getRAasDMS() const;
        double getRA() const;

        std::string getDECasDMS() const;
        double getDEC() const;

        std::string getRADECasHMSDMS() const;
        std::string getRADECasDMSDMS() const;
        std::string getRADEC() const;


        //_____ Operations __________
    public:
        distance_t operator-(const Coordinates& other) const;
        Coordinates operator+(const distance_t& movement) const;
        Coordinates& operator+=(const distance_t& movement);


        //_____ Internal methods __________
    private:
        double parseDMS(const std::smatch& match);
        double parseHMS(const std::smatch& match);
        std::string getAngleAsDMS(double angle) const;


        //_____ Attributes __________
    private:
        double ra = INFINITY;
        double dec = INFINITY;
    };

}
