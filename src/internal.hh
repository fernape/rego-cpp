// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "rego/rego.hh"

#include <chrono>

namespace rego
{
  std::vector<Pass> passes(const BuiltIns& builtins);

  const inline auto ScalarToken =
    T(Int) / T(Float) / T(True) / T(False) / T(Null);
  const inline auto ArithToken =
    T(Add) / T(Subtract) / T(Multiply) / T(Divide) / T(Modulo);
  const inline auto ArithInfixArg = T(Expr) / T(NumTerm) / T(Ref) /
    T(UnaryExpr) / T(ArithInfix) / T(RefTerm) / T(ExprCall);
  const inline auto BinInfixArg = T(Expr) / T(Ref) / T(RefTerm) / T(ExprCall) /
    T(Set) / T(SetCompr) / T(BinInfix);
  const auto inline RefArg = T(RefArgDot) / T(RefArgBrack);
  const inline auto BoolToken = T(Equals) / T(NotEquals) / T(GreaterThan) /
    T(LessThan) / T(GreaterThanOrEquals) / T(LessThanOrEquals);
  const inline auto TermToken = T(Var) / T(Ref) / T(Array) / T(Object) /
    T(Set) / T(ArrayCompr) / T(ObjectCompr) / T(SetCompr);
  const inline auto StringToken = T(JSONString) / T(RawString);
  const inline auto ExprToken = T(Term) / ArithToken / BoolToken / StringToken /
    T(Expr) / ScalarToken / TermToken / T(JSONString) / T(Array) / T(Set) /
    T(Object) / T(Paren) / T(Not) / T(Dot) / T(And) / T(Or) / T(ExprCall);
  const auto inline MembershipToken = ScalarToken / T(JSONString) /
    T(RawString) / T(Var) / T(Object) / T(Array) / T(Set) / T(Dot) / T(Paren) /
    ArithToken / BoolToken / T(And) / T(Or) / T(ExprCall);
  const auto inline RuleRefToken = T(Var) / T(Dot) / T(Array);

  inline const std::set<std::string> Keywords(
    {"if", "in", "contains", "every"});

  inline const std::set<Token> RuleTypes(
    {RuleComp, RuleFunc, RuleSet, RuleObj, DefaultRule});

  bool all_alnum(const std::string_view& str);
  bool contains_local(const Node& node);
  bool contains_ref(const Node& node);
  Node concat_refs(const Node& lhs, const Node& rhs);
  std::string flatten_ref(const Node& ref);
  bool is_in(const Node& node, const std::set<Token>& token);
  bool in_query(const Node& node);
  bool is_constant(const Node& node);
  bool is_instance(const Node& value, const std::set<Token>& types);
  bool is_falsy(const Node& node);
  bool is_truthy(const Node& node);
  bool is_undefined(const Node& node);
  bool is_ref_to_type(const Node& var, const std::set<Token>& types);
  bool is_module(const Node& var);
  std::string strip_quotes(const std::string_view& str);
  std::string type_name(const Token& type, bool specify_number = false);
  std::string type_name(const Node& node, bool specify_number = false);

  inline bool is_quoted(const std::string_view& str)
  {
    return str.size() >= 2 && str.front() == str.back() && str.front() == '"';
  }

  using namespace trieste;
  using PrintNode = std::ostream& (*)(std::ostream&, const Node&);

