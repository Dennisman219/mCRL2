#ifndef MCRL2_NEW_DATA_BOOL__H
#define MCRL2_NEW_DATA_BOOL__H

#include "mcrl2/new_data/basic_sort.h"
#include "mcrl2/new_data/function_sort.h"
#include "mcrl2/new_data/function_symbol.h"
#include "mcrl2/new_data/application.h"
#include "mcrl2/new_data/data_equation.h"
#include "mcrl2/new_data/utility.h"
#include "mcrl2/new_data/standard.h"


namespace mcrl2 {

  namespace new_data {

    namespace sort_bool_ {

      // Sort expression Bool
      inline
      basic_sort bool_()
      {
        static basic_sort bool_("Bool");
        return bool_;
      }

      // Recogniser for sort expression Bool
      inline
      bool is_bool_(const sort_expression& e)
      {
        if (e.is_basic_sort())
        {
          return static_cast<const basic_sort&>(e) == bool_();
        }
        return false;
      }

      // Function symbol true
      inline
      function_symbol true_()
      {
        //static function_symbol true_("true", sort_bool_::bool_());
        function_symbol true_("true", sort_bool_::bool_());
        return true_;
      }

      // Recogniser for true
      inline
      bool is_true__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "true";
        }
        return false;
      }

      // Function symbol false
      inline
      function_symbol false_()
      {
        //static function_symbol false_("false", sort_bool_::bool_());
        function_symbol false_("false", sort_bool_::bool_());
        return false_;
      }

