//
// Created by wwt on 6/14/20.
//

#include <sstream>
#include "DCN.h"

// Initialization function
DCN::DCN() = default;

// Add SuperBlock to this DCN
void DCN::add_superblock(double link_speed) {
	sb_list_.push_back(new SuperBlock(link_speed));
}

// find the best routing policy in the DCN level
SCIP_RETCODE DCN::find_best_dcn_routing(double send_speed) {
	int num_sb = sb_list_.size(); // get the number of SuperBlocks

	SCIP* scip = nullptr;
	SCIP_CALL(SCIPcreate(&scip)); // create the SCIP environment

	SCIP_CALL(SCIPincludeDefaultPlugins(scip)); // include default plugins
	SCIP_CALL(SCIPcreateProbBasic(scip, "MLU")); // create the SCIP problem
	SCIP_CALL(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE)); // set object sense to be minimize

	std::vector<std::vector<SCIP_VAR*>> x; // initialize the variables
	for (int i=0; i<num_sb; ++i) {
		x.emplace_back(std::vector<SCIP_VAR*>());
		for (int j=0; j<num_sb; ++j) {
			x[i].emplace_back((SCIP_VAR *) nullptr);
			std::stringstream ss;
			ss << "x_" << i << "_" << j;
			SCIP_CALL(SCIPcreateVarBasic(scip,
			                             &x[i][j], // variable
			                             ss.str().c_str(), // name
			                             0.0, // lower bound
			                             1.0, // upper bound
			                             200/sb_list_[i]->dcn_link_speed, // objective
			                             SCIP_VARTYPE_CONTINUOUS)); // variable type
			SCIP_CALL(SCIPaddVar(scip, x[i][j]));  //Adding the variable
		}
	}

	std::vector<std::vector<SCIP_CONS*>> cons; // set the constraint 0<x_{ij}<1
	for (int i=0; i<num_sb; ++i) {
		cons.emplace_back(std::vector<SCIP_CONS*>());
		for (int j=0; j<num_sb; ++j) {
			cons[i].emplace_back((SCIP_CONS *) nullptr); // add constraint

			SCIP_Real values[1]={1.0};
			std::stringstream ss;
			ss << "cons_" << i;
			SCIP_CALL(SCIPcreateConsBasicLinear(scip,
			                                    &cons[i][j], // constraint
			                                    ss.str().c_str(), // name
			                                    1, // how many variables
			                                    &x[i][j], // array of pointers to various variables
			                                    values, // array of values of the coefficients of corresponding variables
			                                    0, // LHS of the constraint
			                                    1)); // RHS of the constraint
            SCIP_CALL(SCIPaddCons(scip, cons[i][j]));
		}
	}

	std::vector<SCIP_CONS*> equal_cons; // set the constraint Sum(x_{ij})=1
	for (int i=0; i<num_sb; ++i) {
		equal_cons.emplace_back((SCIP_CONS *) nullptr); // add constraint

		SCIP_Real values[num_sb];
		for (int j=0; j<num_sb; ++j) values[j]=1;
		std::stringstream ss;
		ss << "cons_" << i;
		SCIP_CALL(SCIPcreateConsBasicLinear(scip,
		                                    &equal_cons[i], // constraint
		                                    ss.str().c_str(), // name
		                                    num_sb, // how many variables
		                                    &x[i][0], // array of pointers to various variables
		                                    values, // array of values of the coefficients of corresponding variables
		                                    1, // LHS of the constraint
		                                    1)); // RHS of the constraint
        SCIP_CALL(SCIPaddCons(scip, equal_cons[i]));
	}

	// set the constraint: len(path) <= 2
	// I encounter a problem here:
	// the number of variables is num_sb*(num_sb-1)*(num*sb-1), I suppose there is a problem with my implementation

	// Release the constraints
	for (int i=0; i<num_sb; ++i) {
		SCIP_CALL(SCIPreleaseCons(scip, &equal_cons[i]));
		for (int j=0;j<num_sb; ++j) {
			SCIP_CALL(SCIPreleaseCons(scip, &cons[i][j]));
		}
	}

	// Solve the problem
	SCIP_CALL(SCIPsolve(scip));

	// Get the solutions
	SCIP_SOL* sol = nullptr;
	sol = SCIPgetBestSol(scip);

	SCIP_CALL((SCIPwriteOrigProblem(scip, "MLU.lp", nullptr, FALSE)));
	for (int i=0; i<num_sb; ++i) {
		for (int j=0;j<num_sb; ++j) {
			SCIP_CALL(SCIPreleaseVar(scip, &x[i][j]));
		}
	}
	SCIP_CALL(SCIPfree(&scip));
	return SCIP_OKAY;
}




