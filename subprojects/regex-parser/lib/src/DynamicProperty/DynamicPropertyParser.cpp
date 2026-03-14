// A Bison parser, made by GNU Bison 3.5.1.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.





#include "DynamicPropertyParser.hpp"


// Unqualified %code blocks.
#line 28 "src/DynamicProperty/DynamicPropertyParser.y"

    #include "Compiler.h"

    DynamicProperty::Parser::symbol_type dynprop_lex(yyscan_t scanner, DynamicProperty::Driver& driver);
    #define yylex dynprop_lex

#line 52 "src/DynamicProperty/DynamicPropertyParser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif



// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 4 "src/DynamicProperty/DynamicPropertyParser.y"
namespace DynamicProperty {
#line 125 "src/DynamicProperty/DynamicPropertyParser.cpp"


  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Parser::Parser (DynamicProperty::Driver& driver_yyarg, yyscan_t scanner_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      driver (driver_yyarg),
      scanner (scanner_yyarg)
  {}

  Parser::~Parser ()
  {}

  Parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/



  // by_state.
  Parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  Parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  Parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  Parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  Parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  Parser::symbol_number_type
  Parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[+state];
  }

  Parser::stack_symbol_type::stack_symbol_type ()
  {}

  Parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state))
  {
    switch (that.type_get ())
    {
      case 29: // assignment
        value.YY_MOVE_OR_COPY< DynamicProperty::AssignmentStatement > (YY_MOVE (that.value));
        break;

      case 30: // bool_expr
      case 31: // or_expr
      case 32: // and_expr
      case 33: // predicate
        value.YY_MOVE_OR_COPY< DynamicProperty::BooleanExpressionPtr > (YY_MOVE (that.value));
        break;

      case 26: // conditional
        value.YY_MOVE_OR_COPY< DynamicProperty::ConditionalAstPtr > (YY_MOVE (that.value));
        break;

      case 34: // value_expr
      case 35: // term
      case 36: // primary
        value.YY_MOVE_OR_COPY< DynamicProperty::ValueExpressionPtr > (YY_MOVE (that.value));
        break;

      case 24: // NUMBER
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case 22: // IDENT
      case 23: // STRING
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      case 27: // else_clause
      case 28: // assignment_list
        value.YY_MOVE_OR_COPY< std::vector<DynamicProperty::AssignmentStatement> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  Parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s)
  {
    switch (that.type_get ())
    {
      case 29: // assignment
        value.move< DynamicProperty::AssignmentStatement > (YY_MOVE (that.value));
        break;

      case 30: // bool_expr
      case 31: // or_expr
      case 32: // and_expr
      case 33: // predicate
        value.move< DynamicProperty::BooleanExpressionPtr > (YY_MOVE (that.value));
        break;

      case 26: // conditional
        value.move< DynamicProperty::ConditionalAstPtr > (YY_MOVE (that.value));
        break;

      case 34: // value_expr
      case 35: // term
      case 36: // primary
        value.move< DynamicProperty::ValueExpressionPtr > (YY_MOVE (that.value));
        break;

      case 24: // NUMBER
        value.move< int > (YY_MOVE (that.value));
        break;

      case 22: // IDENT
      case 23: // STRING
        value.move< std::string > (YY_MOVE (that.value));
        break;

      case 27: // else_clause
      case 28: // assignment_list
        value.move< std::vector<DynamicProperty::AssignmentStatement> > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 29: // assignment
        value.copy< DynamicProperty::AssignmentStatement > (that.value);
        break;

      case 30: // bool_expr
      case 31: // or_expr
      case 32: // and_expr
      case 33: // predicate
        value.copy< DynamicProperty::BooleanExpressionPtr > (that.value);
        break;

      case 26: // conditional
        value.copy< DynamicProperty::ConditionalAstPtr > (that.value);
        break;

      case 34: // value_expr
      case 35: // term
      case 36: // primary
        value.copy< DynamicProperty::ValueExpressionPtr > (that.value);
        break;

      case 24: // NUMBER
        value.copy< int > (that.value);
        break;

      case 22: // IDENT
      case 23: // STRING
        value.copy< std::string > (that.value);
        break;

      case 27: // else_clause
      case 28: // assignment_list
        value.copy< std::vector<DynamicProperty::AssignmentStatement> > (that.value);
        break;

      default:
        break;
    }

    return *this;
  }

  Parser::stack_symbol_type&
  Parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.type_get ())
    {
      case 29: // assignment
        value.move< DynamicProperty::AssignmentStatement > (that.value);
        break;

      case 30: // bool_expr
      case 31: // or_expr
      case 32: // and_expr
      case 33: // predicate
        value.move< DynamicProperty::BooleanExpressionPtr > (that.value);
        break;

      case 26: // conditional
        value.move< DynamicProperty::ConditionalAstPtr > (that.value);
        break;

      case 34: // value_expr
      case 35: // term
      case 36: // primary
        value.move< DynamicProperty::ValueExpressionPtr > (that.value);
        break;

      case 24: // NUMBER
        value.move< int > (that.value);
        break;

      case 22: // IDENT
      case 23: // STRING
        value.move< std::string > (that.value);
        break;

      case 27: // else_clause
      case 28: // assignment_list
        value.move< std::vector<DynamicProperty::AssignmentStatement> > (that.value);
        break;

      default:
        break;
    }

    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  Parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
  Parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " (";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  void
  Parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  Parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  Parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  Parser::state_type
  Parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::operator() ()
  {
    return parse ();
  }

  int
  Parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (scanner, driver));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case 29: // assignment
        yylhs.value.emplace< DynamicProperty::AssignmentStatement > ();
        break;

      case 30: // bool_expr
      case 31: // or_expr
      case 32: // and_expr
      case 33: // predicate
        yylhs.value.emplace< DynamicProperty::BooleanExpressionPtr > ();
        break;

      case 26: // conditional
        yylhs.value.emplace< DynamicProperty::ConditionalAstPtr > ();
        break;

      case 34: // value_expr
      case 35: // term
      case 36: // primary
        yylhs.value.emplace< DynamicProperty::ValueExpressionPtr > ();
        break;

      case 24: // NUMBER
        yylhs.value.emplace< int > ();
        break;

      case 22: // IDENT
      case 23: // STRING
        yylhs.value.emplace< std::string > ();
        break;

      case 27: // else_clause
      case 28: // assignment_list
        yylhs.value.emplace< std::vector<DynamicProperty::AssignmentStatement> > ();
        break;

      default:
        break;
    }



      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 59 "src/DynamicProperty/DynamicPropertyParser.y"
                                                  {
        yylhs.value.as < DynamicProperty::ConditionalAstPtr > () = std::make_shared<DynamicProperty::ConditionalAst>(yystack_[3].value.as < DynamicProperty::BooleanExpressionPtr > (), std::move(yystack_[1].value.as < std::vector<DynamicProperty::AssignmentStatement> > ()), std::move(yystack_[0].value.as < std::vector<DynamicProperty::AssignmentStatement> > ()));
        driver.set_result(yylhs.value.as < DynamicProperty::ConditionalAstPtr > ());
    }
