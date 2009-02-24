#ifndef MCRL2_NEW_DATA_BAG_H
#define MCRL2_NEW_DATA_BAG_H

#include "mcrl2/new_data/basic_sort.h"
#include "mcrl2/new_data/function_sort.h"
#include "mcrl2/new_data/function_symbol.h"
#include "mcrl2/new_data/application.h"
#include "mcrl2/new_data/data_equation.h"
#include "mcrl2/new_data/detail/utility.h"
#include "mcrl2/new_data/standard.h"
#include "mcrl2/new_data/bool.h"
#include "mcrl2/new_data/nat.h"
#include "mcrl2/new_data/set.h"
#include "mcrl2/new_data/container_sort.h"
#include "mcrl2/new_data/lambda.h"
#include "mcrl2/new_data/forall.h"


namespace mcrl2 {

  namespace new_data {

    namespace sort_bag {

      // Sort expression Bag(s)
      inline
      container_sort bag(const sort_expression& s)
      {
        //static container_sort bag("bag", s);
        container_sort bag("bag", s);
        return bag;
      }

      // Recogniser for sort expression Bag(s)
      inline
      bool is_bag(const sort_expression& e)
      {
        if (e.is_container_sort())
        {
          return static_cast<const container_sort&>(e).container_name() == "bag";
        }
        return false;
      }

      // Function symbol @bag
      inline
      function_symbol bag_comprehension(const sort_expression& s)
      {
        //static function_symbol bag_comprehension("@bag", function_sort(function_sort(s, sort_nat::nat()), sort_bag::bag(s)));
        function_symbol bag_comprehension("@bag", function_sort(function_sort(s, sort_nat::nat()), sort_bag::bag(s)));
        return bag_comprehension;
      }

      // Recogniser for @bag
      inline
      bool is_bag_comprehension_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "@bag";
        }
        return false;
      }

      // Application of @bag
      inline
      application bag_comprehension(const sort_expression& s, const data_expression& arg0)
      {
        
        return application(bag_comprehension(s),arg0);
      }

      // Recogniser for application of @bag
      inline
      bool is_bag_comprehension_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bag_comprehension_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol {}
      inline
      function_symbol emptybag(const sort_expression& s)
      {
        //static function_symbol emptybag("{}", sort_bag::bag(s));
        function_symbol emptybag("{}", sort_bag::bag(s));
        return emptybag;
      }

