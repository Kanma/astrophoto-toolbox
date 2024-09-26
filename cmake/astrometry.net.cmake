# Download and patch the code
set(ASTROMETRY_NET_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/astrometry-net.patch)

FetchContent_Declare(
    astrometry-net
    GIT_REPOSITORY https://github.com/dstndstn/astrometry.net.git
    GIT_TAG "fbca48ebec403d4f954c97bc83d260ea40643577" # aka "0.95"
    PATCH_COMMAND ${ASTROMETRY_NET_PATCH}
    UPDATE_DISCONNECTED 1
)

FetchContent_MakeAvailable(astrometry-net)

set(ASTROMETRY_NET_SRC_DIR "${FETCHCONTENT_BASE_DIR}/astrometry-net-src")


# Configure GSL
# Note: all the definitions in the config file aren't used
if (NOT IS_READABLE ${CMAKE_BINARY_DIR}/generated/astrometry/config.h)
    include(CheckSymbolExists)
    include(CheckIncludeFile)
    include(CheckCSourceRuns)
    include(CheckCSourceCompiles)

    function(check_math_function_exists FUNC DEFINITION)
        set(CMAKE_REQUIRED_LIBRARIES "m")

        check_c_source_compiles(
            "
#include <math.h>
int main(void)
{
    double a = ${FUNC}(1.0);
    return 0;
}"
            ${DEFINITION}
        )

        if(${DEFINITION})
            add_definitions(-D${DEFINITION})
        endif()

        unset(CMAKE_REQUIRED_LIBRARIES)
    endfunction()


    function(check_math_function_exists2 FUNC PARAMS DEFINITION)
        set(CMAKE_REQUIRED_LIBRARIES "m")

        check_c_source_compiles(
            "
#include <math.h>
int main(void)
{
    double a = ${FUNC}(${PARAMS});
    return 0;
}"
            ${DEFINITION}
        )

        if(${DEFINITION})
            add_definitions(-D${DEFINITION})
        endif()

        unset(CMAKE_REQUIRED_LIBRARIES)
    endfunction()


    check_math_function_exists(acosh HAVE_DECL_ACOSH)
    check_math_function_exists(asinh HAVE_DECL_ASINH)
    check_math_function_exists(atanh HAVE_DECL_ATANH)
    check_math_function_exists(expm1 HAVE_DECL_EXPM1)
    check_math_function_exists(finite HAVE_DECL_FINITE)
    check_math_function_exists2(frexp "1.0, NULL" HAVE_DECL_FREXP)
    check_math_function_exists2(hypot "1.0, 0.5" HAVE_DECL_HYPOT)
    check_math_function_exists(isfinite HAVE_DECL_ISFINITE)
    check_math_function_exists(isinf HAVE_DECL_ISINF)
    check_math_function_exists(isnan HAVE_DECL_ISNAN)
    check_math_function_exists2(ldexp "1.0, 1" HAVE_DECL_LDEXP)
    check_math_function_exists(log1p HAVE_DECL_LOG1P)

    check_include_file("ieeefp.h" HAVE_IEEEFP_H)
    if(HAVE_IEEEFP_H)
        add_definitions(-DHAVE_IEEEFP_H)
    endif()

    check_c_source_runs(
        "
#include <math.h>
int main(void)
{
    int status; double inf, nan;
    inf = exp(1.0e10);
    nan = inf / inf ;
    status = (nan == nan);
    exit (status);
}"
        HAVE_IEEE_COMPARISONS
    )
    if(HAVE_IEEE_COMPARISONS)
        add_definitions(-DHAVE_IEEE_COMPARISONS)
    endif()

    check_c_source_runs(
        "
extern inline double foo (double x) { return x + 1.0; }

int main()
{
    foo(1.0);
    return 0;
}"
        HAVE_INLINE
    )
    if(HAVE_INLINE)
        add_definitions(-DHAVE_INLINE)
    endif()

    check_c_source_runs(
        "
int main()
{
    static int test_array [1 - 2 * !(((char) -1) < 0)];
    test_array [0] = 0;
    return 0;
}"
        __CHAR_UNSIGNED__
    )
    if(__CHAR_UNSIGNED__ AND NOT CMAKE_COMPILER_IS_GNUC)
        add_definitions(-D__CHAR_UNSIGNED__)
    endif()

    check_c_source_compiles(
        "
#include <stdlib.h>

int main()
{
    size_t a = 0;
    return a;
}"
        HAVE_SIZE_T
    )

    check_c_source_runs(
        "
int main()
{
    volatile int a = 0;
    return a;
}"
        HAVE_VOLATILE
    )

    file(READ ${ASTROMETRY_NET_SRC_DIR}/gsl-an/config.h.in FILE_CONTENTS)

    foreach (DEF_NAME
        "HAVE_C99_INLINE"
        "HAVE_IEEE_COMPARISONS"
        "HAVE_IEEE_DENORMALS"
        "HAVE_INLINE"
        "HIDE_INLINE_STATIC"
        "HAVE_PRINTF_LONGDOUBLE"
        "HAVE_GNUSPARC_IEEE_INTERFACE"
        "HAVE_GNUM68K_IEEE_INTERFACE"
        "HAVE_GNUPPC_IEEE_INTERFACE"
        "HAVE_GNUX86_IEEE_INTERFACE"
        "HAVE_SUNOS4_IEEE_INTERFACE"
        "HAVE_SOLARIS_IEEE_INTERFACE"
        "HAVE_HPUX11_IEEE_INTERFACE"
        "HAVE_HPUX_IEEE_INTERFACE"
        "HAVE_TRU64_IEEE_INTERFACE"
        "HAVE_IRIX_IEEE_INTERFACE"
        "HAVE_AIX_IEEE_INTERFACE"
        "HAVE_FREEBSD_IEEE_INTERFACE"
        "HAVE_OS2EMX_IEEE_INTERFACE"
        "HAVE_NETBSD_IEEE_INTERFACE"
        "HAVE_OPENBSD_IEEE_INTERFACE"
        "HAVE_DARWIN_IEEE_INTERFACE"
        "HAVE_DARWIN86_IEEE_INTERFACE"
    )
        string(REPLACE "#undef ${DEF_NAME}" "#cmakedefine ${DEF_NAME}" FILE_CONTENTS "${FILE_CONTENTS}")
    endforeach()

    string(REPLACE "#undef" "#cmakedefine01" FILE_CONTENTS "${FILE_CONTENTS}")
    string(REPLACE "# undef" "#cmakedefine01" FILE_CONTENTS "${FILE_CONTENTS}")

    string(REPLACE "#cmakedefine01 inline" "/* #undef inline */" FILE_CONTENTS "${FILE_CONTENTS}")

    if(HAVE_SIZE_T)
        string(REPLACE "#cmakedefine01 size_t" "/* #undef size_t */" FILE_CONTENTS "${FILE_CONTENTS}")
    else()
        string(REPLACE "#cmakedefine01 size_t" "#define size_t unsigned int" FILE_CONTENTS "${FILE_CONTENTS}")
    endif()

    if(HAVE_VOLATILE)
        string(REPLACE "#cmakedefine01 volatile" "/* #undef volatile */" FILE_CONTENTS "${FILE_CONTENTS}")
    else()
        string(REPLACE "#cmakedefine01 volatile" "#define volatile" FILE_CONTENTS "${FILE_CONTENTS}")
    endif()

    file(WRITE ${CMAKE_BINARY_DIR}/generated/astrometry/config.h.in "${FILE_CONTENTS}")

    configure_file(${CMAKE_BINARY_DIR}/generated/astrometry/config.h.in ${CMAKE_BINARY_DIR}/generated/astrometry/config.h)

    file(WRITE ${CMAKE_BINARY_DIR}/generated/astrometry/os-features-config.h)
