// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/new_data/data_specification.h
/// \brief The class data_specification.

#ifndef MCRL2_NEW_DATA_DATA_SPECIFICATION_H
#define MCRL2_NEW_DATA_DATA_SPECIFICATION_H

#include <iostream>
#include <algorithm>

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/atermpp/table.h"
#include "mcrl2/atermpp/set.h"

// sorts
#include "sort_expression.h"
#include "alias.h"
#include "container_sort.h"
#include "function_sort.h"
#include "structured_sort.h"

// new_data expressions
#include "data_expression.h"
#include "function_symbol.h"
#include "application.h"

#include "data_equation.h"
#include "mcrl2/new_data/detail/sequence_algorithm.h"
#include "mcrl2/new_data/detail/compatibility.h"
#include "mcrl2/new_data/detail/data_utility.h"
#include "mcrl2/new_data/utility.h"

namespace mcrl2 {

  namespace new_data {

    /// \brief new_data specification.
    class data_specification
    {
      private:

      /// \brief Determines the sorts on which a constructor depends
      ///
      /// \param[in] f A function symbol.
      /// \pre f is a constructor.
      /// \ret All sorts on which f depends.
      inline
      sort_expression_list dependent_sorts(const function_symbol& f, atermpp::set<sort_expression>& visited) const
      {
        if (f.sort().is_basic_sort())
        {
          return sort_expression_list();
        }
        else
        {
          sort_expression_list result;
          function_sort f_sort(f.sort());
          for (sort_expression_list::const_iterator i = f_sort.domain().begin(); i != f_sort.domain().end(); ++i)
          {
            result.push_back(*i);
            visited.insert(*i);
            sort_expression_list l(dependent_sorts(*i, visited));
            result.insert(result.end(), l.begin(), l.end());
          }
          return result;
        }
      }

      /// \brief Determines the sorts on which a sort expression depends
      ///
      /// \param[in] s A sort expression.
      /// \ret All sorts on which s depends.
      inline
      sort_expression_list dependent_sorts(const sort_expression& s, atermpp::set<sort_expression>& visited) const
      {
        if (visited.find(s) != visited.end())
        {
          return sort_expression_list();
        }
        visited.insert(s);

        if (s.is_basic_sort())
        {
          sort_expression_list result;
          boost::iterator_range< function_symbol_list::const_iterator > cl(constructors(s));
          for (function_symbol_list::const_iterator i = cl.begin(); i != cl.end(); ++i)
          {
            sort_expression_list l(dependent_sorts(*i, visited));
            result.insert(result.end(), l.begin(), l.end());
          }

          return result;
        }
        else if (s.is_container_sort())
        {
          return dependent_sorts(static_cast<container_sort>(s).element_sort(), visited);
        }
        else if (s.is_function_sort())
        {
          sort_expression_list result;
          function_sort fs(s);

          for (sort_expression_list::const_iterator i = fs.domain().begin(); i != fs.domain().end(); ++i)
          {
            sort_expression_list l(dependent_sorts(*i, visited));
            result.insert(result.end(), l.begin(), l.end());
          }

          sort_expression_list l(dependent_sorts(fs.codomain(), visited));
          result.insert(result.end(), l.begin(), l.end());
          return result;
        }
        else if (s.is_structured_sort())
        {
          sort_expression_list result;
          boost::iterator_range<structured_sort_constructor_list::const_iterator> scl(static_cast<structured_sort>(s).struct_constructors());

          for (structured_sort_constructor_list::const_iterator i = scl.begin(); i != scl.end(); ++i)
          {
            boost::iterator_range<structured_sort_constructor_argument_list::const_iterator> scal(i->arguments());
            for (structured_sort_constructor_argument_list::const_iterator j = scal.begin(); j != scal.end(); ++j)
            {
              sort_expression_list sl(dependent_sorts(j->sort(), visited));
              result.insert(result.end(), sl.begin(), sl.end());
            }
          }

          return result;
        }
        else
        {
          assert(false);
        }
      }

