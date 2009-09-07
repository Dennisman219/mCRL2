import re
import string
from path import *

# /// \brief The or operator for state formulas
# class or_: public state_formula
# {
#   public:
#     /// \brief Constructor.
#     /// \param term A term
#     or_(atermpp::aterm_appl term)
#       : state_formula(term)
#     {
#       assert(core::detail::check_term_StateOr(m_term));
#     }
# 
#     /// \brief Constructor.
#     /// \param left A process expression
#     /// \param d A data expression
#     or_(const state_formula& left, const state_formula& right)
#       : or_(core::detail::gsMakeStateOr(left, right))
#     {}
# };
CLASS_DEFINITION = r'''/// \\brief DESCRIPTION
class CLASSNAME: public SUPERCLASS
{
  public:
    /// \\brief Constructor.
    /// \\param term A term
    CLASSNAME(atermpp::aterm_appl term)
      : SUPERCLASS(term)
    {
      assert(core::detail::check_term_ATERM(m_term));
    }

    /// \\brief Constructor.
    /// \\param left A process expression
    /// \\param d A data expression
    CONSTRUCTOR
      : SUPERCLASS(core::detail::gsMakeATERM(PARAMETERS))
    {}MEMBER_FUNCTIONS
};'''

MEMBER_FUNCTION = '''    TYPE NAME() const
    {
      return atermpp::ARG(*this);
    }'''

STATE_FORMULA_CLASSES = r'''
StateTrue       | true_()                                                                                                         | The value true for state formulas
StateFalse      | false_()                                                                                                        | The value false for state formulas
StateNot        | not_(const state_formula& operand)                                                                              | The not operator for state formulas
StateAnd        | and_(const state_formula& left, const state_formula& right)                                                     | The and operator for state formulas
StateOr         | or_(const state_formula& left, const state_formula& right)                                                      | The or operator for state formulas
StateImp        | imp(const state_formula& left, const state_formula& right)                                                      | The implication operator for state formulas
StateForall     | forall(const data::variable_list& variables, const state_formula& operand)                                      | The universal quantification operator for state formulas
StateExists     | exists(const data::variable_list& variables, const state_formula& operand)                                      | The existential quantification operator for state formulas
StateMust       | must(const regular_formulas::regular_formula& formula, const state_formula& operand)                            | The must operator for state formulas
StateMay        | may(const regular_formulas::regular_formula& formula, const state_formula& operand)                             | The may operator for state formulas
StateYaled      | yaled()                                                                                                         | The yaled operator for state formulas
StateYaledTimed | yaled_timed(const data::data_expression& time_stamp)                                                            | The timed yaled operator for state formulas
StateDelay      | delay()                                                                                                         | The delay operator for state formulas      
StateDelayTimed | delay_timed(const data::data_expression& time_stamp)                                                            | The timed delay operator for state formulas
StateVar        | variable(const core::identifier_string& name, const data::data_expression_list& arguments)                      | The state formula variable
StateNu         | nu(const core::identifier_string& name, const data::assignment_list& assignments, const state_formula& operand) | The nu operator for state formulas
StateMu         | mu(const core::identifier_string& name, const data::assignment_list& assignments, const state_formula& operand) | The mu operator for state formulas
'''

ACTION_FORMULA_CLASSES = r'''
ActTrue   | true_()                                                                     | The value true for action formulas  
ActFalse  | false_()                                                                    | The value false for action formulas 
ActNot    | not_(const action_formula& operand)                                         | The not operator for action formulas
ActAnd    | and_(const action_formula& left, const action_formula& right)               | The and operator for action formulas
ActOr     | or_(const action_formula& left, const action_formula& right)                | The or operator for action formulas 
ActImp    | imp(const action_formula& left, const action_formula& right)                | The implication operator for action formulas               
ActForall | forall(const data::variable_list& variables, const action_formula& operand) | The universal quantification operator for action formulas  
ActExists | exists(const data::variable_list& variables, const action_formula& operand) | The existential quantification operator for action formulas
ActAt     | at(const action_formula& operand, const data::data_expression& time_stamp)  | The at operator for action formulas
'''                                                                                       

