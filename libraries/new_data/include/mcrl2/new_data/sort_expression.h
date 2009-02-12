// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/new_data/sort_expression.h
/// \brief The class sort_expression.

#ifndef MCRL2_NEW_DATA_SORT_EXPRESSION_H
#define MCRL2_NEW_DATA_SORT_EXPRESSION_H

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/struct.h" // for gsIsSortExpr

namespace mcrl2 {
  
  namespace new_data {

    /// \brief sort expression.
    ///
    /// A sort expression can be any of:
    /// - basic sort
    /// - structured sort
    /// - container sort
    /// - function sort
    /// - alias
    class sort_expression: public atermpp::aterm_appl
    {
      public:

        /// \brief Constructor.
        ///
        sort_expression()
          : atermpp::aterm_appl(core::detail::constructSortId())
        {}

        /// \internal
        /// \brief Constructor.
        /// \param[in] t A term.
        /// \pre t has the internal structure of a sort expression.
        sort_expression(const atermpp::aterm_appl& t)
          : atermpp::aterm_appl(t)
        {}

        /// \brief Returns true iff this expression is a basic sort.
        inline
        bool is_basic_sort() const
        {
          return core::detail::gsIsSortId(*this);
        }

        /// \brief Returns true iff this expression is a structured sort.
        inline
        bool is_structured_sort() const
        {
          return core::detail::gsIsSortStruct(*this);
        }

        /// \brief Returns true iff this expression is a container sort.
        inline
        bool is_container_sort() const
        {
          return core::detail::gsIsSortCons(*this);
        }

        /// \brief Returns true iff this expression is a function sort.
        inline
        bool is_function_sort() const
        {
          return core::detail::gsIsSortArrow(*this);
        }

        /// \brief Returns true iff this expression is a sort alias.
        inline
        bool is_alias() const
        {
          return core::detail::gsIsSortRef(*this);
        }

        /// \brief Returns the target sort of this expression.
        /// \ret codomain if this is a function sort,
        ///      this sort otherwise
        inline
        sort_expression target_sort() const
        {
          if(is_function_sort())
          {
            return atermpp::arg2(*this);
          }
          else
          {
            return *this;
          }
        }

    }; // class sort_expression

    /// \brief list of sorts
    ///
    typedef atermpp::vector<sort_expression> sort_expression_list;

    /// \brief Returns true if the term t is a sort_expression
    /// \param t A term
    /// \return True if the term is a sort expression.
    inline
    bool is_sort_expression(atermpp::aterm_appl t)
    {
      return core::detail::gsIsSortId(t) || core::detail::gsIsSortArrow(t);
    }

  } // namespace new_data

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::new_data::sort_expression)
/// \endcond

#endif // MCRL2_NEW_DATA_SORT_EXPRESSION_H