      protected:

        ///\brief The basic sorts and structured sorts in the specification.
        sort_expression_list m_sorts;

        ///\brief A mapping of sort expressions to the constructors
        ///corresponding to that sort.
        atermpp::map<sort_expression, function_symbol_list> m_constructors;

        ///\brief The mappings of the specification.
        function_symbol_list m_mappings;

        ///\brief The equations of the specification.
        data_equation_list m_equations;

        ///\brief Table containing system defined sorts.
        atermpp::table m_sys_sorts;

        ///\brief Table containing system defined constructors.
        atermpp::table m_sys_constructors;

        ///\brief Table containing system defined mappings.
        atermpp::table m_sys_mappings;

        ///\brief Table containing system defined equations.
        atermpp::table m_sys_equations;

      public:

      ///\brief Default constructor
      data_specification()
      {}

      ///\internal
      data_specification(const atermpp::aterm_appl& t)
        : m_sorts(detail::aterm_sort_spec_to_sort_expression_list(atermpp::arg1(t))),
          m_constructors(detail::aterm_cons_spec_to_constructor_map(atermpp::arg2(t))),
          m_mappings(detail::aterm_map_spec_to_function_list(atermpp::arg3(t))),
          m_equations(detail::aterm_data_eqn_spec_to_equation_list(atermpp::arg4(t)))
      {}

      ///\brief Constructor
      data_specification(const boost::iterator_range<sort_expression_list::const_iterator>& sorts,
                         const boost::iterator_range<atermpp::map<sort_expression, function_symbol_list>::const_iterator>& constructors,
                         const boost::iterator_range<function_symbol_list::const_iterator>& mappings,
                         const boost::iterator_range<data_equation_list::const_iterator>& equations)
        : m_sorts(sorts.begin(), sorts.end()),
          m_constructors(constructors.begin(), constructors.end()),
          m_mappings(mappings.begin(), mappings.end()),
          m_equations(equations.begin(), equations.end())
      {}

      /// \brief Gets the sort declarations
      ///
      /// \ret The sort declarations of this specification.
      inline
      sort_expression_const_range sorts() const
      {
        return boost::make_iterator_range(m_sorts.begin(), m_sorts.end());
      }

      /// \brief Gets the aliases
      ///
      /// \param[in] s A sort expression
      /// \ret The aliases of sort s
      inline
      alias_const_range aliases(sort_expression& s) const
      {
        //TODO
        return boost::make_iterator_range(alias_list());
      }

      /// \brief Gets all constructors
      ///
      /// \ret All constructors in this specification, including those for
      /// structured sorts.

      inline
      function_symbol_list constructors() const
      {
        function_symbol_list result;
        for (atermpp::map<sort_expression, function_symbol_list>::const_iterator i = m_constructors.begin(); i != m_constructors.end(); ++i)
        {
          result.insert(result.end(), i->second.begin(), i->second.end());
        }
        result.ATprotectTerms();
        return result;
      }

      /// \brief Gets all constructors of a sort.
      ///
      /// \param[in] s A sort expression.
      /// \ret The constructors for sort s in this specification.
      inline
      function_symbol_const_range constructors(const sort_expression& s) const
      {
        if (m_constructors.find(s) == m_constructors.end())
        {
          return boost::make_iterator_range(function_symbol_list());
        }
        else
        {
          return boost::make_iterator_range(m_constructors.find(s)->second);
        }
      }

      /// \brief Gets all mappings in this specification
      ///
      /// \ret All mappings in this specification, including recognisers and
      /// projection functions from structured sorts.
      inline
      function_symbol_const_range mappings() const
      {
        return boost::make_iterator_range(m_mappings);
      }