PROCESS_EXPRESSION_CLASSES = r'''
Action            | action(const lps::action_label& l, const data::data_expression_list& v) (label, arguments)                                                       | An action
Process           | process_instance(const process_identifier pi, const data::data_expression_list& v) (identifier, actual_parameters)                               | A process
ProcessAssignment | process_instance_assignment(const process_identifier& pi, const data::assignment_list& v) (identifier, assignments)                              | A process assignment
Delta             | delta() ()                                                                                                                                       | The value delta
Tau               | tau() ()                                                                                                                                         | The value tau
Sum               | sum(const data::variable_list& v, const process_expression& right) (bound_variables, operand)                                                    | The sum operator
Block             | block(const core::identifier_string_list& s, const process_expression& right) (block_set, operand)                                               | The block operator
Hide              | hide(const core::identifier_string_list& s, const process_expression& right) (hide_set, operand)                                                 | The hide operator
Rename            | rename(const rename_expression_list& r, const process_expression& right) (rename_set, operand)                                                   | The rename operator
Comm              | comm(const communication_expression_list& c, const process_expression& right) (comm_set, operand)                                                | The communication operator
Allow             | allow(const action_name_multiset_list& s, const process_expression& right) (allow_set, operand)                                                  | The allow operator
Sync              | sync(const process_expression& left, const process_expression& right) (left, right)                                                              | The synchronization operator
AtTime            | at(const process_expression& left, const data::data_expression& d) (operand, time_stamp)                                                         | The at operator
Seq               | seq(const process_expression& left, const process_expression& right) (left, right)                                                               | The sequential composition
IfThen            | if_then(const data::data_expression& d, const process_expression& right) (condition, then_case)                                                  | The if-then operator
IfThenElse        | if_then_else(const data::data_expression& d, const process_expression& left, const process_expression& right) (condition, then_case, else_case)  | The if-then-else operator
BInit             | bounded_init(const process_expression& left, const process_expression& right) (left, right)                                                      | The bounded initialization
Merge             | merge(const process_expression& left, const process_expression& right) (left, right)                                                             | The merge operator
LMerge            | left_merge(const process_expression& left, const process_expression& right) (left, right)                                                        | The left merge operator
Choice            | choice(const process_expression& left, const process_expression& right) (left, right)                                                            | The choice operator
'''

PROCESS_EXPRESSION_TEXT = '''action(const lps::action_label& l, const data::data_expression_list& v) (label, arguments)
process_instance(const process_identifier pi, const data::data_expression_list& v) (identifier, actual_parameters)
process_instance_assignment(const process_identifier& pi, const data::assignment_list& v) (identifier, assignments)
delta() ()
tau() ()
sum(const data::variable_list& v, const process_expression& right) (bound_variables, operand)
block(const core::identifier_string_list& s, const process_expression& right) (block_set, operand)
hide(const core::identifier_string_list& s, const process_expression& right) (hide_set, operand)
rename(const rename_expression_list& r, const process_expression& right) (rename_set, operand)
comm(const communication_expression_list& c, const process_expression& right) (comm_set, operand)
allow(const action_name_multiset_list& s, const process_expression& right) (allow_set, operand)
sync(const process_expression& left, const process_expression& right) (left, right)
at(const process_expression& left, const data::data_expression& d) (operand, time_stamp)
seq(const process_expression& left, const process_expression& right) (left, right)
if_then(const data::data_expression& d, const process_expression& right) (condition, then_case)
if_then_else(const data::data_expression& d, const process_expression& left, const process_expression& right) (condition, then_case, else_case)
bounded_init(const process_expression& left, const process_expression& right) (left, right)
merge(const process_expression& left, const process_expression& right) (left, right)
left_merge(const process_expression& left, const process_expression& right) (left, right)
choice(const process_expression& left, const process_expression& right) (left, right)'''

PROCESS_EXPRESSION_TERMS = '''
  action                      Action           
  process_instance            Process          
  process_instance_assignment ProcessAssignment
  delta                       Delta            
  tau                         Tau              
  sum                         Sum              
  block                       Block            
  hide                        Hide             
  rename                      Rename           
  comm                        Comm             
  allow                       Allow            
  sync                        Sync             
  at                          AtTime           
  seq                         Seq              
  if_then                     IfThen           
  if_then_else                IfThenElse       
  bounded_init                BInit            
  merge                       Merge            
  left_merge                  LMerge           
  choice                      Choice           
'''