  class Resolver
  {
  public:
    static Node scalar(BigInt value);
    static Node scalar(double value);
    static Node scalar(bool value);
    static Node scalar(const char* value);
    static Node scalar(const std::string& value);
    static Node scalar();
    static Node term(BigInt value);
    static Node term(double value);
    static Node term(bool value);
    static Node term(const char* value);
    static Node term(const std::string& value);
    static Node term();
    static Node resolve_query(const Node& query, const BuiltIns& builtins);
    static void stmt_str(logging::Log&, const Node& stmt);
    static void func_str(logging::Log&, const Node& func);
    static void arg_str(logging::Log&, const Node& arg);
    static void expr_str(logging::Log&, const Node& unifyexpr);
    static void enum_str(logging::Log&, const Node& unifyexprenum);
    static void with_str(logging::Log&, const Node& unifyexprwith);
    static void compr_str(logging::Log&, const Node& unifyexprcompr);
    static void ref_str(logging::Log&, const Node& ref);
    static void body_str(logging::Log&, const Node& rego);
    static void not_str(logging::Log&, const Node& rego);
    static void term_str(logging::Log&, const Node& rego);
    static void rego_str(logging::Log&, const Node& rego);
    static Node negate(const Node& value);
    static Node unary(const Node& value);
    static Node arithinfix(const Node& op, const Node& lhs, const Node& rhs);
    static Node bininfix(const Node& op, const Node& lhs, const Node& rhs);
    static Node boolinfix(const Node& op, const Node& lhs, const Node& rhs);
    static std::optional<Nodes> apply_access(
      const Node& container, const Node& index);
    static Node object(const Node& object_items, bool is_rule);
    static Node array(const Node& array_members);
    static Node set(const Node& set_members);
    static Node set_intersection(const Node& lhs, const Node& rhs);
    static Node set_union(const Node& lhs, const Node& rhs);
    static Node set_difference(const Node& lhs, const Node& rhs);
    static Nodes resolve_varseq(const Node& varseq);
    static Nodes object_lookdown(const Node& object, const Node& query);
    static Nodes module_lookdown(const Node& module, const std::string& name);
    static Node inject_args(const Node& rulefunc, const Nodes& args);
    static Node membership(
      const Node& index, const Node& item, const Node& itemseq);
    static Node membership(const Node& item, const Node& itemseq);
    static std::vector<std::string> array_find(
      const Node& array, const std::string& search);
    static std::vector<std::string> object_find(
      const Node& object, const std::string& search);
    static Node to_term(const Node& value);
    static void flatten_terms_into(const Node& termset, Node& terms);
    static void flatten_items_into(const Node& termset, Node& terms);
    static Node reduce_termset(const Node& termset);
    static void insert_into_object(
      Node& object, const std::string& path, const Node& value);
  };

  PassDef input_data();
  PassDef modules();
  PassDef imports();
  PassDef keywords();
  PassDef lists();
  PassDef ifs();
  PassDef elses();
  PassDef rules();
  PassDef build_calls();
  PassDef membership();
  PassDef build_refs();
  PassDef structure();
  PassDef strings();
  PassDef merge_data();
  PassDef lift_refheads();
  PassDef symbols();
  PassDef replace_argvals();
  PassDef lift_query();
  PassDef expand_imports();
  PassDef constants();
  PassDef explicit_enums();
  PassDef body_locals(const BuiltIns& builtins);
  PassDef value_locals(const BuiltIns& builtins);
  PassDef rules_to_compr();
  PassDef compr();
  PassDef absolute_refs();
  PassDef merge_modules();
  PassDef datarule();
  PassDef skips();
  PassDef unary();
  PassDef multiply_divide();
  PassDef add_subtract();
  PassDef comparison();
  PassDef assign(const BuiltIns& builtins);
  PassDef skip_refs(const BuiltIns& builtins);
  PassDef simple_refs();
  PassDef init();
  PassDef implicit_enums();
  PassDef enum_locals();
  PassDef rulebody();
  PassDef lift_to_rule();
  PassDef functions();
  PassDef unify(const BuiltIns& builtins);
  PassDef query();

  template <typename I, typename S, typename F>
  S& join(S& stream, const I& begin, const I& end, const char* sep, F&& writer)
  {
    if (begin == end)
    {
      return stream;
    }

    bool write_sep = false;
    for (auto it = begin; it != end; ++it)
    {
      if (write_sep)
      {
        stream << sep;
      }

      write_sep = writer(stream, *it);
    }

    return stream;
  }

  // Provide types to delay pretty printing various rego node types.
  using ArgStr = logging::Lazy<Node, Resolver::arg_str>;
  using BodyStr = logging::Lazy<Node, Resolver::body_str>;
  using ExprStr = logging::Lazy<Node, Resolver::expr_str>;
  using NotStr = logging::Lazy<Node, Resolver::not_str>;
  using RefStr = logging::Lazy<Node, Resolver::ref_str>;
  using RegoStr = logging::Lazy<Node, Resolver::rego_str>;
  using StmtStr = logging::Lazy<Node, Resolver::stmt_str>;
  using TermStr = logging::Lazy<Node, Resolver::term_str>;
  using WithStr = logging::Lazy<Node, Resolver::with_str>;

  template <typename K, typename V>
  struct MapValuesStr
  {
    const std::map<K, V>& values;
    MapValuesStr(const std::map<K, V>& values_) : values(values_) {}
  };
  // Deduction guide.
  template <typename K, typename V>
  MapValuesStr(const std::map<K, V>&) -> MapValuesStr<K, V>;

