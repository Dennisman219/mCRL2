// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file processor.cpp
/// \brief Add your file description here.

#include <algorithm>
#include <string>
#include <vector>
#include <iosfwd>
#include <ctime>
#include <exception>

#include <boost/function.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "tipi/controller.hpp"
#include "tipi/utility/logger.hpp"

#include "processor.ipp"
#include "project_manager.ipp"
#include "task_monitor.hpp"

namespace squadt {

  using namespace boost::filesystem;

  /// \cond INTERNAL_DOCS
  /**
   * \brief Helper function for writing object status to stream
   * \param[in] s stream to read from
   * \param[in] t the status to write
   **/
  std::istream& operator >> (std::istream& stream, processor::object_descriptor::status_type& s) {
    size_t t;

    stream >> t;

    s = static_cast < processor::object_descriptor::status_type > (t);

    return (stream);
  }

  /**
   * \param[in] o an object descriptor
   * \param[in] t the new status
   **/
  bool processor_impl::try_change_status(processor::object_descriptor& o, processor::object_descriptor::status_type s) {
    processor_impl::object_descriptor& object = static_cast < processor_impl::object_descriptor& > (o);

    if (object.status != object_descriptor::generation_in_progress && s < o.status) {
      object.status = s;

      boost::shared_ptr < processor::monitor > m(object.generator.lock()->get_monitor());

      if (!m->status_change_handler.empty()) {
        m->status_change_handler();
      }

      return (true);
    }

    return (false);
  }

  static processor::configurated_object_descriptor make_configurated_object_descriptor(
        tipi::configuration::parameter_identifier const& id, boost::shared_ptr < processor::object_descriptor > const& o) {

    processor::configurated_object_descriptor new_descriptor;

    new_descriptor.identifier = id;
    new_descriptor.object     = o;

    return new_descriptor;
  }

  /**
   * \param id the identifier for the object
   * \param p shared pointer to an object descriptor
   **/
  void processor_impl::append_input(tipi::configuration::parameter_identifier const& id, boost::shared_ptr < object_descriptor > const& p) {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      if (p->location == boost::static_pointer_cast< processor_impl::object_descriptor >(i->object)->location) {
        throw std::runtime_error("Failed to append output object due to conflict: file exists!");
      }
    }

