/*
 * SPDX-FileCopyrightText: 2024 Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-FileContributor: Philip Abbet <philip.abbet@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <filesystem>


int main(int argc, char** argv)
{
    // Ensures the temp folders used during the tests are empty
    std::filesystem::remove_all(TEMP_DIR);
    std::filesystem::create_directory(TEMP_DIR);
    std::filesystem::create_directory(TEMP_DIR "/empty");

    // Executes the tests
    int result = Catch::Session().run(argc, argv);

    return result;
}