      /// \brief Gets all mappings of a sort
      ///
      /// \param[in] s A sort expression.
      /// \ret All mappings in this specification, for which s occurs as a
      /// righthandside of the mapping's sort.
      inline
      function_symbol_list mappings(const sort_expression& s) const
      {
        function_symbol_list result;
        for (function_symbol_list::const_iterator i = m_mappings.begin(); i != m_mappings.end(); ++i)
        {
          if(i->sort().is_function_sort())
          {
            if(static_cast<function_sort>(i->sort()).codomain() == s) //TODO check.
            {
              result.push_back(*i);
            }
          }
          else
          {
            if(i->sort() == s)
            {
              result.push_back(*i);
            }
          }
        }
        return result;
      }

      /// \brief Gets all equations in this specification
      ///
      /// \ret All equations in this specification, including those for
      ///  structured sorts.
      inline
      data_equation_const_range equations() const
      {
        return boost::make_iterator_range(m_equations);
      }

      /// \brief Gets all equations with a new_data expression as head
      /// on one of its sides.
      ///
      /// \param[in] d A new_data expression.
      /// \ret All equations with d as head in one of its sides.
      inline
      data_equation_list equations(const data_expression& d) const
      {
        data_equation_list result;
        for (data_equation_list::const_iterator i = m_equations.begin(); i != m_equations.end(); ++i)
        {
          if (i->lhs() == d || i->rhs() == d)
          {
            result.push_back(*i);
          }
          else if(i->lhs().is_application())
          {
            if(static_cast<application>(i->lhs()).head() == d)
            {
              result.push_back(*i);
            }
          }
          else if (i->rhs().is_application())
          {
            if(static_cast<application>(i->rhs()).head() == d)
            {
              result.push_back(*i);
            }
          }
        }
        return result;
      }

      /// \brief Adds a sort to this specification
      ///
      /// \param[in] s A sort expression.
      /// \pre s does not yet occur in this specification.
      inline
      void add_sort(const sort_expression& s)
      {
        assert(std::find(m_sorts.begin(), m_sorts.end(), s) == m_sorts.end());
        m_sorts.push_back(s);
      }

      /// \brief Adds a sort to this specification, and marks it as system
      ///        defined
      ///
      /// \param[in] s A sort expression.
      /// \pre s does not yet occur in this specification.
      /// \post is_system_defined(s) = true
      inline
      void add_system_defined_sort(const sort_expression& s)
      {
        add_sort(s);
        m_sys_sorts.put(s,s);
      }

      /// \brief Adds a constructor to this specification
      ///
      /// \param[in] f A function symbol.
      /// \pre f does not yet occur in this specification.
      inline
      void add_constructor(const function_symbol& f)
      {
        function_symbol_list cs(constructors());
        assert(std::count(cs.begin(), cs.end(), f) == 0);
        assert(std::find(m_mappings.begin(), m_mappings.end(), f) == m_mappings.end());
        sort_expression s;
        if (f.sort().is_function_sort())
        {
          s = static_cast<function_sort>(f.sort()).codomain();
        }
        else
        {
          s = f.sort();
        }
        m_constructors[s].push_back(f);
      }

      /// \brief Adds a constructor to this specification, and marks it as
      ///        system defined.
      ///
      /// \param[in] f A function symbol.
      /// \pre f does not yet occur in this specification.
      /// \post is_system_defined(f) == true
      inline
      void add_system_defined_constructor(const function_symbol& f)
      {
        add_constructor(f);
        m_sys_constructors.put(f,f);
      }

      /// \brief Adds a mapping to this specification
      ///
      /// \param[in] f A function symbol.
      /// \pre f does not yet occur in this specification.
      inline
      void add_mapping(const function_symbol& f)
      {
        sort_expression s;
        if (f.sort().is_function_sort())
        {
          s = static_cast<function_sort>(f.sort()).codomain();
        }
        else
        {
          s = f.sort();
        }

        if (m_constructors.find(s) != m_constructors.end())
        {
          function_symbol_list fl(m_constructors.find(s)->second);
          assert(std::count(fl.begin(), fl.end(), f) == 0);
        }
        assert(std::count(m_mappings.begin(), m_mappings.end(), f) == 0);
        m_constructors.find(s)->second.push_back(f);
      }