PBES_EXPRESSION_TEXT = '''true_() ()
false_() ()
not_(const pbes_expression& p) (operand)
and_(const pbes_expression& left, const pbes_expression& right) (left, right)
or_(const pbes_expression& left, const pbes_expression& right) (left, right)
imp(const pbes_expression& left, const pbes_expression& right) (left, right)
forall(const data::variable_list& l, const pbes_expression& p) (variables, body)
exists(const data::variable_list& l, const pbes_expression& p) (variables, body)'''

PBES_EXPRESSION_TERMS = '''
  true_                      PBESTrue
  false_                     PBESFalse
  not_                       PBESNot
  and_                       PBESAnd
  or_                        PBESOr
  imp                        PBESImp
  forall                     PBESForall
  exists                     PBESExists
'''

def member_function(arg, n):
    p = arg.rpartition(' ')
    type = p[0].strip()
    type = re.sub('^const\s*', '', type)
    type = re.sub('\s*&$', '', type)
    name = p[2].strip()
    arg = 'arg' + str(n)
    if type.endswith('list'):
        arg = 'list_' + arg
    text = MEMBER_FUNCTION
    text = re.sub('TYPE', type, text)
    text = re.sub('NAME', name, text)
    text = re.sub('ARG', arg, text)
    return text
                                                                                          
def parse_classes(text, superclass):
    result = []
    lines = text.rsplit('\n')
    for line in lines:
        words = line.split('|')
        if len(words) != 3:
            continue
        aterm = words[0].strip()
        description = words[2].strip()
        classname = re.sub('\(.*', '', words[1].strip())
        constructor = words[1].strip()
        w = words[1].strip()
        w = re.sub('.*\(', '', w)
        w = re.sub('\).*', '', w)
        args = w.split(',')
        parameters = []
        member_functions = []
        index = 1
        for arg in args:
            if arg.strip() == '':
                continue
            parameters.append(arg.split(' ')[-1])
            member_functions.append(member_function(arg, index))
            index = index + 1
        parameters = ', '.join(parameters)
        mtext = '\n\n'.join(member_functions)
        if mtext != '':
            mtext = '\n\n' + mtext
        ctext = CLASS_DEFINITION
        ctext = re.sub('DESCRIPTION'     , description, ctext)
        ctext = re.sub('CLASSNAME'       , classname  , ctext)
        ctext = re.sub('ATERM'           , aterm      , ctext)
        ctext = re.sub('CONSTRUCTOR'     , constructor, ctext)
        ctext = re.sub('PARAMETERS'      , parameters , ctext)
        ctext = re.sub('SUPERCLASS'      , superclass , ctext)
        ctext = re.sub('MEMBER_FUNCTIONS', mtext, ctext)
        result.append(ctext)
    return result

def make_expression_classes(filename, class_text, class_name):
    classes = parse_classes(class_text, class_name)
    ctext = '\n\n'.join(classes) + '\n'
    text = path(filename).text()
    text = re.compile(r'//--- start generated text ---//.*//--- end generated text ---//', re.S).sub(
                  '//--- start generated text ---//\n' + ctext + '//--- end generated text ---//',
                  text)
    path(filename).write_text(text)   

EXPRESSION_VISITOR_CODE = r'''/// \\brief Visitor class for expressions.
///
/// There is a visit_<node> and a leave_<node>
/// function for each type of node. By default these functions do nothing, so they
/// must be overridden to add behavior. If the visit_<node> function returns true,
/// the recursion is continued in the children of the node.
template <typename Arg=void>
struct MYEXPRESSION_visitor
{
  /// \\brief The type of the additional argument for the recursion
  typedef Arg argument_type;

  /// \\brief These names can be used as return types of the visit functions, to make
  /// the code more readible.
  enum return_type
  {
    stop_recursion = false,
    continue_recursion = true
  };

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_visitor()
  { }
%s
  /// \\brief Visits the nodes of the pbes expression, and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  /// \param a An additional argument for the recursion
  void visit(const MYEXPRESSION& x, Arg& a)
  {
%s
  }
};

/// \\brief Visitor class for expressions.
///
/// If a visit_<node> function returns true, the recursion is continued
/// in the children of this node, otherwise not.
template <>
struct MYEXPRESSION_visitor<void>
{
  /// \\brief The type of the additional argument for the recursion
  typedef void argument_type;

  /// \\brief These names can be used as return types of the visit functions, to make
  /// the code more readible.
  enum return_type
  {
    stop_recursion = false,
    continue_recursion = true
  };

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_visitor()
  { }
%s

  /// \\brief Visits the nodes of the expression and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  void visit(const MYEXPRESSION& x)
  {
%s
  }
};
'''

