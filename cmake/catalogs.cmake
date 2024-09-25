file(DOWNLOAD
    https://github.com/mattiaverga/OpenNGC/raw/17397545d4e64708e2f4867796d97d33a1fff40f/database_files/NGC.csv
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/catalogs/NGC.csv
    EXPECTED_HASH MD5=642ddf1e7193c94e875469429894b5c8
)

file(DOWNLOAD
    https://github.com/mattiaverga/OpenNGC/raw/17397545d4e64708e2f4867796d97d33a1fff40f/database_files/addendum.csv
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/catalogs/NGC_addendum.csv
    EXPECTED_HASH MD5=be2dce7e2861cb642700793c1e95b426
)
