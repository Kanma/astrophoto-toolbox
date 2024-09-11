#include <catch.hpp>
#include <astrophoto-toolbox/stacking/starmatcher.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;


TEST_CASE("Matching stars", "[StarMatcher]")
{
    FITS input;

    REQUIRE(input.open(DATA_DIR "matching_stars1.fits"));
    size2d_t imageSize;
    star_list_t stars1 = input.readStars(0, &imageSize);
    input.close();

    REQUIRE(input.open(DATA_DIR "matching_stars2.fits"));
    star_list_t stars2 = input.readStars();
    input.close();

    Transformation transformation;
    stacking::StarMatcher matcher;

    REQUIRE(matcher.computeTransformation(stars1, stars2, imageSize, transformation));

    double dx, dy;
    transformation.offsets(dx, dy);

    REQUIRE(dx == Approx(-78.9177));
    REQUIRE(dy == Approx(98.5097));

    double angle = transformation.angle(imageSize.width);
    REQUIRE(angle == Approx(-0.0002532126));

    REQUIRE(matcher.computeTransformation(stars2, stars1, imageSize, transformation));

    transformation.offsets(dx, dy);

    REQUIRE(dx == Approx(78.9042));
    REQUIRE(dy == Approx(-98.49931));

    angle = transformation.angle(imageSize.width);
    REQUIRE(angle == Approx(0.0002597837));
}
