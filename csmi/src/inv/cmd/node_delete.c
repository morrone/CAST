/*================================================================================

    csmi/src/inv/cmd/node_delete.c

  © Copyright IBM Corporation 2015-2017. All Rights Reserved

    This program is licensed under the terms of the Eclipse Public License
    v1.0 as published by the Eclipse Foundation and available at
    http://www.eclipse.org/legal/epl-v10.html

    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
    restricted by GSA ADP Schedule Contract with IBM Corp.

================================================================================*/
/*
* Author: Nick Buonarota
* Email: nbuonar@us.ibm.com
*/
/*C Include*/
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
/*CORAL includes*/
#include "utilities/include/string_tools.h"
/*CSM Include*/
#include "csmi/include/csm_api_inventory.h"
/*Needed for infrastructure logging*/
#include "csmutil/include/csmutil_logging.h"
/* Command line macros for ease of use. */
#include "csmi/src/common/include/csmi_internal_macros.h"

#define API_PARAMETER_INPUT_TYPE csm_node_delete_input_t
#define API_PARAMETER_OUTPUT_TYPE csm_node_delete_output_t

///< For use as the usage variable in the input parsers.
#define USAGE csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input); help

void help(){
	puts("_____CSM_NODE_DELETE_CMD_HELP_____");
	puts("USAGE:");
	puts("  csm_node_delete ARGUMENTS [OPTIONS]");
	puts("  csm_node_delete -n node_names [-h] [-v verbose_level]");
	puts("");
	puts("SUMMARY: Used to delete a record in the 'csm_node' table of the csm database.");
	puts("");
	puts("EXIT STATUS:");
	puts("  0  if OK,");
	puts("  1  if ERROR.");
	puts("");
	puts("ARGUMENTS:");
	puts("  MANDATORY:");
	puts("    csm_node_delete expects 1 mandatory parameter");
	puts("    Argument         | Example value   | Description  ");                                                 
	puts("    -----------------|-----------------|--------------");
	puts("    -n, --node_names | \"node01,node02\" | (STRING) This is a csv field of valid node names. Identifies which nodes will be updated.");
	puts("                     |                 | ");
	puts("");
	puts("GENERAL OPTIONS:");
	puts("[-h, --help]                  | Help.");
	puts("[-v, --verbose verbose_level] | Set verbose level. Valid verbose levels: {off, trace, debug, info, warning, error, critical, always, disable}");
	puts("");
	puts("EXAMPLE OF USING THIS COMMAND:");
	puts("  csm_node_delete -n node01 ");
	puts("____________________");
}

struct option longopts[] = {
	//general options
	{"help",                    no_argument,       0, 'h'},
	{"verbose",                 required_argument, 0, 'v'},
	//api arguments
	{"node_names",              required_argument, 0, 'n'},
	{0,0,0,0}
};