  class ValueDef;
  using rank_t = std::size_t;
  using Value = std::shared_ptr<ValueDef>;
  using Values = std::vector<Value>;
  using RankedNode = std::pair<rank_t, Node>;

  class ValueDef
  {
  public:
    void mark_as_valid();
    void mark_as_invalid();
    void reduce_set();

    const std::string& str() const;
    const std::string& json() const;
    bool depends_on(const Value& value) const;
    bool invalid() const;
    const Location& var() const;
    const Node& node() const;
    const Values& sources() const;
    Node to_term() const;
    rank_t rank() const;
    friend std::ostream& operator<<(std::ostream& os, const Value& value);
    friend std::ostream& operator<<(std::ostream& os, const ValueDef& value);
    friend bool operator==(const Value& lhs, const Value& rhs);
    friend bool operator<(const Value& lhs, const Value& rhs);

    static Value create(const RankedNode& value);
    static Value create(const Location& var, const RankedNode& value);
    static Value create(
      const Location& var, const RankedNode& value, const Values& sources);
    static Value create(const Node& value);
    static Value create(const Location& var, const Node& value);
    static Value create(
      const Location& var, const Node& value, const Values& sources);
    static Value copy_to(const Value& value, const Location& var);
    static Values filter_by_rank(const Values& values);
    static rank_t get_rank(const Node& node);

  private:
    static void build_string(
      std::ostream& buf,
      const ValueDef& current,
      const Location& root,
      bool first);
    Location m_var;
    Node m_node;
    Values m_sources;
    bool m_invalid;
    rank_t m_rank;
    std::string m_str;
    std::string m_json;
    ValueDef(
      const Location& var,
      const Node& value,
      const Values& sources,
      rank_t rank);
    ValueDef(const Node& value);
    ValueDef(const RankedNode& value);
    ValueDef(const Location& var, const Node& value);
    ValueDef(const Location& var, const RankedNode& value);
    ValueDef(const Location& var, const Node& value, const Values& sources);
    ValueDef(
      const Location& var, const RankedNode& value, const Values& sources);
  };

  class ValueMap
  {
  public:
    ValueMap();
    ValueMap(
      const Values::const_iterator& begin, const Values::const_iterator& end);
    bool contains(const Value& value) const;
    bool intersect_with(const Values& values);
    bool remove_values_not_contained_in(const Values& values);
    void clear();
    bool insert(const Value& value);
    bool erase(const std::string& json);
    bool empty() const;
    Nodes to_terms() const;
    Values valid_values() const;
    bool remove_invalid_values();
    void mark_valid_values(bool include_falsy);
    void mark_invalid_values();

    friend std::ostream& operator<<(std::ostream& os, const ValueMap& values);

  private:
    std::multimap<std::string, Value> m_map;
    std::set<std::pair<std::string, std::string>> m_values;
    std::set<std::string> m_keys;
  };

  /**
   * The Args class provides an interable interface over the Cartesian product
   * of all sets of arguments to a function. It will contain N sets of argument
   * sources, and will produce s_0 * s_1 * ... * s_n arguments, where s_i is the
   * size of the i-th argument source.
   */
  class Args
  {
  public:
    Args();
    void push_back_source(const Values& values);
    void mark_invalid(const std::set<Value>& active) const;
    void mark_invalid_except(const std::set<Value>& active) const;
    Values at(std::size_t index) const;
    std::size_t size() const;
    std::string str() const;
    std::size_t source_size() const;
    const Values& source_at(std::size_t index) const;
    Args subargs(std::size_t start) const;
    friend std::ostream& operator<<(std::ostream& os, const Args& args);

  private:
    std::vector<Values> m_values;
    std::vector<size_t> m_stride;
    std::size_t m_size;
  };

  class Variable
  {
  public:
    Variable(const Node& local, std::size_t id);
    bool unify(const Values& others);
    std::string str() const;
    bool remove_invalid_values();
    void mark_invalid_values();
    void mark_valid_values();
    Values valid_values() const;
    Node to_term() const;
    Node bind();
    void reset();

    bool is_unify() const;
    bool is_user_var() const;
    Location name() const;
    std::size_t id() const;

    static bool is_unify(const std::string_view& name);
    static bool is_user_var(const std::string_view& name);

    friend std::ostream& operator<<(std::ostream& os, const Variable& variable);
    friend std::ostream& operator<<(
      std::ostream& os, const std::map<Location, Variable>& variables);

