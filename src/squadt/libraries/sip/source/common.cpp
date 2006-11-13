#define SIP_IMPORT_STATIC_DEFINITIONS

#include <sip/configuration.h>
#include <sip/exception.h>
#include <sip/detail/basic_messenger.h>
#include <sip/detail/common.h>

namespace sip {
  namespace messaging {

    /** \brief The type identifier for messages of which the type is not known */
    template < >
    const sip::message_identifier_t message< sip::message_identifier_t, sip::message_unknown, sip::message_any >::message_unknown = sip::message_unknown;

    /** \brief The type identifier for messages of any type */
    template < >
    const sip::message_identifier_t message< sip::message_identifier_t, sip::message_unknown, sip::message_any >::message_any     = sip::message_any;
  }

  /** \brief global variable that indicates whether or not the library was initialised */
  bool initialised = datatype::basic_datatype::initialise();
}

