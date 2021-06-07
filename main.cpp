/* This file is part of HSPlasmaBench.
 *
 * HSPlasmaBench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HSPlasmaBench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HSPlasmaBench.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <string_view>

#include <cxxopts.hpp>
#include <string_theory/stdio>

#include <ResManager/plResManager.h>

// ===========================================================================

using ClockT = std::chrono::steady_clock;

using namespace std::literals::string_view_literals;

constexpr std::string_view kDefaultFile = R"(G:\Plasma\Games\MOULa\dat\Garrison.age)";
#define kDefaultNumIts 2
#define _STRINGIZE_IMPL(x) #x
#define STRINGIZE(x) _STRINGIZE_IMPL(x)

// Define to enable exception handling
#define SHOULD_I_TRY

// ===========================================================================

static void DoBenchmark(const std::filesystem::path file, int count)
{
    plDebug::Init(plDebug::kDLNone); // NO!

    ST::printf("Benchmarking loading '{}' {} time{}...\n", file, count, count == 1 ? ""sv : "s"sv);

    bool isAge = file.extension().compare(".age"sv) == 0;
    auto elapsed = ClockT::duration::zero();

    for (int i = 0; i < count; ++i) {
        ST::printf("\rRunning iteration {}...", i + 1);

        // We don't want to time the destruction of the res manager, so
        // we allocate a new one each time.
        auto mgr = std::make_unique<plResManager>();

        auto begin = ClockT::now();
        if (isAge)
            mgr->ReadAge(file, true);
        else
            mgr->ReadPage(file);
        auto end = ClockT::now();

        elapsed += end - begin;
    }

    ST::printf("\n... Done!\n\n");

    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    auto avg_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed / count);
    auto total_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
    auto avg_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed / count);

    ST::printf("Results:\n");
    ST::printf("Total: {.4f} seconds ({} us)\n", total_sec.count(), total_us.count());
    ST::printf("Average: {.4f} seconds ({} us)\n", avg_sec.count(), avg_us.count());
    ST::printf("Have a nice day!\n");
}

// ===========================================================================

int main(int argc, char** argv)
{
    cxxopts::Options options("HSPlasmaBench", "Benchmark utility for libHSPlasma IO");
    options.add_options()
        ("c,count", "Number of iterations", cxxopts::value<int>()->default_value(STRINGIZE(kDefaultNumIts)))
        ("f,file", "Age or PRP to read", cxxopts::value<std::filesystem::path>()->default_value(kDefaultFile.data()))
        ;
    auto result = options.parse(argc, argv);

    auto its = result["count"].as<int>();
    if (its < 1) {
        ST::printf("Invalid count specified ({}), resetting to {}\n", its, kDefaultNumIts);
        its = kDefaultNumIts;
    }

    auto file = result["file"].as<std::filesystem::path>();
    if (file.empty() || !(std::filesystem::is_regular_file(file) || std::filesystem::is_symlink(file))) {
        ST::printf("File '{}' does not exist, using '{}'\n", file, kDefaultFile);
        file = kDefaultFile;
    }

    ST::string ext = file.extension();
    if (!(ext.compare_i(".age"sv) || ext.compare_i(".prp"sv))) {
        ST::printf("Invalid file extension '{}', resetting to '{}'\n", ext, kDefaultFile);
        file = kDefaultFile;
    }

#ifdef SHOULD_I_TRY
    try {
#endif
        DoBenchmark(file, its);
#ifdef SHOULD_I_TRY
    } catch (const std::exception& ex) {
        ST::printf(stderr, "\nHmmm... Error: {}\n", ex.what());
        return 1;
    }
#endif

    return 0;
}