  private:
    bool intersect_with(const Values& others);
    bool initialize(const Values& others);

    Node m_local;
    ValueMap m_values;
    bool m_unify;
    bool m_user_var;
    bool m_initialized;
    std::size_t m_id;
  };

  enum class UnifierType
  {
    RuleBody,
    RuleValue
  };

  struct UnifierKey
  {
    Location key;
    UnifierType type;
    bool operator<(const UnifierKey& other) const;
  };

  class UnifierDef;
  using Unifier = std::shared_ptr<UnifierDef>;
  using CallStack = std::shared_ptr<std::vector<Location>>;
  using ValuesLookup = std::map<std::string, Values>;
  using WithStack = std::shared_ptr<std::vector<ValuesLookup>>;
  using UnifierCache = std::shared_ptr<std::map<UnifierKey, Unifier>>;

  class UnifierDef
  {
  public:
    UnifierDef(
      const Location& rule,
      const Node& rulebody,
      CallStack call_stack,
      WithStack with_stack,
      const BuiltIns& builtins,
      UnifierCache unifier_cache);
    Node unify();
    Nodes expressions() const;
    Nodes bindings() const;
    std::string str() const;
    std::string dependency_str() const;
    static Unifier create(
      const UnifierKey& key,
      const Location& rule,
      const Node& rulebody,
      const CallStack& call_stack,
      const WithStack& with_stack,
      const BuiltIns& builtins,
      const UnifierCache& unifier_cache);
    std::optional<Node> resolve_rule(const Nodes& defs) const;
    std::optional<Node> resolve_rulecomp(const Nodes& rulecomp) const;
    std::optional<Node> resolve_ruleset(const Nodes& ruleset) const;
    std::optional<Node> resolve_ruleobj(const Nodes& ruleobj) const;
    std::optional<RankedNode> resolve_rulefunc(
      const Node& rulefunc, const Nodes& args) const;
    Node resolve_module(const Node& module) const;

    struct Dependency
    {
      std::string name;
      std::set<std::size_t> dependencies;
      std::size_t score;
    };

    struct Statement
    {
      std::size_t id;
      Node node;
    };

  private:
    Unifier rule_unifier(
      const UnifierKey& key, const Location& rule, const Node& rulebody) const;
    void init_from_body(
      const Node& rulebody,
      std::vector<Statement>& statements,
      std::size_t root);
    std::size_t add_variable(const Node& local);
    std::size_t add_unifyexpr(const Node& unifyexpr);
    void add_withpush(const Node& withpush);
    void add_withpop(const Node& withpop);
    void reset();

    void compute_dependency_scores();
    std::size_t compute_dependency_score(
      std::size_t index, std::set<size_t>& visited);
    std::size_t dependency_score(const Variable& var) const;
    std::size_t dependency_score(const Statement& stmt) const;
    std::size_t detect_cycles() const;
    bool has_cycle(std::size_t id) const;

    Values evaluate(const Location& var, const Node& value);
    Values evaluate_function(
      const Location& var, const std::string& func_name, const Args& args);
    Values resolve_var(const Node& var, bool exclude_with = false);
    Values enumerate(const Location& var, const Node& container);
    Values resolve_skip(const Node& skip);
    Values check_with(const Node& var, bool bypass_recurse_check = false);
    Values apply_access(const Location& var, const Values& args);
    std::optional<Value> call_function(
      const Location& var, const Values& args) const;
    std::optional<Value> call_named_function(
      const Location& var, const std::string& name, const Values& args);
    bool is_local(const Node& var);
    std::size_t scan_vars(const Node& expr, std::vector<Location>& locals);
    void pass();
    void execute_statements(
      std::vector<Statement>::iterator begin,
      std::vector<Statement>::iterator end);
    void remove_invalid_values();
    void mark_invalid_values();
    Variable& get_variable(const Location& name);
    bool is_variable(const Location& name) const;
    Node bind_variables();
    Args create_args(const Node& args);
    bool would_recurse(const Node& node);

    bool push_rule(const Location& rule);
    void pop_rule(const Location& rule);

    void push_with(const Node& withseq);
    void pop_with();

    void push_not();
    void pop_not();

    Location m_rule;
    std::map<Location, Variable> m_variables;
    std::vector<Statement> m_statements;
    std::map<std::size_t, std::vector<Statement>> m_nested_statements;
    CallStack m_call_stack;
    WithStack m_with_stack;
    const BuiltIns& m_builtins;
    UnifierCache m_cache;
    std::size_t m_retries;
    Token m_parent_type;
    std::vector<Dependency> m_dependency_graph;
    bool m_negate;
  };