#line 719 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 3:
#line 66 "src/DynamicProperty/DynamicPropertyParser.y"
           {
        yylhs.value.as < std::vector<DynamicProperty::AssignmentStatement> > () = {};
    }
#line 727 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 4:
#line 69 "src/DynamicProperty/DynamicPropertyParser.y"
                           {
        yylhs.value.as < std::vector<DynamicProperty::AssignmentStatement> > () = std::move(yystack_[0].value.as < std::vector<DynamicProperty::AssignmentStatement> > ());
    }
#line 735 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 5:
#line 75 "src/DynamicProperty/DynamicPropertyParser.y"
               {
        yylhs.value.as < std::vector<DynamicProperty::AssignmentStatement> > ().push_back(std::move(yystack_[0].value.as < DynamicProperty::AssignmentStatement > ()));
    }
#line 743 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 6:
#line 78 "src/DynamicProperty/DynamicPropertyParser.y"
                                     {
        yylhs.value.as < std::vector<DynamicProperty::AssignmentStatement> > () = std::move(yystack_[2].value.as < std::vector<DynamicProperty::AssignmentStatement> > ());
        yylhs.value.as < std::vector<DynamicProperty::AssignmentStatement> > ().push_back(std::move(yystack_[0].value.as < DynamicProperty::AssignmentStatement > ()));
    }