      // Recogniser for false
      inline
      bool is_false__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "false";
        }
        return false;
      }

      // Function symbol !
      inline
      function_symbol not_()
      {
        //static function_symbol not_("!", function_sort(sort_bool_::bool_(), sort_bool_::bool_()));
        function_symbol not_("!", function_sort(sort_bool_::bool_(), sort_bool_::bool_()));
        return not_;
      }

      // Recogniser for !
      inline
      bool is_not__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "!";
        }
        return false;
      }

      // Application of !
      inline
      application not_(const data_expression& arg0)
      {
        //assert(sort_bool_::is_bool_(arg0.sort()));
        
        return application(not_(),arg0);
      }

      // Recogniser for application of !
      inline
      bool is_not__application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_not__function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol &&
      inline
      function_symbol and_()
      {
        //static function_symbol and_("&&", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        function_symbol and_("&&", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        return and_;
      }

      // Recogniser for &&
      inline
      bool is_and__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "&&";
        }
        return false;
      }

      // Application of &&
      inline
      application and_(const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bool_::is_bool_(arg0.sort()));
        //assert(sort_bool_::is_bool_(arg1.sort()));
        
        return application(and_(),arg0, arg1);
      }

      // Recogniser for application of &&
      inline
      bool is_and__application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_and__function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol ||
      inline
      function_symbol or_()
      {
        //static function_symbol or_("||", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        function_symbol or_("||", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        return or_;
      }

      // Recogniser for ||
      inline
      bool is_or__function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "||";
        }
        return false;
      }

      // Application of ||
      inline
      application or_(const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bool_::is_bool_(arg0.sort()));
        //assert(sort_bool_::is_bool_(arg1.sort()));
        
        return application(or_(),arg0, arg1);
      }

      // Recogniser for application of ||
      inline
      bool is_or__application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_or__function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Function symbol =>
      inline
      function_symbol implies()
      {
        //static function_symbol implies("=>", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        function_symbol implies("=>", function_sort(sort_bool_::bool_(), sort_bool_::bool_(), sort_bool_::bool_()));
        return implies;
      }

      // Recogniser for =>
      inline
      bool is_implies_function_symbol(const data_expression& e)
      {
        if (e.is_function_symbol())
        {
          return static_cast<const function_symbol&>(e).name() == "=>";
        }
        return false;
      }

      // Application of =>
      inline
      application implies(const data_expression& arg0, const data_expression& arg1)
      {
        //assert(sort_bool_::is_bool_(arg0.sort()));
        //assert(sort_bool_::is_bool_(arg1.sort()));
        
        return application(implies(),arg0, arg1);
      }

      // Recogniser for application of =>
      inline
      bool is_implies_application(const data_expression& e)
      {
        if (e.is_application())
        {
          return is_implies_function_symbol(static_cast<const application&>(e).head());
        }
        return false;
      }

      // Give all system defined constructors for Bool
      inline
      function_symbol_list bool__generate_constructors_code()
      {
        function_symbol_list result;
        result.push_back(true_());
        result.push_back(false_());

        return result;
      }

      // Give all system defined constructors for Bool
      inline
      function_symbol_list bool__generate_functions_code()
      {
        function_symbol_list result;
        result.push_back(not_());
        result.push_back(and_());
        result.push_back(or_());
        result.push_back(implies());

        return result;
      }

      // Function for projecting out right
      inline
      data_expression right(const data_expression& e)
      {
        //assert( || is_and__application(e) || is_or__application(e) || is_implies_application(e));
        
        if (is_and__application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_or__application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        if (is_implies_application(e))
        {
          return static_cast<const application&>(e).arguments()[1];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out arg
      inline
      data_expression arg(const data_expression& e)
      {
        //assert( || is_not__application(e));
        
        if (is_not__application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Function for projecting out left
      inline
      data_expression left(const data_expression& e)
      {
        //assert( || is_and__application(e) || is_or__application(e) || is_implies_application(e));
        
        if (is_and__application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_or__application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        if (is_implies_application(e))
        {
          return static_cast<const application&>(e).arguments()[0];
        }

        // This should never be reached, otherwise something is severely wrong.
        assert(false); 
      }

      // Give all system defined equations for Bool
      inline
      data_equation_list bool__generate_equations_code()
      {
        data_equation_list result;
        result.push_back(data_equation(variable_list(), sort_bool_::not_(sort_bool_::true_()), sort_bool_::false_()));
        result.push_back(data_equation(variable_list(), sort_bool_::not_(sort_bool_::false_()), sort_bool_::true_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::not_(sort_bool_::not_(variable("b", sort_bool_::bool_()))), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::and_(variable("b", sort_bool_::bool_()), sort_bool_::true_()), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::and_(variable("b", sort_bool_::bool_()), sort_bool_::false_()), sort_bool_::false_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::and_(sort_bool_::true_(), variable("b", sort_bool_::bool_())), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::and_(sort_bool_::false_(), variable("b", sort_bool_::bool_())), sort_bool_::false_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::or_(variable("b", sort_bool_::bool_()), sort_bool_::true_()), sort_bool_::true_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::or_(variable("b", sort_bool_::bool_()), sort_bool_::false_()), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::or_(sort_bool_::true_(), variable("b", sort_bool_::bool_())), sort_bool_::true_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::or_(sort_bool_::false_(), variable("b", sort_bool_::bool_())), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::implies(variable("b", sort_bool_::bool_()), sort_bool_::true_()), sort_bool_::true_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::implies(variable("b", sort_bool_::bool_()), sort_bool_::false_()), sort_bool_::not_(variable("b", sort_bool_::bool_()))));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::implies(sort_bool_::true_(), variable("b", sort_bool_::bool_())), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), sort_bool_::implies(sort_bool_::false_(), variable("b", sort_bool_::bool_())), sort_bool_::true_()));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), equal_to(sort_bool_::true_(), variable("b", sort_bool_::bool_())), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), equal_to(sort_bool_::false_(), variable("b", sort_bool_::bool_())), sort_bool_::not_(variable("b", sort_bool_::bool_()))));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), equal_to(variable("b", sort_bool_::bool_()), sort_bool_::true_()), variable("b", sort_bool_::bool_())));
        result.push_back(data_equation(make_vector(variable("b", sort_bool_::bool_())), equal_to(variable("b", sort_bool_::bool_()), sort_bool_::false_()), sort_bool_::not_(variable("b", sort_bool_::bool_()))));

        return result;
      }

    } // namespace bool_
  } // namespace new_data
} // namespace mcrl2

#endif // MCRL2_NEW_DATA_BOOL__H