    inputs.push_back(make_configurated_object_descriptor(id, p));
  }

  /**
   * \param id the identifier for the object
   * \param p shared pointer to an object descriptor
   **/
  void processor_impl::append_output(tipi::configuration::parameter_identifier const& id, boost::shared_ptr < object_descriptor > const& p, object_descriptor::status_type const& s) {
    p->generator = interface_object;
    p->status    = s;

    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if (p->location == boost::static_pointer_cast< processor_impl::object_descriptor >(i->object)->location) {
        throw std::runtime_error("Failed to append output object due to conflict: file exists!");
      }
    }

    outputs.push_back(make_configurated_object_descriptor(id, p));
  }

  /**
   * \param[in] o a tipi::object object that describes an output object
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] s the status of the new object
   **/
  void processor_impl::append_output(tipi::configuration::parameter_identifier const& id, tipi::object const& o, object_descriptor::status_type const& s) {
    boost::shared_ptr< object_descriptor > p(new object_descriptor(interface_object, o.get_mime_type(), o.get_location()));

    append_output(id, p, s);
  }

  /**
   * \param[in] p the object descriptor that should be replaced
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] o a tipi::object object that describes an output object
   * \param[in] s the new status of the object
   **/
  void processor_impl::replace_output(tipi::configuration::parameter_identifier const& id,
        boost::shared_ptr< object_descriptor > p, tipi::object const& o, object_descriptor::status_type const& s) {

    p->mime_type  = o.get_mime_type();
    p->location   = o.get_location();
    p->status     = s;
  }

  /**
   * \param[in] r whether to check recursively or not
   *
   * \return whether or not the cached status has been changed or is being changed
   **/
  bool processor_impl::check_status(const bool r) {
    bool   result = false;

    if (!is_active()) {
      time_t maximum_input_timestamp  = 0;
     
      if (r) {
        /* Check recursively */
        BOOST_FOREACH(processor::configurated_object_descriptor& i, inputs) {
          boost::shared_ptr < object_descriptor > input_object(boost::static_pointer_cast< object_descriptor >(i.object));
      
          if (input_object.get() == 0) {
            throw std::runtime_error("dependency on a missing object");
          }
      
          boost::shared_ptr < processor > p(input_object->generator);
      
          if (p.get() != 0 && p.get() != interface_object.lock().get()) {
            result |= p->check_status(true);
          }
        }
      }
       
      boost::shared_ptr < project_manager > g(manager.lock());
     
      if (g.get()) {
        /* Find the maximum timestamp of the inputs */
        BOOST_FOREACH(processor::configurated_object_descriptor& i, inputs) {
          boost::shared_ptr < object_descriptor > output_object(boost::static_pointer_cast< object_descriptor >(i.object));
        
          if (output_object.get() == 0) {
            output_object->self_check(*g);
     
            maximum_input_timestamp = (std::max)(maximum_input_timestamp, output_object->timestamp);
     
            result |= (output_object->status != object_descriptor::original) && (output_object->status != object_descriptor::reproducible_up_to_date);
          }
        }
       
        /* Check whether outputs all exist and find the minimum timestamp of the inputs */
        BOOST_FOREACH(processor::configurated_object_descriptor& o, outputs) {
          boost::shared_ptr < object_descriptor > output_object(boost::static_pointer_cast< object_descriptor >(o.object));

          output_object->self_check(*g, static_cast < const long int > (maximum_input_timestamp));
     
          result |= (output_object->status != object_descriptor::original) && (output_object->status != object_descriptor::reproducible_up_to_date);
        }
       
        if (result) {
          if (0 < inputs.size()) {
            BOOST_FOREACH(processor::configurated_object_descriptor& o, outputs) {
              if (o.object->status == object_descriptor::reproducible_up_to_date) {
                try_change_status(*o.object, object_descriptor::reproducible_out_of_date);
              }
            }
          }
          else {
            /* User added files are always up to date */
            g->demote_status(interface_object.lock());
          }
        }
      }
     
      return (result);
    }

    return (true);
  }

  bool processor_impl::demote_status() {
    bool result = false;

    if (!is_active()) {
      BOOST_FOREACH(processor::configurated_object_descriptor& o, outputs) {
        result |= try_change_status(*o.object, object_descriptor::reproducible_out_of_date);
      }
    }

    return (result);
  }

  void processor_impl::flush_outputs() {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() && !is_active()) {
      /* Make sure any output objects are removed from storage */
      for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
        boost::shared_ptr< object_descriptor > output_object(boost::static_pointer_cast< object_descriptor >(i->object));

        path p(g->get_path_for_name(output_object->location));

        if (exists(p)) {
          remove(p);

          try_change_status(*output_object, object_descriptor::reproducible_nonexistent);
        }
      }
      
      g->update_status(interface_object.lock());
    }
  }

  boost::shared_ptr < processor_impl::object_descriptor > processor_impl::find_primary_input() {
    boost::shared_ptr< object_descriptor > object;

    if (inputs.size() != 0) {
      if (selected_input_configuration.get()) {
        return boost::static_pointer_cast< processor_impl::object_descriptor > (find_input_by_id(selected_input_configuration->get_primary_object_descriptor().first));
      }

      return boost::static_pointer_cast< object_descriptor >(inputs[0].object);
    }

    return object;
  }

  /**
   * \param[in] id the identifier of the object to find
   **/
  boost::shared_ptr < processor::object_descriptor > processor_impl::find_output_by_id(tipi::configuration::parameter_identifier const& id) {
    boost::shared_ptr < processor::object_descriptor > object;

    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if (i->identifier == id) {
        object = i->object;

        break;
      }
    }

    return object;
  }

  /**
   * \param[in] id the identifier of the object to find
   **/
  boost::shared_ptr < processor::object_descriptor > processor_impl::find_input_by_id(tipi::configuration::parameter_identifier const& id) {
    boost::shared_ptr < processor::object_descriptor > object;

    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      if (i->identifier == id) {
        object = i->object;

        break;
      }
    }
                               
    return object;
  }

  /**
   * \param[in] o the name (location) of the object
   * \param[in] n the new name (location) of the object
   **/
  void processor_impl::relocate_input(object_descriptor const& o, std::string const& n) {
    for (input_list::const_iterator i = inputs.begin(); i != inputs.end(); ++i) {
      if (i->object.get() == &o) {
        relocate_object(const_cast < processor::configurated_object_descriptor& > (*i), n);

        return;
      }
    }

    assert(false);
  }

  /**
   * \param[in] o the name (location) of the object
   * \param[in] n the new name (location) of the object
   **/
  void processor_impl::relocate_output(object_descriptor const& o, std::string const& n) {
    for (output_list::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
      if (i->object.get() == &o) {
        relocate_object(const_cast < processor::configurated_object_descriptor& > (*i), n);

        return;
      }
    }

    assert(false);
  }

  /**
   * \param[in] o the name (location) of the object to change
   * \param[in] n the new name (location) of the object
   **/
  void processor_impl::relocate_object(processor::configurated_object_descriptor& d, std::string const& n) {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() != 0) {
      boost::shared_ptr < object_descriptor > object(boost::static_pointer_cast< object_descriptor >(d.object));

      path source(g->get_path_for_name(object->location));
      path target(g->get_path_for_name(n));
      
      if (exists(source) && source != target) {
        if (exists(target)) {
          remove(target);
        }
      
        rename(source, target);
      }
      
      object->location = n;
      
      /* Update configuration */
      boost::shared_ptr < tipi::configuration > c = current_monitor->get_configuration();
      
      if (c.get() != 0) {
        c->get_output(d.identifier).set_location(n);
      }
    }
  }

  /**
   * \param[in] c a reference to the new configuration object
   * \param[in] p the previous set of output objects part of the old configuration
   * \param[in] check whether or not to check for existence of concrete outputs
   **/
  void processor_impl::process_configuration(boost::shared_ptr < tipi::configuration > const& c,
                                                    std::set < tipi::object const* >& p, bool check) {
    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get() != 0) {
      tipi::configuration::const_iterator_output_range ir(c->get_output_objects());

      /* Extract information about output objects from the configuration */
      for (tipi::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
        tipi::configuration::object const& object(static_cast < tipi::configuration::object& > (*i));

        tipi::configuration::parameter_identifier  id = c->get_identifier(*i);
        boost::shared_ptr < object_descriptor >    o  = boost::static_pointer_cast< object_descriptor > (find_output_by_id(id));
       
        if (o.get() == 0) {
          /* Output not registered yet */
          append_output(id, object, object_descriptor::reproducible_nonexistent);
        }
        else {
          if (object.get_location() != o->location) {
            /* Output already known, but filenames do not match */
            remove(g->get_path_for_name(o->location));
          }
       
          replace_output(id, o, object);

          /* Check status */
          o->self_check(*g);
        }

        if (!boost::filesystem::exists(g->get_path_for_name(object.get_location())) && check) {
          current_monitor->get_logger()->log(1, "Warning, output file with name: " + object.get_location() + " does not exist!\n");
        }

        /* Remove object from p if it is part of the new configuration too */
        for (std::set< tipi::object const* >::iterator j = p.begin(); j != p.end(); ++j) {
          if ((*j)->get_location() == object.get_location()) {
            p.erase(j);
            break;
          }
        }
      }

      /* Remove files from the old configuration that do not appear in the new one */
      for (std::set< tipi::object const* >::const_iterator i = p.begin(); i != p.end(); ++i) {
        remove(g->get_path_for_name((*i)->get_location()));
      }

      if (0 < outputs.size()) {
        g->commit(interface_object.lock());
      }
    }
  }

  /*
   * Prepends the project store to the argument and returns a native filesystem path
   *
   * \param[in] w a directory relative to the project store
   */
  std::string processor_impl::make_output_path(std::string const& w) const {
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get()) {
      path output_path(g->get_project_store());
    
      if (!output_directory.empty()) {
        output_path /= path(output_directory);
      }

      return (output_path.native_file_string());
    }

    return (w);
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] ic the input combination that is to be used
   * \param[in] l relative (to project store) path to the file that serves as main input
   * \param[in] w the path to the directory in which to run the tool
   *
   * \attention This function is non-blocking
   * \pre t.get() == this
   **/
  void processor_impl::configure(interface_ptr const& t, boost::shared_ptr < const tool::input_configuration > ic, const boost::filesystem::path& l, std::string const& w) {
    using namespace boost;
    using namespace boost::filesystem;

    boost::shared_ptr < project_manager > g(manager.lock());

    if (g.get()) {
      assert(ic != 0);

      selected_input_configuration = ic;

      boost::shared_ptr < tipi::configuration > c(tipi::controller::communicator::new_configuration(*selected_input_configuration));

      c->set_output_prefix(str(format("%s-%03X") % (get_output_prefix(find_primary_input()->location)) % g->get_unique_count()));

      c->add_input(ic->get_primary_object_descriptor().first, ic->get_primary_object_descriptor().second.as_string(), l.string());

      configure(t, c, w);
    }
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] b whether or not to run when there are no input objects defined
   * \param[in] c the configuration object to use
   *
   * \attention This function is non-blocking
   *
   * \pre !is_active() and t.get() == this
   **/
  void processor_impl::run(interface_ptr const& t, boost::shared_ptr < tipi::configuration > c, bool b) {
    if (!is_active()) {
      if (b || 0 < inputs.size()) {
        boost::shared_ptr < project_manager > g(manager);

        update_configuration(*c);

        assert(t->impl.get() == this && g.get());

        /* Check that dependent files exist and rebuild if this is not the case */
        BOOST_FOREACH(input_list::value_type& i, inputs) {
          object_descriptor& object(*boost::static_pointer_cast< object_descriptor >(i.object));

          if (!object.present_in_store(*g)) {
            boost::shared_ptr < processor > p(object.generator.lock());

            if (p.get() != 0) {
              /* Reschedule process operation after process p has completed */
              p->run(boost::bind(&processor_impl::run, this, t, c, false));
     
              return;
            }
            else {
              /* Should signal an error via the monitor ... */
              throw std::runtime_error("Do not know how to (re)create " + object.location);
            }
          }
        }

        current_monitor->start_tool_operation(t, c);

        global_build_system.get_tool_manager()->impl->execute(*tool_descriptor, make_output_path(output_directory),
           boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), false);
      }
      else {
        /* Signal completion to environment via monitor */
        current_monitor->signal_change(execution::process::aborted);
      }
    }
  }

  /**
   * \param[in] t shared pointer to the interface object
   * \param[in] c the configuration object to use
   * \param[in] b whether or not to run when there are no input objects are specified
   *
   * \attention This function is non-blocking
   *
   * \pre !is_active() and t.get() == this
   **/
  void processor_impl::update(interface_ptr const& t, boost::shared_ptr < tipi::configuration > c, bool b) {
    assert(t->impl.get() == this);

    if (!is_active()) {
      if (b || 0 < inputs.size()) {

        update_configuration(*c);

        /* Check that dependent files exist and rebuild if this is not the case */
        BOOST_FOREACH(input_list::value_type& i, inputs) {
          boost::shared_ptr < object_descriptor > object(boost::static_pointer_cast< object_descriptor >(i.object));

          boost::shared_ptr < processor > p(object->generator.lock());

          if (p.get() != 0) {
            if (p->check_status(true)) {

              /* Reschedule process operation after process p has completed */
              p->update(boost::bind(&processor_impl::update_on_success, this, object, t, c, false));

              return;
            }
          }
          else {
            /* Should signal an error via the monitor ... */
            throw std::runtime_error("Do not know how to (re)create " + object->location);
          }
        }

        current_monitor->start_tool_operation(t, c);

        global_build_system.get_tool_manager()->impl->execute(*tool_descriptor, make_output_path(output_directory),
           boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), false);
      }
      else {
        /* Signal completion to environment via monitor */
        current_monitor->signal_change(execution::process::aborted);
      }
    }
  }

  void processor_impl::edit_completed() {
    boost::shared_ptr < processor > p(interface_object.lock());

    if (p.get()) {
      boost::shared_ptr < project_manager > m(manager.lock());

      object_descriptor::status_type new_status = (inputs.size() == 0) ?
                object_descriptor::original : object_descriptor::reproducible_up_to_date;

      for (output_list::iterator i = outputs.begin(); i != outputs.end(); ++i) {
        i->object->status = new_status;
      }

      if (check_status(true) && m) {
        m->update_status(p, inputs.size() == 0);
      }
    }
  }

  /**
   * \param[in] c the edit command to execute
   **/
  void processor_impl::edit(execution::command* c) {
    std::string target(make_output_path(output_directory));

    assert(c != 0);

    // Try creating the file
    if (!boost::filesystem::exists(target)) {
      std::ofstream f(target.c_str(), std::ios::out);

      f.close();
    }
        
    c->set_working_directory(target);

    current_monitor->once_on_completion(boost::bind(&processor_impl::edit_completed, this));

    global_build_system.get_tool_manager()->impl->execute(c, boost::dynamic_pointer_cast < execution::task_monitor > (current_monitor), true);
  }

  tipi::mime_type processor_impl::object_descriptor::get_format() const {
    return mime_type;
  }

  tipi::uri processor_impl::object_descriptor::get_location() const {
    return location;
  }

  boost::shared_ptr < processor > processor_impl::object_descriptor::get_generator() const {
    return generator.lock();
  }

  boost::md5::digest_type processor_impl::object_descriptor::get_checksum() const {
    return checksum;
  }

  std::time_t processor_impl::object_descriptor::get_timestamp() const {
    return timestamp;
  }

  /**
   * \param[in] m the associated project manager
   **/
  bool processor_impl::object_descriptor::present_in_store(project_manager const& m) {
    path l(m.get_path_for_name(location));

    if (exists(l)) {
      return (true);
    }
    else {
      processor_impl::try_change_status(*this, reproducible_nonexistent);

      return (false);
    }
  }

  bool processor::object_descriptor::is_up_to_date() {
    if (status != original && status != reproducible_up_to_date) {
      return (false);
    }
    else {
      boost::shared_ptr < processor > p(get_generator());

      if (p) {
        return (!p->check_status(true));
      }
      else {
        return (false);
      }
    }
  }

  bool processor_impl::object_descriptor::self_check(project_manager const& m) {
    return self_check(m, 0);
  }

  /**
   * \param[in] m a reference to a project manager, used to obtain complete paths files in the project
   * \param[in] t objects older than this time stamp are considered obsolete
   **/
  bool processor_impl::object_descriptor::self_check(project_manager const& m, const long int t) {
    using namespace boost::filesystem;

    if (!generator.lock()->is_active()) {
      path l(m.get_path_for_name(location));
      
      if (exists(l)) {
        /* Input exists, get timestamp */ 
        time_t stamp = last_write_time(l);
      
        if (stamp < t) {
          return (processor_impl::try_change_status(*this, reproducible_out_of_date));
        }
        else if (timestamp < stamp) {
          /* Compare checksums and update recorded checksum */
          boost::md5::digest_type old = checksum;
      
          checksum = boost::md5(l).digest();
    
          if (timestamp != 0 && old != checksum) {
            return processor_impl::try_change_status(*this, reproducible_up_to_date);
          }
    
          timestamp = stamp;
        }
      }
      else {
        return (processor_impl::try_change_status(*this, reproducible_nonexistent));
      }
    }

    return (false);
  }
  /// \endcond

  processor::object_descriptor::status_type processor::object_descriptor::get_status() const {
    return status;
  }

  processor::object_descriptor::~object_descriptor() {
  }

  /**
   * \param[in] o the processor that owns of this object
   **/
  processor::monitor::monitor(processor& o) : owner(o) {
  }

  /**
   * \param[in] s the new status
   **/
  void processor::monitor::signal_change(const execution::process::status s) {
    boost::shared_ptr < execution::process > p;

    signal_change(p, s);
  }

  /**
   * \param[in] s the new status
   * \param[in] p the process
   **/
  void processor::monitor::signal_change(boost::shared_ptr < execution::process > p, const execution::process::status s) {
    using namespace execution;

    boost::iterator_range< processor::output_object_iterator > output_range(owner.get_output_iterators());

    if (s == process::running) { // process started execution
      BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
        o->status = object_descriptor::generation_in_progress;
      }
    }
    else if (owner.number_of_inputs() == 0) { // output does not depend on input
      BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
        o->status = object_descriptor::original;
      }
    }
    else { // output depends on input
      switch (s) {
        case process::stopped:
          BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
            if (exists(o->get_location())) {
              o->status = object_descriptor::reproducible_out_of_date;
            }
            else {
              o->status = object_descriptor::reproducible_nonexistent;
            }
          }
          break;
        case process::completed:
          // Task status determines object status
          break;
        default: /* aborted... */
          if (p.get() && p->get_identifier()) {
            get_logger()->log(1, boost::format("process aborted `%s' (process id %u)\n") % p->get_executable_name() %
                               p->get_identifier());
          }

          BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
            if (o->status == object_descriptor::generation_in_progress) {
              o->status = object_descriptor::reproducible_nonexistent;
            }
          }
          break;
      }
    }

    task_monitor::signal_change(p, s);

    if (s != process::running) {
      owner.check_status(false);
    }

    /* Update status for known processor outputs */
    if (!status_change_handler.empty()) {
      status_change_handler();
    }
  }

  execution::process::status processor::monitor::get_status() {
    boost::shared_ptr < execution::process > p = get_process();

    if (p.get() != 0) {
      return (p->get_status());
    }

    return (execution::process::stopped);
  }

  void processor::monitor::tool_configuration(boost::shared_ptr< processor > t, boost::shared_ptr < tipi::configuration > c) {
    assert(t.get() == &owner);

    /* collect set of output arguments of the existing configuration */
    std::set < tipi::object const* > old_outputs;

    tipi::configuration::const_iterator_output_range ir(c->get_output_objects());

    for (tipi::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
      old_outputs.insert(static_cast < tipi::object const* > (&*i));
    }

    /* Wait until the tool has connected and identified itself */
    if (await_connection()) {
      /* Make sure that the task_monitor state is not cleaned up if the tool quits unexpectedly */
      send_configuration(c);

      /* Wait until configuration is accepted, or the tool has terminated */
      if (await_message(tipi::message_configuration).get() != 0) {
        /* End tool execution */
        finish();

        /* Operation completed successfully */
        t->impl->process_configuration(get_configuration(), old_outputs, false);
      }
    }
    else {
      /* End tool execution */
      finish();
    }
  }

  void processor::monitor::tool_operation(boost::shared_ptr< processor > t, boost::shared_ptr < tipi::configuration > const& c) {
    assert(t.get() == &owner);

    /* collect set of output arguments of the existing configuration */
    std::set < tipi::object const* > old_outputs;

    tipi::configuration::const_iterator_output_range ir(c->get_output_objects());

    for (tipi::configuration::const_iterator_output_range::const_iterator i = ir.begin(); i != ir.end(); ++i) {
      old_outputs.insert(static_cast < tipi::object const* > (&*i));
    }

    /* Wait until the tool has connected and identified itself */
    if (await_connection()) {
      /* Make sure that the task_monitor state is not cleaned up if the tool quits unexpectedly */
      send_configuration(c);

      /* Wait until configuration is accepted, or the tool has terminated */
      if (await_message(tipi::message_configuration).get() != 0) {
        /* Do not let process status influence return status */
        clear_handlers(tipi::message_task_done);

        send_start_signal();

        if (await_completion()) {
          /* Operation completed successfully */
          t->impl->process_configuration(get_configuration(), old_outputs);

          /* Successful, set new status */
          boost::iterator_range< processor::output_object_iterator > output_range(owner.get_output_iterators());

          BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
            o->status = object_descriptor::reproducible_up_to_date;
          }
        }
        else {
          /* Task completed unsuccessfully, set new status */
          boost::iterator_range< processor::output_object_iterator > output_range(owner.get_output_iterators());

          BOOST_FOREACH(boost::shared_ptr< processor::object_descriptor > const& o, output_range) {
            o->status = object_descriptor::reproducible_out_of_date;
          }
        }
      }
    }

    /* End tool execution */
    finish();

    /* Force the project manager to do a status update */
    boost::shared_ptr < project_manager > g(t->impl->manager);

    if (g.get() != 0) {
      g->update_status(t);
    }
  }

  /**
   * \param[in] h the function or functor that is invoked at layout change
   **/
  void processor::monitor::set_display_layout_handling(display_layout_callback_function const& hi, display_update_callback_function const& hu) {
    /* Set the handler for incoming layout messages */
    activate_display_layout_handling(hi, hu);
  }

  /**
   * \param[in] h the function or functor that is invoked at layout change
   **/
  void processor::monitor::set_status_message_handler(status_message_callback_function h) {
    /* Set the handler for incoming layout messages */
    activate_status_message_handler(h);
  }

  void processor::monitor::reset_display_layout_handling() {
    /* Set the handler for incoming layout messages */
    deactivate_display_layout_handling();
  }

  void processor::monitor::reset_status_message_handler() {
    /* Set the handler for incoming layout messages */
    deactivate_status_message_handler();
  }

  void processor::monitor::reset_handlers() {
    /* Set the handler for incoming layout messages */
    deactivate_status_message_handler();
    deactivate_display_layout_handling();
    status_change_handler.clear();
  }

  void processor::monitor::set_status_handler(status_callback_function h) {
    status_change_handler = h;
  }

  void processor::monitor::start_tool_configuration(boost::shared_ptr< processor > const& t, boost::shared_ptr < tipi::configuration > const& c) {
    boost::thread thread(boost::bind(&processor::monitor::tool_configuration, this, t, c));
  }

  void processor::monitor::start_tool_operation(boost::shared_ptr< processor > const& t, boost::shared_ptr < tipi::configuration > const& c) {
    boost::thread thread(boost::bind(&processor::monitor::tool_operation, this, t, c));
  }

  processor::processor() {
  }

  /**
   * \param[in] p the associated project manager
   **/
  boost::shared_ptr< processor > processor::create(boost::weak_ptr < project_manager > const& p) {
    boost::shared_ptr< processor > n(new processor());

    n->impl.reset(new processor_impl(n, p));

    return (n);
  }

  /**
   * \param[in] p the associated project manager
   * \param[in] t the tool to use
   * \param[in] c the input configuration to use
   **/
  boost::shared_ptr< processor > processor::create(boost::weak_ptr < project_manager > const& p,
                        boost::shared_ptr < const tool > t, boost::shared_ptr < const tool::input_configuration > c) {

    boost::shared_ptr< processor > n(new processor());

    n->impl.reset(new processor_impl(n, p, t, c));

    return (n);
  }

  /**
   * \param[in] r whether to check recursively or not
   **/
  bool processor::check_status(bool r) {
    return (impl->check_status(r));
  }

  bool processor::demote_status() {
    return (impl->demote_status());
  }

  /**
   * \param[in] t the tool descriptor of the tool that is to be used to produce the output from the input
   **/
  void processor::set_tool(boost::shared_ptr< tool > const& t) {
    impl->tool_descriptor = t;
  }

  boost::shared_ptr < const tool > processor::get_tool() const {
    return (impl->tool_descriptor);
  }

  /**
   * \param[in] ic the input combination to use
   **/
  void processor::set_input_configuration(boost::shared_ptr< const tool::input_configuration > ic) {
    impl->selected_input_configuration = ic;
  }

  bool processor::has_input_configuration() {
    return impl->selected_input_configuration.get() != 0;
  }

  boost::shared_ptr< const tool::input_configuration > processor::get_input_configuration() const {
    return impl->selected_input_configuration;
  }

  boost::shared_ptr < processor::monitor > processor::get_monitor() {
    return impl->current_monitor;
  }

  boost::iterator_range < processor::input_object_iterator > processor::get_input_iterators() const {
    struct local {
      static boost::shared_ptr < object_descriptor > const& project(configurated_object_descriptor const& c) {
        return c.object;
      }
    };

    static processor::projection_functor_type h(boost::bind(local::project, _1));

    return boost::make_iterator_range(
                input_object_iterator(impl->inputs.begin(), h),
                input_object_iterator(impl->inputs.end(), h));
  }

  /**
   * \param p weak pointer to an object descriptor
   **/
  void processor::append_input(tipi::configuration::parameter_identifier const& id, boost::shared_ptr< object_descriptor > const& p) {
    impl->append_input(id, boost::static_pointer_cast< processor_impl::object_descriptor > (p));
  }

  /**
   * \param o the name (location) of the object to change
   * \param n the new name (location) of the object
   **/
  void processor::relocate_input(object_descriptor& o, std::string const& n) {
    impl->relocate_input(static_cast < processor_impl::object_descriptor& > (o), n);
  }

  /**
   * \param o the name (location) of the object to change
   * \param n the new name (location) of the object
   **/
  void processor::relocate_output(object_descriptor& o, std::string const& n) {
    impl->relocate_output(static_cast < processor_impl::object_descriptor& > (o), n);
  }

  boost::iterator_range < processor::output_object_iterator > processor::get_output_iterators() const {
    struct local {
      static boost::shared_ptr < object_descriptor > const& project(configurated_object_descriptor const& c) {
        return c.object;
      }
    };

    static processor::projection_functor_type h(boost::bind(local::project, _1));

    return boost::make_iterator_range(
                input_object_iterator(impl->outputs.begin(), h),
                input_object_iterator(impl->outputs.end(), h));
  }

  void processor::shutdown() {
    impl->shutdown();
  }

  void processor::flush_outputs() {
    impl->flush_outputs();
  }

  /**
   * \param[in] m the mime type of l
   * \param[in] id the unique identifier for this object in the configuration
   * \param[in] u a URI (local path) to where the file is stored
   * \param[in] s the status of the new object
   **/
  void processor::append_output(tipi::configuration::parameter_identifier const& id, build_system::mime_type const& m, const std::string& u, object_descriptor::status_type const& s) {
    boost::shared_ptr< processor_impl::object_descriptor > p(new processor_impl::object_descriptor(impl->interface_object, m, u));

    impl->append_output(id, p, s);
  }

  void processor::configure(std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->configure(impl->interface_object.lock(), impl->current_monitor->get_configuration(), w);
  }

  void processor::configure(boost::shared_ptr < const tool::input_configuration > i, const boost::filesystem::path& p, std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->configure(impl->interface_object.lock(), i, p, w);
  }

  void processor::reconfigure(std::string const& w) {
    assert(impl->interface_object.lock().get() == this);

    impl->reconfigure(impl->interface_object.lock(), boost::shared_ptr < tipi::configuration > (new tipi::configuration(*impl->current_monitor->get_configuration())), w);
  }

  /**
   * \param[in] h the function to execute when the process terminates
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  void processor::update(boost::function < void () > h, bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->update(impl->interface_object.lock(), h, impl->current_monitor->get_configuration(), b);
  }

  void processor::update(bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->update(impl->interface_object.lock(), impl->current_monitor->get_configuration(), b);
  }

  /**
   * \param[in] h the function to execute when the process terminates
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  void processor::run(boost::function < void () > h, bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->run(impl->interface_object.lock(), h, impl->current_monitor->get_configuration(), b);
  }

  /**
   * \param[in] b whether or not to run when there are no input objects defined
   *
   * \pre t.get() == this
   **/
  void processor::run(bool b) {
    assert(impl->interface_object.lock().get() == this);

    impl->run(impl->interface_object.lock(), impl->current_monitor->get_configuration(), b);
  }

  const size_t processor::number_of_inputs() const {
    return (impl->inputs.size());
  }

  const size_t processor::number_of_outputs() const {
    return (impl->outputs.size());
  }

  bool processor::is_active() const {
    return (impl->is_active());
  }

  /**
   * \param[in] c the edit command to execute
   **/
  void processor::edit(execution::command* c) {
    if (c != 0) {
      impl->edit(c);
    }
  }
}

