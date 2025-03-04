#include "rego_test.h"

namespace rego_test
{
  const auto inline SequenceToken = T(Sequence) / T(Mapping) / T(Scalar);
  const auto inline ScalarToken =
    T(String) / T(Integer) / T(Float) / T(True) / T(False) / T(Null);
  const auto inline ValToken = T(Mapping) / T(Sequence) / T(Scalar);

  PassDef entry()
  {
    return {
      "entry",
      wf_pass_entry,
      dir::topdown,
      {
        In(Top) * T(File)[File] >> [](Match& _) { return Block << *_[File]; },

        In(Group) * (T(Brace)[Brace] << End) >>
          [](Match& _) { return EmptyMapping ^ _(Brace); },

        In(Group) * (T(Colon) * T(Integer)[Integer] * T(String)[String]) >>
          [](Match& _) {
            Location int_loc = _(Integer)->location();
            Location str_loc = _(String)->location();
            std::size_t end = str_loc.pos + str_loc.len;
            Location loc = int_loc;
            loc.len = end - loc.pos;
            return Seq << Colon << (String ^ loc);
          },

        In(Block) *
            ((T(Group) << (T(String) * T(Colon) * End)) *
             (T(Group)[Group] << (T(String) * T(Colon)))) >>
          [](Match& _) { return _(Group); },

        In(Group) * T(Brace)[Brace] >>
          [](Match& _) { return Block << *_[Brace]; },

        In(Group) * T(Square)[Square] >>
          [](Match& _) {
            if (_(Square)->size() == 0)
            {
              return EmptySequence ^ _(Square);
            }

            Node seq = NodeDef::create(Seq);
            for (auto group : *_(Square))
            {
              seq << (Entry << group);
            }
            return seq;
          },

        In(Group) * (T(Hyphen) * (T(Block) / T(Group))[Entry]) >>
          [](Match& _) { return Entry << _(Entry); },

        In(DoubleQuoteString) *
            (T(Group) << (T(NewLine) * ~T(String)++[String])) >>
          [](Match& _) {
            std::ostringstream buf;
            buf << "\n";
            for (auto it = _[String].first; it != _[String].second; ++it)
            {
              Node str = *it;
              buf << str->location().view();
            }
            return String ^ buf.str();
          },

        (In(LiteralString, FoldedString, SingleQuoteString) /
         In(DoubleQuoteString)) *
            (T(Group) << T(String)[String]) >>
          [](Match& _) { return _(String); },

        (In(LiteralString, FoldedString, SingleQuoteString) /
         In(DoubleQuoteString)) *
            (T(Group) << T(Blank)) >>
          [](Match&) { return String ^ ""; },

        T(Group) << (T(Blank) * End) >> [](Match&) { return Node{}; },

        // errors
        In(Group) * T(Blank)[Blank] >>
          [](Match& _) { return err(_(Blank), "Invalid blank line."); },

        In(Entry, Block) * (T(Group)[Group] << End) >>
          [](Match& _) { return err(_(Group), "Syntax error: empty group"); },

        In(Group) * T(Hyphen)[Hyphen] >>
          [](Match& _) {
            return err(_(Hyphen), "Invalid sequence entry declaration");
          },

        In(Group) * (T(Entry)[Entry] << End) >>
          [](Match& _) { return err(_(Entry), "Invalid entry declaration"); },

        In(LiteralString) * T(Group)[Group] >>
          [](Match& _) {
            return err(_(Group), "Invalid literal string element");
          },

        In(FoldedString) * T(Group)[Group] >>
          [](Match& _) {
            return err(_(Group), "Invalid folded string element");
          },

        In(SingleQuoteString) * T(Group)[Group] >>
          [](Match& _) {
            return err(_(Group), "Invalid single-quoted string element");
          },

        In(DoubleQuoteString) * T(Group)[Group] >>
          [](Match& _) {
            return err(_(Group), "Invalid double-quoted string element");
          },

        In(Group) * T(NewLine)[NewLine] >>
          [](Match& _) { return err(_(NewLine), "Invalid newline"); },
      }};
  }

  PassDef sequence()
  {
    return {
      "sequence",
      wf_pass_sequence,
      dir::topdown,
      {In(Group) * T(EmptySequence)[EmptySequence] >>
         [](Match& _) { return Sequence ^ _(EmptySequence); },

       In(Group) * (T(Entry)[Head] * T(Entry)++[Tail] * End) >>
         [](Match& _) { return Sequence << _(Head) << _[Tail]; },

       In(Group) * (T(LiteralString) << (T(String)++[String])) >>
         [](Match& _) {
           std::ostringstream buf;
           for (auto it = _[String].first; it != _[String].second; ++it)
           {
             if (it != _[String].first)
             {
               buf << "\n";
             }
             buf << (*it)->location().view();
           }
           return String ^ buf.str();
         },

       In(Group) *
           ((T(FoldedString) / T(SingleQuoteString) / T(DoubleQuoteString))
            << (T(String)++[String])) >>
         [](Match& _) {
           std::ostringstream buf;
           for (auto it = _[String].first; it != _[String].second; ++it)
           {
             if (it != _[String].first)
             {
               buf << " ";
             }
             buf << (*it)->location().view();
           }
           return String ^ buf.str();
         },

       // errors

       In(LiteralString) * T(Group)[Group] >>
         [](Match& _) {
           return err(_(Group), "Invalid literal string element");
         },

       In(FoldedString) * T(Group)[Group] >>
         [](Match& _) {
           return err(_(Group), "Invalid folded string element");
         },

       In(Group) * T(Entry)[Entry] >>
         [](Match& _) { return err(_(Entry), "Invalid entry location"); }}};
  }

