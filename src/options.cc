/* Handles parsing the Options provided to the user.
   Copyright (C) 1989-1998, 2000, 2002 Free Software Foundation, Inc.
   written by Douglas C. Schmidt (schmidt@ics.uci.edu)

This file is part of GNU GPERF.

GNU GPERF is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU GPERF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU GPERF; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111, USA.  */

#include <stdio.h>
#include <stdlib.h> /* declares atoi(), abs(), exit() */
#include <string.h> /* declares strcmp() */
#include <ctype.h>  /* declares isdigit() */
#include "getopt.h"
#include "options.h"
#include "vectors.h"
#include "version.h"

/* Global option coordinator for the entire program. */
Options option;

/* Records the program name. */
const char *program_name;

/* Size to jump on a collision. */
static const int DEFAULT_JUMP_VALUE = 5;

/* Default name for generated lookup function. */
static const char *const DEFAULT_NAME = "in_word_set";

/* Default name for the key component. */
static const char *const DEFAULT_KEY = "name";

/* Default struct initializer suffix. */
static const char *const DEFAULT_INITIALIZER_SUFFIX = "";

/* Default name for the generated class. */
static const char *const DEFAULT_CLASS_NAME = "Perfect_Hash";

/* Default name for generated hash function. */
static const char *const DEFAULT_HASH_NAME = "hash";

/* Default name for generated hash table array. */
static const char *const DEFAULT_WORDLIST_NAME = "wordlist";

/* Default delimiters that separate keywords from their attributes. */
static const char *const DEFAULT_DELIMITERS = ",\n";

int Options::_option_word;
int Options::_total_switches;
int Options::_total_keysig_size;
int Options::_size;
int Options::_key_pos;
int Options::_jump;
int Options::_initial_asso_value;
int Options::_argument_count;
int Options::_iterations;
char **Options::_argument_vector;
const char *Options::_function_name;
const char *Options::_key_name;
const char *Options::_initializer_suffix;
const char *Options::_class_name;
const char *Options::_hash_name;
const char *Options::_wordlist_name;
const char *Options::_delimiters;
char Options::_key_positions[MAX_KEY_POS];

/* Prints program usage to given stream. */

void
Options::short_usage (FILE * strm)
{
  fprintf (strm, "Usage: %s [-cCdDef[num]F<initializers>GhH<hashname>i<init>Ijk<keys>K<keyname>lL<language>nN<function name>ors<size>S<switches>tTvW<wordlistname>Z<class name>7] [input-file]\n"
                 "Try `%s --help' for more information.\n",
                 program_name, program_name);
}

