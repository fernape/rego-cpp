#include "internal.hh"
#include "rego.hh"

namespace rego
{
  const std::string alpha =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const std::string numeric = "0123456789";
  const std::string alphanumeric = alpha + numeric;
  const std::size_t max_string_length = 10;

  template <typename T>
  std::string rand_string(T& rnd)
  {
    // TODO add valid non-alphanum characters
    std::ostringstream buf;
    buf << '"';
    std::size_t length = rnd() % max_string_length;
    for (std::size_t i = 0; i < length; ++i)
    {
      buf << alphanumeric[rnd() % alphanumeric.size()];
    }
    buf << '"';
    return buf.str();
  }

  template <typename T>
  std::string rand_raw_string(T& rnd)
  {
    // TODO add valid non-alphanum characters
    std::ostringstream buf;
    buf << '`';
    std::size_t length = rnd() % max_string_length;
    for (std::size_t i = 0; i < length; ++i)
    {
      buf << alphanumeric[rnd() % alphanumeric.size()];
    }
    buf << '`';
    return buf.str();
  }

  Parse parser()
  {
    Parse p(depth::file, wf_parser);

    p("start",
      {
        // A newline sometimes terminates.
        "\r*\n([[:blank:]]*)" >>
          [](auto& m) {
            if (m.in(List))
            {
              return;
            }

            m.term({List});
            if (m.in(Some))
            {
              m.pop(Some);
              m.term();
            }
            if (m.in(With))
            {
              m.pop(With);
              m.term();
            }
          },

        // Whitespace between tokens.
        "[[:blank:]]+" >> [](auto&) {},

        // terminator
        ";" >> [](auto& m) { m.term(); },

        // Assign.
        ":=" >> [](auto& m) { m.add(Assign); },

        // Key/Value
        ":" >> [](auto& m) { m.add(Colon); },

        // List
        "," >> [](auto& m) { m.seq(List); },

        // Or
        "\\|" >> [](auto& m) { m.add(Or); },

        // And
        "&" >> [](auto& m) { m.add(And); },

        // OPA local variable.
        R"(__[[:alnum:]]+__\b)" >> [](auto& m) { m.add(Var); },

        // single underscore variable
        R"(_[[:alnum:]_]+\b)" >> [](auto& m) { m.add(Var); },

        // Placeholder
        "_" >> [](auto& m) { m.add(Placeholder); },

        // Brace.
        R"((\{)[[:blank:]]*)" >> [](auto& m) { m.push(Brace, 1); },

        R"(\})" >>
          [](auto& m) {
            m.term({List});
            if (m.in(Some))
            {
              m.pop(Some);
              m.term();
            }
            if (m.in(With))
            {
              m.pop(With);
              m.term();
            }
            m.pop(Brace);
          },

        // Square.
        R"((\[)[[:blank:]]*)" >> [](auto& m) { m.push(Square, 1); },

        R"(\])" >>
          [](auto& m) {
            m.term({List});
            if (m.in(Some))
            {
              m.pop(Some);
              m.term();
            }
            if (m.in(With))
            {
              m.pop(With);
              m.term();
            }
            m.pop(Square);
          },

        // Parens.
        R"((\()[[:blank:]]*)" >>
          [](auto& m) {
            m.push(Paren, 1);
            // deal with empty parens
            m.push(Group);
          },

        R"(\))" >>
          [](auto& m) {
            m.term({List});
            if (m.in(Some))
            {
              m.pop(Some);
              m.term();
            }
            if (m.in(With))
            {
              m.pop(With);
              m.term();
            }
            m.pop(Paren);
          },

        // Float.
        R"([[:digit:]]+\.[[:digit:]]+(?:e[+-]?[[:digit:]]+)?\b)" >>
          [](auto& m) { m.add(Float); },

        // String.
        R"("(?:\\(?:["\\\/bfnrtx]|u[a-fA-F0-9]{4})|[^"\\\0-\x1F\x7F]+)*")" >>
          [](auto& m) { m.add(JSONString); },

        // Raw string.
        R"(`[^`]*`)" >> [](auto& m) { m.add(RawString); },

        // Int.
        R"([[:digit:]]+\b)" >> [](auto& m) { m.add(Int); },

        // Float with exponent but no decimal.
        R"([[:digit:]]+(?:e[+-]?[[:digit:]]+)?\b)" >>
          [](auto& m) { m.add(Float); },

        // True.
        "true\\b" >> [](auto& m) { m.add(True); },

        // False.
        "false\\b" >> [](auto& m) { m.add(False); },

        // Null.
        "null\\b" >> [](auto& m) { m.add(Null); },

        // Default.
        "default\\b" >> [](auto& m) { m.add(Default); },

        // Some.
        "some\\b" >> [](auto& m) { m.push(Some); },

        // Else.
        "else\\b" >> [](auto& m) { m.add(Else); },

        // Import
        "import\\b" >> [](auto& m) { m.add(Import); },

        // As
        "as\\b" >>
          [](auto& m) {
            m.term();
            m.add(As);
          },

        // With
        "with\\b" >>
          [](auto& m) {
            m.term();
            if (m.in(With))
            {
              m.pop(With);
            }
            m.push(With);
          },

        // Empty set.
        R"(set\(\))" >> [](auto& m) { m.add(EmptySet); },

        // Not
        "not\\b" >> [](auto& m) { m.add(Not); },

        // Dot.
        R"(\.)" >> [](auto& m) { m.add(Dot); },

        // Line comment.
        "#[^\n]*" >> [](auto&) {},

        // Package.
        R"(package\b)" >> [](auto& m) { m.add(Package); },

        // Query.
        R"(query\b)" >> [](auto& m) { m.add(Query); },

        // Identifier.
        R"([_[:alpha:]][_[:alnum:]]*\b)" >> [](auto& m) { m.add(Var); },

        // Add ('+' is a reserved RegEx character)
        R"(\+)" >> [](auto& m) { m.add(Add); },

        // Subtract
        "-" >> [](auto& m) { m.add(Subtract); },

        // Multiply ('*' is a reserved RegEx character)
        R"(\*)" >> [](auto& m) { m.add(Multiply); },

        // Divide
        "/" >> [](auto& m) { m.add(Divide); },

        // Modulo
        "%" >> [](auto& m) { m.add(Modulo); },

        // Equals
        "==" >> [](auto& m) { m.add(Equals); },

        // NotEquals
        "!=" >> [](auto& m) { m.add(NotEquals); },

        // LessThanOrEquals
        "<=" >> [](auto& m) { m.add(LessThanOrEquals); },

        // Less than
        "<" >> [](auto& m) { m.add(LessThan); },

        // GreaterThanOrEquals
        ">=" >> [](auto& m) { m.add(GreaterThanOrEquals); },

        // GreaterThan
        ">" >> [](auto& m) { m.add(GreaterThan); },

        // Unify
        "=" >> [](auto& m) { m.add(Unify); },

      });

    p.done([](auto& m) { m.term({List, Some, With}); });

    p.gen({
      Int >> [](auto& rnd) { return std::to_string(rnd() % 100); },
      Float >>
        [](auto& rnd) {
          std::uniform_real_distribution<> dist(-10.0, 10.0);
          return std::to_string(dist(rnd));
        },
      True >> [](auto&) { return "true"; },
      False >> [](auto&) { return "false"; },
      Null >> [](auto&) { return "null"; },
      JSONString >> [](auto& rnd) { return rand_string(rnd); },
      RawString >> [](auto& rnd) { return rand_raw_string(rnd); },
    });

    return p;
  }
}