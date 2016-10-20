// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//
// This file defines the different types of vertex time domains.

#ifndef DCA_PHYS_DOMAINS_TIME_AND_FREQUENCY_VERTEX_TIME_NAME_HPP
#define DCA_PHYS_DOMAINS_TIME_AND_FREQUENCY_VERTEX_TIME_NAME_HPP

#include <string>

namespace dca {
namespace phys {
namespace domains {
// dca::phys::domains::

enum VERTEX_TIME_NAME {
  SP_TIME_DOMAIN,
  SP_TIME_DOMAIN_POSITIVE,
  SP_TIME_DOMAIN_LEFT_ORIENTED,
  SP_TIME_DOMAIN_LEFT_ORIENTED_POSITIVE,
  TP_TIME_DOMAIN,
  TP_TIME_DOMAIN_POSITIVE
};

std::string to_str(VERTEX_TIME_NAME NAME);

}  // domains
}  // phys
}  // dca

#endif  // DCA_PHYS_DOMAINS_TIME_AND_FREQUENCY_VERTEX_TIME_NAME_HPP