void
Options::long_usage (FILE * strm)
{
  fprintf (strm,
           "GNU `gperf' generates perfect hash functions.\n"
           "\n"
           "Usage: %s [OPTION]... [INPUT-FILE]\n"
           "\n"
           "If a long option shows an argument as mandatory, then it is mandatory\n"
           "for the equivalent short option also.\n"
           "\n"
           "Input file interpretation:\n"
           "  -e, --delimiters=DELIMITER-LIST\n"
           "                         Allow user to provide a string containing delimiters\n"
           "                         used to separate keywords from their attributes.\n"
           "                         Default is \",\\n\".\n"
           "  -t, --struct-type      Allows the user to include a structured type\n"
           "                         declaration for generated code. Any text before %%%%\n"
           "                         is considered part of the type declaration. Key\n"
           "                         words and additional fields may follow this, one\n"
           "                         group of fields per line.\n"
           "\n"
           "Language for the output code:\n"
           "  -L, --language=LANGUAGE-NAME\n"
           "                         Generates code in the specified language. Languages\n"
           "                         handled are currently C++, ANSI-C, C, and KR-C. The\n"
           "                         default is C.\n"
           "\n"
           "Details in the output code:\n"
           "  -K, --slot-name=NAME   Select name of the keyword component in the keyword\n"
           "                         structure.\n"
           "  -F, --initializer-suffix=INITIALIZERS\n"
           "                         Initializers for additional components in the keyword\n"
           "                         structure.\n"
           "  -H, --hash-fn-name=NAME\n"
           "                         Specify name of generated hash function. Default is\n"
           "                         `hash'.\n"
           "  -N, --lookup-fn-name=NAME\n"
           "                         Specify name of generated lookup function. Default\n"
           "                         name is `in_word_set'.\n"
           "  -Z, --class-name=NAME  Specify name of generated C++ class. Default name is\n"
           "                         `Perfect_Hash'.\n"
           "  -7, --seven-bit        Assume 7-bit characters.\n"
           "  -c, --compare-strncmp  Generate comparison code using strncmp rather than\n"
           "                         strcmp.\n"
           "  -C, --readonly-tables  Make the contents of generated lookup tables\n"
           "                         constant, i.e., readonly.\n"
           "  -E, --enum             Define constant values using an enum local to the\n"
           "                         lookup function rather than with defines.\n"
           "  -I, --includes         Include the necessary system include file <string.h>\n"
           "                         at the beginning of the code.\n"
           "  -G, --global           Generate the static table of keywords as a static\n"
           "                         global variable, rather than hiding it inside of the\n"
           "                         lookup function (which is the default behavior).\n"
           "  -W, --word-array-name=NAME\n"
           "                         Specify name of word list array. Default name is\n"
           "                         `wordlist'.\n"
           "  -S, --switch=COUNT     Causes the generated C code to use a switch\n"
           "                         statement scheme, rather than an array lookup table.\n"
           "                         This can lead to a reduction in both time and space\n"
           "                         requirements for some keyfiles. The COUNT argument\n"
           "                         determines how many switch statements are generated.\n"
           "                         A value of 1 generates 1 switch containing all the\n"
           "                         elements, a value of 2 generates 2 tables with 1/2\n"
           "                         the elements in each table, etc. If COUNT is very\n"
           "                         large, say 1000000, the generated C code does a\n"
           "                         binary search.\n"
           "  -T, --omit-struct-type\n"
           "                         Prevents the transfer of the type declaration to the\n"
           "                         output file. Use this option if the type is already\n"
           "                         defined elsewhere.\n"
           "\n"
           "Algorithm employed by gperf:\n"
           "  -k, --key-positions=KEYS\n"
           "                         Select the key positions used in the hash function.\n"
           "                         The allowable choices range between 1-%d, inclusive.\n"
           "                         The positions are separated by commas, ranges may be\n"
           "                         used, and key positions may occur in any order.\n"
           "                         Also, the meta-character '*' causes the generated\n"
           "                         hash function to consider ALL key positions, and $\n"
           "                         indicates the ``final character'' of a key, e.g.,\n"
           "                         $,1,2,4,6-10.\n"
           "  -l, --compare-strlen   Compare key lengths before trying a string\n"
           "                         comparison. This helps cut down on the number of\n"
           "                         string comparisons made during the lookup.\n"
           "  -D, --duplicates       Handle keywords that hash to duplicate values. This\n"
           "                         is useful for certain highly redundant keyword sets.\n"
           "  -f, --fast=ITERATIONS  Generate the gen-perf.hash function ``fast''. This\n"
           "                         decreases gperf's running time at the cost of\n"
           "                         minimizing generated table size. The numeric\n"
           "                         argument represents the number of times to iterate\n"
           "                         when resolving a collision. `0' means ``iterate by\n"
           "                         the number of keywords''.\n"
           "  -i, --initial-asso=N   Provide an initial value for the associate values\n"
           "                         array. Default is 0. Setting this value larger helps\n"
           "                         inflate the size of the final table.\n"
           "  -j, --jump=JUMP-VALUE  Affects the ``jump value'', i.e., how far to advance\n"
           "                         the associated character value upon collisions. Must\n"
           "                         be an odd number, default is %d.\n"
           "  -n, --no-strlen        Do not include the length of the keyword when\n"
           "                         computing the hash function.\n"
           "  -o, --occurrence-sort  Reorders input keys by frequency of occurrence of\n"
           "                         the key sets. This should decrease the search time\n"
           "                         dramatically.\n"
           "  -r, --random           Utilizes randomness to initialize the associated\n"
           "                         values table.\n"
           "  -s, --size-multiple=N  Affects the size of the generated hash table. The\n"
           "                         numeric argument N indicates ``how many times larger\n"
           "                         or smaller'' the associated value range should be,\n"
           "                         in relationship to the number of keys, e.g. a value\n"
           "                         of 3 means ``allow the maximum associated value to\n"
           "                         be about 3 times larger than the number of input\n"
           "                         keys.'' Conversely, a value of -3 means ``make the\n"
           "                         maximum associated value about 3 times smaller than\n"
           "                         the number of input keys. A larger table should\n"
           "                         decrease the time required for an unsuccessful\n"
           "                         search, at the expense of extra table space. Default\n"
           "                         value is 1.\n"
           "\n"
           "Informative output:\n"
           "  -h, --help             Print this message.\n"
           "  -v, --version          Print the gperf version number.\n"
           "  -d, --debug            Enables the debugging option (produces verbose\n"
           "                         output to the standard error).\n"
           "\n"
           "Report bugs to <bug-gnu-utils@gnu.org>.\n"
           , program_name, MAX_KEY_POS - 1, DEFAULT_JUMP_VALUE);
}

