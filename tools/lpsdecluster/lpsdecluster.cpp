// ======================================================================
//
// Copyright (c) 2006 TU/e
//
// ----------------------------------------------------------------------
//
// file          : lpsdecluster 
// date          : 22-12-2006
// version       : 0.5
//
// author(s)     : Jeroen Keiren <j.j.a.keiren@student.tue.nl>
//
// ======================================================================

//C++
#include <exception>
#include <cstdio>

//Boost
#include <boost/program_options.hpp>

//Lowlevel library for gsErrorMsg
#include <libprint_c.h>

//Aterms
#include <atermpp/aterm.h>
#include <atermpp/algorithm.h>

//LPS Framework
#include <mcrl2/lps/function.h>
#include <mcrl2/lps/linear_process.h>
#include <mcrl2/lps/specification.h>
#include <mcrl2/lps/sort_utility.h>

//Enumerator
#include <libnextstate.h>
#include <enum_standard.h>

using namespace std;
using namespace atermpp;
using namespace lps;

namespace po = boost::program_options;

#define VERSION "0.5"

///////////////////////////////////////////////////////////////////////////////
/// \brief structure that holds all options available for the tool.
///
typedef struct
{
  std::string input_file; ///< Name of the file to read input from
  std::string output_file; ///< Name of the file to write output to (or stdout)
  bool finite_only; ///< Only decluster finite sorts
  RewriteStrategy strategy; ///< Rewrite strategy to use, default inner
}tool_options;

//Squadt connectivity
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <utilities/mcrl2_squadt.h>

//Forward declaration because do_decluster() is called within squadt_interactor class
int do_decluster(const tool_options& options);

class squadt_interactor: public mcrl2_squadt::tool_interface
{
  private:

    static const char*  lps_file_for_input;  ///< file containing an LPS that can be imported
    static const char*  lps_file_for_output; ///< file used to write the output to

    static const char*  option_finite_only;
    static const char*  option_rewrite_strategy;

  public:
    
    /** \brief configures tool capabilities */
    void set_capabilities(sip::tool::capabilities&) const;

    /** \brief queries the user via SQuADT if needed to obtain configuration information */
    void user_interactive_configuration(sip::configuration&);

    /** \brief check an existing configuration object to see if it is usable */
    bool check_configuration(sip::configuration const&) const;

    /** \brief performs the task specified by a configuration */
    bool perform_task(sip::configuration&);
};

const char* squadt_interactor::lps_file_for_input  = "lps_in";
const char* squadt_interactor::lps_file_for_output = "lps_out";

const char* squadt_interactor::option_finite_only      = "finite_only";
const char* squadt_interactor::option_rewrite_strategy = "rewrite_strategy";