EXPRESSION_BUILDER_CODE = r'''/// \\brief Modifying visitor class for expressions.
///
/// During traversal
/// of the nodes, the expression is rebuilt from scratch.
/// If a visit_<node> function returns MYEXPRESSION(), the recursion is continued
/// in the children of this node, otherwise not.
/// An arbitrary additional argument may be passed during the recursion.
template <typename Arg = void>
struct MYEXPRESSION_builder
{
  /// \\brief The type of the additional argument for the recursion
  typedef Arg argument_type;

  /// \\brief Returns true if the expression is not equal to MYEXPRESSION().
  /// This is used to determine if the recursion in a node needs to be continued.
  /// \\param x A expression
  /// \\return True if the term is not equal to MYEXPRESSION()
  bool is_finished(const MYEXPRESSION& x)
  {
    return x != MYEXPRESSION();
  }

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_builder()
  { }
%s
  /// \\brief Visits the nodes of the expression, and calls the corresponding visit_<node>
  /// member functions. If the return value of a visit function equals MYEXPRESSION(),
  /// the recursion in this node is continued automatically, otherwise the returned
  /// value is used for rebuilding the expression.
  /// \\param x A expression
  /// \\param a An additional argument for the recursion
  /// \\return The visit result
  MYEXPRESSION visit(const MYEXPRESSION& x, Arg& a)
  {
#ifdef MCRL2_PROCESS_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit>" << pp(x) << std::endl;
#endif
    MYEXPRESSION result;
%s
#ifdef MCRL2_PROCESS_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit result>" << pp(result) << std::endl;
#endif
    return result;
  }
};

/// \\brief Modifying visitor class for expressions.
///
/// If a visit_<node> function returns true, the recursion is continued
/// in the children of this node, otherwise not.
template <>
struct MYEXPRESSION_builder<void>
{
  /// \\brief The type of the additional argument for the recursion
  typedef void argument_type;

  /// \\brief Returns true if the expression is not equal to MYEXPRESSION().
  /// This is used to determine if the recursion in a node needs to be continued.
  /// \\param x A expression
  /// \\return True if the term is not equal to MYEXPRESSION()
  bool is_finished(const MYEXPRESSION& x)
  {
    return x != MYEXPRESSION();
  }

  /// \\brief Destructor.
  virtual ~MYEXPRESSION_builder()
  { }
%s

  /// \\brief Visits the nodes of the expression and calls the corresponding visit_<node>
  /// member functions. If the return value of a member function equals false, then the
  /// recursion in this node is stopped.
  /// \param x A term
  MYEXPRESSION visit(const MYEXPRESSION& x)
  {
#ifdef MCRL2_PROCESS_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit>" << pp(x) << std::endl;
#endif
    MYEXPRESSION result;
%s
#ifdef MCRL2_PROCESS_EXPRESSION_BUILDER_DEBUG
  std::cerr << "<visit result>" << pp(result) << std::endl;
#endif
    return result;
  }
};
'''

EXPRESSION_VISITOR_NODE_TEXT = r'''
  /// \\brief Visit NODE node
  /// \\return The result of visiting the node
  virtual bool visit_NODE(const MYEXPRESSION& xARGUMENTSEXTRA_ARG)
  {
    return continue_recursion;
  }

  /// \\brief Leave NODE node
  virtual void leave_NODE()
  {}
'''