/* Output command-line Options. */

void
Options::print_options ()
{
  int i;

  printf ("/* Command-line: ");

  for (i = 0; i < _argument_count; i++)
    {
      const char *arg = _argument_vector[i];

      /* Escape arg if it contains shell metacharacters. */
      if (*arg == '-')
        {
          putchar (*arg);
          arg++;
          if (*arg >= 'A' && *arg <= 'Z' || *arg >= 'a' && *arg <= 'z')
            {
              putchar (*arg);
              arg++;
            }
        }
      if (strpbrk (arg, "\t\n !\"#$&'()*;<>?[\\]`{|}~") != NULL)
        {
          if (strchr (arg, '\'') != NULL)
            {
              putchar ('"');
              for (; *arg; arg++)
                {
                  if (*arg == '\"' || *arg == '\\' || *arg == '$')
                    putchar ('\\');
                  putchar (*arg);
                }
              putchar ('"');
            }
          else
            {
              putchar ('\'');
              for (; *arg; arg++)
                {
                  if (*arg == '\\')
                    putchar ('\\');
                  putchar (*arg);
                }
              putchar ('\'');
            }
        }
      else
        printf ("%s", arg);

      printf (" ");
    }

  printf (" */");
}

/* Parses a string denoting key positions.  */

class PositionStringParser
{
public:
  PositionStringParser (const char *s, int lo, int hi, int word_end, int bad_val, int key_end);
  int nextPosition ();
private:
  const char *str;  /* A pointer to the string provided by the user. */
  int end;          /* Value returned after last key is processed. */
  int end_word;     /* A value marking the abstract ``end of word'' ( usually '$'). */
  int error_value;  /* Error value returned when input is syntactically erroneous. */
  int hi_bound;     /* Greatest possible value, inclusive. */
  int lo_bound;     /* Smallest possible value, inclusive. */
  int size;
  int curr_value;
  int upper_bound;
};

PositionStringParser::PositionStringParser (const char *s, int lo, int hi, int word_end, int bad_val, int key_end)
  : str (s), end (key_end), end_word (word_end), error_value (bad_val), hi_bound (hi), lo_bound (lo),
    size (0), curr_value (0), upper_bound (0)
{
}

