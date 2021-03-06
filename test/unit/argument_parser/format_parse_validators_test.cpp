// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2019, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2019, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file and also available at: https://github.com/seqan/seqan3/blob/master/LICENSE
// -----------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include <fstream>

#include <range/v3/view/remove_if.hpp>
#include <range/v3/algorithm/equal.hpp>

#include <seqan3/argument_parser/all.hpp>
#include <seqan3/alphabet/all.hpp>
#include <seqan3/io/stream/parse_condition.hpp>
#include <seqan3/range/view/persist.hpp>
#include <seqan3/std/filesystem>
#include <seqan3/test/tmp_filename.hpp>

using namespace seqan3;

TEST(validator_test, fullfill_concept)
{
    EXPECT_FALSE(validator_concept<int>);

    EXPECT_TRUE(validator_concept<detail::default_validator<int>>);
    EXPECT_TRUE(validator_concept<detail::default_validator<int> const>);
    EXPECT_TRUE(validator_concept<detail::default_validator<int> &>);

    EXPECT_TRUE(validator_concept<detail::default_validator<std::vector<int>>>);
    EXPECT_TRUE(validator_concept<arithmetic_range_validator>);
    EXPECT_TRUE(validator_concept<value_list_validator<double>>);
    EXPECT_TRUE(validator_concept<value_list_validator<std::string>>);
    EXPECT_TRUE(validator_concept<regex_validator>);
    EXPECT_TRUE(validator_concept<file_ext_validator>);
    EXPECT_TRUE(validator_concept<path_existence_validator>);

    EXPECT_TRUE(validator_concept<decltype(file_ext_validator{{"t"}} | regex_validator{".*"})>);
}

