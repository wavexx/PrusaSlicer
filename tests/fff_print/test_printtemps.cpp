#include <catch2/catch.hpp>

#include "libslic3r/libslic3r.h"
#include "libslic3r/GCodeReader.hpp"

#include "test_data.hpp"

#include <algorithm>
#include <boost/regex.hpp>

using namespace Slic3r;
using namespace Slic3r::Test;

SCENARIO( "GCode bed temperature control", "[PrintTemps]") {
    GIVEN("A default configuration and a print test object") {
        WHEN("the output is executed with no bed temperature control") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"" },
                { "first_layer_bed_temperature",	0 },
                { "bed_temperature",	            0 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("no bed temperature regulation is emitted") {
                REQUIRE(gcode.find("M140 ") == std::string::npos);
                REQUIRE(gcode.find("M190 ") == std::string::npos);
            }
        }
        WHEN("the output is executed with first layer bed temperature only") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            0 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("first layer bed temperature is set") {
                REQUIRE(gcode.find("M190 S60 ;") != std::string::npos);
            }
            THEN("other layer bed_temperature is unset") {
                REQUIRE(gcode.find("M140 ") == std::string::npos);
            }
        }
        WHEN("the output is executed with the same first/other bed temperature") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            60 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("other layer bed_temperature is unset") {
                REQUIRE(gcode.find("M140 ") == std::string::npos);
            }
        }
        WHEN("the output is executed with different first/other bed temperature") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            70 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("both values are set") {
                REQUIRE(gcode.find("M190 S60 ;") != std::string::npos);
                REQUIRE(gcode.find("M140 S70 ;") != std::string::npos);
            }
        }
        WHEN("the output is executed with a custom M190 start_gcode bed temperature") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"M190 S70 ;" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            60 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("start_gcode takes precedence") {
                REQUIRE(gcode.find("M190 S70 ;") != std::string::npos);
                REQUIRE(gcode.find("M190 S60 ;") == std::string::npos);
            }
            THEN("other layer bed_temperature is emitted because different") {
                REQUIRE(gcode.find("M140 S60 ;") != std::string::npos);
            }
        }
        WHEN("the output is executed with a custom M140 start_gcode bed temperature") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"M140 S70 ;custom" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            70 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("start_gcode takes precedence") {
                REQUIRE(gcode.find("M140 S70 ;custom") != std::string::npos);
                REQUIRE(gcode.find("M190 S60 ;") == std::string::npos);
            }
            THEN("other layer bed_temperature is not emitted because parsed value is the same") {
                REQUIRE(gcode.find("M140 S70 ; set bed temperature") == std::string::npos);
            }
        }
        WHEN("the output is executed with a custom M190 start_gcode with placeholder") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"M190 S[first_layer_bed_temperature] ;custom" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            60 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("start_gcode takes precedence") {
                REQUIRE(gcode.find("M190 S60 ; set bed temperature") == std::string::npos);
            }
            THEN("first_layer_bed_temperature is replaced correctly") {
                REQUIRE(gcode.find("M190 S60 ;custom") != std::string::npos);
            }
        }
        WHEN("the output is executed with a bed_temp_offset") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            60 },
                { "bed_temp_offset",	            5 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("first_layer_temperature has an offset") {
                REQUIRE(gcode.find("M190 S65 ;") != std::string::npos);
            }
            THEN("other bed_temperature is not emitted because the same") {
                REQUIRE(gcode.find("M140 ") == std::string::npos);
            }
        }
        WHEN("the output is executed with a bed_temp_offset and custom start_gcode") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "start_gcode",					"M190 S65 ;custom" },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            60 },
                { "bed_temp_offset",	            5 },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("other bed_temperature is not emitted because the same") {
                REQUIRE(gcode.find("M140 ") == std::string::npos);
            }
        }
        WHEN("the output is executed with a bed_temp_offset and custom start_gcode with placeholders") {
            Slic3r::Print print;
            Slic3r::Model model;
            Slic3r::Test::init_print({TestMesh::cube_20x20x20}, print, model, {
                { "gcode_comments",					true },
                { "first_layer_bed_temperature",	60 },
                { "bed_temperature",	            70 },
                { "bed_temp_offset",	            5 },
                { "start_gcode",
                  "; v1: [first_layer_bed_temperature]\n"
                  "; v2: [bed_temperature]\n"
                  "; v3: {first_layer_bed_temperature-bed_temp_offset}\n" },
                });
            std::string gcode = Slic3r::Test::gcode(print);
            THEN("first_layer_bed_temperature in gcode has bed_temp_offset applied") {
                REQUIRE(gcode.find("; v1: 65") != std::string::npos);
            }
            THEN("bed_temperature in gcode has bed_temp_offset applied") {
                REQUIRE(gcode.find("; v2: 75") != std::string::npos);
            }
            THEN("bed_temp_offset can be used in expressions to recover initial temperature") {
                REQUIRE(gcode.find("; v3: 60") != std::string::npos);
            }
        }
    }
}