  PassDef scalar()
  {
    return {
      "scalar",
      wf_pass_scalar,
      dir::topdown,
      {
        In(Group) * ScalarToken[Scalar] >>
          [](Match& _) { return Scalar << _(Scalar); },
      }};
  }

  PassDef keyvalue()
  {
    return {
      "keyvalue",
      wf_pass_keyvalue,
      {
        In(Entry, Block) *
            (T(Group)
             << ((T(Scalar) << T(String))[Key] * T(Colon) * Any[Val])) >>
          [](Match& _) {
            Location key_loc = _(Key)->location();
            std::string key = std::string(key_loc.view());
            std::size_t length = key.find(' ');
            if (length != std::string::npos)
            {
              key_loc.len = length;
            }

            return KeyValue << (Key ^ key_loc) << (Group << _(Val));
          },

        In(Entry, Block) *
            ((T(Group) << ((T(Scalar) << T(String))[Key] * T(Colon) * End)) *
             T(Group)[Val]) >>
          [](Match& _) {
            Location key_loc = _(Key)->location();
            std::string key = std::string(key_loc.view());
            std::size_t length = key.find(' ');
            if (length != std::string::npos)
            {
              key_loc.len = length;
            }

            return KeyValue << (Key ^ key_loc) << _(Val);
          },

        In(Entry, Block) *
            ((T(Group) << ((T(Scalar) << T(String))[Key] * End)) *
             (T(Group) << (T(Colon) * Any[Val]))) >>
          [](Match& _) {
            Location key_loc = _(Key)->location();
            std::string key = std::string(key_loc.view());
            std::size_t length = key.find(' ');
            if (length != std::string::npos)
            {
              key_loc.len = length;
            }

            return KeyValue << (Key ^ key_loc) << (Group << _[Val]);
          },

        In(Entry, Block) *
            ((T(Group) << ((T(Scalar) << T(String))[Key] * T(Colon))) * End) >>
          [](Match& _) {
            Location key_loc = _(Key)->location();
            std::string key = std::string(key_loc.view());
            std::size_t length = key.find(' ');
            if (length != std::string::npos)
            {
              key_loc.len = length;
            }

            return KeyValue << (Key ^ key_loc) << (Group << Sequence);
          },

        In(Entry) * T(KeyValue)[KeyValue] >>
          [](Match& _) { return Group << (Block << _(KeyValue)); },

        In(Entry) * (T(Block) << (T(Group)[Group] * End)) >>
          [](Match& _) { return _(Group); },

        In(Entry) * T(Block)[Block] >>
          [](Match& _) { return Group << _(Block); },

        // errors
        In(Group) * T(Colon)[Colon] >>
          [](Match& _) {
            return err(_(Colon), "Invalid key/value declaration");
          },

        In(Group) * (Any * Any[Val]) >>
          [](Match& _) { return err(_(Val), "Syntax error"); },
      }};
  }

  PassDef mapping()
  {
    return {
      "mapping",
      wf_pass_mapping,
      dir::topdown,
      {
        In(Group) * (T(Block) << (T(KeyValue)++[Mapping] * End)) >>
          [](Match& _) { return Mapping << _[Mapping]; },

        In(Group) * T(EmptyMapping)[EmptyMapping] >>
          [](Match& _) { return Mapping ^ _(EmptyMapping); },

        // errors
        In(Entry, Group) * T(Block)[Block] >>
          [](Match& _) { return err(_(Block), "Invalid indented block"); },

        In(Group) * T(KeyValue)[KeyValue] >>
          [](Match& _) {
            return err(_(KeyValue), "Invalid key/value declaration");
          },
      }};
  }

  PassDef structure()
  {
    return {
      "structure",
      wf_pass_structure,
      dir::topdown,
      {
        In(KeyValue) * (T(Group) << ValToken[Val]) >>
          [](Match& _) { return _(Val); },

        In(Entry) * (T(Group) << ValToken[Val]) >>
          [](Match& _) { return _(Val); },

        In(Top) * (T(Block) << (T(KeyValue)[KeyValue] * End)) >>
          [](Match& _) { return Document << _(KeyValue); },

        // errors
        In(Top) * T(Block)[Block] >>
          [](Match& _) { return err(_(Block), "Invalid rego test case"); },
      }};
  }

  bool name_equals(const Node& node, const std::set<std::string>& names)
  {
    std::string name = std::string(node->location().view());
    return names.find(name) != names.end();
  }