      /// \brief Adds a mapping to this specification, and marks it as system
      ///        defined.
      ///
      /// \param[in] f A function symbol.
      /// \pre f does not yet occur in this specification.
      /// \post is_system_defined(f) == true
      inline
      void add_system_defined_mapping(const function_symbol& f)
      {
        add_mapping(f);
        m_sys_mappings.put(f,f);
      }

      /// \brief Adds an equation to this specification
      ///
      /// \param[in] e An equation.
      /// \pre e does not yet occur in this specification.
      inline
      void add_equation(const data_equation& e)
      {
        assert(std::count(m_equations.begin(), m_equations.end(), e) == 0);
        m_equations.push_back(e);
      }

      /// \brief Adds an equation to this specification, and marks it as system
      ///        defined.
      ///
      /// \param[in] e An equation.
      /// \pre e does not yet occur in this specification.
      /// \post is_system_defined(f) == true
      inline
      void add_system_defined_equation(const data_equation& e)
      {
        add_equation(e);
        m_sys_equations.put(e,e);
      }

      /// \brief Adds sorts to this specification
      ///
      /// \param[in] sl A range of sort expressions.
      inline
      void add_sorts(const sort_expression_const_range& sl)
      {
        for (sort_expression_list::const_iterator i = sl.begin(); i != sl.end(); ++i)
        {
          add_sort(*i);
        }
      }

      /// \brief Adds sorts to this specification, and marks them as system
      /// defined.
      ///
      /// \param[in] sl A range of sort expressions.
      /// \post for all s in sl: is_system_defined(s)
      inline
      void add_system_defined_sorts(const sort_expression_const_range& sl)
      {
        for (sort_expression_list::const_iterator i = sl.begin(); i != sl.end(); ++i)
        {
          add_system_defined_sort(*i);
        }
      }

      /// \brief Adds constructors to this specification
      ///
      /// \param[in] fl A range of function symbols.
      inline
      void add_constructors(const function_symbol_const_range& fl)
      {
        for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
        {
          add_constructor(*i);
        }
      }

      /// \brief Adds constructors to this specification, and marks them as
      ///        system defined.
      ///
      /// \param[in] fl A range of function symbols.
      /// \post for all f in fl: is_system_defined(f)
      inline
      void add_system_defined_constructors(const function_symbol_const_range& fl)
      {
        for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
        {
          add_system_defined_constructor(*i);
        }
      }

      /// \brief Adds mappings to this specification
      ///
      /// \param[in] fl A range of function symbols.
      inline
      void add_mappings(const function_symbol_const_range& fl)
      {
        for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
        {
          add_mapping(*i);
        }
      }

      /// \brief Adds mappings to this specification, and marks them as system
      ///        defined.
      ///
      /// \param[in] fl A range of function symbols.
      /// \post for all f in fl: is_system_defined(f)
      inline
      void add_system_defined_mappings(const function_symbol_const_range& fl)
      {
        for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
        {
          add_system_defined_mapping(*i);
        }
      }

      /// \brief Adds equations to this specification
      ///
      /// \param[in] el A range of equations.
      inline
      void add_equations(const data_equation_const_range& el)
      {
        for (data_equation_list::const_iterator i = el.begin(); i != el.end(); ++i)
        {
          add_equation(*i);
        }
      }

      /// \brief Adds equations to this specification, and marks them as system
      ///        defined.
      ///
      /// \param[in] el A range of equations.
      /// \post for all e in el: is_system_defined(e)
      inline
      void add_system_defined_equations(const data_equation_const_range& el)
      {
        for (data_equation_list::const_iterator i = el.begin(); i != el.end(); ++i)
        {
          add_system_defined_equation(*i);
        }
      }

      /// \brief Removes sort from specification.
      ///
      /// Note that this does not remove constructors, mappings and equations
      /// for a sort.
      /// \param[in] s A sort expression.
      /// \post s does not occur in this specification.
      inline
      void remove_sort(const sort_expression& s)
      {
        if (is_system_defined(s))
        {
          m_sys_sorts.remove(s);
        }

        m_sorts.erase(std::find(m_sorts.begin(), m_sorts.end(), s));
      }