TEST(validator_test, no_file)
{
    // file
    {
        std::filesystem::path p{"./sandbox.fasta"};
        std::string s{"./stonebox.fasta"};
        path_existence_validator my_validator{};
        EXPECT_THROW(my_validator(p), parser_invalid_argument);
        EXPECT_THROW(my_validator(s), parser_invalid_argument);

        std::filesystem::path file_in_path;

        // option
        const char * argv[] = {"./argument_parser_test", "-i", "./sandbox.fasta"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(file_in_path, 'i', "int-option", "desc",
                          option_spec::DEFAULT, path_existence_validator{});

        EXPECT_THROW(parser.parse(), parser_invalid_argument);
    }

    // directory
    {
        std::filesystem::path p{"./sandbox/"};
        std::string s{"./stonebox/"};
        path_existence_validator my_validator{};
        EXPECT_THROW(my_validator(p), parser_invalid_argument);
        EXPECT_THROW(my_validator(s), parser_invalid_argument);

        std::filesystem::path dir_in_path;

        // option
        const char * argv[] = {"./argument_parser_test", "-i", "./sandbox/"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(dir_in_path, 'i', "int-option", "desc",
                          option_spec::DEFAULT, path_existence_validator{});

        EXPECT_THROW(parser.parse(), parser_invalid_argument);
    }
}

TEST(validator_test, file_exists)
{
    test::tmp_filename tmp_name{"testbox.fasta"};

    // file
    {
        std::ofstream tmp_file(tmp_name.get_path());
        path_existence_validator my_validator{};
        EXPECT_NO_THROW(my_validator(tmp_name.get_path()));

        std::filesystem::path file_in_path;

        // option
        std::filesystem::path path =  tmp_name.get_path();
        const char * argv[] = {"./argument_parser_test", "-i", path.c_str()};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(file_in_path, 'i', "int-option", "desc",
                          option_spec::DEFAULT, path_existence_validator{});

        EXPECT_NO_THROW(parser.parse());
    }

    // directory
    {
        std::ofstream tmp_dir(tmp_name.get_path().parent_path());
        path_existence_validator my_validator{};
        EXPECT_NO_THROW(my_validator(tmp_name.get_path().parent_path()));

        std::filesystem::path dir_in_path;

        // option
        std::filesystem::path path = tmp_name.get_path().parent_path();
        const char * argv[] = {"./argument_parser_test", "-i", path.c_str()};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(dir_in_path, 'i', "int-option", "desc",
                          option_spec::DEFAULT, path_existence_validator{});

        EXPECT_NO_THROW(parser.parse());
    }

    {
        // get help page message
        std::filesystem::path path;
        const char * argv[] = {"./argument_parser_test", "-h"};
        argument_parser parser("test_parser", 2, argv);
        parser.add_positional_option(path, "desc", path_existence_validator{});

        testing::internal::CaptureStdout();
        EXPECT_EXIT(parser.parse(), ::testing::ExitedWithCode(EXIT_SUCCESS), "");
        std::string my_stdout = testing::internal::GetCapturedStdout();
        std::string expected = std::string("test_parser"
                               "==========="
                               "POSITIONAL ARGUMENTS"
                               "    ARGUMENT-1 (std::filesystem::path)"
                               "          desc The file or directory is checked for existence."
                               "VERSION"
                               "    Last update: "
                               "    test_parser version: "
                               "    SeqAn version: ") + seqan3_version;

        EXPECT_TRUE(ranges::equal((my_stdout | std::view::filter(!is_space)), expected | std::view::filter(!is_space)));
    }
}

TEST(validator_test, file_ext_validator)
{
    std::string option_value;
    std::vector<std::string> option_vector;
    file_ext_validator case_sensitive_file_ext_validator({"sAm", "FASTQ", "fasta" }, true);
    file_ext_validator case_insensitive_file_ext_validator({"sAm", "FASTQ", "fasta"}, false);
    file_ext_validator no_extension_file_ext_validator({""});
    file_ext_validator default_file_ext_validator({"sAm", "FASTQ", "fasta"});

    // check case insensitive validator => success
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sam"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, case_insensitive_file_ext_validator);

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.sam");
    }

    // check case sensitive validator => failure
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sam"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, case_sensitive_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // check default (case insensitive) validator => success
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.FaStQ"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, default_file_ext_validator);

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.FaStQ");    
    }

    // check case sensitive validator => success
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.FASTQ"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, case_sensitive_file_ext_validator);

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.FASTQ");    
    }

    // check case insensitive validator => failure
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.bAm"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, case_insensitive_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // check no file suffix => failure
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, case_sensitive_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // check file suffix with no_ext_validator => failure
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.txt"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, no_extension_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // check trailing dot with no_ext_validator => failure
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file."};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, no_extension_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // check no file extension with no_ext_validator => success
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, no_extension_file_ext_validator);

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file");        
    }
}

