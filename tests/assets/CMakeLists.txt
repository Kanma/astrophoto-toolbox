file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/starfield.CR2
    ${CMAKE_CURRENT_LIST_DIR}/downloads/starfield.CR2
    EXPECTED_HASH MD5=eb153ca5b44f1f7468bf573905325d21
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/dark1.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/dark1.fits
    EXPECTED_HASH MD5=4617d115ebeb588ce0de290f19b72162
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/dark2.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/dark2.fits
    EXPECTED_HASH MD5=47fd68b8e7526b7f9891f0c6febfd2a5
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/dark3.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/dark3.fits
    EXPECTED_HASH MD5=f4af44172d536556cdefb33fe73aa29f
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/light1.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/light1.fits
    EXPECTED_HASH MD5=a0876d2ccb369e5e51d1793ee6698e5e
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/light2.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/light2.fits
    EXPECTED_HASH MD5=f2582f3a73a1ce657785645286235af1
)

file(DOWNLOAD
    https://github.com/Kanma/astrophoto-toolbox/releases/download/assets/light3.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/light3.fits
    EXPECTED_HASH MD5=122e883c288fd500624e56affa84794e
)

file(DOWNLOAD
    http://data.astrometry.net/4100/index-4109.fits
    ${CMAKE_CURRENT_LIST_DIR}/downloads/indexes/index-4109.fits
    EXPECTED_HASH MD5=9a65a52ce04e3e75af950e5866f81b1b
)
