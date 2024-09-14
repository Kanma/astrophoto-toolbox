#include <catch.hpp>
#include <astrophoto-toolbox/coordinates.h>

using namespace astrophototoolbox;


TEST_CASE("Coordinates created from angles", "[Coordinates]")
{
    SECTION("with milliseconds")
    {
        Coordinates coords(208.4582, 20.36791);

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("without milliseconds")
    {
        Coordinates coords(208.0, 20.0);

        REQUIRE(coords.getRA() == Approx(208.0));
        REQUIRE(coords.getDEC() == Approx(20.0));

        REQUIRE(coords.getRAasHMS() == "13:52:00");
        REQUIRE(coords.getRAasDMS() == "208°00'00\"");

        REQUIRE(coords.getDECasDMS() == "20°00'00\"");

        REQUIRE(coords.getRADEC() == "208.0, 20.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("small digit values")
    {
        Coordinates coords(30.7667, 5.035001);

        REQUIRE(coords.getRA() == Approx(30.7667));
        REQUIRE(coords.getDEC() == Approx(5.035001));

        REQUIRE(coords.getRAasHMS() == "02:03:04.008");
        REQUIRE(coords.getRAasDMS() == "30°46'00.120\"");

        REQUIRE(coords.getDECasDMS() == "05°02'06.004\"");

        REQUIRE(coords.getRADEC() == "30.7667, 5.035001");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("small angle values")
    {
        Coordinates coords(30.76, 5.03);

        REQUIRE(coords.getRA() == Approx(30.76));
        REQUIRE(coords.getDEC() == Approx(5.03));

        REQUIRE(coords.getRAasHMS() == "02:03:02.400");
        REQUIRE(coords.getRAasDMS() == "30°45'36\"");

        REQUIRE(coords.getDECasDMS() == "05°01'48\"");

        REQUIRE(coords.getRADEC() == "30.76, 5.03");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("negative angles")
    {
        Coordinates coords(-200.0, -20.36791);

        REQUIRE(coords.getRA() == Approx(160.0));
        REQUIRE(coords.getDEC() == Approx(-20.36791));

        REQUIRE(coords.getRAasHMS() == "10:40:00");
        REQUIRE(coords.getRAasDMS() == "160°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "160.0, -20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big positive angles")
    {
        Coordinates coords(1000.0, 100.0);

        REQUIRE(coords.getRA() == Approx(280.0));
        REQUIRE(coords.getDEC() == Approx(90.0));

        REQUIRE(coords.getRAasHMS() == "18:40:00");
        REQUIRE(coords.getRAasDMS() == "280°00'00\"");

        REQUIRE(coords.getDECasDMS() == "90°00'00\"");

        REQUIRE(coords.getRADEC() == "280.0, 90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big negative angles")
    {
        Coordinates coords(-1000.0, -100.0);

        REQUIRE(coords.getRA() == Approx(80.0));
        REQUIRE(coords.getDEC() == Approx(-90.0));

        REQUIRE(coords.getRAasHMS() == "05:20:00");
        REQUIRE(coords.getRAasDMS() == "80°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-90°00'00\"");

        REQUIRE(coords.getRADEC() == "80.0, -90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("using set() method")
    {
        Coordinates coords;
        REQUIRE(coords.isNull());

        coords.set(208.4582, 20.36791);

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }
}


TEST_CASE("Coordinates created from (HMS, DMS)", "[Coordinates]")
{
    SECTION("with milliseconds")
    {
        Coordinates coords("13:53:49.968", "20:22:04.476");

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("without milliseconds")
    {
        Coordinates coords("13:52:00", "20:00:00");

        REQUIRE(coords.getRA() == Approx(208.0));
        REQUIRE(coords.getDEC() == Approx(20.0));

        REQUIRE(coords.getRAasHMS() == "13:52:00");
        REQUIRE(coords.getRAasDMS() == "208°00'00\"");

        REQUIRE(coords.getDECasDMS() == "20°00'00\"");

        REQUIRE(coords.getRADEC() == "208.0, 20.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("negative angles")
    {
        Coordinates coords("-13:20:00", "-20:22:04.476");

        REQUIRE(coords.getRA() == Approx(160.0));
        REQUIRE(coords.getDEC() == Approx(-20.36791));

        REQUIRE(coords.getRAasHMS() == "10:40:00");
        REQUIRE(coords.getRAasDMS() == "160°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "160.0, -20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big positive angles")
    {
        Coordinates coords("66:40:00", "100:00:00");

        REQUIRE(coords.getRA() == Approx(280.0));
        REQUIRE(coords.getDEC() == Approx(90.0));

        REQUIRE(coords.getRAasHMS() == "18:40:00");
        REQUIRE(coords.getRAasDMS() == "280°00'00\"");

        REQUIRE(coords.getDECasDMS() == "90°00'00\"");

        REQUIRE(coords.getRADEC() == "280.0, 90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big negative angles")
    {
        Coordinates coords("-66:40:00", "-100:00:00");

        REQUIRE(coords.getRA() == Approx(80.0));
        REQUIRE(coords.getDEC() == Approx(-90.0));

        REQUIRE(coords.getRAasHMS() == "05:20:00");
        REQUIRE(coords.getRAasDMS() == "80°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-90°00'00\"");

        REQUIRE(coords.getRADEC() == "80.0, -90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("using set() method")
     {
        Coordinates coords;
        REQUIRE(coords.isNull());
        
        coords.set("13:53:49.968", "20:22:04.476");

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }
}


TEST_CASE("Coordinates created from (DMS, DMS)", "[Coordinates]")
{
    SECTION("with milliseconds")
    {
        Coordinates coords("208°27'29.520\"", "20°22'04.476\"");

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("without milliseconds")
    {
        Coordinates coords("208°00'00\"", "20°00'00\"");

        REQUIRE(coords.getRA() == Approx(208.0));
        REQUIRE(coords.getDEC() == Approx(20.0));

        REQUIRE(coords.getRAasHMS() == "13:52:00");
        REQUIRE(coords.getRAasDMS() == "208°00'00\"");

        REQUIRE(coords.getDECasDMS() == "20°00'00\"");

        REQUIRE(coords.getRADEC() == "208.0, 20.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("negative angles")
    {
        Coordinates coords("-200°00'00\"", "-20°22'04.476\"");

        REQUIRE(coords.getRA() == Approx(160.0));
        REQUIRE(coords.getDEC() == Approx(-20.36791));

        REQUIRE(coords.getRAasHMS() == "10:40:00");
        REQUIRE(coords.getRAasDMS() == "160°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "160.0, -20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big positive angles")
    {
        Coordinates coords("1000°40':00\"", "100°00'00\"");

        REQUIRE(coords.getRA() == Approx(280.0));
        REQUIRE(coords.getDEC() == Approx(90.0));

        REQUIRE(coords.getRAasHMS() == "18:40:00");
        REQUIRE(coords.getRAasDMS() == "280°00'00\"");

        REQUIRE(coords.getDECasDMS() == "90°00'00\"");

        REQUIRE(coords.getRADEC() == "280.0, 90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("too big negative angles")
    {
        Coordinates coords("-1000°00'00\"", "-100°00'00\"");

        REQUIRE(coords.getRA() == Approx(80.0));
        REQUIRE(coords.getDEC() == Approx(-90.0));

        REQUIRE(coords.getRAasHMS() == "05:20:00");
        REQUIRE(coords.getRAasDMS() == "80°00'00\"");

        REQUIRE(coords.getDECasDMS() == "-90°00'00\"");

        REQUIRE(coords.getRADEC() == "80.0, -90.0");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }

    SECTION("using set() method")
    {
        Coordinates coords;
        REQUIRE(coords.isNull());
        
        coords.set("208°27'29.520\"", "20°22'04.476\"");

        REQUIRE(!coords.isNull());

        REQUIRE(coords.getRA() == Approx(208.4582));
        REQUIRE(coords.getDEC() == Approx(20.36791));

        REQUIRE(coords.getRAasHMS() == "13:53:49.968");
        REQUIRE(coords.getRAasDMS() == "208°27'29.520\"");

        REQUIRE(coords.getDECasDMS() == "20°22'04.476\"");

        REQUIRE(coords.getRADEC() == "208.4582, 20.36791");
        REQUIRE(coords.getRADECasHMSDMS() == coords.getRAasHMS() + ", " + coords.getDECasDMS());
        REQUIRE(coords.getRADECasDMSDMS() == coords.getRAasDMS() + ", " + coords.getDECasDMS());
    }
}


TEST_CASE("Coordinates created with invalid strings", "[Coordinates]")
{
    SECTION("RA")
    {
        Coordinates coords("invalid", "20°00'00\"");

        REQUIRE(coords.isNull());
        REQUIRE(std::isinf(coords.getRA()));
        REQUIRE(coords.getDEC() == Approx(20.0));
    }

    SECTION("DEC")
    {
        Coordinates coords("1:00:00", "invalid");

        REQUIRE(coords.isNull());
        REQUIRE(coords.getRA() == Approx(15.0));
        REQUIRE(std::isinf(coords.getDEC()));
    }
}


TEST_CASE("Distance between coordinates", "[Coordinates]")
{
    Coordinates coords1("12:00:00", "5:00:00");
    Coordinates coords2("10:00:00", "0:00:00");

    auto distance = coords1 - coords2;

    REQUIRE(distance.ra == Approx(30.0));
    REQUIRE(distance.dec == Approx(5.0));
}


TEST_CASE("Add distance to coordinates", "[Coordinates]")
{
    Coordinates coords("10:00:00", "0:00:00");
    Coordinates::distance_t distance(30.0, 5.0);

    SECTION("operator+")
    {
        auto result = coords + distance;
        REQUIRE(result.getRA() == Approx(180.0));
        REQUIRE(result.getDEC() == Approx(5.0));
    }

    SECTION("operator+")
    {
        coords += distance;
        REQUIRE(coords.getRA() == Approx(180.0));
        REQUIRE(coords.getDEC() == Approx(5.0));
    }
}
