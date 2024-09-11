#include <catch.hpp>
#include <astrophoto-toolbox/stacking/registration.h>
#include <astrophoto-toolbox/images/raw.h>
#include <astrophoto-toolbox/data/fits.h>

using namespace astrophototoolbox;


TEST_CASE("Registration of a bitmap", "[Registration]")
{
    RawImage image;

    REQUIRE(image.open(DATA_DIR "downloads/starfield.CR2"));

    DoubleColorBitmap bitmap;
    REQUIRE(image.toBitmap(&bitmap));
    REQUIRE(bitmap.width() == 3906);
    REQUIRE(bitmap.height() == 2602);

    stacking::Registration registration;
    star_list_t stars = registration.registerBitmap(&bitmap);

    REQUIRE(stars.size() == 41);

    FITS input;
    REQUIRE(input.open(DATA_DIR "starfield_stars.fits"));

    star_list_t ref = input.readStars();

    REQUIRE(stars.size() == ref.size());

    std::sort(stars.begin(), stars.end());
    std::sort(ref.begin(), ref.end());

    for (size_t i = 0; i < stars.size(); ++i)
    {
        REQUIRE(stars[i].position.x == Approx(ref[i].position.x));
        REQUIRE(stars[i].position.y == Approx(ref[i].position.y));
        REQUIRE(stars[i].intensity == Approx(ref[i].intensity));
    }
}