      /// \brief Removes sorts from specification.
      ///
      /// \param[in] sl A range of sorts.
      /// \post for all s in sl: s no in sorts()
      inline
      void remove_sorts(const sort_expression_const_range& sl)
      {
        for (sort_expression_list::const_iterator i = sl.begin(); i != sl.end(); ++i)
        {
          remove_sort(*i);
        }
      }

      /// \brief Removes constructor from specification.
      ///
      /// Note that this does not remove equations containing the constructor.
      /// \param[in] f A constructor.
      /// \pre f occurs in the specification as constructor.
      /// \post f does not occur as constructor.
      inline
      void remove_constructor(const function_symbol& f)
      {
        function_symbol_list cs(constructors());
        assert(std::count(cs.begin(), cs.end(), f) != 0);
        if (is_system_defined(f))
        {
          m_sys_constructors.remove(f);
        }

        sort_expression s;
        if (f.sort().is_function_sort())
        {
          s = static_cast<function_sort>(f.sort()).codomain();
        }
        else
        {
          s = f.sort();
        }

        atermpp::map<sort_expression, function_symbol_list>::iterator i = m_constructors.find(s);
        i->second.erase(std::find(i->second.begin(), i->second.end(), f));
      }

      /// \brief Removes constructors from specification.
      ///
      /// \param[in] cl A range of constructors.
      /// \post for all c in cl: c not in constructors()
      inline
      void remove_constructors(const function_symbol_const_range& cl)
      {
        for (function_symbol_list::const_iterator i = cl.begin(); i != cl.end(); ++i)
        {
          remove_constructor(*i);
        }
      }

      /// \brief Removes mapping from specification.
      ///
      /// Note that this does not remove equations in which the mapping occurs.
      /// \param[in] f A function.
      /// \post f does not occur as constructor.
      inline
      void remove_mapping(const function_symbol& f)
      {
        if (is_system_defined(f))
        {
          m_sys_mappings.remove(f);
        }
        m_mappings.erase(std::find(m_mappings.begin(), m_mappings.end(), f));
      }

      /// \brief Removes mappings from specification.
      ///
      /// \param[in] fl A range of constructors.
      /// \post for all f in fl: f not in mappings()
      inline
      void remove_mappings(const function_symbol_const_range& fl)
      {
        for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
        {
          remove_mapping(*i);
        }
      }

      /// \brief Removes equation from specification.
      ///
      /// \param[in] e An equation.
      /// \post e is removed from this specification.
      inline
      void remove_equation(const data_equation& e)
      {
        if (is_system_defined(e))
        {
          m_sys_equations.remove(e);
        }
        m_equations.erase(std::find(m_equations.begin(), m_equations.end(), e));
      }

      /// \brief Removes equations from specification.
      ///
      /// \param[in] el A range of equations.
      /// \post for all e in el: e not in equations()
      inline
      void remove_equations(const data_equation_const_range& el)
      {
        for (data_equation_list::const_iterator i = el.begin(); i != el.end(); ++i)
        {
          remove_equation(*i);
        }
      }

      /// \brief Checks whether a sort is system defined.
      ///
      /// \param[in] s A sort expression.
      /// \ret true iff s is system defined, false otherwise.
      inline
      bool is_system_defined(const sort_expression& s)
      {
        return m_sys_sorts.get(s) != atermpp::aterm();
      }

      /// \brief Checks whether a function symbol is system defined.
      ///
      /// \param[in[ f A function symbol.
      /// \ret true iff f is system defined (either as constructor or as
      ///      mapping), false otherwise.
      inline
      bool is_system_defined(const function_symbol& f)
      {
        return (m_sys_constructors.get(f) != atermpp::aterm() ||
                m_sys_mappings.get(f)    != atermpp::aterm());
      }