int PositionStringParser::nextPosition ()
{
  if (size)
    {
      if (++curr_value >= upper_bound)
        size = 0;
      return curr_value;
    }
  else
    {
      while (*str)
        switch (*str)
          {
          default: return error_value;
          case ',': str++; break;
          case '$': str++; return end_word;
          case '0': case '1': case '2': case '3': case '4':
          case '5': case '6': case '7': case '8': case '9':
            for (curr_value = 0; isdigit ((unsigned char)(*str)); str++)
              curr_value = curr_value * 10 + (*str - '0');

            if (*str == '-')
              {

                for (size = 1, upper_bound = 0;
                     isdigit ((unsigned char)(*++str));
                     upper_bound = upper_bound * 10 + (*str - '0'));

                if (upper_bound <= curr_value || upper_bound > hi_bound)
                  return error_value;
              }
            return curr_value >= lo_bound && curr_value <= hi_bound
              ? curr_value : error_value;
          }

      return end;
    }
}

/* Sorts the key positions *IN REVERSE ORDER!!*
   This makes further routines more efficient.  Especially when generating code.
   Uses a simple Insertion Sort since the set is probably ordered.
   Returns 1 if there are no duplicates, 0 otherwise. */

inline int
Options::key_sort (char *base, int len)
{
  /* Bubble sort.  */
  for (int i = 1; i < len; i++)
    {
      int j;
      int tmp;

      for (j = i, tmp = base[j]; j > 0 && tmp >= base[j - 1]; j--)
        if ((base[j] = base[j - 1]) == tmp) /* oh no, a duplicate!!! */
          return 0;

      base[j] = tmp;
    }

  return 1;
}

/* Sets the default Options. */

Options::Options ()
{
  _key_positions[0]   = WORD_START;
  _key_positions[1]   = WORD_END;
  _key_positions[2]   = EOS;
  _total_keysig_size  = 2;
  _delimiters         = DEFAULT_DELIMITERS;
  _jump               = DEFAULT_JUMP_VALUE;
  _option_word        = DEFAULTCHARS | C;
  _function_name      = DEFAULT_NAME;
  _key_name           = DEFAULT_KEY;
  _initializer_suffix = DEFAULT_INITIALIZER_SUFFIX;
  _hash_name          = DEFAULT_HASH_NAME;
  _wordlist_name      = DEFAULT_WORDLIST_NAME;
  _class_name         = DEFAULT_CLASS_NAME;
  _size               = 1;
  _total_switches     = 1;
  _iterations         = 0;
  _initial_asso_value = 0;
}

/* Dumps option status when debug is set. */

Options::~Options ()
{
  if (_option_word & DEBUG)
    {
      char *ptr;

      fprintf (stderr, "\ndumping Options:"
               "\nDEBUG is.......: %s"
               "\nORDER is.......: %s"
               "\nTYPE is........: %s"
               "\nRANDOM is......: %s"
               "\nDEFAULTCHARS is: %s"
               "\nSWITCH is......: %s"
               "\nNOLENGTH is....: %s"
               "\nLENTABLE is....: %s"
               "\nDUP is.........: %s"
               "\nFAST is........: %s"
               "\nCOMP is........: %s"
               "\nNOTYPE is......: %s"
               "\nGLOBAL is......: %s"
               "\nCONST is.......: %s"
               "\nKRC is.........: %s"
               "\nC is...........: %s"
               "\nANSIC is.......: %s"
               "\nCPLUSPLUS is...: %s"
               "\nENUM is........: %s"
               "\nINCLUDE is.....: %s"
               "\nSEVENBIT is....: %s"
               "\niterations = %d"
               "\nlookup function name = %s"
               "\nhash function name = %s"
               "\nword list name = %s"
               "\nkey name = %s"
               "\ninitializer suffix = %s"
               "\njump value = %d"
               "\nmax associated value = %d"
               "\ninitial associated value = %d"
               "\ndelimiters = %s"
               "\nnumber of switch statements = %d\n",
               _option_word & DEBUG ? "enabled" : "disabled",
               _option_word & ORDER ? "enabled" : "disabled",
               _option_word & TYPE ? "enabled" : "disabled",
               _option_word & RANDOM ? "enabled" : "disabled",
               _option_word & DEFAULTCHARS ? "enabled" : "disabled",
               _option_word & SWITCH ? "enabled" : "disabled",
               _option_word & NOLENGTH ? "enabled" : "disabled",
               _option_word & LENTABLE ? "enabled" : "disabled",
               _option_word & DUP ? "enabled" : "disabled",
               _option_word & FAST ? "enabled" : "disabled",
               _option_word & COMP ? "enabled" : "disabled",
               _option_word & NOTYPE ? "enabled" : "disabled",
               _option_word & GLOBAL ? "enabled" : "disabled",
               _option_word & CONST ? "enabled" : "disabled",
               _option_word & KRC ? "enabled" : "disabled",
               _option_word & C ? "enabled" : "disabled",
               _option_word & ANSIC ? "enabled" : "disabled",
               _option_word & CPLUSPLUS ? "enabled" : "disabled",
               _option_word & ENUM ? "enabled" : "disabled",
               _option_word & INCLUDE ? "enabled" : "disabled",
               _option_word & SEVENBIT ? "enabled" : "disabled",
               _iterations,
               _function_name, _hash_name, _wordlist_name, _key_name,
               _initializer_suffix, _jump, _size - 1, _initial_asso_value,
               _delimiters, _total_switches);
      if (_option_word & ALLCHARS)
        fprintf (stderr, "all characters are used in the hash function\n");

      fprintf (stderr, "maximum keysig size = %d\nkey positions are: \n",
               _total_keysig_size);

      for (ptr = _key_positions; *ptr != EOS; ptr++)
        if (*ptr == WORD_END)
          fprintf (stderr, "$\n");
        else
          fprintf (stderr, "%d\n", *ptr);

      fprintf (stderr, "finished dumping Options\n");
    }
}


