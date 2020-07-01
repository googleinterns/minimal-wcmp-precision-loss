//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_DCN_H
#define WCMP_DCN_H


#include <iostream>
#include <vector>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "traffic_trace/Trace.h"
#include "definitions.h"

namespace dcn {

// The virtual class for DCN network with a number of SuperBlocks,
// each of them has different northbound bandwidth.
//
// No virtual function included because the function is still under
// construction and updated very frequently. Will be added later.
class DCN {

};

} // namespace
#endif //WCMP_DCN_H
