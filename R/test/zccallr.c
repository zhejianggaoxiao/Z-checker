/**
 *  @file zccallr.c
 *  @author Sheng Di
 *  @date June, 2017
 *  @brief zccallr.c
 *  (C) 2017 by Mathematics and Computer Science (MCS), Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "ZC_callR.h"
#include "zc.h"

void usage()
{
	printf("Usage: zccallr <options>\n");
	printf("Options:\n");
	printf("* Rscript file:\n");
	printf("	-s <script_file>: specify the path of the R_script_file\n");
	printf("	-c <function_name>: specify the function\n");
	printf("* data type:\n");
	printf("	-i : integer data (int type)\n");
	printf("	-f : single precision (float type)\n");
	printf("	-d : double precision (double type)\n");
	printf("* input data files:\n");
	printf("	-e <endian_type>: endian type of the binary data in input files: 0(little-endian); 1(big-endian)\n");	
	printf("	-A <first data file> : first data file such as original data file\n");
	printf("	-B <second data file> : second data file such as decompressed data file\n");
	printf("* output type of result file: \n");
	printf("	-r : print the result on the screen.\n");
	printf("	-b : analysis result stored in binary format\n");
	printf("	-t : analysis result stored in text format\n");
	printf("	-o <output file path> : the path of the output file.\n"); 
	printf("* dimensions: \n");
	printf("	-1 <nx> : dimension for 1D data such as data[nx]\n");
	printf("	-2 <nx> <ny> : dimensions for 2D data such as data[ny][nx]\n");
	printf("	-3 <nx> <ny> <nz> : dimensions for 3D data such as data[nz][ny][nx] \n");
	printf("* examples: \n");
	printf("	zccallr -s func.R -c add1 -r -f -e 0 -A ../../examples/testdata/x86/testfloat_8_8_128.dat -3 8 8 128\n");
	printf("	zccallr -s func.R -c computeErr -r -f -e 0 -A ../../examples/testdata/x86/testfloat_8_8_128.dat -B ../../examples/testdata/x86/testfloat_8_8_128.dat.sz.out -3 8 8 128\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	size_t r5 = 0;
	size_t r4 = 0;
	size_t r3 = 0;
	size_t r2 = 0; 
	size_t r1 = 0;
	
	char *dataPath1 = NULL, *dataPath2 = NULL;
	char *rscriptPath = NULL, *funcName = NULL;
	char *outputPath = NULL;
	int arrNum = 0;
	int binaryOutput = 0;
	int printOntoScreen = 0;
	int dim = 0;
	size_t i = 0;
	int endianType = 0;
	int dataType;	
	
	if(argc==1)
		usage();	

	for(i=1;i<argc;i++)
	{
		if (argv[i][0] != '-' || argv[i][2])
			usage();
		switch (argv[i][1])
		{
		case 'b': 
			binaryOutput = 1;
			break;
		case 't': 
			binaryOutput = 0;
			break;			
		case 'i':
			dataType = ZC_R_Int;
			break;
		case 'f': 
			dataType = ZC_R_Float;
			break;
		case 'd':
			dataType = ZC_R_Double;
			break;
		case 's':
			if (++i == argc)
				usage();
			rscriptPath = argv[i];
			break;
		case 'r':
			printOntoScreen = 1;
			break;
		case 'c':
			if (++i == argc)
				usage();
			funcName = argv[i];
			break;
		case 'e':
			if (++i == argc)
				usage();
			endianType = atoi(argv[i]);
			dataEndianType = endianType;
			break;
		case 'A':
			if (++i == argc)
				usage();
			dataPath1 = argv[i];
			arrNum ++;
			break;
		case 'B':
			if (++i == argc)
				usage();
			dataPath2 = argv[i];
			arrNum ++;
			break;
		case 'o': 
			if (++i == argc)
				usage();
			outputPath = argv[i];
		case '1': 
			if (++i == argc || sscanf(argv[i], "%zd", &r1) != 1)
				usage();
			dim = 1;
			break;
		case '2':
			if (++i == argc || sscanf(argv[i], "%zd", &r1) != 1 || 
				++i == argc || sscanf(argv[i], "%zd", &r2) != 1)
				usage();
			dim = 2;
			break;
		case '3':
			if (++i == argc || sscanf(argv[i], "%zd", &r1) != 1 ||
				++i == argc || sscanf(argv[i], "%zd", &r2) != 1 ||
				++i == argc || sscanf(argv[i], "%zd", &r3) != 1)
				usage();
			dim = 3;		
			break;
		default: 
			usage();
			break;
		}
	}

	if(printOntoScreen==0 && outputPath == NULL)
	{
		printf("Note: You didn't specify any output way. Use -r or -o to do it.\n");
		usage();
	}

	size_t nbEle1 = 0, nbEle2 = 0;
	float *f_data1 = NULL, *f_data2 = NULL;
	double *d_data1 = NULL, *d_data2 = NULL;
	switch(dataType)
	{
	case ZC_R_Int:
		printf("Error: dosn't support ZC_R_Int type yet.\n");
		break;
	case ZC_R_Float: 
		if(dataPath1!=NULL && dataPath2!=NULL)
		{
			f_data1 = ZC_readFloatData(dataPath1, &nbEle1);
			f_data2 = ZC_readFloatData(dataPath2, &nbEle2);
		}
		else
		{
			if(dataPath1!=NULL)
				f_data1 = ZC_readFloatData(dataPath1, &nbEle1);
			else if(dataPath2!=NULL)
				f_data1 = ZC_readFloatData(dataPath2, &nbEle1);
			else
			{
				printf("Error: dataPath cannot be NULL; please use -A and/or -B to specify data file(s).\n");
				usage();
			}
		}			
		break;
	case ZC_R_Double: 
		if(dataPath1!=NULL && dataPath2!=NULL)
		{
			d_data1 = ZC_readDoubleData(dataPath1, &nbEle1);
			d_data2 = ZC_readDoubleData(dataPath2, &nbEle2);
		}
		else
		{
			if(dataPath1!=NULL)
				d_data1 = ZC_readDoubleData(dataPath1, &nbEle1);
			else if(dataPath2!=NULL)
				d_data1 = ZC_readDoubleData(dataPath2, &nbEle1);
			else
			{
				printf("Error: dataPath cannot be NULL; please use -A and/or -B to specify data file(s).\n");
				usage();
			}
		}	
		break;
	}

//	for(i=0;i<10;i++)
//		printf("data: %d %f\n", i, f_data1[i]);

    // Intialize the R environment.
    int r_argc = 2;
    char *r_argv[] = { "R", "--silent" };
    Rf_initEmbeddedR(r_argc, r_argv);

    // Invoke a function in R
    source(rscriptPath);
    int outLen;
    double *out;


	//perform the script
	switch(arrNum)
	{
	case 1:
		switch(dim)
		{
		case 1:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_1_1d(funcName, dataType, r1, f_data1, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_1_1d(funcName, dataType, r1, d_data1, &outLen, &out);
				break;
			}
			break;
		case 2:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_1_2d(funcName, dataType, r2, r1, f_data1, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_1_2d(funcName, dataType, r2, r1, d_data1, &outLen, &out);
				break;
			}	
			break;
		case 3:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_1_3d(funcName, dataType, r3, r2, r1, f_data1, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_1_3d(funcName, dataType, r3, r2, r1, d_data1, &outLen, &out);
				break;
			}			
			break;
		}
		break;
	case 2:
		switch(dim)
		{
		case 1:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_2_1d(funcName, dataType, r1, f_data1, r1, f_data2, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_2_1d(funcName, dataType, r1, d_data1, r1, d_data2, &outLen, &out);
				break;
			}
			break;
		case 2:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_2_2d(funcName, dataType, r2, r1, f_data1, r2, r1, f_data2, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_2_2d(funcName, dataType, r2, r1, d_data1, r2, r1, d_data2, &outLen, &out);
				break;
			}		
			break;
		case 3:
			switch(dataType)
			{
			case ZC_R_Float:
				ZC_callR_2_3d(funcName, dataType, r3, r2, r1, f_data1, r3, r2, r1, f_data2, &outLen, &out);
				break;
			case ZC_R_Double:
				ZC_callR_2_3d(funcName, dataType, r3, r2, r1, d_data1, r3, r2, r1, d_data2, &outLen, &out);
				break;
			}				
			break;
		}	
		break;
	default: 
		printf("number of arrays can only be 1 or 2.\n");
		exit(0);
	}
	
	if(printOntoScreen)
		for(i=0;i<outLen;i++)
			printf("%zu %f\n", i, out[i]);
		
	//The output results are always in double-precision.
	if(outputPath!=NULL)
	{
		if(binaryOutput==1)
			ZC_writeDoubleData_inBytes(out, outLen, outputPath);
		else
			ZC_writeDoubleData(out, outLen, outputPath);
	}
	
	free(out);

    // Release R environment
    Rf_endEmbeddedR(0);
    return(0);
}