endif()


# Compile the library
set(SRC_FILES
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdtree.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdtree_dim.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdtree_mem.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdtree_fits_io.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_ddd.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_fff.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_ddu.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_duu.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_dds.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_dss.c
    ${ASTROMETRY_NET_SRC_DIR}/libkd/kdint_lll.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/blas/blas.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/block/init.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/block/block.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/caxpy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ccopy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cdotc_sub.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cdotu_sub.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cgemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cgemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cgerc.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cgeru.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/chemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/chemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cher.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cher2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cher2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cherk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/csscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/cswap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/csymm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/csyr2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/csyrk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ctrmm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ctrmv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ctrsm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ctrsv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dasum.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/daxpy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dcopy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ddot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dgemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dgemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dger.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dnrm2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/drot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/drotg.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/drotm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/drotmg.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsdot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dswap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsymm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsymv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsyr.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsyr2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsyr2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dsyrk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dtrmm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dtrmv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dtrsm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dtrsv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dzasum.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/dznrm2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/hypot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/icamax.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/idamax.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/isamax.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/izamax.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sasum.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/saxpy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/scasum.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/scnrm2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/scopy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sdot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sdsdot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sgemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sgemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sger.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/snrm2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/srot.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/srotg.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/srotm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/srotmg.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/sswap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssymm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssymv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssyr.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssyr2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssyr2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ssyrk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/strmm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/strmv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/strsm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/strsv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/xerbla.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zaxpy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zcopy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zdotc_sub.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zdotu_sub.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zdscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zgemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zgemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zgerc.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zgeru.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zhemm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zhemv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zher.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zher2.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zher2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zherk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zscal.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zswap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zsymm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zsyr2k.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/zsyrk.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ztrmm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ztrmv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ztrsm.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/cblas/ztrsv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/err/error.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/err/stream.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/err/strerror.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/linalg/bidiag.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/linalg/householder.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/linalg/lu.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/linalg/qr.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/linalg/svd.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/copy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/init.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/matrix.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/rowcol.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/submatrix.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/swap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/matrix/view.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/permutation/init.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/permutation/permutation.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/permutation/permute.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/sys/coerce.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/sys/fdiv.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/sys/ldfrexp.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/sys/infnan.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/copy.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/init.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/oper.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/prop.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/subvector.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/swap.c
    ${ASTROMETRY_NET_SRC_DIR}/gsl-an/vector/vector.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/anqfits.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_byteswap.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_card.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_convert.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_error.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_float.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_header.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_memory.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_rw.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_table.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_time.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_tools.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/onefield.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/quad-utils.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/solvedfile.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/solver.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/tweak2.c
    ${ASTROMETRY_NET_SRC_DIR}/solver/verify.c
    ${ASTROMETRY_NET_SRC_DIR}/util/an-endian.c
    ${ASTROMETRY_NET_SRC_DIR}/util/bl.c
    ${ASTROMETRY_NET_SRC_DIR}/util/bl-sort.c
    ${ASTROMETRY_NET_SRC_DIR}/util/codekd.c
    ${ASTROMETRY_NET_SRC_DIR}/util/ctmf.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dallpeaks.c
    ${ASTROMETRY_NET_SRC_DIR}/util/datalog.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dcen3x3.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dfind.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dmedsmooth.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dobjects.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dpeaks.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dselip.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dsigma.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dsmooth.c
    ${ASTROMETRY_NET_SRC_DIR}/util/errors.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fitsbin.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fitsfile.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fitsioutils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fitstable.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fit-wcs.c
    ${ASTROMETRY_NET_SRC_DIR}/util/gslutils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/healpix.c
    ${ASTROMETRY_NET_SRC_DIR}/util/image2xy.c
    ${ASTROMETRY_NET_SRC_DIR}/util/index.c
    ${ASTROMETRY_NET_SRC_DIR}/util/ioutils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/log.c
    ${ASTROMETRY_NET_SRC_DIR}/util/matchfile.c
    ${ASTROMETRY_NET_SRC_DIR}/util/matchobj.c
    ${ASTROMETRY_NET_SRC_DIR}/util/mathutil.c
    ${ASTROMETRY_NET_SRC_DIR}/util/permutedsort.c
    ${ASTROMETRY_NET_SRC_DIR}/util/quadfile.c
    ${ASTROMETRY_NET_SRC_DIR}/util/rdlist.c
    ${ASTROMETRY_NET_SRC_DIR}/util/resample.c
    ${ASTROMETRY_NET_SRC_DIR}/util/scamp-catalog.c
    ${ASTROMETRY_NET_SRC_DIR}/util/simplexy.c
    ${ASTROMETRY_NET_SRC_DIR}/util/sip.c
    ${ASTROMETRY_NET_SRC_DIR}/util/sip_qfits.c
    ${ASTROMETRY_NET_SRC_DIR}/util/sip-utils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/starkd.c
    ${ASTROMETRY_NET_SRC_DIR}/util/starutil.c
    ${ASTROMETRY_NET_SRC_DIR}/util/starxy.c
    ${ASTROMETRY_NET_SRC_DIR}/util/tic.c
    ${ASTROMETRY_NET_SRC_DIR}/util/xylist.c
)

add_library(astrometry-net ${SRC_FILES})

target_include_directories(astrometry-net
    PUBLIC
        ${ASTROMETRY_NET_SRC_DIR}/include
    PRIVATE
        ${ASTROMETRY_NET_SRC_DIR}/include/astrometry
        ${ASTROMETRY_NET_SRC_DIR}/util
        ${ASTROMETRY_NET_SRC_DIR}/gsl-an
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_BINARY_DIR}/generated/astrometry
)

target_compile_definitions(astrometry-net
    PUBLIC
        AN_GIT_REVISION="0.95"
        AN_GIT_DATE="Mon_May_6_18:41:28_2024_-0400"
        AN_GIT_URL="https://github.com/dstndstn/astrometry.net"
)

target_compile_options(astrometry-net PRIVATE "-w")
