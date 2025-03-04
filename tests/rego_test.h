#pragma once

// Trieste-based rewriter for Rego YAML test cases

#include <rego/rego.hh>

namespace rego_test
{
  using namespace trieste;
  using namespace wf::ops;

  inline const auto Document = TokenDef("yaml-document", flag::symtab);
  inline const auto Block = TokenDef("yaml-block");
  inline const auto Sequence = TokenDef("yaml-sequence");
  inline const auto Entry = TokenDef("yaml-entry");
  inline const auto KeyValue = TokenDef("yaml-key-value", flag::lookdown);
  inline const auto Mapping = TokenDef("yaml-mapping", flag::symtab);
  inline const auto Key = TokenDef("yaml-key", flag::print);
  inline const auto Val = TokenDef("yaml-val");
  inline const auto String = TokenDef("yaml-string", flag::print);
  inline const auto Integer = TokenDef("yaml-integer", flag::print);
  inline const auto Float = TokenDef("yaml-float", flag::print);
  inline const auto True = TokenDef("yaml-true", flag::print);
  inline const auto False = TokenDef("yaml-false", flag::print);
  inline const auto Null = TokenDef("yaml-null", flag::print);
  inline const auto LiteralString = TokenDef("yaml-literal-string");
  inline const auto FoldedString = TokenDef("yaml-folded-string");
  inline const auto SingleQuoteString = TokenDef("yaml-single-quote-string");
  inline const auto DoubleQuoteString = TokenDef("yaml-double-quote-string");
  inline const auto Scalar = TokenDef("yaml-scalar");
  inline const auto Data = TokenDef("yaml-data");
  inline const auto Input = TokenDef("yaml-input");
  inline const auto Colon = TokenDef("yaml-:");
  inline const auto Hyphen = TokenDef("yaml--");
  inline const auto WantResult = TokenDef("yaml-want-result");
  inline const auto Head = TokenDef("yaml-head");
  inline const auto Tail = TokenDef("yaml-tail");
  inline const auto Blank = TokenDef("yaml-blank");
  inline const auto Brace = TokenDef("yaml-brace");
  inline const auto Square = TokenDef("yaml-square");
  inline const auto NewLine = TokenDef("yaml-newline");
  inline const auto EmptySequence = TokenDef("yaml-empty-sequence");
  inline const auto EmptyMapping = TokenDef("yaml-empty-mapping");

  inline const auto wf_parse_tokens = Block | String | Integer | Float | True |
    False | Null | Colon | LiteralString | FoldedString | SingleQuoteString |
    DoubleQuoteString | Hyphen | Blank | Brace | Square | NewLine;

  // clang-format off
  inline const auto wf_parser =
    (Top <<= File)
    | (File <<= Group++[1])
    | (Block <<= Group++)
    | (LiteralString <<= Group++[1])
    | (FoldedString <<= Group++[1])
    | (SingleQuoteString <<= Group++)
    | (DoubleQuoteString <<= Group++)
    | (Brace <<= Group++)
    | (Square <<= Group++)
    | (Group <<= wf_parse_tokens++[1])
    ;
  // clang-format on

  inline const auto wf_entry_tokens = Block | String | Integer | Float | True |
    False | Null | Colon | Entry | LiteralString | FoldedString |
    SingleQuoteString | DoubleQuoteString | EmptySequence | EmptyMapping;

  // clang-format off
  inline const auto wf_pass_entry =
    wf_parser
    | (Top <<= Block)
    | (LiteralString <<= String++[1])
    | (FoldedString <<= String++[1])
    | (SingleQuoteString <<= String++)
    | (DoubleQuoteString <<= String++)
    | (Entry <<= Block | Group)
    | (Group <<= wf_entry_tokens++[1])
    ;
  // clang-format on    

  inline const auto wf_sequence_tokens =
    Block | String | Integer | Float | True | False | Null | Colon | Sequence | EmptyMapping;

  // clang-format off
  inline const auto wf_pass_sequence =
    wf_pass_entry
    | (Sequence <<= Entry++)
    | (Group <<= wf_sequence_tokens++[1])
    ;
  // clang-format on   

  // clang-format off
  inline const auto wf_pass_scalar =
    wf_pass_sequence
    | (Scalar <<= String | Integer | Float | True | False | Null)
    | (Group <<= (Block | Scalar | Sequence | Colon | EmptyMapping)++[1])
    ;
  // clang-format on

  // clang-format off
  inline const auto wf_pass_keyvalue =
    wf_pass_scalar
    | (KeyValue <<= Key * Group)
    | (Block <<= (Group | KeyValue)++)
    | (Group <<= Block | Scalar | Sequence | KeyValue | EmptyMapping)
    ;
  // clang-format on

  // clang-format off
  inline const auto wf_pass_mapping =
    wf_pass_keyvalue
    | (Mapping <<= KeyValue++)
    | (Entry <<= Group)
    | (Group <<= Mapping | Scalar | Sequence)
    ;

  // clang-format off
  inline const auto wf_pass_structure =
    (Top <<= Document)
    | (Document <<= KeyValue)
    | (KeyValue <<= Key * (Val >>= Scalar | Sequence | Mapping))[Key]
    | (Sequence <<= Entry++)
    | (Mapping <<= KeyValue++)
    | (Entry <<= Scalar | Sequence | Mapping)
    | (Scalar <<= String | Integer | Float | True | False | Null)
    ;
  // clang-format on

  const auto inline wf_rego_term_tokens =
    rego::Scalar | rego::Object | rego::Array | rego::Set;
  const auto inline wf_rego_scalar_tokens = rego::Int | rego::Float |
    rego::JSONString | rego::True | rego::False | rego::Null;

  // clang-format off
  inline const auto wf_pass_to_rego =
    wf_pass_structure
    | (KeyValue <<= Key * (Val >>= Mapping | Sequence | Scalar | rego::File | WantResult))[Key]
    | (rego::File <<= Group++)
    | (rego::Brace <<= rego::List)
    | (rego::Square <<= rego::List)
    | (rego::List <<= Group++)
    | (Group <<= rego::wf_parse_tokens++[1])
    | (WantResult <<= rego::Binding++)
    | (rego::Binding <<= rego::Var * rego::Term)
    | (rego::Term <<= wf_rego_term_tokens)
    | (rego::Object <<= rego::ObjectItem++)
    | (rego::ObjectItem <<= rego::Key * rego::Term)
    | (rego::Array <<= rego::Term++)
    | (rego::Set <<= rego::Term++)
    | (rego::Scalar <<= wf_rego_scalar_tokens)
    ;
  // clang-format on

  inline auto err(Node node, const std::string& msg)
  {
    return Error << (ErrorMsg ^ msg) << (ErrorAst << node->clone());
  }

  inline auto err(const std::string& msg)
  {
    return Error << (ErrorMsg ^ msg);
  }

  Parse parser();
  Driver& driver();
  std::vector<Pass> passes();
}