EXPRESSION_BUILDER_NODE_TEXT = r'''              
  /// \\brief Visit NODE node
  /// \\return The result of visiting the node
  virtual MYEXPRESSION visit_NODE(const MYEXPRESSION& xARGUMENTSEXTRA_ARG)
  {
    return MYEXPRESSION();
  }
'''

def indent_text(text, indent):
    lines = []
    for line in string.split(text, '\n'):
        lines.append(indent + line)
    return string.join(lines, '\n')

def split_arguments(arguments):
    if arguments.strip() == '':
        return ([], [])
    arguments = re.sub('const ', '', arguments)
    arguments = re.sub('&', '', arguments)
    words = map(string.strip, arguments.split(','))
    variables = []
    names = []
    for word in words:
        w = word.split(' ')
        (v, n) = w
        variables.append(v)
        names.append(n)
    return (variables, names)

def make_expression_visitor(filename, expression, expression_text):
    lines = expression_text.split('\n')
    vtext = ''
    wtext = ''
    else_text = ''
    for line in lines:
        words = map(string.strip, re.split('\(', line))
        node = words[0]
        arguments = words[1][:-1]
        accessors = re.split(r',\s*', words[2][:-1])
    
        text = EXPRESSION_VISITOR_NODE_TEXT

        text = re.sub('MYEXPRESSION', expression, text)
        text = re.sub('NODE', node, text)
        args = arguments
        if args.strip() != '':
            args = ', ' + args
        text = re.sub('ARGUMENTS', args, text)
        vtext = vtext + text
    
        (types, names) = split_arguments(arguments)
    
        #--- generate code fragments like this
        #
        #    if (is_imp(e))
        #    {
        #      term_type l = left(e);
        #      term_type r = right(e);
        #      bool result = visit_imp(e, l, r);
        #      if (result) {
        #        visit(l);
        #        visit(r);
        #      }
        #      leave_imp();
        #    }
        text = ''
        text = text + '%sif (is_%s(x))\n' % (else_text, node)
        if else_text == '':
            else_text = 'else '
        text = text + '{\n'
        for i in range(len(types)):
            text = text + '  %s %s = %s(x).%s();\n' % (types[i], names[i], node, accessors[i])
        has_children = re.search('process_expression', line) != None
        args = ', '.join(names)
        if args != '':
            args = ', ' + args
        rtext = ''
        if has_children:
            rtext = 'bool result = '
        text = text + '  %svisit_%s(x%sEXTRA_ARG);\n' % (rtext, node, args)
        if has_children:
            text = text + '  if (result) {\n'
            for i in range(len(types)):
                if types[i] == 'process_expression':
                    text = text + '    visit(%sEXTRA_ARG);\n' % names[i]
            text = text + '  }\n'
        text = text + '  leave_%s();\n' % node
        text = text + '}\n'
        wtext = wtext + text
    wtext = indent_text(wtext, '    ')
    
    vtext1 = re.sub('EXTRA_ARG', '', vtext)
    vtext2 = re.sub('EXTRA_ARG', ', Arg& /* a */', vtext)
    wtext1 = re.sub('EXTRA_ARG', '', wtext)
    wtext2 = re.sub('EXTRA_ARG', ', a', wtext)
    
    rtext = EXPRESSION_VISITOR_CODE % (vtext2, wtext2, vtext1, wtext1)
    rtext = re.sub('MYEXPRESSION', expression, rtext)
    text = path(filename).text()
    text = re.compile(r'//--- start generated text ---//.*//--- end generated text ---//', re.S).sub(
                  '//--- start generated text ---//\n' + rtext + '//--- end generated text ---//',
                  text)
    path(filename).write_text(text)