  class ActionMetrics
  {
  public:
    ActionMetrics(const char* file, std::size_t line);
    ~ActionMetrics();
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    using duration = clock::duration;

    static void print();

  private:
    struct key_t
    {
      std::string_view file;
      std::size_t line;

      bool operator<(const key_t& other) const;
    };

    struct info_t
    {
      std::size_t count;
      duration time_spent;
    };

    key_t m_key;
    time_point m_start;
    static std::map<key_t, info_t> s_action_info;
  };

#ifdef REGOCPP_USE_CXX17
  template <typename T, typename I>
  inline bool contains(const std::shared_ptr<T>& container, const I& item)
  {
    return container->find(item) != container->end();
  }

  template <typename T, typename I>
  inline bool contains(const T& container, const I& item)
  {
    return container.find(item) != container.end();
  }

  inline bool starts_with(
    const std::string_view& str, const std::string_view& prefix)
  {
    if (prefix.size() > str.size())
    {
      return false;
    }

    auto str_it = str.begin();
    for (auto prefix_it = prefix.begin(); prefix_it != prefix.end();
         ++prefix_it, ++str_it)
    {
      if (*prefix_it != *str_it)
      {
        return false;
      }
    }

    return true;
  }

  inline bool starts_with(const std::string_view& str, const char* prefix)
  {
    return starts_with(str, std::string_view(prefix));
  }

  inline bool starts_with(const std::string_view& s, char c)
  {
    return s.front() == c;
  }

  inline bool ends_with(
    const std::string_view& str, const std::string_view& suffix)
  {
    if (suffix.size() > str.size())
    {
      return false;
    }

    auto str_it = str.rbegin();
    for (auto suffix_it = suffix.rbegin(); suffix_it != suffix.rend();
         ++suffix_it, ++str_it)
    {
      if (*suffix_it != *str_it)
      {
        return false;
      }
    }

    return true;
  }

  inline bool ends_with(const std::string_view& str, const char* suffix)
  {
    return ends_with(str, std::string_view(suffix));
  }

  inline bool ends_with(const std::string_view& s, char c)
  {
    return s.back() == c;
  }
#else
  template <typename T, typename I>
  inline bool contains(const std::shared_ptr<T>& container, const I& item)
  {
    return container->contains(item);
  }

  template <typename T, typename I>
  inline bool contains(const T& container, const I& item)
  {
    return container.contains(item);
  }

  template <typename S, typename P>
  inline bool starts_with(const S& str, const P& prefix)
  {
    return str.starts_with(prefix);
  }

  template <typename S, typename P>
  inline bool ends_with(const S& s, const P& suffix)
  {
    return s.ends_with(suffix);
  }
#endif

}

// Use ADL to extend Trieste logging capabilities with additional types.
namespace trieste::logging
{
  inline void append(
    Log& log, const std::vector<rego::UnifierDef::Dependency>& deps)
  {
    for (auto it = deps.begin(); it != deps.end(); ++it)
    {
      auto& dep = *it;
      log << "[" << dep.name << "](" << dep.score << ") -> {";

      logging::Sep sep{", "};
      for (auto& idx : dep.dependencies)
      {
        log << sep << deps[idx].name;
      }
      log << "}" << std::endl;
    }
  }

  template <typename T>
  inline void append(Log& log, const std::vector<T>& values)
  {
    log << "[";
    logging::Sep sep{", "};
    for (auto& value : values)
    {
      log << sep << value;
    }
    log << "]" << std::endl;
  }

  inline void append(Log& log, const Location& loc)
  {
    log << loc.view();
  }

  inline void append(Log& log, const rego::UnifierDef::Statement& statement)
  {
    rego::Resolver::stmt_str(log, statement.node);
  }

  template <typename K, typename V>
  inline void append(Log& log, const rego::MapValuesStr<K, V>& map)
  {
    log << "{" << std::endl;
    for (auto& [_, value] : map.values)
    {
      log << value << std::endl;
    }
    log << "}" << std::endl;
  }
} // trieste::logging

#ifdef REGOCPP_ACTION_METRICS
#define ACTION() rego::ActionMetrics __action_metrics(__FILE__, __LINE__)
#define PRINT_ACTION_METRICS() rego::ActionMetrics::print()
#else
#define ACTION()
#define PRINT_ACTION_METRICS()
#endif