  PassDef to_rego()
  {
    return {
      "to_rego",
      wf_pass_to_rego,
      dir::topdown,
      {In(Mapping) *
           (T(KeyValue)
            << ((T(Key)[Key]([](auto& n) {
                  return name_equals(*n.first, {"data", "input"});
                })) *
                T(Mapping)[Mapping])) >>
         [](Match& _) {
           return KeyValue
             << _(Key)
             << (rego::File
                 << (Group << (rego::Brace << (rego::List << *_[Mapping]))));
         },

       In(Mapping) *
           (T(KeyValue)
            << ((T(Key)[Key]([](auto& n) {
                  return name_equals(*n.first, {"data", "input"});
                })) *
                (T(Scalar) << T(Null)))) >>
         [](Match&) { return Node{}; },

       In(Mapping) *
           (T(KeyValue)
            << ((T(Key)[Key]([](auto& n) {
                  return name_equals(*n.first, {"want_result"});
                })) *
                T(Sequence)[Sequence])) >>
         [](Match& _) {
           return KeyValue << _(Key) << (WantResult << *_[Sequence]);
         },

       In(rego::List) *
           (T(KeyValue)
            << (T(Key)[Key] * (T(Mapping) / T(Sequence) / T(Scalar))[Val])) >>
         [](Match& _) { return Group << _(Key) << rego::Colon << _(Val); },

       In(rego::List) * (T(Scalar) / T(Mapping) / T(Sequence))[Val] >>
         [](Match& _) { return Group << _(Val); },

       In(rego::List) * T(Entry)[Entry] >>
         [](Match& _) { return _(Entry)->front(); },

       In(Group) * T(Key)[Key] >>
         [](Match& _) {
           std::string key_str =
             "\"" + std::string(_(Key)->location().view()) + "\"";
           return rego::JSONString ^ key_str;
         },

       In(rego::Scalar, Group) * (T(Scalar) << T(String)[String]) >>
         [](Match& _) {
           std::string str =
             "\"" + std::string(_(String)->location().view()) + "\"";
           return rego::JSONString ^ str;
         },

       In(rego::Scalar, Group) * (T(Scalar) << T(Integer)[Integer]) >>
         [](Match& _) { return rego::Int ^ _(Integer); },

       In(rego::Scalar, Group) * (T(Scalar) << T(Float)[Float]) >>
         [](Match& _) { return rego::Float ^ _(Float); },

       In(rego::Scalar, Group) * (T(Scalar) << T(True)[True]) >>
         [](Match& _) { return rego::True ^ _(True); },

       In(rego::Scalar, Group) * (T(Scalar) << T(False)[False]) >>
         [](Match& _) { return rego::False ^ _(False); },

       In(rego::Scalar, Group) * (T(Scalar) << T(Null)[Null]) >>
         [](Match& _) { return rego::Null ^ _(Null); },

       In(Group) * T(Mapping)[Mapping] >>
         [](Match& _) {
           // test for set
           return rego::Brace << (rego::List << *_[Mapping]);
         },

       In(Group) * T(Sequence)[Sequence] >>
         [](Match& _) { return rego::Square << (rego::List << *_[Sequence]); },

       In(WantResult) * (T(KeyValue) << (T(Key)[Key] * Any[Val])) >>
         [](Match& _) {
           return rego::Binding << (rego::Var ^ _(Key))
                                << (rego::Term << _(Val));
         },

       In(WantResult) * (T(Entry) << T(Mapping)[Mapping]) >>
         [](Match& _) { return Seq << *_[Mapping]; },

       In(rego::Term) * T(Mapping)[Mapping] >>
         [](Match& _) {
           bool is_set = _(Mapping)->size() > 0;
           for (auto& keyvalue : *_(Mapping))
           {
             Node val = keyvalue / Val;
             if (val->type() != Scalar)
             {
               is_set = false;
               break;
             }
             val = val->front();
             if (val->type() != Null)
             {
               is_set = false;
               break;
             }
           }

           if (is_set)
           {
             Node set = NodeDef::create(rego::Set);
             for (auto& keyvalue : *_(Mapping))
             {
               set
                 << (rego::Term
                     << (rego::Scalar << (rego::JSONString ^ keyvalue / Key)));
             }
             return set;
           }

           return rego::Object << *_[Mapping];
         },

       In(rego::Object) * (T(KeyValue) << (T(Key)[Key] * Any[Val])) >>
         [](Match& _) {
           return rego::ObjectItem << (rego::Key ^ _(Key))
                                   << (rego::Term << _(Val));
         },

       In(rego::Term) * T(Scalar)[Scalar] >>
         [](Match& _) { return rego::Scalar << _(Scalar); },

       In(rego::Term) * T(Sequence)[Sequence] >>
         [](Match& _) { return rego::Array << *_[Sequence]; },

       In(rego::Array) * T(Entry)[Entry] >>
         [](Match& _) { return rego::Term << *_[Entry]; }}};
  }

  std::vector<Pass> passes()
  {
    return {
      entry(),
      sequence(),
      scalar(),
      keyvalue(),
      mapping(),
      structure(),
      to_rego()};
  }
  Driver& driver()
  {
    static Driver d("rego_test", nullptr, parser(), passes());
    return d;
  }
}