def make_expression_builder(filename, expression, expression_text):
    lines = expression_text.split('\n')
    vtext = ''
    wtext = ''
    else_text = ''
    for line in lines:
        words = map(string.strip, re.split('\(', line))
        node = words[0]
        arguments = words[1][:-1]
        accessors = re.split(r',\s*', words[2][:-1])
    
        text = EXPRESSION_BUILDER_NODE_TEXT

        text = re.sub('MYEXPRESSION', expression, text)
        text = re.sub('NODE', node, text)
        args = arguments
        if args.strip() != '':
            args = ', ' + args
        text = re.sub('ARGUMENTS', args, text)
        vtext = vtext + text
    
        (types, names) = split_arguments(arguments)
    
        #--- generate code fragments like this
        #
        #    if (is_and(x))
        #    {
        #      process_expression l = left(x);
        #      process_expression r = right(x);
        #      result = visit_and(x, l, r, a);
        #      if (!is_finished(result)) {
        #        result = core::optimized_and(visit(l, a), visit(r, a));
        #      }
        #    }  
        text = ''
        text = text + '%sif (is_%s(x))\n' % (else_text, node)
        if else_text == '':
            else_text = 'else '
        text = text + '{\n'
        for i in range(len(types)):
            text = text + '  %s %s = %s(x).%s();\n' % (types[i], names[i], node, accessors[i])
        args = ', '.join(names)
        if args != '':
            args = ', ' + args
        text = text + '  result = visit_%s(x%sEXTRA_ARG);\n' % (node, args)
        text = text + '  if (!is_finished(result))\n'
        text = text + '  {\n'
        stext = ''
        for i in range(len(types)):
            if stext != '':
                stext = stext + ', '
            if types[i] == 'process_expression':
                stext = stext + 'visit(%sEXTRA_ARG)' % names[i]
            else:
                stext = stext + names[i]
        text = text + '    result = %s(%s);\n' % (node, stext)
        text = text + '  }\n'
        text = text + '}\n'
        wtext = wtext + text
    wtext = indent_text(wtext, '    ')
    
    vtext1 = re.sub('EXTRA_ARG', '', vtext)
    vtext2 = re.sub('EXTRA_ARG', ', Arg& /* a */', vtext)
    wtext1 = re.sub('EXTRA_ARG', '', wtext)
    wtext2 = re.sub('EXTRA_ARG', ', a', wtext)
    
    rtext = EXPRESSION_BUILDER_CODE % (vtext2, wtext2, vtext1, wtext1)
    rtext = re.sub('MYEXPRESSION', expression, rtext)
    text = path(filename).text()
    text = re.compile(r'//--- start generated text ---//.*//--- end generated text ---//', re.S).sub(
                  '//--- start generated text ---//\n' + rtext + '//--- end generated text ---//',
                  text)
    path(filename).write_text(text)

def make_is_functions(filename, term_text):
    TERM_TRAITS_TEXT = r'''
    /// \\brief Test for a %s expression
    /// \\param t A term
    /// \\return True if it is a %s expression
    inline
    bool is_%s(const process_expression& t)
    {
      return core::detail::gsIs%s(t);
    }
'''

    rtext = ''
    terms = term_text.rsplit('\n')
    for t in terms:
        words = t.split()
        if len(words) != 2:
            continue
        rtext = rtext + TERM_TRAITS_TEXT % (words[0], words[0], words[0], words[1])
    text = path(filename).text()
    text = re.compile(r'//--- start generated text ---//.*//--- end generated text ---//', re.S).sub(
                  '//--- start generated text ---//\n' + rtext + '//--- end generated text ---//',
                  text)
    path(filename).write_text(text)

make_expression_visitor('../include/mcrl2/process/process_expression_visitor.h', 'process_expression', PROCESS_EXPRESSION_TEXT)
make_expression_builder('../include/mcrl2/process/process_expression_builder.h', 'process_expression', PROCESS_EXPRESSION_TEXT)
make_is_functions('../include/mcrl2/process/process_expression.h', PROCESS_EXPRESSION_TERMS)

# make_expression_visitor('../../pbes/include/mcrl2/pbes/pbes_expression_visitor.h', 'pbes_expression', PBES_EXPRESSION_TEXT)
# make_expression_builder('../../pbes/include/mcrl2/pbes/pbes_expression_builder.h', 'pbes_expression', PBES_EXPRESSION_TEXT)
# make_is_functions('../../pbes/include/mcrl2/pbes/pbes_expression.h', PBES_EXPRESSION_TERMS)

make_expression_classes('../include/mcrl2/modal_formula/state_formula.h', STATE_FORMULA_CLASSES, 'state_formula')
make_expression_classes('../include/mcrl2/modal_formula/action_formula.h', ACTION_FORMULA_CLASSES, 'action_formula')
