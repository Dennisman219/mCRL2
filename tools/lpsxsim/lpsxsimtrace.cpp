// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lpsxsimtrace.cpp

#include "wx.hpp" // precompiled headers

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA) && !defined(__clang__)
#pragma implementation "lpsxsimtrace.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/wx.h>
#include <sstream>
#include "lpsxsimtrace.h"
#include "mcrl2/utilities/logger.h"

using namespace std;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::lps;
using namespace mcrl2::data;

//------------------------------------------------------------------------------
// XSimMain
//------------------------------------------------------------------------------

const int ID_LISTVIEW = 10101;

BEGIN_EVENT_TABLE(XSimTrace,wxFrame)
  EVT_CLOSE(XSimTrace::OnCloseWindow)
  EVT_LIST_ITEM_ACTIVATED(ID_LISTVIEW,XSimTrace::OnListItemActivated)
END_EVENT_TABLE()


static void PrintState(stringstream& ss, const mcrl2::lps::state &s)
{
  for (size_t i=0; i<s.size(); i++)
  {
    if (i > 0)
    {
      ss << ", ";
    }

    const data_expression a = s[i];
    if (mcrl2::data::is_variable(a))
    {
      ss << "_";
    }
    else
    {
      ss << pp(a);
    }
  }
}

template < typename T >
static int wxCALLBACK compare_items(long a, long b, T /*d*/)
{
  return a-b;
}

void XSimTrace::_reset(const state &State)
{
  stringstream ss;

  traceview->DeleteAllItems();
  traceview->InsertItem(0,wxT("0"));
  traceview->SetItemData(0,0);
  traceview->SetItem(0,1,wxT(""));
  PrintState(ss,State);
  traceview->SetItem(0,2,wxConvLocal.cMB2WX(ss.str().c_str()));
  traceview->SetColumnWidth(2,wxLIST_AUTOSIZE);
  current_pos = 0;
}

void XSimTrace::_add_state(const mcrl2::lps::multi_action Transition, const mcrl2::lps::state &State, bool enabled)
{
  stringstream ss;
  long l = traceview->GetItemCount();
  long real_l;

  real_l = traceview->InsertItem(l,wxString::Format(wxT("%li"),l));
  traceview->SetItemData(real_l,l);
  real_l = traceview->FindItem(-1,l);
  traceview->SetItem(real_l,1,wxConvLocal.cMB2WX(pp(Transition).c_str()));
  PrintState(ss,State);
  traceview->SetItem(real_l,2,wxConvLocal.cMB2WX(ss.str().c_str()));
  traceview->SetColumnWidth(2,wxLIST_AUTOSIZE);
  if (enabled)
  {
    wxColor col(255,255,255);
    traceview->SetItemBackgroundColour(real_l,col);
  }
  else
  {
    wxColor col(245,245,245);
    traceview->SetItemBackgroundColour(real_l,col);
  }
}

void XSimTrace::_update()
{
  traceview->SortItems(compare_items,0);
}


XSimTrace::XSimTrace(wxWindow* parent) :
  wxFrame(parent, -1, wxT("XSim Trace"), wxDefaultPosition, wxSize(300,400), wxDEFAULT_FRAME_STYLE)
{
  wxPanel* panel = new wxPanel(this,-1);
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  wxStaticBox* box = new wxStaticBox(panel,-1,wxT("Transitions"));
  wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(box,wxVERTICAL);
  traceview = new wxListView(panel,ID_LISTVIEW,wxDefaultPosition,wxSize(0,0),wxLC_REPORT|wxSUNKEN_BORDER|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL);
  traceview->InsertColumn(0,wxT("#"));
  traceview->InsertColumn(1,wxT("Action"));
  traceview->InsertColumn(2,wxT("State"));
  traceview->SetColumnWidth(0,30);
  traceview->SetColumnWidth(1,120);
  boxsizer->Add(traceview,1,wxGROW|wxALIGN_CENTER|wxALL,5);
  sizer->Add(boxsizer,1,wxGROW|wxALIGN_CENTER|wxALL,5);
  panel->SetSizer(sizer);

  simulator = NULL;
}


void XSimTrace::Registered(SimulatorInterface* Simulator)
{
  simulator = Simulator;
}

void XSimTrace::Unregistered()
{
  simulator = NULL;
  traceview->DeleteAllItems();
}

void XSimTrace::Initialise(const mcrl2::data::variable_list /* Pars */)
{
}

void XSimTrace::AddState(const multi_action Transition, const state &State, bool enabled)
{
  _add_state(Transition,State,enabled);
  _update();
}

void XSimTrace::StateChanged(
                  mcrl2::lps::state , 
                  atermpp::vector<mcrl2::lps::multi_action>, 
                  std::vector<mcrl2::lps::state>)
{
}

void XSimTrace::StateChanged(
                  mcrl2::lps::multi_action Transition, 
                  mcrl2::lps::state State, 
                  atermpp::vector<mcrl2::lps::multi_action>, 
                  std::vector<mcrl2::lps::state>)
{
  size_t l = traceview->GetItemCount()-1;

  while (l > current_pos)
  {
    traceview->DeleteItem(traceview->FindItem(-1,l));
    l--;
  }
  AddState(Transition,State,true);
  current_pos++;
}

void XSimTrace::Reset(const state State)
{
  _reset(State);
  _update();
}

void XSimTrace::Undo(size_t Count)
{
  while (Count > 0)
  {
    wxColor col(245,245,245);
    traceview->SetItemBackgroundColour(traceview->FindItem(-1,current_pos),col);
    current_pos--;
    Count--;
  }
  _update();
}

void XSimTrace::Redo(size_t Count)
{
  while (Count > 0)
  {
    wxColor col(255,255,255);
    current_pos++;
    traceview->SetItemBackgroundColour(traceview->FindItem(-1,current_pos),col);
    Count--;
  }
  _update();
}

void XSimTrace::TraceChanged(mcrl2::trace::Trace tr, size_t From)
{
  size_t l = traceview->GetItemCount();

  while (l > From)
  {
    l--;
    traceview->DeleteItem(traceview->FindItem(-1,l));
  }

  for (size_t i=0; i<tr.number_of_actions(); ++i, From++)
  {
    tr.setPosition(i);
    if (From == 0)
    {
      _reset(tr.currentState());
    }
    else
    {
      _add_state(tr.nextAction(),tr.currentState(),current_pos >= From);
    }
    From++;
  }
  _update();
}

void XSimTrace::TracePosChanged(size_t Index)
{
  if (current_pos > Index)
  {
    Undo(current_pos-Index);
  }
  else if (current_pos < Index)
  {
    Redo(Index-current_pos);
  }
}


void XSimTrace::OnCloseWindow(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    Show(FALSE);
    event.Veto();
  }
  else
  {
    Destroy();
  }
}

void XSimTrace::OnListItemActivated(wxListEvent& event)
{
  if (simulator != NULL)
  {
    simulator->SetTracePos(event.GetData());
  }
}