TEST(validator_test, arithmetic_range_validator_success)
{
    int option_value;
    std::vector<int> option_vector;

    // option
    const char * argv[] = {"./argument_parser_test", "-i", "10"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, 'i', "int-option", "desc",
                      option_spec::DEFAULT, arithmetic_range_validator(1, 20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, 10);

    // option - negative values
    const char * argv2[] = {"./argument_parser_test", "-i", "-10"};
    argument_parser parser2("test_parser", 3, argv2);
    parser2.add_option(option_value, 'i', "int-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(-20, 20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser2.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, -10);

    // positional option
    const char * argv3[] = {"./argument_parser_test", "10"};
    argument_parser parser3("test_parser", 2, argv3);
    parser3.add_positional_option(option_value, "desc", arithmetic_range_validator(1, 20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser3.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, 10);

    // positional option - negative values
    const char * argv4[] = {"./argument_parser_test", "--", "-10"};
    argument_parser parser4("test_parser", 3, argv4);
    parser4.add_positional_option(option_value, "desc", arithmetic_range_validator(-20, 20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser4.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, -10);

    // option - vector
    const char * argv5[] = {"./argument_parser_test", "-i", "-10", "-i", "48"};
    argument_parser parser5("test_parser", 5, argv5);
    parser5.add_option(option_vector, 'i', "int-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(-50,50));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser5.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector[0], -10);
    EXPECT_EQ(option_vector[1], 48);

    // positional option - vector
    option_vector.clear();
    const char * argv6[] = {"./argument_parser_test", "--", "-10", "1"};
    argument_parser parser6("test_parser", 4, argv6);
    parser6.add_positional_option(option_vector, "desc", arithmetic_range_validator(-20,20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser6.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector[0], -10);
    EXPECT_EQ(option_vector[1], 1);

    // get help page message
    option_vector.clear();
    const char * argv7[] = {"./argument_parser_test", "-h"};
    argument_parser parser7("test_parser", 2, argv7);
    parser7.add_positional_option(option_vector, "desc", arithmetic_range_validator(-20,20));


    testing::internal::CaptureStdout();
    EXPECT_EXIT(parser7.parse(), ::testing::ExitedWithCode(EXIT_SUCCESS), "");
    std::string my_stdout = testing::internal::GetCapturedStdout();
    std::string expected = std::string("test_parser"
                           "==========="
                           "POSITIONAL ARGUMENTS"
                           "    ARGUMENT-1 (List of signed 32 bit integer's)"
                           "          desc Value must be in range [-20,20]."
                           "VERSION"
                           "    Last update: "
                           "    test_parser version: "
                           "    SeqAn version: ") + seqan3_version;
    EXPECT_TRUE(ranges::equal((my_stdout   | std::view::filter(!is_space)),
                               expected | std::view::filter(!is_space)));

    // option - double value
    double double_option_value;
    const char * argv8[] = {"./argument_parser_test", "-i", "10.9"};
    argument_parser parser8("test_parser", 3, argv8);
    parser8.add_option(double_option_value, 'i', "double-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(1, 20));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser8.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_FLOAT_EQ(double_option_value, 10.9);
}

TEST(validator_test, arithmetic_range_validator_error)
{
    int option_value;
    std::vector<int> option_vector;

    // option - above max
    const char * argv[] = {"./argument_parser_test", "-i", "30"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, 'i', "int-option", "desc",
                      option_spec::DEFAULT, arithmetic_range_validator(1, 20));

    EXPECT_THROW(parser.parse(), validation_failed);

    // option - below min
    const char * argv2[] = {"./argument_parser_test", "-i", "-21"};
    argument_parser parser2("test_parser", 3, argv2);
    parser2.add_option(option_value, 'i', "int-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(-20, 20));

    EXPECT_THROW(parser2.parse(), validation_failed);

    // positional option - above max
    const char * argv3[] = {"./argument_parser_test", "30"};
    argument_parser parser3("test_parser", 2, argv3);
    parser3.add_positional_option(option_value, "desc", arithmetic_range_validator(1, 20));

    EXPECT_THROW(parser3.parse(), validation_failed);

    // positional option - below min
    const char * argv4[] = {"./argument_parser_test", "--", "-21"};
    argument_parser parser4("test_parser", 3, argv4);
    parser4.add_positional_option(option_value, "desc", arithmetic_range_validator(-20, 20));

    EXPECT_THROW(parser4.parse(), validation_failed);

    // option - vector
    const char * argv5[] = {"./argument_parser_test", "-i", "-100"};
    argument_parser parser5("test_parser", 3, argv5);
    parser5.add_option(option_vector, 'i', "int-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(-50, 50));

    EXPECT_THROW(parser5.parse(), validation_failed);

    // positional option - vector
    option_vector.clear();
    const char * argv6[] = {"./argument_parser_test", "--", "-10", "100"};
    argument_parser parser6("test_parser", 4, argv6);
    parser6.add_positional_option(option_vector, "desc", arithmetic_range_validator(-20, 20));

    EXPECT_THROW(parser6.parse(), validation_failed);

    // option - double value
    double double_option_value;
    const char * argv7[] = {"./argument_parser_test", "-i", "0.9"};
    argument_parser parser7("test_parser", 3, argv7);
    parser7.add_option(double_option_value, 'i', "double-option", "desc",
                       option_spec::DEFAULT, arithmetic_range_validator(1, 20));

    EXPECT_THROW(parser7.parse(), validation_failed);
}

TEST(validator_test, value_list_validator_success)
{
    std::string option_value;
    int option_value_int;
    std::vector<std::string> option_vector;
    std::vector<int> option_vector_int;

    // option
    const char * argv[] = {"./argument_parser_test", "-s", "ba"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, 's', "string-option", "desc",
                      option_spec::DEFAULT, value_list_validator({"ha", "ba", "ma"}));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, "ba");

    // option with integers
    const char * argv2[] = {"./argument_parser_test", "-i", "-21"};
    argument_parser parser2("test_parser", 3, argv2);
    parser2.add_option(option_value_int, 'i', "int-option", "desc",
                       option_spec::DEFAULT, value_list_validator<int>({0, -21, 10}));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser2.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value_int, -21);

    // positional option
    const char * argv3[] = {"./argument_parser_test", "ma"};
    argument_parser parser3("test_parser", 2, argv3);
    parser3.add_positional_option(option_value, "desc", value_list_validator({"ha", "ba", "ma"}));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser3.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, "ma");

    // positional option - vector
    const char * argv4[] = {"./argument_parser_test", "ha", "ma"};
    argument_parser parser4("test_parser", 3, argv4);
    parser4.add_positional_option(option_vector, "desc",
                                  value_list_validator({"ha", "ba", "ma"}));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser4.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector[0], "ha");
    EXPECT_EQ(option_vector[1], "ma");

    // option - vector
    const char * argv5[] = {"./argument_parser_test", "-i", "-10", "-i", "48"};
    argument_parser parser5("test_parser", 5, argv5);
    parser5.add_option(option_vector_int, 'i', "int-option", "desc",
                       option_spec::DEFAULT, value_list_validator<int>({-10,48,50}));

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser5.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector_int[0], -10);
    EXPECT_EQ(option_vector_int[1], 48);

    // get help page message
    option_vector.clear();
    const char * argv7[] = {"./argument_parser_test", "-h"};
    argument_parser parser7("test_parser", 2, argv7);
    parser7.add_option(option_vector_int, 'i', "int-option", "desc",
                       option_spec::DEFAULT, value_list_validator<int>({-10,48,50}));

    testing::internal::CaptureStdout();
    EXPECT_EXIT(parser7.parse(), ::testing::ExitedWithCode(EXIT_SUCCESS), "");
    std::string my_stdout = testing::internal::GetCapturedStdout();
    std::string expected = std::string("test_parser"
                           "==========="
                           "OPTIONS"
                           "    -i, --int-option (List of signed 32 bit integer's)"
                           "          desc Value must be one of [-10,48,50]."
                           "VERSION"
                           "    Last update: "
                           "    test_parser version: "
                           "    SeqAn version: ") + seqan3_version;
    EXPECT_TRUE(ranges::equal((my_stdout   | std::view::filter(!is_space)),
                               expected | std::view::filter(!is_space)));
}

TEST(validator_test, value_list_validator_error)
{
    std::string option_value;
    int option_value_int;
    std::vector<std::string> option_vector;
    std::vector<int> option_vector_int;

    // option
    const char * argv[] = {"./argument_parser_test", "-s", "sa"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, 's', "string-option", "desc",
                      option_spec::DEFAULT, value_list_validator({"ha", "ba", "ma"}));

    EXPECT_THROW(parser.parse(), validation_failed);

    // positional option
    const char * argv3[] = {"./argument_parser_test", "30"};
    argument_parser parser3("test_parser", 2, argv3);
    parser3.add_positional_option(option_value_int, "desc", value_list_validator({0, 5, 10}));

    EXPECT_THROW(parser3.parse(), validation_failed);

    // positional option - vector
    const char * argv4[] = {"./argument_parser_test", "fo", "ma"};
    argument_parser parser4("test_parser", 3, argv4);
    parser4.add_positional_option(option_vector, "desc",
                                  value_list_validator({"ha", "ba", "ma"}));

    EXPECT_THROW(parser4.parse(), validation_failed);

    // option - vector
    const char * argv5[] = {"./argument_parser_test", "-i", "-10", "-i", "488"};
    argument_parser parser5("test_parser", 5, argv5);
    parser5.add_option(option_vector_int, 'i', "int-option", "desc",
                       option_spec::DEFAULT, value_list_validator<int>({-10,48,50}));

    EXPECT_THROW(parser5.parse(), validation_failed);
}

TEST(validator_test, regex_validator_success)
{
    std::string option_value;
    std::vector<std::string> option_vector;
    regex_validator email_validator("[a-zA-Z]+@[a-zA-Z]+\\.com");
    regex_validator email_vector_validator("[a-zA-Z]+@[a-zA-Z]+\\.com");

    // option
    const char * argv[] = {"./argument_parser_test", "-s", "ballo@rollo.com"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, 's', "string-option", "desc",
                      option_spec::DEFAULT, email_validator);

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, "ballo@rollo.com");

    // positional option
    const char * argv2[] = {"./argument_parser_test", "chr1"};
    argument_parser parser2("test_parser", 2, argv2);
    parser2.add_positional_option(option_value, "desc",
                                  regex_validator{"^chr[0-9]+"});

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser2.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_value, "chr1");

    // positional option - vector
    const char * argv3[] = {"./argument_parser_test", "rollo", "bollo", "lollo"};
    argument_parser parser3("test_parser", 4, argv3);
    parser3.add_positional_option(option_vector, "desc",
                                  regex_validator{".*oll.*"});

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser3.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector[0], "rollo");
    EXPECT_EQ(option_vector[1], "bollo");
    EXPECT_EQ(option_vector[2], "lollo");

    // option - vector
    option_vector.clear();
    const char * argv4[] = {"./argument_parser_test", "-s", "rita@rambo.com", "-s", "tina@rambo.com"};
    argument_parser parser4("test_parser", 5, argv4);
    parser4.add_option(option_vector, 's', "string-option", "desc",
                       option_spec::DEFAULT, email_vector_validator);

    testing::internal::CaptureStderr();
    EXPECT_NO_THROW(parser4.parse());
    EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
    EXPECT_EQ(option_vector[0], "rita@rambo.com");
    EXPECT_EQ(option_vector[1], "tina@rambo.com");

    // get help page message
    option_vector.clear();
    const char * argv7[] = {"./argument_parser_test", "-h"};
    argument_parser parser7("test_parser", 2, argv7);
    parser7.add_option(option_vector, 's', "string-option", "desc",
                       option_spec::DEFAULT, email_vector_validator);

    testing::internal::CaptureStdout();
    EXPECT_EXIT(parser7.parse(), ::testing::ExitedWithCode(EXIT_SUCCESS), "");
    std::string my_stdout = testing::internal::GetCapturedStdout();
    std::string expected = std::string("test_parser"
                           "==========="
                           "OPTIONS"
                           "    -s, --string-option (List of std::string's)"
                           "          desc Value must match the pattern '[a-zA-Z]+@[a-zA-Z]+\\.com'."
                           "VERSION"
                           "    Last update: "
                           "    test_parser version: "
                           "    SeqAn version: ") + seqan3_version;
    EXPECT_TRUE(ranges::equal((my_stdout   | ranges::view::remove_if(is_space)),
                               expected | ranges::view::remove_if(is_space)));
}

TEST(validator_test, regex_validator_error)
{
    std::string option_value;
    std::vector<std::string> option_vector;

    // option
    const char * argv[] = {"./argument_parser_test", "--string-option", "sally"};
    argument_parser parser("test_parser", 3, argv);
    parser.add_option(option_value, '\0', "string-option", "desc",
                      option_spec::DEFAULT, regex_validator("tt"));

    EXPECT_THROW(parser.parse(), validation_failed);

    // positional option
    const char * argv2[] = {"./argument_parser_test", "jessy"};
    argument_parser parser2("test_parser", 2, argv2);
    parser2.add_positional_option(option_value, "desc",
                                  regex_validator("[0-9]"));

    EXPECT_THROW(parser2.parse(), validation_failed);

    // positional option - vector
    const char * argv3[] = {"./argument_parser_test", "rollo", "bttllo", "lollo"};
    argument_parser parser3("test_parser", 4, argv3);
    parser3.add_positional_option(option_vector, "desc",
                                  regex_validator{".*oll.*"});

    EXPECT_THROW(parser3.parse(), validation_failed);

    // option - vector
    option_vector.clear();
    const char * argv4[] = {"./argument_parser_test", "-s", "gh", "-s", "tt"};
    argument_parser parser4("test_parser", 5, argv4);
    parser4.add_option(option_vector, 's', "", "desc",
                       option_spec::DEFAULT, regex_validator("tt"));

    EXPECT_THROW(parser4.parse(), validation_failed);
}

TEST(validator_test, chaining_validators)
{
    std::string option_value;
    std::vector<std::string> option_vector;
    regex_validator absolute_path_validator("(/[^/]+)+/.*\\.[^/\\.]+$");
    file_ext_validator my_file_ext_validator({"sa", "so"});

    // option
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sa"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, absolute_path_validator | my_file_ext_validator);

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.sa");
    }

    {
        const char * argv[] = {"./argument_parser_test", "-s", "relative/path/file.sa"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, absolute_path_validator | my_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absoulte/path/file.notValidExtension"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT, absolute_path_validator | my_file_ext_validator);

        EXPECT_THROW(parser.parse(), validation_failed);
    }

    // with temporary validators
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sa"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT,
                          regex_validator{"(/[^/]+)+/.*\\.[^/\\.]+$"} |
                          file_ext_validator{"sa", "so"});

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.sa");
    }

    // three validators
    {
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sa"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT,
                          regex_validator{"(/[^/]+)+/.*\\.[^/\\.]+$"} |
                          file_ext_validator{"sa", "so"} |
                          regex_validator{".*"});

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_value, "/absolute/path/file.sa");
    }

    // help page message
    {
        const char * argv[] = {"./argument_parser_test", "-h"};
        argument_parser parser("test_parser", 2, argv);
        parser.add_option(option_value, 's', "string-option", "desc",
                          option_spec::DEFAULT,
                          regex_validator{"(/[^/]+)+/.*\\.[^/\\.]+$"} |
                          file_ext_validator{"sa", "so"} |
                          regex_validator{".*"});

        testing::internal::CaptureStdout();
        EXPECT_EXIT(parser.parse(), ::testing::ExitedWithCode(EXIT_SUCCESS), "");
        std::string my_stdout = testing::internal::GetCapturedStdout();
        std::string expected = std::string("test_parser"
                               "==========="
                               "OPTIONS"
                               "    -s, --string-option (std::string)"
                               "          desc Value must match the pattern '(/[^/]+)+/.*\\.[^/\\.]+$'. "
                               "          File name extension must be one of [sa,so]."
                               "          Value must match the pattern '.*'."
                               "VERSION"
                               "    Last update: "
                               "    test_parser version: "
                               "    SeqAn version: ") + seqan3_version;
        EXPECT_TRUE(ranges::equal((my_stdout   | ranges::view::remove_if(is_space)),
                                   expected | ranges::view::remove_if(is_space)));
    }

    // chaining with a container option value type
    {
        std::vector<std::string> option_list_value{};
        const char * argv[] = {"./argument_parser_test", "-s", "/absolute/path/file.sa"};
        argument_parser parser("test_parser", 3, argv);
        parser.add_option(option_list_value, 's', "string-option", "desc",
                          option_spec::DEFAULT,
                          regex_validator{"(/[^/]+)+/.*\\.[^/\\.]+$"} | file_ext_validator{"sa", "so"});

        testing::internal::CaptureStderr();
        EXPECT_NO_THROW(parser.parse());
        EXPECT_TRUE((testing::internal::GetCapturedStderr()).empty());
        EXPECT_EQ(option_list_value[0], "/absolute/path/file.sa");
    }
}