/*
* Summary: Simple command line interface for the CSM API 'node attributes update'. Takes in information for a single node attributes update (nau) struct via cmd line parameters.
*/
int main(int argc, char *argv[])
{	
	/*CSM Variables*/
	csm_api_object *csm_obj = NULL;
	/*helper Variables*/
	int return_value = 0;
	int requiredParameterCounter = 0;
	int optionalParameterCounter = 0;
	const int NUMBER_OF_REQUIRED_ARGUMENTS = 1;
	const int MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS = 0;
	/*Variables for checking cmd line args*/
	int opt;

	/* getopt_long stores the option index here. */
	int indexptr = 0;
	/*i var for 'for loops'*/
	uint32_t i = 0;

	/*Set up data to call API*/
	API_PARAMETER_INPUT_TYPE* input = NULL;
	/* CSM API initialize and malloc function*/
	csm_init_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
	API_PARAMETER_OUTPUT_TYPE* output = NULL;
	
	/*check optional args*/
	while ((opt = getopt_long(argc, argv, "hv:n:", longopts, &indexptr)) != -1) {
		switch(opt){
			case 'h':
                USAGE();
				return CSMI_HELP;
			case 'v':
				csm_set_verbosity( optarg, USAGE )
				break;
			case 'n':
			{
                csm_optarg_test( "-n, --node_names", optarg, USAGE )
                csm_parse_csv( optarg, input->node_names, input->node_names_count, char*,
                            csm_str_to_char, NULL, "-n, --node_names", USAGE )
				/* Increment requiredParameterCounter so later we can check if arguments were correctly set before calling API. */
				requiredParameterCounter++;
				break;
			}
			default:
                csmutil_logging(error, "unknown arg: '%c'\n", opt);
                USAGE();
				return CSMERR_INVALID_PARAM;
		}
	}

	/*Handle command line args*/
	argc -= optind;
	argv += optind;
	
	/*Collect mandatory args*/
	/*Check to see if expected number of arguments is correct.*/
	if(requiredParameterCounter < NUMBER_OF_REQUIRED_ARGUMENTS || optionalParameterCounter < MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS){
		/*We don't have the correct number of needed arguments passed in.*/
		csmutil_logging(error, "%s-%d:", __FILE__, __LINE__);
		csmutil_logging(error, "  Missing operand(s).");
		csmutil_logging(error, "    Encountered %i required parameter(s). Expected %i required parameter(s).", requiredParameterCounter, NUMBER_OF_REQUIRED_ARGUMENTS);
		csmutil_logging(error, "    Encountered %i optional parameter(s). Expected at least %i optional parameter(s).", optionalParameterCounter, MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS);
        USAGE();
		return CSMERR_MISSING_PARAM;
	}
	
	/* Success required to be able to communicate between library and daemon - csmi calls must be made inside the frame created by csm_init_lib() and csm_term_lib()*/
	return_value = csm_init_lib();
	if( return_value != 0){
		csmutil_logging(error, "%s-%d:", __FILE__, __LINE__);
		csmutil_logging(error, "  csm_init_lib rc= %d, Initialization failed. Success is required to be able to communicate between library and daemon. Are the daemons running?", return_value);
        csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
		return return_value;           
	}
	
	//This will print out the contents of the struct that we will pass to the api
	csmutil_logging(debug, "%s-%d:", __FILE__, __LINE__);
	csmutil_logging(debug, "  Preparing to call the CSM API...");
	csmutil_logging(debug, "  value of input:         %p", input);
	csmutil_logging(debug, "  address of input:       %p", &input);
	csmutil_logging(debug, "  input contains the following:");
	csmutil_logging(debug, "    node_names_count:        %i", input->node_names_count);
	csmutil_logging(debug, "    node_names:              %p", input->node_names);
	for(i = 0; i < input->node_names_count; i++){
		csmutil_logging(debug, "      node_names[%i]: %s", i, input->node_names[i]);
	}
	csmutil_logging(debug, "  value of output:        %p", output);
	csmutil_logging(debug, "  address of output:      %p", &output);
	
	/* Call the actual CSM API */
	return_value = csm_node_delete(&csm_obj, input, &output);
	csmutil_logging(debug, "  value of output:        %p", output);

    switch( return_value )
    {
        case CSMI_SUCCESS:
            printf( "---\n# All %i record(s) successfully deleted.\n...\n",
                input->node_names_count);
            break;

        case CSMERR_DEL_MISMATCH:
            printf( "---\n# Could not delete %i of the %i record(s):\n", 
                output->failure_count,
                input->node_names_count);
            printf("total_failures: %i\nnode_names:\n", output->failure_count );

            for( i = 0; i < output->failure_count; i++ )
            {
                printf( " - %s\n",  output->failure_node_names[i] );
            }

            printf("...\n");
            break;

        default:
            printf("%s FAILED: errcode: %d errmsg: %s\n",
                argv[0], return_value,  csm_api_object_errmsg_get(csm_obj));
    }
	
	//Use CSM API free to release arguments. We no longer need them.
	csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
	//Call internal CSM API clean up.
    csm_api_object_destroy(csm_obj);
	
    // Cleanup the library and print the error.
    int lib_return_value = csm_term_lib();
    if( lib_return_value != 0 )
    {
        csmutil_logging(error, "csm_term_lib rc= %d, Initialization failed. Success "
            "is required to be able to communicate between library and daemon. Are the "
            "daemons running?", lib_return_value);
        return lib_return_value;
    }
	
	return return_value;
}