/* Parses the command line Options and sets appropriate flags in option_word. */

static const struct option long_options[] =
{
  { "delimiters", required_argument, 0, 'e' },
  { "struct-type", no_argument, 0, 't' },
  { "language", required_argument, 0, 'L' },
  { "slot-name", required_argument, 0, 'K' },
  { "initializer-suffix", required_argument, 0, 'F' },
  { "hash-fn-name", required_argument, 0, 'H' },
  { "lookup-fn-name", required_argument, 0, 'N' },
  { "class-name", required_argument, 0, 'Z' },
  { "seven-bit", no_argument, 0, '7' },
  { "compare-strncmp", no_argument, 0, 'c' },
  { "readonly-tables", no_argument, 0, 'C' },
  { "enum", no_argument, 0, 'E' },
  { "includes", no_argument, 0, 'I' },
  { "global", no_argument, 0, 'G' },
  { "word-array-name", required_argument, 0, 'W' },
  { "switch", required_argument, 0, 'S' },
  { "omit-struct-type", no_argument, 0, 'T' },
  { "key-positions", required_argument, 0, 'k' },
  { "compare-strlen", no_argument, 0, 'l' },
  { "duplicates", no_argument, 0, 'D' },
  { "fast", required_argument, 0, 'f' },
  { "initial-asso", required_argument, 0, 'i' },
  { "jump", required_argument, 0, 'j' },
  { "no-strlen", no_argument, 0, 'n' },
  { "occurrence-sort", no_argument, 0, 'o' },
  { "random", no_argument, 0, 'r' },
  { "size-multiple", required_argument, 0, 's' },
  { "help", no_argument, 0, 'h' },
  { "version", no_argument, 0, 'v' },
  { "debug", no_argument, 0, 'd' },
  { 0, no_argument, 0, 0 }
};