void squadt_interactor::set_capabilities(sip::tool::capabilities& capabilities) const
{
  // The tool has only one main input combination
  capabilities.add_input_combination(lps_file_for_input, sip::mime_type("lps", sip::mime_type::application), sip::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(sip::configuration& configuration)
{
  using namespace sip;
  using namespace sip::layout;
  using namespace sip::datatype;
  using namespace sip::layout::elements;

  /* Set defaults where the supplied configuration does not have values */
  if (!configuration.output_exists(lps_file_for_output)) {
    configuration.add_output(lps_file_for_output, sip::mime_type("lps", sip::mime_type::application), configuration.get_output_name(".lps"));
  }

  if (!configuration.option_exists(option_rewrite_strategy)) {
    configuration.add_option(option_rewrite_strategy).append_argument(mcrl2_squadt::rewrite_strategy_enumeration, 0);
  }

  if (!configuration.option_exists(option_finite_only)) {
    configuration.add_option(option_finite_only).
       set_argument_value< 0, sip::datatype::boolean >(true, false);
  }

  /* Prepare user interaction */
  layout::manager::aptr top(layout::vertical_box::create());
  layout::manager* current_box = new horizontal_box();

  squadt_utility::radio_button_helper < RewriteStrategy >
                                        strategy_selector(current_box, GS_REWR_INNER, "Inner");
  strategy_selector.associate(current_box, GS_REWR_INNERC, "Innerc");
  strategy_selector.associate(current_box, GS_REWR_JITTY,  "Jitty");
  strategy_selector.associate(current_box, GS_REWR_JITTYC, "Jittyc");

  if (configuration.option_exists(option_rewrite_strategy)) {
    strategy_selector.set_selection(static_cast < RewriteStrategy > (
        configuration.get_option_argument< size_t >(option_rewrite_strategy, 0)));
  }
  
  top->add(new label("Rewrite strategy"));
  top->add(current_box);

  current_box = new horizontal_box();
  checkbox* finite_only = new checkbox("Only decluster variables of finite sorts", 
        configuration.get_option_argument< bool >(option_finite_only));
  
  current_box->add(finite_only, layout::left);
  top->add(new label(" "));
  top->add(current_box);
  
  button* okay_button = new button("OK");
  top->add(new label(" "));
  top->add(okay_button, layout::right);

  send_display_layout(top);

  okay_button->await_change();
  
  /* Update configuration */
  configuration.get_option(option_finite_only).
     set_argument_value< 0, sip::datatype::boolean >(finite_only->get_status());

  configuration.get_option(option_rewrite_strategy).replace_argument(0, mcrl2_squadt::rewrite_strategy_enumeration, strategy_selector.get_selection());
}

bool squadt_interactor::check_configuration(sip::configuration const& configuration) const
{
  bool result = true;
  result |= configuration.input_exists(lps_file_for_input);
  result |= configuration.output_exists(lps_file_for_output);
  result |= configuration.option_exists(option_rewrite_strategy);

  return result;
}

bool squadt_interactor::perform_task(sip::configuration& configuration)
{
  using namespace sip;
  using namespace sip::layout;
  using namespace sip::datatype;
  using namespace sip::layout::elements;

  bool result = true;
  
  tool_options options;
  options.input_file = configuration.get_input(lps_file_for_input).get_location();
  options.output_file = configuration.get_output(lps_file_for_output).get_location();
  options.finite_only = configuration.option_exists(option_finite_only);
  options.strategy = static_cast < RewriteStrategy > (boost::any_cast < size_t > (configuration.get_option_argument(option_rewrite_strategy, 0)));

  layout::manager::aptr top(layout::vertical_box::create());
  
  top->add(new label("Declustering in progress"), layout::left);
  send_display_layout(top);

  //Perform declustering
  top = layout::vertical_box::create();
  int decluster_result = do_decluster(options);
  if (decluster_result == 0) {
    top->add(new label("Declustering succeeded"));
    result = true;
  }
  else
  {
    top->add(new label("Declustering failed"));
  }

  send_display_layout(top);

  return result;
}

#endif //ENABLE_SQUADT_CONNECTIVITY

/////////////////////////////////////////////////////////////////
// Helpsr functions
/////

///Used to assist in occurs_in function.
struct compare_data_variable
{
  aterm v;

  compare_data_variable(data_variable v_)
    : v(aterm_appl(v_))
  {}
  
  bool operator()(aterm t) const
  {
    return v == t;
  }
};

///\ret variable v occurs in l.
template <typename data_type>
bool occurs_in(data_type l, lps::data_variable v)
{
  return find_if(l, compare_data_variable(v)) != aterm();
}


///\ret a list of all data_variables of sort s in vl
lps::data_variable_list get_occurrences(const data_variable_list& vl, const lps::sort& s)
{
  data_variable_list result;
  for (data_variable_list::iterator i = vl.begin(); i != vl.end(); ++i)
  {
    if (i->sort() == s)
    {
      result = push_front(result, *i);
    }
  }
  result = reverse(result);
  return result;
}

///\ret the list of all data_variables in vl, which are unequal to v
lps::data_variable_list filter(const data_variable_list& vl, const data_variable& v)
{
  gsDebugMsg("filter:vl = %s, v = %s\n", vl.to_string().c_str(), v.to_string().c_str());
  data_variable_list result;
  for (data_variable_list::iterator i = vl.begin(); i != vl.end(); ++i)
  {
    if (!(*i == v))
    {
      result = push_front(result, *i);
    }
  }
  gsDebugMsg("filter:result = %s\n", result.to_string().c_str());
  return result;
}

///\ret the list of all date_variables in vl, that are not in rl
lps::data_variable_list filter(const data_variable_list& vl, const data_variable_list& rl)
{
  data_variable_list result;
  for (data_variable_list::iterator i = vl.begin(); i != vl.end(); ++i)
  {
    if (!occurs_in(rl, *i))
    {
      result = push_front(result, *i);
    }
  }

  return result;
}

///\pre fl is a list of constructors
///\ret a list of declusterable sorts in sl
sort_list get_finite_sorts(const function_list& fl, const sort_list& sl)
{
  sort_list result;
  for(sort_list::iterator i = sl.begin(); i != sl.end(); ++i)
  {
    if (is_finite(fl, *i))
    {
      result = push_front(result, *i);
    }
  }
  reverse(result);
  return result;
}

///\ret a list of all variables of a sort that occurs in sl
data_variable_list get_variables(const data_variable_list& vl, const sort_list& sl)
{
  data_variable_list result;
  for (data_variable_list::iterator i = vl.begin(); i != vl.end(); ++i)
  {
    if (occurs_in(sl, i->sort()))
    {
      result = push_front(result, *i);
    }
  }
  result = reverse(result);
  return result;
}


////////////////////////////////////////////////////////////////
// Declustering
/////

///\pre specification is the specification belonging to summand
///\post the declustered version of summand has been appended to result
///\ret none
void decluster_summand(const lps::specification& specification, const lps::summand& summand_, lps::summand_list& result, EnumeratorStandard& enumerator, bool finite_only)
{
  int nr_summands = 0; // Counter for the nummer of new summands, used for verbose output

  gsVerboseMsg("initialization...");

  data_variable_list variables; // The variables we need to consider in declustering
  if (finite_only)
  {
    // Only consider finite variables
    variables = get_variables(summand_.summation_variables(), get_finite_sorts(specification.data().constructors(), specification.data().sorts()));
  }
  else
  {
    variables = summand_.summation_variables();
  }

  // List of variables with the declustered variables removed (can be done upfront, which is more efficient,
  // because we only need to calculate it once.
  data_variable_list new_vars = filter(summand_.summation_variables(), variables);

  ATermList vars = ATermList(variables);

  ATerm expr = enumerator.getRewriter()->toRewriteFormat(aterm_appl(summand_.condition()));

  // Solutions
  EnumeratorSolutions* sols = enumerator.findSolutions(vars, expr, false, NULL);

  gsVerboseMsg("processing...");
  // sol is a solution in internal rewriter format
  ATermList sol;
  while (sols->next(&sol))
  {
    data_assignment_list substitutions; 

    // Convenience cast, so that the iterator, and the modifications from the atermpp library can be used
    aterm_list solution = aterm_list(sol);

    // Translate internal rewriter solution to lps data_assignment_list
    for (aterm_list::iterator i = solution.begin(); i != solution.end(); ++i)
    {
      // lefthandside of substitution
      data_variable var = data_variable(ATgetArgument(ATerm(*i), 0));

      // righthandside of substitution in internal rewriter format
      ATerm arg = ATgetArgument(ATerm(*i),1);

      // righthandside of substitution in lps format
      data_expression res = data_expression(aterm_appl(enumerator.getRewriter()->fromRewriteFormat(arg)));

      // Substitution to be performed
      data_assignment substitution = data_assignment(var, res);
      substitutions = push_front(substitutions, substitution);
    }
    gsDebugMsg("substitutions: %s\n", substitutions.to_string().c_str());

    summand s = summand(new_vars,
                                summand_.condition().substitute(assignment_list_substitution(substitutions)),
                                summand_.is_delta(),
                                summand_.actions().substitute(assignment_list_substitution(substitutions)),
                                summand_.time().substitute(assignment_list_substitution(substitutions)),
                                summand_.assignments().substitute(assignment_list_substitution(substitutions))
                                );
    
    result = push_front(result, s);
    ++nr_summands;
  }

  gsVerboseMsg("done...\n");
  gsVerboseMsg("Replaced with %d summands\n", nr_summands);
}

///Takes the summand list sl, declusters it,
///and returns the declustered summand list
lps::summand_list decluster_summands(const lps::specification& specification,
                                     const lps::summand_list& sl,
                                     EnumeratorStandard& enumerator, 
                                     const tool_options& options)
{
  lps::summand_list result;

  // decluster_summand(..) is called only in this function, therefore, it is safe to count the summands here for verbose output.
  lps::summand_list summands = reverse(sl); // This is not absolutely necessary, but it helps in comparing input and output of the decluster algorithm (that is, the relative order is preserved (because decluster_summand plainly appends to result)
  int j = 1;
  for (summand_list::iterator i = summands.begin(); i != summands.end(); ++i, ++j)
  {
    gsVerboseMsg("Summand %d\n", j);
    lps::summand s = *i;
    decluster_summand(specification, s, result, enumerator, options.finite_only);
  }

  return result;
}

///Takes the specification in specification, declusters it,
///and returns the declustered specification.
lps::specification decluster(const lps::specification& specification, const tool_options& options)
{
  gsVerboseMsg("Declustering...\n");
  gsVerboseMsg("Using rewrite strategy %d\n", options.strategy);
  lps::linear_process lps = specification.process();

  gsVerboseMsg("Input: %d summands.\n", lps.summands().size());

  // Some use of internal format because we need it for the rewriter
  Rewriter* rewriter = createRewriter(specification.data(), options.strategy);
  EnumeratorStandard enumerator = EnumeratorStandard(specification, rewriter);

  lps::summand_list sl = decluster_summands(specification, lps.summands(), enumerator, options);
  lps = set_summands(lps, sl);

  gsVerboseMsg("Output: %d summands.\n", lps.summands().size());

  return set_lps(specification, lps);
}

///Reads a specification from input_file, 
///applies declustering to it and writes the result to output_file.
int do_decluster(const tool_options& options)
{
  lps::specification lps_specification;
  
  if (lps_specification.load(options.input_file)) {
    // decluster lps_specification and save the output to a binary file
    if (!decluster(lps_specification, options).save(options.output_file, true)) 
    {
      // An error occurred when saving
      gsErrorMsg("Could not save to '%s'\n", options.output_file.c_str());
      return (1);
    }
  }
  else {
    gsErrorMsg("lpsdecluster: Unable to load LPS from `%s'\n", options.input_file.c_str());
    return (1);
  }

  return 0;
}

///Parses command line and sets settings from command line switches
void parse_command_line(int ac, char** av, tool_options& t_options) {
  po::options_description desc;
  std::string rewriter;

  desc.add_options()
      ("help,h",      "display this help")
      ("verbose,v",   "turn on the display of short intermediate messages")
      ("debug,d",     "turn on the display of detailed intermediate messages")
      ("finite,f",    "only decluster finite sorts")
      ("rewriter,R",   po::value<std::string>(&rewriter)->default_value("inner"), "use rewriter arg (default 'inner');"
                      "available rewriters are inner, jitty, innerc and jittyc")
      ("version",     "display version information")
  ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("INFILE", po::value< string >(), "input file")
      ("OUTFILE", po::value< string >(), "output file")
  ;
  
  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);
      
  po::options_description visible("Allowed options");
  visible.add(desc);

  po::positional_options_description p;
  p.add("INFILE", 1);
  p.add("OUTFILE", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(ac, av).
    options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
  
  if (0 < vm.count("help")) {
    cerr << "Usage: "<< av[0] << " [OPTION]... [INFILE] [OUTFILE]" << endl;
    cerr << "Decluster the LPS in INFILE and store the result to OUTFILE" << endl;

    cerr << endl;
    cerr << desc;

    exit (0);
  }
      
  if (0 < vm.count("version")) {
    cerr << "lpsdecluster " << VERSION << " (revision " << REVISION << ")" << endl;

    exit (0);
  }

  if (0 < vm.count("debug")) {
    gsSetDebugMsg();
  }

  if (0 < vm.count("verbose")) {
    gsSetVerboseMsg();
  }

  t_options.finite_only = (0 < vm.count("finite"));

  t_options.strategy = RewriteStrategyFromString(rewriter.c_str());
  if (t_options.strategy == GS_REWR_INVALID)
  {
    cerr << rewriter << " is not a valid rewriter strategy" << endl;
    exit(EXIT_FAILURE);
  }
  
  t_options.input_file = (0 < vm.count("INFILE")) ? vm["INFILE"].as< string >() : "-";
  t_options.output_file = (0 < vm.count("OUTFILE")) ? vm["OUTFILE"].as< string >() : "-";
}

int main(int ac, char** av) {
  ATerm bot;
  ATinit(ac, av, &bot);
  tool_options options;
  gsEnableConstructorFunctions();

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if (mcrl2_squadt::interactor< squadt_interactor >::free_activation(ac, av)) {
    return 0;
  }
#endif

  parse_command_line(ac,av, options);
  return do_decluster(options);
}