#line 752 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 7:
#line 85 "src/DynamicProperty/DynamicPropertyParser.y"
                            {
        yylhs.value.as < DynamicProperty::AssignmentStatement > () = DynamicProperty::AssignmentStatement(std::move(yystack_[2].value.as < std::string > ()), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 760 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 8:
#line 91 "src/DynamicProperty/DynamicPropertyParser.y"
            {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = yystack_[0].value.as < DynamicProperty::BooleanExpressionPtr > ();
    }
#line 768 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 9:
#line 97 "src/DynamicProperty/DynamicPropertyParser.y"
             {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = yystack_[0].value.as < DynamicProperty::BooleanExpressionPtr > ();
    }
#line 776 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 10:
#line 100 "src/DynamicProperty/DynamicPropertyParser.y"
                          {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::LogicalExpression>(DynamicProperty::LogicalOperator::OR, yystack_[2].value.as < DynamicProperty::BooleanExpressionPtr > (), yystack_[0].value.as < DynamicProperty::BooleanExpressionPtr > ());
    }
#line 784 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 11:
#line 106 "src/DynamicProperty/DynamicPropertyParser.y"
              {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = yystack_[0].value.as < DynamicProperty::BooleanExpressionPtr > ();
    }
#line 792 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 12:
#line 109 "src/DynamicProperty/DynamicPropertyParser.y"
                             {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::LogicalExpression>(DynamicProperty::LogicalOperator::AND, yystack_[2].value.as < DynamicProperty::BooleanExpressionPtr > (), yystack_[0].value.as < DynamicProperty::BooleanExpressionPtr > ());
    }
#line 800 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 13:
#line 115 "src/DynamicProperty/DynamicPropertyParser.y"
                            {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = yystack_[1].value.as < DynamicProperty::BooleanExpressionPtr > ();
    }
#line 808 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 14:
#line 118 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::EQUAL, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 816 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 15:
#line 121 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::NOT_EQUAL, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 824 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 16:
#line 124 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::LESS_THAN, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 832 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 17:
#line 127 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::LESS_OR_EQUAL, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 840 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 18:
#line 130 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::GREATER_THAN, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 848 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 19:
#line 133 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::GREATER_OR_EQUAL, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 856 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 20:
#line 136 "src/DynamicProperty/DynamicPropertyParser.y"
                                     {
        yylhs.value.as < DynamicProperty::BooleanExpressionPtr > () = std::make_shared<DynamicProperty::ComparisonExpression>(DynamicProperty::ComparisonOperator::CONTAINS, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 864 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 21:
#line 142 "src/DynamicProperty/DynamicPropertyParser.y"
         {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ();
    }
#line 872 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 22:
#line 145 "src/DynamicProperty/DynamicPropertyParser.y"
                           {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::ADD, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 880 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 23:
#line 148 "src/DynamicProperty/DynamicPropertyParser.y"
                            {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::SUBTRACT, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 888 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 24:
#line 154 "src/DynamicProperty/DynamicPropertyParser.y"
            {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ();
    }
#line 896 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 25:
#line 157 "src/DynamicProperty/DynamicPropertyParser.y"
                         {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::MULTIPLY, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 904 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 26:
#line 160 "src/DynamicProperty/DynamicPropertyParser.y"
                          {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::BinaryValueExpression>(DynamicProperty::ArithmeticOperator::DIVIDE, yystack_[2].value.as < DynamicProperty::ValueExpressionPtr > (), yystack_[0].value.as < DynamicProperty::ValueExpressionPtr > ());
    }
#line 912 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 27:
#line 166 "src/DynamicProperty/DynamicPropertyParser.y"
          {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::IdentifierExpression>(std::move(yystack_[0].value.as < std::string > ()));
    }
#line 920 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 28:
#line 169 "src/DynamicProperty/DynamicPropertyParser.y"
             {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::StringLiteralExpression>(std::move(yystack_[0].value.as < std::string > ()));
    }
#line 928 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 29:
#line 172 "src/DynamicProperty/DynamicPropertyParser.y"
             {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = std::make_shared<DynamicProperty::NumberLiteralExpression>(yystack_[0].value.as < int > ());
    }
#line 936 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;

  case 30:
#line 175 "src/DynamicProperty/DynamicPropertyParser.y"
                               {
        yylhs.value.as < DynamicProperty::ValueExpressionPtr > () = yystack_[1].value.as < DynamicProperty::ValueExpressionPtr > ();
    }
#line 944 "src/DynamicProperty/DynamicPropertyParser.cpp"
    break;


#line 948 "src/DynamicProperty/DynamicPropertyParser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yysyntax_error_ (yystack_[0].state, yyla));
      }


    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[+yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yy_error_token_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yy_error_token_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }


      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  Parser::error (const syntax_error& yyexc)
  {
    error (yyexc.what ());
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    std::ptrdiff_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */
    if (!yyla.empty ())
      {
        symbol_number_type yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];

        int yyn = yypact_[+yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yy_error_token_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char Parser::yypact_ninf_ = -8;

  const signed char Parser::yytable_ninf_ = -1;

  const signed char
  Parser::yypact_[] =
  {
       5,    14,    13,    14,    -8,    -8,    -8,    11,    24,    40,
      -8,    16,    26,    -8,    -8,    33,    -7,    18,    14,    14,
      19,    19,    19,    19,    19,    19,    19,    19,    19,    19,
      19,    -8,    -8,    44,    43,    -8,    40,    -8,    19,    34,
      34,    34,    34,    34,    34,    34,    26,    26,    -8,    -8,
      19,    18,    18,    -8,    -5,    34,    49,    -8
  };

  const signed char
  Parser::yydefact_[] =
  {
       0,     0,     0,     0,    27,    28,    29,     0,     8,     9,
      11,     0,    21,    24,     1,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    30,     0,     3,     5,    10,    12,     0,    14,
      15,    16,    17,    18,    19,    20,    22,    23,    25,    26,
       0,     0,     0,     2,     0,     7,     4,     6
  };

  const signed char
  Parser::yypgoto_[] =
  {
      -8,    -8,    -8,     7,     8,    58,    -8,    45,    46,    -3,
      25,    27
  };

  const signed char
  Parser::yydefgoto_[] =
  {
      -1,     2,    53,    34,    35,     7,     8,     9,    10,    11,
      12,    13
  };

  const signed char
  Parser::yytable_[] =
  {
      16,    20,    21,    22,    23,    24,    25,    26,     1,    27,
      28,    27,    28,    14,    32,    17,    32,    39,    40,    41,
      42,    43,    44,    45,    20,    21,    22,    23,    24,    25,
      26,    18,    27,    28,     3,    54,     4,     5,     6,    38,
      33,     4,     5,     6,    29,    30,    19,    55,    51,    52,
      27,    28,    46,    47,    31,    52,    48,    49,    56,    50,
      57,    15,     0,    36,     0,    37
  };

  const signed char
  Parser::yycheck_[] =
  {
       3,     8,     9,    10,    11,    12,    13,    14,     3,    16,
      17,    16,    17,     0,    21,     4,    21,    20,    21,    22,
      23,    24,    25,    26,     8,     9,    10,    11,    12,    13,
      14,     7,    16,    17,    20,    38,    22,    23,    24,    20,
      22,    22,    23,    24,    18,    19,     6,    50,     5,     6,
      16,    17,    27,    28,    21,     6,    29,    30,    51,    15,
      52,     3,    -1,    18,    -1,    19
  };

  const signed char
  Parser::yystos_[] =
  {
       0,     3,    26,    20,    22,    23,    24,    30,    31,    32,
      33,    34,    35,    36,     0,    30,    34,     4,     7,     6,
       8,     9,    10,    11,    12,    13,    14,    16,    17,    18,
      19,    21,    21,    22,    28,    29,    32,    33,    20,    34,
      34,    34,    34,    34,    34,    34,    35,    35,    36,    36,
      15,     5,     6,    27,    34,    34,    28,    29
  };

  const signed char
  Parser::yyr1_[] =
  {
       0,    25,    26,    27,    27,    28,    28,    29,    30,    31,
      31,    32,    32,    33,    33,    33,    33,    33,    33,    33,
      33,    34,    34,    34,    35,    35,    35,    36,    36,    36,
      36
  };

  const signed char
  Parser::yyr2_[] =
  {
       0,     2,     5,     0,     2,     1,     3,     3,     1,     1,
       3,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     1,     1,
       3
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const Parser::yytname_[] =
  {
  "$end", "error", "$undefined", "IF", "THEN", "ELSE", "AND", "OR", "EQ",
  "NE", "LT", "LE", "GT", "GE", "CONTAINS", "ASSIGN", "PLUS", "MINUS",
  "TIMES", "DIVIDE", "LPAREN", "RPAREN", "IDENT", "STRING", "NUMBER",
  "$accept", "conditional", "else_clause", "assignment_list", "assignment",
  "bool_expr", "or_expr", "and_expr", "predicate", "value_expr", "term",
  "primary", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned char
  Parser::yyrline_[] =
  {
       0,    59,    59,    66,    69,    75,    78,    85,    91,    97,
     100,   106,   109,   115,   118,   121,   124,   127,   130,   133,
     136,   142,   145,   148,   154,   157,   160,   166,   169,   172,
     175
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


#line 4 "src/DynamicProperty/DynamicPropertyParser.y"
} // DynamicProperty
#line 1365 "src/DynamicProperty/DynamicPropertyParser.cpp"

#line 180 "src/DynamicProperty/DynamicPropertyParser.y"


void DynamicProperty::Parser::error(const std::string& msg) {
    driver.set_error(msg);
}