void
Options::operator() (int argc, char *argv[])
{
  int    option_char;

  program_name = argv[0];
  _argument_count  = argc;
  _argument_vector = argv;

  while ((option_char =
            getopt_long (_argument_count, _argument_vector,
                         "adcCDe:Ef:F:gGhH:i:Ij:k:K:lL:nN:oprs:S:tTvW:Z:7",
                         long_options, NULL))
         != -1)
    {
      switch (option_char)
        {
        case 'a':               /* Generated code uses the ANSI prototype format. */
          break;                /* This is now the default. */
        case 'c':               /* Generate strncmp rather than strcmp. */
          {
            _option_word |= COMP;
            break;
          }
        case 'C':               /* Make the generated tables readonly (const). */
          {
            _option_word |= CONST;
            break;
          }
        case 'd':               /* Enable debugging option. */
          {
            _option_word |= DEBUG;
            fprintf (stderr, "Starting program %s, version %s, with debugging on.\n",
                             program_name, version_string);
            break;
          }
        case 'D':               /* Enable duplicate option. */
          {
            _option_word |= DUP;
            break;
          }
        case 'e': /* Allows user to provide keyword/attribute separator */
          {
            _delimiters = /*getopt*/optarg;
            break;
          }
        case 'E':
          {
            _option_word |= ENUM;
            break;
          }
        case 'f':               /* Generate the hash table ``fast.'' */
          {
            _option_word |= FAST;
            if ((_iterations = atoi (/*getopt*/optarg)) < 0)
              {
                fprintf (stderr, "iterations value must not be negative, assuming 0\n");
                _iterations = 0;
              }
            break;
          }
        case 'F':
          {
            _initializer_suffix = /*getopt*/optarg;
            break;
          }
        case 'g':               /* Use the ``inline'' keyword for generated sub-routines, ifdef __GNUC__. */
          break;                /* This is now the default. */
        case 'G':               /* Make the keyword table a global variable. */
          {
            _option_word |= GLOBAL;
            break;
          }
        case 'h':               /* Displays a list of helpful Options to the user. */
          {
            long_usage (stdout);
            exit (0);
          }
        case 'H':               /* Sets the name for the hash function */
          {
            _hash_name = /*getopt*/optarg;
            break;
          }
        case 'i':               /* Sets the initial value for the associated values array. */
          {
            if ((_initial_asso_value = atoi (/*getopt*/optarg)) < 0)
              fprintf (stderr, "Initial value %d should be non-zero, ignoring and continuing.\n", _initial_asso_value);
            if (option[RANDOM])
              fprintf (stderr, "warning, -r option superceeds -i, ignoring -i option and continuing\n");
            break;
          }
        case 'I':               /* Enable #include statements. */
          {
            _option_word |= INCLUDE;
            break;
          }
        case 'j':               /* Sets the jump value, must be odd for later algorithms. */
          {
            if ((_jump = atoi (/*getopt*/optarg)) < 0)
              {
                fprintf (stderr, "Jump value %d must be a positive number.\n", _jump);
                short_usage (stderr);
                exit (1);
              }
            else if (_jump && ((_jump % 2) == 0))
              fprintf (stderr, "Jump value %d should be odd, adding 1 and continuing...\n", _jump++);
            break;
          }
        case 'k':               /* Sets key positions used for hash function. */
          {
            const int BAD_VALUE = -1;
            int       value;
            PositionStringParser sparser (/*getopt*/optarg, 1, MAX_KEY_POS - 1, WORD_END, BAD_VALUE, EOS);

            if (/*getopt*/optarg [0] == '*') /* Use all the characters for hashing!!!! */
              _option_word = (_option_word & ~DEFAULTCHARS) | ALLCHARS;
            else
              {
                char *key_pos;

                for (key_pos = _key_positions; (value = sparser.nextPosition()) != EOS; key_pos++)
                  if (value == BAD_VALUE)
                    {
                      fprintf (stderr, "Illegal key value or range, use 1,2,3-%d,'$' or '*'.\n",
                                       MAX_KEY_POS - 1);
                      short_usage (stderr);
                      exit (1);
                    }
                  else
                    *key_pos = value;;

                *key_pos = EOS;

                if (! (_total_keysig_size = (key_pos - _key_positions)))
                  {
                    fprintf (stderr, "No keys selected.\n");
                    short_usage (stderr);
                    exit (1);
                  }
                else if (! key_sort (_key_positions, _total_keysig_size))
                  {
                    fprintf (stderr, "Duplicate keys selected\n");
                    short_usage (stderr);
                    exit (1);
                  }

                if (_total_keysig_size != 2
                    || (_key_positions[0] != 1 || _key_positions[1] != WORD_END))
                  _option_word &= ~DEFAULTCHARS;
              }
            break;
          }
        case 'K':               /* Make this the keyname for the keyword component field. */
          {
            _key_name = /*getopt*/optarg;
            break;
          }
        case 'l':               /* Create length table to avoid extra string compares. */
          {
            _option_word |= LENTABLE;
            break;
          }
        case 'L':               /* Deal with different generated languages. */
          {
            _option_word &= ~(KRC | C | ANSIC | CPLUSPLUS);
            if (!strcmp (/*getopt*/optarg, "KR-C"))
              _option_word |= KRC;
            else if (!strcmp (/*getopt*/optarg, "C"))
              _option_word |= C;
            else if (!strcmp (/*getopt*/optarg, "ANSI-C"))
              _option_word |= ANSIC;
            else if (!strcmp (/*getopt*/optarg, "C++"))
              _option_word |= CPLUSPLUS;
            else
              {
                fprintf (stderr, "unsupported language option %s, defaulting to C\n", /*getopt*/optarg);
                _option_word |= C;
              }
            break;
          }
        case 'n':               /* Don't include the length when computing hash function. */
          {
            _option_word |= NOLENGTH;
            break;
          }
        case 'N':               /* Make generated lookup function name be optarg */
          {
            _function_name = /*getopt*/optarg;
            break;
          }
        case 'o':               /* Order input by frequency of key set occurrence. */
          {
            _option_word |= ORDER;
            break;
          }
        case 'p':               /* Generated lookup function a pointer instead of int. */
          break;                /* This is now the default. */
        case 'r':               /* Utilize randomness to initialize the associated values table. */
          {
            _option_word |= RANDOM;
            if (_initial_asso_value != 0)
              fprintf (stderr, "warning, -r option superceeds -i, disabling -i option and continuing\n");
            break;
          }
        case 's':               /* Range of associated values, determines size of final table. */
          {
            if (abs (_size = atoi (/*getopt*/optarg)) > 50)
              fprintf (stderr, "%d is excessive, did you really mean this?! (try `%s --help' for help)\n", _size, program_name);
            break;
          }
        case 'S':               /* Generate switch statement output, rather than lookup table. */
          {
            _option_word |= SWITCH;
            if ((_total_switches = atoi (/*getopt*/optarg)) <= 0)
              {
                fprintf (stderr, "number of switches %s must be a positive number\n", /*getopt*/optarg);
                short_usage (stderr);
                exit (1);
              }
            break;
          }
        case 't':               /* Enable the TYPE mode, allowing arbitrary user structures. */
          {
            _option_word |= TYPE;
            break;
          }
        case 'T':   /* Don't print structure definition. */
          {
            _option_word |= NOTYPE;
            break;
          }
        case 'v':               /* Print out the version and quit. */
          fprintf (stdout, "GNU gperf %s\n", version_string);
          exit (0);
        case 'W':               /* Sets the name for the hash table array */
          {
            _wordlist_name = /*getopt*/optarg;
            break;
          }
        case 'Z':               /* Set the class name. */
          {
            _class_name = /*getopt*/optarg;
            break;
          }
        case '7':               /* Assume 7-bit characters. */
          {
            _option_word |= SEVENBIT;
            Vectors::ALPHA_SIZE = 128;
            break;
          }
        default:
          short_usage (stderr);
          exit (1);
        }

    }

  if (argv[/*getopt*/optind] && ! freopen (argv[/*getopt*/optind], "r", stdin))
    {
      fprintf (stderr, "Cannot open keyword file `%s'\n", argv[/*getopt*/optind]);
      short_usage (stderr);
      exit (1);
    }

  if (++/*getopt*/optind < argc)
    {
      fprintf (stderr, "Extra trailing arguments to %s.\n", program_name);
      short_usage (stderr);
      exit (1);
    }
}

#ifndef __OPTIMIZE__

#define INLINE /* not inline */
#include "options.icc"
#undef INLINE

#endif /* not defined __OPTIMIZE__ */