      /// \brief Checks whether an equation is system defined.
      ///
      /// \param[in] e An equation.
      /// \ret true iff e is system defined, false otherwise.
      inline
      bool is_system_defined(const data_equation& e)
      {
        return m_sys_equations.get(e) != atermpp::aterm();
      }

      /// \brief Checks whether a sort is certainly finite.
      ///
      /// \param[in] s A sort expression
      /// \ret true if s can be determined to be finite,
      ///      false otherwise.
      inline
      bool is_certainly_finite(const sort_expression& s) const
      {
        // Check for recursive occurrence.
        atermpp::set<sort_expression> visited;
        sort_expression_list dsl(dependent_sorts(s, visited));
        if (std::find(dsl.begin(), dsl.end(), s) != dsl.end())
        {
          return false;
        }

        if (s.is_basic_sort())
        {
          function_symbol_const_range fl(constructors(s));

          for (function_symbol_list::const_iterator i = fl.begin(); i != fl.end(); ++i)
          {
            atermpp::set<sort_expression> visited;
            sort_expression_list sl(dependent_sorts(*i, visited));
            for (sort_expression_list::const_iterator j = sl.begin(); j != sl.end(); ++j)
            {
              if (!is_certainly_finite(*j))
              {
                return false;
              }
            }
          }
          return !fl.empty();
        }
        else if (s.is_container_sort())
        {
          container_sort cs(s);
          if(cs.is_set_sort())
          {
            return is_certainly_finite(cs.element_sort());
          }
          return false;
        }
        else if (s.is_function_sort())
        {
          function_sort fs(s);
          for (sort_expression_list::const_iterator i = fs.domain().begin(); i != fs.domain().end(); ++i)
          {
            if (!is_certainly_finite(*i))
            {
              return false;
            }
          }

          if (fs.codomain() == s)
          {
            return false;
          }

          return is_certainly_finite(fs.codomain());
        }
        else if (s.is_structured_sort())
        {
          atermpp::set<sort_expression> visited;
          boost::iterator_range<sort_expression_list::const_iterator> sl(dependent_sorts(s, visited));
          for (sort_expression_list::const_iterator i = sl.begin(); i != sl.end(); ++i)
          {
            if (!is_certainly_finite(*i))
            {
              return false;
            }
          }
          return true;
        }
        else
        {
          assert(false);
        }
      }

      /// \brief Returns a default expression for a sort.
      ///
      /// \param[in] s A sort expression.
      /// \ret Default expression of sort s.
      inline
      data_expression default_expression(const sort_expression& s)
      {
        return data_expression();
        //TODO
      }

      /// \brief Returns true if
      /// <ul>
      /// <li>the domain and range sorts of constructors are contained in the list of sorts</li>
      /// <li>the domain and range sorts of mappings are contained in the list of sorts</li>
      /// </ul>
      /// \return True if the data specification is well typed.
      bool is_well_typed() const
      {
        std::set<sort_expression> sorts = detail::make_set(m_sorts);

        // check 1)
        if (!detail::check_data_spec_sorts(constructors(), sorts))
        {
          std::clog << "data_specification::is_well_typed() failed: not all of the sorts appearing in the constructors "
                    << pp(constructors()) << " are declared in " << pp(m_sorts) << std::endl;
          return false;
        }

        // check 2)
        if (!detail::check_data_spec_sorts(mappings(), sorts))
        {
          std::clog << "data_specification::is_well_typed() failed: not all of the sorts appearing in the mappings "
                    << pp(mappings()) << " are declared in " << pp(m_sorts) << std::endl;
          return false;
        }

        return true;
      }

    }; // class data_specification

    inline
    bool operator==(const data_specification& x, const data_specification& y)
    {
      return x.sorts() == y.sorts() &&
             x.constructors() == y.constructors() &&
             x.mappings() == y.mappings() &&
             x.equations() == y.equations();
    }

  } // namespace new_data

} // namespace mcrl2

#endif // MCRL2_NEW_DATA_DATA_SPECIFICATION_H

