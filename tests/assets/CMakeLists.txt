file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/starfield.CR2
    ${CMAKE_CURRENT_LIST_DIR}/downloads/starfield.CR2
    EXPECTED_HASH MD5=eb153ca5b44f1f7468bf573905325d21
)

file(DOWNLOAD
    http://data.astrometry.net/4100/index-4109.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/index-4109.fits
    EXPECTED_HASH MD5=9a65a52ce04e3e75af950e5866f81b1b
)