      // Recogniser for {}
      inline
      bool is_emptybag_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "{}";
        }
        return false;
      }

      // Function symbol count
      inline
      function_symbol count(const sort_expression& s)
      {
        //static function_symbol count("count", function_sort(s, sort_bag::bag(s), sort_nat::nat()));
        function_symbol count("count", function_sort(s, sort_bag::bag(s), sort_nat::nat()));
        return count;
      }

      // Recogniser for count
      inline
      bool is_count_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "count";
        }
        return false;
      }

      // Application of count
      inline
      application count(const sort_expression& s, const data_expression& arg0, const data_expression& arg1)
      {
        //assert(arg0.sort() == s);
        //assert(sort_bag::is_bag(arg1.sort()));
        
        return application(count(s),arg0, arg1);
      }

      // Recogniser for application of count
      inline
      bool is_count_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_count_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol in
      inline
      function_symbol bagin(const sort_expression& s)
      {
        //static function_symbol bagin("in", function_sort(s, sort_bag::bag(s), sort_bool_::bool_()));
        function_symbol bagin("in", function_sort(s, sort_bag::bag(s), sort_bool_::bool_()));
        return bagin;
      }

      // Recogniser for in
      inline
      bool is_bagin_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "in";
        }
        return false;
      }

      // Application of in
      inline
      application bagin(const sort_expression& s, const data_expression& arg0, const data_expression& arg1)
      {
        //assert(arg0.sort() == s);
        //assert(sort_bag::is_bag(arg1.sort()));
        
        return application(bagin(s),arg0, arg1);
      }

      // Recogniser for application of in
      inline
      bool is_bagin_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bagin_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol +
      inline
      function_symbol bagunion_(const sort_expression& s)
      {
        //static function_symbol bagunion_("+", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        function_symbol bagunion_("+", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        return bagunion_;
      }

      // Recogniser for +
      inline
      bool is_bagunion__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "+";
        }
        return false;
      }

      // Application of +
      inline
      application bagunion_(const sort_expression& s, const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bag::is_bag(arg0.sort()));
        //assert(sort_bag::is_bag(arg1.sort()));
        
        return application(bagunion_(s),arg0, arg1);
      }

      // Recogniser for application of +
      inline
      bool is_bagunion__application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bagunion__function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol -
      inline
      function_symbol bagdifference(const sort_expression& s)
      {
        //static function_symbol bagdifference("-", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        function_symbol bagdifference("-", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        return bagdifference;
      }

      // Recogniser for -
      inline
      bool is_bagdifference_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "-";
        }
        return false;
      }

      // Application of -
      inline
      application bagdifference(const sort_expression& s, const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bag::is_bag(arg0.sort()));
        //assert(sort_bag::is_bag(arg1.sort()));
        
        return application(bagdifference(s),arg0, arg1);
      }

      // Recogniser for application of -
      inline
      bool is_bagdifference_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bagdifference_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol *
      inline
      function_symbol bagintersection(const sort_expression& s)
      {
        //static function_symbol bagintersection("*", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        function_symbol bagintersection("*", function_sort(sort_bag::bag(s), sort_bag::bag(s), sort_bag::bag(s)));
        return bagintersection;
      }

      // Recogniser for *
      inline
      bool is_bagintersection_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "*";
        }
        return false;
      }

      // Application of *
      inline
      application bagintersection(const sort_expression& s, const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bag::is_bag(arg0.sort()));
        //assert(sort_bag::is_bag(arg1.sort()));
        
        return application(bagintersection(s),arg0, arg1);
      }

      // Recogniser for application of *
      inline
      bool is_bagintersection_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bagintersection_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol Bag2Set
      inline
      function_symbol bag2set(const sort_expression& s)
      {
        //static function_symbol bag2set("Bag2Set", function_sort(sort_bag::bag(s), sort_set::set(s)));
        function_symbol bag2set("Bag2Set", function_sort(sort_bag::bag(s), sort_set::set(s)));
        return bag2set;
      }

      // Recogniser for Bag2Set
      inline
      bool is_bag2set_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "Bag2Set";
        }
        return false;
      }

      // Application of Bag2Set
      inline
      application bag2set(const sort_expression& s, const data_expression& arg0)
      {
        //assert(sort_bag::is_bag(arg0.sort()));
        
        return application(bag2set(s),arg0);
      }

      // Recogniser for application of Bag2Set
      inline
      bool is_bag2set_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_bag2set_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol Set2Bag
      inline
      function_symbol set2bag(const sort_expression& s)
      {
        //static function_symbol set2bag("Set2Bag", function_sort(sort_set::set(s), sort_bag::bag(s)));
        function_symbol set2bag("Set2Bag", function_sort(sort_set::set(s), sort_bag::bag(s)));
        return set2bag;
      }

      // Recogniser for Set2Bag
      inline
      bool is_set2bag_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "Set2Bag";
        }
        return false;
      }

      // Application of Set2Bag
      inline
      application set2bag(const sort_expression& s, const data_expression& arg0)
      {
        //assert(sort_set::is_set(arg0.sort()));
        
        return application(set2bag(s),arg0);
      }

      // Recogniser for application of Set2Bag
      inline
      bool is_set2bag_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_set2bag_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Give all system defined constructors for Bag
      inline
      function_symbol_list bag_generate_constructors_code()
      {
        function_symbol_list result;

        return result;
      }

      // Give all system defined constructors for Bag
      inline
      function_symbol_list bag_generate_functions_code(const sort_expression& s)
      {
        function_symbol_list result;
        result.push_back(bag_comprehension(s));
        result.push_back(emptybag(s));
        result.push_back(count(s));
        result.push_back(bagin(s));
        result.push_back(bagunion_(s));
        result.push_back(bagdifference(s));
        result.push_back(bagintersection(s));
        result.push_back(bag2set(s));
        result.push_back(set2bag(s));

        return result;
      }

      // Function for projecting out right
      inline
      data_expression right(const data_expression& e)
      {
        //assert( || is_count_application(e) || is_bagin_application(e) || is_bagunion__application(e) || is_bagdifference_application(e) || is_bagintersection_application(e));
        
        if (is_count_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_bagin_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_bagunion__application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_bagdifference_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_bagintersection_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out left
      inline
      data_expression left(const data_expression& e)
      {
        //assert( || is_count_application(e) || is_bagin_application(e) || is_bagunion__application(e) || is_bagdifference_application(e) || is_bagintersection_application(e));
        
        if (is_count_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_bagin_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_bagunion__application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_bagdifference_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_bagintersection_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out arg
      inline
      data_expression arg(const data_expression& e)
      {
        //assert( || is_bag_comprehension_application(e) || is_bag2set_application(e) || is_set2bag_application(e));
        
        if (is_bag_comprehension_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_bag2set_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_set2bag_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Give all system defined equations for Bag
      inline
      data_equation_list bag_generate_equations_code(const sort_expression& s)
      {
        data_equation_list result;
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat()))), equal_to(sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat()))), sort_bag::bag_comprehension(s, variable("g", function_sort(s, sort_nat::nat())))), equal_to(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat())))));
        result.push_back(data_equation(variable_list(), sort_bag::emptybag(s), sort_bag::bag_comprehension(s, lambda(make_vector(variable("x", s)),sort_nat::c0()))));
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("d", s)), sort_bag::count(s, variable("d", s), sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat())))), variable("f", function_sort(s, sort_nat::nat()))(variable("d", s))));
        result.push_back(data_equation(make_vector(variable("d", s), variable("s", sort_bag::bag(s))), sort_bag::bagin(s, variable("d", s), variable("s", sort_bag::bag(s))), greater(sort_bag::count(s, variable("d", s), variable("s", sort_bag::bag(s))), sort_nat::c0())));
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat()))), less_equal(sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat()))), sort_bag::bag_comprehension(s, variable("g", function_sort(s, sort_nat::nat())))), forall(make_vector(variable("x", s)),less_equal(variable("f", function_sort(s, sort_nat::nat()))(variable("x", s)), variable("g", function_sort(s, sort_nat::nat()))(variable("x", s))))));
        result.push_back(data_equation(make_vector(variable("t", sort_bag::bag(s)), variable("s", sort_bag::bag(s))), less(variable("s", sort_bag::bag(s)), variable("t", sort_bag::bag(s))), sort_bool_::and_(less_equal(variable("s", sort_bag::bag(s)), variable("t", sort_bag::bag(s))), not_equal_to(variable("s", sort_bag::bag(s)), variable("t", sort_bag::bag(s))))));
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat()))), sort_bag::bagunion_(s, sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat()))), sort_bag::bag_comprehension(s, variable("g", function_sort(s, sort_nat::nat())))), sort_bag::bag_comprehension(s, lambda(make_vector(variable("x", s)),sort_bag::bagunion_(s, variable("f", function_sort(s, sort_nat::nat()))(variable("x", s)), variable("g", function_sort(s, sort_nat::nat()))(variable("x", s)))))));
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat()))), sort_bag::bagdifference(s, sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat()))), sort_bag::bag_comprehension(s, variable("g", function_sort(s, sort_nat::nat())))), sort_bag::bag_comprehension(s, lambda(make_vector(variable("y", s)),(lambda(make_vector(variable("m", sort_nat::nat())),lambda(make_vector(variable("n", sort_nat::nat())),if_(greater(variable("m", sort_nat::nat()), variable("n", sort_nat::nat())), sort_nat::gtesubt(variable("m", sort_nat::nat()), variable("n", sort_nat::nat())), sort_nat::c0())))(variable("f", function_sort(s, sort_nat::nat()))(variable("y", s))))(variable("g", function_sort(s, sort_nat::nat()))(variable("y", s)))))));
        result.push_back(data_equation(make_vector(variable("f", function_sort(s, sort_nat::nat())), variable("g", function_sort(s, sort_nat::nat()))), sort_bag::bagintersection(s, sort_bag::bag_comprehension(s, variable("f", function_sort(s, sort_nat::nat()))), sort_bag::bag_comprehension(s, variable("g", function_sort(s, sort_nat::nat())))), sort_bag::bag_comprehension(s, lambda(make_vector(variable("x", s)),sort_nat::minimum(variable("f", function_sort(s, sort_nat::nat()))(variable("x", s)), variable("g", function_sort(s, sort_nat::nat()))(variable("x", s)))))));
        result.push_back(data_equation(make_vector(variable("s", sort_bag::bag(s))), sort_bag::bag2set(s, variable("s", sort_bag::bag(s))), sort_set::set_comprehension(s, lambda(make_vector(variable("x", s)),sort_bag::bagin(s, variable("x", s), variable("s", sort_bag::bag(s)))))));
        result.push_back(data_equation(make_vector(variable("u", sort_set::set(s))), sort_bag::set2bag(s, variable("u", sort_set::set(s))), sort_bag::bag_comprehension(s, lambda(make_vector(variable("x", s)),if_(sort_set::setin(s, variable("x", s), variable("u", sort_set::set(s))), sort_nat::cnat(sort_pos::c1()), sort_nat::c0())))));

        return result;
      }

    } // namespace bag
  } // namespace new_data
} // namespace mcrl2

#endif // MCRL2_NEW_DATA_BAG_H
